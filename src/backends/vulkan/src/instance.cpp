#include <pch.hpp>

#include <aperture/vulkan/instance.hpp>

#include <aperture/assert.hpp>
#include <aperture/format.hpp>
#include <aperture/result.hpp>
#include <aperture/swapchain.hpp>
#include <aperture/vulkan/swapchain.hpp>

#include "internal.hpp"

#include <aperture/vulkan/header.hpp>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory_resource>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace aperture::vk {

namespace {

struct InstanceEnumeration {
  std::vector<VkExtensionProperties> extensions;
  std::vector<VkLayerProperties> layers;
};

std::atomic<const InstanceEnumeration*> g_instance_enumeration{nullptr};

constexpr const char* VK_VALIDATION_LAYER = "VK_LAYER_KHRONOS_validation";

[[nodiscard]] const InstanceEnumeration* PublishedEnumeration() noexcept {
  return g_instance_enumeration.load(std::memory_order_acquire);
}

[[nodiscard]] constexpr bool HasLayer(std::span<const VkLayerProperties> layers,
                                      std::string_view name) noexcept {
  return std::ranges::any_of(layers,
                             [name](const VkLayerProperties& layer) noexcept {
                               return std::string_view{layer.layerName} == name;
                             });
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT /*types*/,
    const VkDebugUtilsMessengerCallbackDataEXT* data, void* user) noexcept {
  const char* message =
      (data != nullptr && data->pMessage != nullptr) ? data->pMessage : "";
  const bool is_error =
      (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0;
  if (is_error) {
    log::Error(message);
  } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) !=
             0) {
    log::Warn(message);
  } else if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
    log::Info(message);
  } else {
    log::Debug(message);
  }

  auto* impl = static_cast<Instance*>(user);
  if (is_error && impl != nullptr && impl->validation_fatal) {
    APERTURE_VERIFY(false, "{}", message);
  }
  return VK_FALSE;
}

void GatherMemory(const VkPhysicalDeviceMemoryProperties& memory,
                  uint64_t* local_bytes, uint64_t* shared_bytes,
                  uint64_t* mapped_default) noexcept {
  APERTURE_ASSERT(local_bytes != nullptr);
  APERTURE_ASSERT(shared_bytes != nullptr);
  APERTURE_ASSERT(mapped_default != nullptr);
  *local_bytes = 0;
  *shared_bytes = 0;
  *mapped_default = 0;

  for (uint32_t i = 0; i < memory.memoryHeapCount; ++i) {
    const VkMemoryHeap& heap = memory.memoryHeaps[i];
    if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0) {
      *local_bytes += heap.size;
    } else {
      *shared_bytes += heap.size;
    }
  }

  for (uint32_t i = 0; i < memory.memoryTypeCount; ++i) {
    const VkMemoryType& type = memory.memoryTypes[i];
    const VkMemoryPropertyFlags flags = type.propertyFlags;
    if ((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 &&
        (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
      *mapped_default += memory.memoryHeaps[type.heapIndex].size;
    }
  }
}

constexpr void CountQueues(std::span<const VkQueueFamilyProperties> families,
                           uint32_t* graphics, uint32_t* compute,
                           uint32_t* copy) noexcept {
  APERTURE_ASSERT(graphics != nullptr);
  APERTURE_ASSERT(compute != nullptr);
  APERTURE_ASSERT(copy != nullptr);
  *graphics = 0;
  *compute = 0;
  *copy = 0;
  for (const auto& family : families) {
    if ((family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      *graphics += family.queueCount;
    }
    if ((family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
      *compute += family.queueCount;
    }
    if ((family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
      *copy += family.queueCount;
    }
  }
}

[[nodiscard]] constexpr uint32_t HeapSlotCount(VkDeviceSize heap_bytes,
                                               VkDeviceSize stride) noexcept {
  if (stride == 0) {
    return 0;
  }
  const auto count = heap_bytes / stride;
  if (count > UINT32_MAX) {
    return UINT32_MAX;
  }
  return static_cast<uint32_t>(count);
}

[[nodiscard]] Capability ProbeCapabilities(const Adapter& record) noexcept {
  auto caps = Capability::SplitBarriers;
  if (HasExtension(record.extensions, VK_EXT_MESH_SHADER_EXTENSION_NAME)) {
    caps |= Capability::MeshShading;
  }
  if (HasExtension(record.extensions,
                   VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)) {
    caps |= Capability::RayTracingPipeline;
  }
  if (HasExtension(record.extensions, VK_KHR_RAY_QUERY_EXTENSION_NAME)) {
    caps |= Capability::RayQuery;
  }
  if (HasExtension(record.extensions,
                   VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME)) {
    caps |= Capability::SeparateBlend;
  }
  if (HasExtension(record.extensions, VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME)) {
    caps |= Capability::HostImageCopy;
  }
  if (HasExtension(record.extensions,
                   VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME)) {
    caps |= Capability::UnifiedImageLayouts;
  }
  if (HasExtension(record.extensions,
                   VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME)) {
    caps |= Capability::DeviceGeneratedCommands;
  }
  if (HasExtension(record.extensions,
                   VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME)) {
    caps |= Capability::CooperativeMatrix;
  }
  if (HasExtension(record.extensions,
                   VK_EXT_SHADER_TILE_IMAGE_EXTENSION_NAME) ||
      HasExtension(
          record.extensions,
          VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME)) {
    caps |= Capability::FramebufferFetch;
  }

  if (record.features12.shaderBufferInt64Atomics == VK_TRUE) {
    caps |= Capability::BufferInt64Atomics;
  }

  bool dedicated_compute = false;
  bool dedicated_copy = false;
  bool present_from_compute = false;
  for (uint32_t i = 0; i < record.queue_families.size(); ++i) {
    const VkQueueFamilyProperties& family = record.queue_families[i];
    const bool graphics = (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
    const bool compute = (family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
    const bool transfer = (family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0;
    if (compute && !graphics) {
      dedicated_compute = true;
      if (PresentationSupport(record.physical_device, i)) {
        present_from_compute = true;
      }
    }
    if (transfer && !graphics && !compute) {
      dedicated_copy = true;
    }
  }

  if (dedicated_compute) {
    caps |= Capability::AsyncCompute;
  }
  if (dedicated_copy) {
    caps |= Capability::AsyncCopy;
  }
  if (present_from_compute) {
    caps |= Capability::PresentFromCompute;
  }
  return caps;
}

[[nodiscard]] auto CacheInstanceEnumeration() noexcept -> Result<void> {
  if (PublishedEnumeration() != nullptr) {
    return {};
  }

  auto* snapshot = new InstanceEnumeration{};
  if (auto enumerated = EnumerateVk(
          &snapshot->extensions,
          [](uint32_t* count, VkExtensionProperties* data) noexcept {
            return vkEnumerateInstanceExtensionProperties(nullptr, count, data);
          });
      !enumerated) [[unlikely]] {
    delete snapshot;
    return enumerated;
  }
  if (auto enumerated =
          EnumerateVk(&snapshot->layers,
                      [](uint32_t* count, VkLayerProperties* data) noexcept {
                        return vkEnumerateInstanceLayerProperties(count, data);
                      });
      !enumerated) [[unlikely]] {
    delete snapshot;
    return enumerated;
  }

  const InstanceEnumeration* expected = nullptr;
  if (!g_instance_enumeration.compare_exchange_strong(
          expected, snapshot, std::memory_order_release,
          std::memory_order_acquire)) {
    delete snapshot;
  }
  return {};
}

[[nodiscard]] std::string ConformanceReason(const Adapter& record) noexcept {
  const auto& f = record.features2.features;
  const auto& f11 = record.features11;
  const auto& f12 = record.features12;
  const auto& f13 = record.features13;
  std::string reasons;
  const auto append = [&reasons](std::string_view reason) noexcept {
    if (!reasons.empty()) {
      reasons += ", ";
    }
    reasons += reason;
  };

  if (record.properties.properties.apiVersion < VK_API_VERSION_1_4) {
    append("Vulkan 1.4");
  }
  if (f.shaderInt64 != VK_TRUE) {
    append("shaderInt64");
  }
  if (f.shaderStorageImageReadWithoutFormat != VK_TRUE) {
    append("shaderStorageImageReadWithoutFormat");
  }
  if (f.shaderStorageImageWriteWithoutFormat != VK_TRUE) {
    append("shaderStorageImageWriteWithoutFormat");
  }
  if (f.multiDrawIndirect != VK_TRUE) {
    append("multiDrawIndirect");
  }
  if (f.independentBlend != VK_TRUE) {
    append("independentBlend");
  }
  if (f.samplerAnisotropy != VK_TRUE) {
    append("samplerAnisotropy");
  }
  if (f11.storageBuffer16BitAccess != VK_TRUE &&
      f11.uniformAndStorageBuffer16BitAccess != VK_TRUE) {
    append("16-bit storage");
  }
  if (f11.shaderDrawParameters != VK_TRUE) {
    append("shaderDrawParameters");
  }
  if (f12.bufferDeviceAddress != VK_TRUE) {
    append("bufferDeviceAddress");
  }
  if (f12.timelineSemaphore != VK_TRUE) {
    append("timelineSemaphore");
  }
  if (f12.descriptorIndexing != VK_TRUE) {
    append("descriptorIndexing");
  }
  if (f12.shaderSampledImageArrayNonUniformIndexing != VK_TRUE) {
    append("shaderSampledImageArrayNonUniformIndexing");
  }
  if (f12.descriptorBindingPartiallyBound != VK_TRUE) {
    append("descriptorBindingPartiallyBound");
  }
  if (f12.descriptorBindingSampledImageUpdateAfterBind != VK_TRUE) {
    append("descriptorBindingSampledImageUpdateAfterBind");
  }
  if (f12.descriptorBindingVariableDescriptorCount != VK_TRUE) {
    append("descriptorBindingVariableDescriptorCount");
  }
  if (f12.runtimeDescriptorArray != VK_TRUE) {
    append("runtimeDescriptorArray");
  }
  if (f12.shaderInt8 != VK_TRUE) {
    append("shaderInt8");
  }
  if (f12.shaderFloat16 != VK_TRUE) {
    append("shaderFloat16");
  }
  if (f12.scalarBlockLayout != VK_TRUE) {
    append("scalarBlockLayout");
  }
  if (f12.vulkanMemoryModel != VK_TRUE) {
    append("vulkanMemoryModel");
  }
  if (f12.vulkanMemoryModelDeviceScope != VK_TRUE) {
    append("vulkanMemoryModelDeviceScope");
  }
  if (f12.drawIndirectCount != VK_TRUE) {
    append("drawIndirectCount");
  }
  if (!HasExtension(record.extensions, VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME)) {
    append("VK_EXT_descriptor_heap");
  } else if (record.descriptor_heap_features.descriptorHeap != VK_TRUE) {
    append("descriptorHeap");
  } else if (record.descriptor_heap_props.maxPushDataSize < 16) {
    append("maxPushDataSize");
  }
  if (!HasExtension(record.extensions,
                    VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME)) {
    append("VK_KHR_device_address_commands");
  } else if (record.address_commands.deviceAddressCommands != VK_TRUE) {
    append("deviceAddressCommands");
  }
  if (!HasExtension(record.extensions,
                    VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME)) {
    append("VK_KHR_shader_untyped_pointers");
  } else if (record.untyped_pointers.shaderUntypedPointers != VK_TRUE) {
    append("shaderUntypedPointers");
  }
  if (f13.synchronization2 != VK_TRUE) {
    append("synchronization2");
  }
  if (f13.dynamicRendering != VK_TRUE) {
    append("dynamicRendering");
  }
  if (f13.maintenance4 != VK_TRUE) {
    append("maintenance4");
  }
  if (record.features14.maintenance5 != VK_TRUE) {
    append("maintenance5");
  }
  if (f13.pipelineCreationCacheControl != VK_TRUE) {
    append("pipelineCreationCacheControl");
  }
  if (!HasExtension(record.extensions, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME)) {
    append("VK_EXT_robustness2");
  } else if (record.robustness2.nullDescriptor != VK_TRUE) {
    append("nullDescriptor");
  }
  return reasons;
}

void WireAdapterChains(Adapter* record) noexcept {
  APERTURE_ASSERT(record != nullptr);
  record->props11.pNext = &record->props12;
  record->props12.pNext = &record->props13;
  record->properties.pNext = &record->props11;
  record->descriptor_heap_props.pNext = nullptr;
  record->props13.pNext =
      record->has_descriptor_heap ? &record->descriptor_heap_props : nullptr;

  record->features14.pNext = &record->features13;
  record->features13.pNext = &record->features12;
  record->features12.pNext = &record->features11;
  record->features11.pNext = &record->robustness2;
  record->features2.pNext = &record->features14;

  record->robustness2.pNext = nullptr;
  record->descriptor_heap_features.pNext = nullptr;
  record->mesh_features.pNext = nullptr;
  record->address_commands.pNext = nullptr;
  record->untyped_pointers.pNext = nullptr;

  void** tail = &record->robustness2.pNext;
  if (record->has_descriptor_heap) {
    *tail = &record->descriptor_heap_features;
    tail = &record->descriptor_heap_features.pNext;
  }
  if (HasExtension(record->extensions, VK_EXT_MESH_SHADER_EXTENSION_NAME)) {
    *tail = &record->mesh_features;
    tail = &record->mesh_features.pNext;
  }
  if (HasExtension(record->extensions,
                   VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME)) {
    *tail = &record->address_commands;
    tail = &record->address_commands.pNext;
  }
  if (HasExtension(record->extensions,
                   VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME)) {
    *tail = &record->untyped_pointers;
    tail = &record->untyped_pointers.pNext;
  }

  APERTURE_ASSERT(record->properties.pNext == &record->props11);
  APERTURE_ASSERT(record->props11.pNext == &record->props12);
  APERTURE_ASSERT(record->props12.pNext == &record->props13);
  APERTURE_ASSERT(record->features2.pNext == &record->features14);
  APERTURE_ASSERT(record->features14.pNext == &record->features13);
  APERTURE_ASSERT(record->features13.pNext == &record->features12);
  APERTURE_ASSERT(record->features12.pNext == &record->features11);
  APERTURE_ASSERT(record->features11.pNext == &record->robustness2);
}

[[nodiscard]] auto FillAdapter(Adapter* record, aperture::Adapter* info,
                               uint32_t index) noexcept -> Result<void> {
  APERTURE_ASSERT(record != nullptr);
  APERTURE_ASSERT(info != nullptr);
  record->backend = Backend::Vulkan;
  record->index = index;
  *info = aperture::Adapter{};
  info->index = index;

  if (auto enumerated = EnumerateVk(
          &record->extensions,
          [record](uint32_t* count, VkExtensionProperties* data) noexcept {
            return vkEnumerateDeviceExtensionProperties(record->physical_device,
                                                        nullptr, count, data);
          });
      !enumerated) [[unlikely]] {
    return enumerated;
  }

  record->has_descriptor_heap =
      HasExtension(record->extensions, VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME);
  WireAdapterChains(record);
  vkGetPhysicalDeviceProperties2(record->physical_device, &record->properties);
  vkGetPhysicalDeviceMemoryProperties(record->physical_device, &record->memory);

  uint32_t family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(record->physical_device,
                                           &family_count, nullptr);
  record->queue_families.resize(family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      record->physical_device, &family_count, record->queue_families.data());
  record->queue_families.resize(family_count);

  vkGetPhysicalDeviceFeatures2(record->physical_device, &record->features2);

  info->name = record->properties.properties.deviceName;
  info->conformance_reason = ConformanceReason(*record);
  info->conformance =
      info->conformance_reason.empty() ? Error::Ok : Error::Unsupported;

  info->vendor_id = record->properties.properties.vendorID;
  info->device_id = record->properties.properties.deviceID;
  info->driver_version = record->properties.properties.driverVersion;
  info->discrete = record->properties.properties.deviceType ==
                   VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  info->max_texture_dimension_2d =
      record->properties.properties.limits.maxImageDimension2D;
  info->subgroup_size = record->props11.subgroupSize;
  std::memcpy(info->uuid, record->props11.deviceUUID, 16);
  std::memcpy(info->luid, record->props11.deviceLUID, 8);
  info->luid_valid = record->props11.deviceLUIDValid == VK_TRUE;

  GatherMemory(record->memory, &info->local_memory_bytes,
               &info->shared_memory_bytes, &info->mapped_default_capacity);
  CountQueues(record->queue_families, &info->graphics_queue_count,
              &info->compute_queue_count, &info->copy_queue_count);

  bool presentable = false;
  for (uint32_t i = 0; i < record->queue_families.size(); ++i) {
    if ((record->queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
        PresentationSupport(record->physical_device, i)) {
      presentable = true;
      break;
    }
  }
  info->present_modes = presentable ? PresentMode::Fifo : PresentMode::None;
  if (presentable &&
      HasExtension(record->extensions, VK_KHR_PRESENT_WAIT_EXTENSION_NAME) &&
      HasExtension(record->extensions, VK_KHR_PRESENT_ID_EXTENSION_NAME)) {
    info->present_modes |= PresentMode::Waitable;
  }

  if (record->has_descriptor_heap) {
    const VkDeviceSize image_stride =
        record->descriptor_heap_props.imageDescriptorSize;
    const VkDeviceSize sampler_stride =
        record->descriptor_heap_props.samplerDescriptorSize;
    info->texture_descriptor_stride = static_cast<uint32_t>(image_stride);
    info->sampler_descriptor_stride = static_cast<uint32_t>(sampler_stride);
    info->max_texture_heap_slots = HeapSlotCount(
        record->descriptor_heap_props.maxResourceHeapSize, image_stride);
    info->max_sampler_heap_slots = HeapSlotCount(
        record->descriptor_heap_props.maxSamplerHeapSize, sampler_stride);
  }
  if (info->max_texture_heap_slots == 0) {
    info->max_texture_heap_slots = 1024;
  }
  if (info->max_sampler_heap_slots == 0) {
    info->max_sampler_heap_slots = 1024;
  }

  const VkPhysicalDeviceLimits& limits = record->properties.properties.limits;
  info->timestamps.period_ns = limits.timestampPeriod;
  const VkQueueFamilyProperties* copy_family = nullptr;
  const VkQueueFamilyProperties* graphics_family = nullptr;
  for (const VkQueueFamilyProperties& family : record->queue_families) {
    const VkQueueFlags flags = family.queueFlags;
    if (copy_family == nullptr && (flags & VK_QUEUE_TRANSFER_BIT) != 0 &&
        (flags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
        (flags & VK_QUEUE_COMPUTE_BIT) == 0) {
      copy_family = &family;
    }
    if (graphics_family == nullptr && (flags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      graphics_family = &family;
    }
  }
  const VkQueueFamilyProperties* transfer =
      copy_family != nullptr ? copy_family : graphics_family;
  if (transfer != nullptr) {
    info->copy_texture_granularity = {
        .x = transfer->minImageTransferGranularity.width,
        .y = transfer->minImageTransferGranularity.height,
        .z = transfer->minImageTransferGranularity.depth,
    };
  }
  uint64_t quantum = limits.bufferImageGranularity;
  if (quantum < 16) {
    quantum = 16;
  }
  if (!std::has_single_bit(quantum)) {
    quantum = std::bit_ceil(quantum);
  }
  info->texture_heap_alignment = quantum;

  for (const VkQueueFamilyProperties& family : record->queue_families) {
    if (family.timestampValidBits == 0) {
      continue;
    }
    const VkQueueFlags flags = family.queueFlags;
    if ((flags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      info->timestamps.graphics = true;
    }
    if ((flags & VK_QUEUE_COMPUTE_BIT) != 0 &&
        (flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
      info->timestamps.compute = true;
    }
    if ((flags & VK_QUEUE_TRANSFER_BIT) != 0 &&
        (flags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
        (flags & VK_QUEUE_COMPUTE_BIT) == 0) {
      info->timestamps.copy = true;
    }
  }

  info->capabilities = ProbeCapabilities(*record);
  return {};
}

[[nodiscard]] auto CollectInstanceLayers(
    const InstanceDesc& desc, const InstanceExtras& extras,
    std::span<const VkLayerProperties> available,
    std::pmr::vector<const char*>* layers) noexcept -> Result<void> {
  APERTURE_ASSERT(layers != nullptr);
  if (desc.enable_validation) {
#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
    if (HasLayer(available, VK_VALIDATION_LAYER)) {
      layers->push_back(VK_VALIDATION_LAYER);
    } else {
      LogFailed(desc.log_failed_results,
                "Vulkan validation layer is not available!");
      return std::unexpected(Error::Unsupported);
    }
#else
    LogFailed(desc.log_failed_results,
              "Validation support was not compiled into this build!");
    return std::unexpected(Error::Unsupported);
#endif
  }

  for (const char* layer : extras.layers) {
    if (!HasLayer(available, layer)) [[unlikely]] {
      LogFailed(desc.log_failed_results,
                "Requested instance layer is missing: {}!", layer);
      return std::unexpected(Error::Unsupported);
    }
    layers->push_back(layer);
  }
  return {};
}

[[nodiscard]] auto CollectInstanceExtensions(
    const InstanceDesc& desc, const InstanceExtras& extras,
    std::span<const VkExtensionProperties> available,
    std::pmr::vector<const char*>* extensions) noexcept -> Result<void> {
  APERTURE_ASSERT(extensions != nullptr);
  auto maybe_add_ext = [extensions, available](const char* name) noexcept {
    if (HasExtension(available, name)) {
      extensions->push_back(name);
    }
  };
  maybe_add_ext(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  AppendSurfaceInstanceExtensions(available, extensions);

  for (const char* ext : extras.extensions) {
    if (!HasExtension(available, ext)) [[unlikely]] {
      LogFailed(desc.log_failed_results,
                "Requested instance extension is missing: {}!", ext);
      return std::unexpected(Error::Unsupported);
    }
    extensions->push_back(ext);
  }
  return {};
}

void CreateDebugMessenger(
    [[maybe_unused]] Instance* impl,
    [[maybe_unused]] std::span<const VkExtensionProperties> available,
    [[maybe_unused]] bool enable_validation) noexcept {
  APERTURE_ASSERT(impl != nullptr);
#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
  if (!enable_validation) {
    return;
  }
  if (!HasExtension(available, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) ||
      vkCreateDebugUtilsMessengerEXT == nullptr) {
    return;
  }
  VkDebugUtilsMessengerCreateInfoEXT msg_ci{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = DebugCallback,
      .pUserData = impl,
  };
  vkCreateDebugUtilsMessengerEXT(impl->instance, &msg_ci, nullptr,
                                 &impl->messenger);
#endif
}

[[nodiscard]] auto EnumerateAdapters(
    Instance* impl, std::pmr::memory_resource* scratch) noexcept
    -> Result<void> {
  APERTURE_ASSERT(impl != nullptr);
  APERTURE_ASSERT(scratch != nullptr);
  std::pmr::vector<VkPhysicalDevice> physicals{scratch};
  if (auto enumerated = EnumerateVk(
          &physicals,
          [impl](uint32_t* count, VkPhysicalDevice* data) noexcept {
            return vkEnumeratePhysicalDevices(impl->instance, count, data);
          });
      !enumerated) [[unlikely]] {
    return enumerated;
  }

  const auto device_count = ToU32(physicals.size());
  if (!device_count) [[unlikely]] {
    return std::unexpected(device_count.error());
  }
  impl->records.resize(*device_count);
  impl->adapters.resize(*device_count);
  for (uint32_t i = 0; i < *device_count; ++i) {
    impl->records[i].owner = impl;
    impl->records[i].physical_device = physicals[i];
    if (auto filled = FillAdapter(&impl->records[i], &impl->adapters[i], i);
        !filled) [[unlikely]] {
      return filled;
    }
  }
  for (uint32_t i = 0; i < impl->adapters.size(); ++i) {
    impl->adapters[i].impl = &impl->records[i];
    impl->adapters[i].name = impl->records[i].properties.properties.deviceName;
  }
  return {};
}

}  // namespace

auto CreateInstance(const InstanceDesc& desc) noexcept -> Result<Instance*> {
  return CreateInstance(desc, InstanceExtras{});
}

auto CreateInstance(const InstanceDesc& desc,
                    const InstanceExtras& extras) noexcept
    -> Result<Instance*> {
  const VkResult volk_result = volkInitialize();
  if (volk_result != VK_SUCCESS) [[unlikely]] {
    LogFailed(desc.log_failed_results, "volkInitialize failed: {}!",
              ToString(volk_result));
    return std::unexpected(MapVkResult(volk_result));
  }

  if (auto cached = CacheInstanceEnumeration(); !cached) [[unlikely]] {
    LogFailed(desc.log_failed_results,
              "Failed to enumerate instance layers/extensions: {}!",
              ToString(cached.error()));
    return std::unexpected(cached.error());
  }
  const InstanceEnumeration* enumeration = PublishedEnumeration();
  APERTURE_ASSERT(enumeration != nullptr);

  Scratch scratch;
  std::pmr::vector<const char*> layers{&scratch.resource};
  std::pmr::vector<const char*> extensions{&scratch.resource};
  if (auto collected =
          CollectInstanceLayers(desc, extras, enumeration->layers, &layers);
      !collected) [[unlikely]] {
    return std::unexpected(collected.error());
  }
  if (auto collected = CollectInstanceExtensions(
          desc, extras, enumeration->extensions, &extensions);
      !collected) [[unlikely]] {
    return std::unexpected(collected.error());
  }

  const auto layer_count = ToU32(layers.size());
  if (!layer_count) [[unlikely]] {
    return std::unexpected(layer_count.error());
  }
  const auto extension_count = ToU32(extensions.size());
  if (!extension_count) [[unlikely]] {
    return std::unexpected(extension_count.error());
  }

  VkApplicationInfo app{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "aperture",
      .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
      .pEngineName = "aperture",
      .engineVersion = VK_MAKE_VERSION(0, 1, 0),
      .apiVersion = VK_API_VERSION_1_4,
  };

  VkInstanceCreateInfo ci{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app,
      .enabledLayerCount = *layer_count,
      .ppEnabledLayerNames = layers.empty() ? nullptr : layers.data(),
      .enabledExtensionCount = *extension_count,
      .ppEnabledExtensionNames =
          extensions.empty() ? nullptr : extensions.data(),
  };

  VkInstance vk_instance = VK_NULL_HANDLE;
  const VkResult created = vkCreateInstance(&ci, nullptr, &vk_instance);
  if (created != VK_SUCCESS) [[unlikely]] {
    LogFailed(desc.log_failed_results, "vkCreateInstance failed: {}",
              ToString(created));
    return std::unexpected(MapVkResult(created));
  }
  volkLoadInstance(vk_instance);

  auto* impl = new Instance{};
  impl->instance = vk_instance;
  impl->log_failed_results = desc.log_failed_results;
  impl->validation_fatal = desc.validation_fatal;
  CreateDebugMessenger(impl, enumeration->extensions, desc.enable_validation);
  if (auto enumerated = EnumerateAdapters(impl, &scratch.resource); !enumerated)
      [[unlikely]] {
    LogFailed(desc.log_failed_results, "Failed to enumerate adapters: {}!",
              ToString(enumerated.error()));
    Destroy(impl);
    return std::unexpected(enumerated.error());
  }
  return impl;
}

void Destroy(Instance* instance) noexcept {
  if (instance == nullptr) {
    return;
  }

  if (instance->messenger != VK_NULL_HANDLE &&
      vkDestroyDebugUtilsMessengerEXT != nullptr) {
    vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->messenger,
                                    nullptr);
  }
  if (instance->instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance->instance, nullptr);
  }
  delete instance;
}

auto InstanceExtensions() noexcept -> std::span<const VkExtensionProperties> {
  if (PublishedEnumeration() == nullptr) {
    if (volkInitialize() == VK_SUCCESS) {
      std::ignore = CacheInstanceEnumeration();
    }
  }
  const InstanceEnumeration* enumeration = PublishedEnumeration();
  if (enumeration == nullptr) {
    return {};
  }
  return enumeration->extensions;
}

auto InstanceLayers() noexcept -> std::span<const VkLayerProperties> {
  if (PublishedEnumeration() == nullptr) {
    if (volkInitialize() == VK_SUCCESS) {
      std::ignore = CacheInstanceEnumeration();
    }
  }
  const InstanceEnumeration* enumeration = PublishedEnumeration();
  if (enumeration == nullptr) {
    return {};
  }
  return enumeration->layers;
}

bool HasInstanceExtension(std::string_view name) noexcept {
  return HasExtension(InstanceExtensions(), name);
}

Instance& GetNative(aperture::Instance instance) noexcept {
  APERTURE_ASSERT(instance.ptr != nullptr);
  auto* impl = static_cast<Instance*>(instance.ptr);
  APERTURE_ASSERT(impl->backend == Backend::Vulkan);
  return *impl;
}

}  // namespace aperture::vk
