#include <pch.hpp>

#include <aperture/vulkan/device.hpp>

#include <aperture/adapter.hpp>
#include <aperture/assert.hpp>
#include <aperture/capability.hpp>
#include <aperture/device.hpp>
#include <aperture/result.hpp>
#include <aperture/vulkan/queue.hpp>

#include "internal.hpp"
#include "memory/storage.hpp"

#include <vk_mem_alloc.h>
#include <volk.h>
#include <aperture/vulkan/header.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <memory_resource>
#include <span>

namespace aperture::vk {

namespace {

struct QueueSelection {
  uint32_t graphics_family = 0;
  uint32_t compute_family = 0;
  uint32_t copy_family = 0;
  bool want_compute = false;
  bool want_copy = false;
  float priority = 1.F;
  std::array<VkDeviceQueueCreateInfo, 3> create_infos = {};
  uint32_t create_info_count = 0;
};

struct DeviceFeatureChain {
  VkPhysicalDeviceFeatures2 features2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
  VkPhysicalDeviceVulkan11Features v11{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
  VkPhysicalDeviceVulkan12Features v12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  VkPhysicalDeviceVulkan13Features v13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  VkPhysicalDeviceVulkan14Features v14{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES};
  VkPhysicalDeviceRobustness2FeaturesEXT robustness2{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT};
  VkPhysicalDeviceDescriptorHeapFeaturesEXT heap_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT};
  VkPhysicalDeviceMeshShaderFeaturesEXT mesh_features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT};
  VkPhysicalDeviceDeviceAddressCommandsFeaturesKHR address_commands{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR};
  VkPhysicalDeviceShaderUntypedPointersFeaturesKHR untyped_pointers{
      .sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR};
};

void EnableRequiredFeatures(DeviceFeatureChain* chain,
                            Capability requested) noexcept {
  APERTURE_ASSERT(chain != nullptr);
  VkPhysicalDeviceFeatures& f = chain->features2.features;
  f.shaderInt64 = VK_TRUE;
  f.shaderStorageImageReadWithoutFormat = VK_TRUE;
  f.shaderStorageImageWriteWithoutFormat = VK_TRUE;
  f.multiDrawIndirect = VK_TRUE;
  f.independentBlend = VK_TRUE;
  f.samplerAnisotropy = VK_TRUE;
  f.shaderInt16 = VK_TRUE;

  chain->v11.storageBuffer16BitAccess = VK_TRUE;
  chain->v11.uniformAndStorageBuffer16BitAccess = VK_TRUE;
  chain->v11.shaderDrawParameters = VK_TRUE;

  chain->v12.bufferDeviceAddress = VK_TRUE;
  chain->v12.timelineSemaphore = VK_TRUE;
  chain->v12.descriptorIndexing = VK_TRUE;
  chain->v12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
  chain->v12.descriptorBindingPartiallyBound = VK_TRUE;
  chain->v12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
  chain->v12.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
  chain->v12.descriptorBindingVariableDescriptorCount = VK_TRUE;
  chain->v12.runtimeDescriptorArray = VK_TRUE;
  chain->v12.shaderInt8 = VK_TRUE;
  chain->v12.shaderFloat16 = VK_TRUE;
  chain->v12.scalarBlockLayout = VK_TRUE;
  chain->v12.vulkanMemoryModel = VK_TRUE;
  chain->v12.vulkanMemoryModelDeviceScope = VK_TRUE;
  chain->v12.drawIndirectCount = VK_TRUE;
  if (HasAll(requested, Capability::BufferInt64Atomics)) {
    chain->v12.shaderBufferInt64Atomics = VK_TRUE;
  }

  chain->v13.synchronization2 = VK_TRUE;
  chain->v13.dynamicRendering = VK_TRUE;
  chain->v13.maintenance4 = VK_TRUE;
  chain->v13.pipelineCreationCacheControl = VK_TRUE;
  chain->v14.maintenance5 = VK_TRUE;

  chain->robustness2.nullDescriptor = VK_TRUE;

  chain->heap_features.descriptorHeap = VK_TRUE;
  chain->address_commands.deviceAddressCommands = VK_TRUE;
  chain->untyped_pointers.shaderUntypedPointers = VK_TRUE;
}

[[nodiscard]] bool PickGraphicsFamily(const Adapter& record,
                                      uint32_t* family) noexcept {
  APERTURE_ASSERT(family != nullptr);
  for (uint32_t i = 0; i < record.queue_families.size(); ++i) {
    if ((record.queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      *family = i;
      return true;
    }
  }
  return false;
}

[[nodiscard]] bool PickDedicatedCompute(const Adapter& record,
                                        uint32_t* family) noexcept {
  APERTURE_ASSERT(family != nullptr);
  for (uint32_t i = 0; i < record.queue_families.size(); ++i) {
    const VkQueueFlags flags = record.queue_families[i].queueFlags;
    if ((flags & VK_QUEUE_COMPUTE_BIT) != 0 &&
        (flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
      *family = i;
      return true;
    }
  }
  return false;
}

[[nodiscard]] bool PickDedicatedCopy(const Adapter& record,
                                     uint32_t* family) noexcept {
  APERTURE_ASSERT(family != nullptr);
  for (uint32_t i = 0; i < record.queue_families.size(); ++i) {
    const VkQueueFlags flags = record.queue_families[i].queueFlags;
    if ((flags & VK_QUEUE_TRANSFER_BIT) != 0 &&
        (flags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
        (flags & VK_QUEUE_COMPUTE_BIT) == 0) {
      *family = i;
      return true;
    }
  }
  return false;
}

void AddQueueCreateInfo(QueueSelection* queues, uint32_t family) noexcept {
  APERTURE_ASSERT(queues != nullptr);
  for (uint32_t i = 0; i < queues->create_info_count; ++i) {
    if (queues->create_infos[i].queueFamilyIndex == family) {
      return;
    }
  }
  queues->create_infos[queues->create_info_count] = VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = family,
      .queueCount = 1,
      .pQueuePriorities = &queues->priority,
  };
  ++queues->create_info_count;
}

[[nodiscard]] auto ValidateDeviceRequest(
    Instance* inst, const DeviceDesc& desc, Adapter** record,
    const aperture::Adapter** info) noexcept -> Result<void> {
  APERTURE_ASSERT(inst != nullptr);
  APERTURE_ASSERT(record != nullptr);
  APERTURE_ASSERT(info != nullptr);
  const bool log = inst->log_failed_results;

  if (desc.adapter >= inst->adapters.size() ||
      desc.adapter >= inst->records.size()) [[unlikely]] {
    LogFailed(log, "Adapter index '{}' is out of range (count={})!",
              desc.adapter, inst->adapters.size());
    return std::unexpected(Error::Invalid);
  }

  if (desc.texture_heap_slots < 1 || desc.sampler_heap_slots < 1) [[unlikely]] {
    LogFailed(log, "Heap slot counts must be at least 1!");
    return std::unexpected(Error::Invalid);
  }

  Adapter& selected = inst->records[desc.adapter];
  const aperture::Adapter& selected_info = inst->adapters[desc.adapter];
  if (!selected_info.conformant) [[unlikely]] {
    LogFailed(log, "Adapter '{}' is not conformant: {} ({})!",
              selected_info.name, selected_info.conformance_reason,
              ToString(selected_info.conformance));
    return std::unexpected(Error::Unsupported);
  }

  if (desc.texture_heap_slots > selected_info.max_texture_heap_slots ||
      desc.sampler_heap_slots > selected_info.max_sampler_heap_slots)
      [[unlikely]] {
    LogFailed(log, "Requested heap slots exceed adapter '{}' limits!",
              selected_info.name);
    return std::unexpected(Error::Invalid);
  }

  if (!HasAll(selected_info.capabilities, desc.capabilities)) [[unlikely]] {
    LogFailed(log,
              "Requested capabilities are not present on adapter '{}'"
              "(requested={}, have={})!",
              selected_info.name, ToString(desc.capabilities),
              ToString(selected_info.capabilities));
    return std::unexpected(Error::Unsupported);
  }

  *record = &selected;
  *info = &selected_info;
  return {};
}

[[nodiscard]] auto SelectDeviceQueues(const Adapter& record,
                                      Capability requested, bool log,
                                      QueueSelection* queues) noexcept
    -> Result<void> {
  APERTURE_ASSERT(queues != nullptr);
  if (!PickGraphicsFamily(record, &queues->graphics_family)) [[unlikely]] {
    LogFailed(log, "Adapter '{}' has no graphics queue!",
              record.properties.properties.deviceName);
    return std::unexpected(Error::Unsupported);
  }

  queues->want_compute = HasAll(requested, Capability::AsyncCompute);
  if (queues->want_compute &&
      !PickDedicatedCompute(record, &queues->compute_family)) [[unlikely]] {
    LogFailed(log,
              "{} requested but adapter '{}' has no dedicated compute queue!",
              ToString(Capability::AsyncCompute),
              record.properties.properties.deviceName);
    return std::unexpected(Error::Unsupported);
  }

  queues->want_copy = HasAll(requested, Capability::AsyncCopy);
  if (queues->want_copy && !PickDedicatedCopy(record, &queues->copy_family))
      [[unlikely]] {
    LogFailed(log, "{} requested but adapter '{}' has no dedicated copy queue!",
              ToString(Capability::AsyncCopy),
              record.properties.properties.deviceName);
    return std::unexpected(Error::Unsupported);
  }

  AddQueueCreateInfo(queues, queues->graphics_family);
  if (queues->want_compute) {
    AddQueueCreateInfo(queues, queues->compute_family);
  }
  if (queues->want_copy) {
    AddQueueCreateInfo(queues, queues->copy_family);
  }
  return {};
}

[[nodiscard]] auto CollectDeviceExtensions(
    const Adapter& record, const DeviceDesc& desc, const DeviceExtras& extras,
    bool log, std::pmr::vector<const char*>* device_exts) noexcept
    -> Result<void> {
  APERTURE_ASSERT(device_exts != nullptr);
  device_exts->reserve(16 + extras.extensions.size());
  auto add_ext = [&record, device_exts, log](const char* name,
                                             bool required) noexcept -> bool {
    if (HasExtension(record.extensions, name)) {
      device_exts->push_back(name);
      return true;
    }
    if (required) {
      LogFailed(log, "Required device extension is missing: {}!", name);
      return false;
    }
    return true;
  };

  if (!add_ext(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, true)) [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (!add_ext(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME, true)) [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (!add_ext(VK_KHR_DEVICE_ADDRESS_COMMANDS_EXTENSION_NAME, true))
      [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (!add_ext(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME, true))
      [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  add_ext(VK_KHR_SWAPCHAIN_EXTENSION_NAME, false);
  if (HasAll(desc.capabilities, Capability::MeshShading) &&
      !add_ext(VK_EXT_MESH_SHADER_EXTENSION_NAME, true)) [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (HasAll(desc.capabilities, Capability::HostImageCopy) &&
      !add_ext(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME, true)) [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (HasAll(desc.capabilities, Capability::UnifiedImageLayouts) &&
      !add_ext(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME, true))
      [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (HasAll(desc.capabilities, Capability::SeparateBlend) &&
      !add_ext(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, true))
      [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }
  if (HasAll(desc.capabilities, Capability::DeviceGeneratedCommands) &&
      !add_ext(VK_EXT_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME, true))
      [[unlikely]] {
    return std::unexpected(Error::Unsupported);
  }

  for (const char* ext : extras.extensions) {
    if (!HasExtension(record.extensions, ext)) [[unlikely]] {
      LogFailed(log, "Requested device extension is missing: {}!", ext);
      return std::unexpected(Error::Unsupported);
    }
    device_exts->push_back(ext);
  }
  return {};
}

struct ExtrasSplice {
  static constexpr uint32_t MAX_LINKS = 16;

  struct Link {
    VkBaseOutStructure* node = nullptr;
    VkBaseOutStructure* p_next = nullptr;
  };

  ExtrasSplice() noexcept = default;
  ExtrasSplice(const ExtrasSplice&) = delete;
  ExtrasSplice(ExtrasSplice&& other) noexcept
      : links(other.links), count(other.count) {
    other.count = 0;
  }
  ~ExtrasSplice() noexcept { Restore(); }

  ExtrasSplice& operator=(const ExtrasSplice&) = delete;
  ExtrasSplice& operator=(ExtrasSplice&& other) noexcept;

  void Restore() noexcept;

  std::array<Link, MAX_LINKS> links = {};
  uint32_t count = 0;
};

ExtrasSplice& ExtrasSplice::operator=(ExtrasSplice&& other) noexcept {
  if (this == &other) [[unlikely]] {
    return *this;
  }

  Restore();
  links = other.links;
  count = other.count;
  other.count = 0;
  return *this;
}

void ExtrasSplice::Restore() noexcept {
  while (count > 0) {
    --count;
    links[count].node->pNext = links[count].p_next;
  }
}

[[nodiscard]] constexpr bool IsOwnedFeatureStruct(
    VkStructureType type) noexcept {
  switch (type) {
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR:
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR:
      return true;
    default:
      return false;
  }
}

void AdoptFeatureStruct(DeviceFeatureChain* chain,
                        VkBaseOutStructure* node) noexcept {
  APERTURE_ASSERT(chain != nullptr);
  APERTURE_ASSERT(node != nullptr);
  switch (node->sType) {
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceVulkan11Features*>(node);
      void* keep = chain->v11.pNext;
      chain->v11 = *src;
      chain->v11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
      chain->v11.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceVulkan12Features*>(node);
      void* keep = chain->v12.pNext;
      chain->v12 = *src;
      chain->v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
      chain->v12.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceVulkan13Features*>(node);
      void* keep = chain->v13.pNext;
      chain->v13 = *src;
      chain->v13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
      chain->v13.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceVulkan14Features*>(node);
      void* keep = chain->v14.pNext;
      chain->v14 = *src;
      chain->v14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
      chain->v14.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceRobustness2FeaturesEXT*>(node);
      void* keep = chain->robustness2.pNext;
      chain->robustness2 = *src;
      chain->robustness2.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
      chain->robustness2.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceDescriptorHeapFeaturesEXT*>(
              node);
      void* keep = chain->heap_features.pNext;
      chain->heap_features = *src;
      chain->heap_features.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT;
      chain->heap_features.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT: {
      const auto* src =
          reinterpret_cast<const VkPhysicalDeviceMeshShaderFeaturesEXT*>(node);
      void* keep = chain->mesh_features.pNext;
      chain->mesh_features = *src;
      chain->mesh_features.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
      chain->mesh_features.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR: {
      const auto* src = reinterpret_cast<
          const VkPhysicalDeviceDeviceAddressCommandsFeaturesKHR*>(node);
      void* keep = chain->address_commands.pNext;
      chain->address_commands = *src;
      chain->address_commands.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_ADDRESS_COMMANDS_FEATURES_KHR;
      chain->address_commands.pNext = keep;
      break;
    }
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR: {
      const auto* src = reinterpret_cast<
          const VkPhysicalDeviceShaderUntypedPointersFeaturesKHR*>(node);
      void* keep = chain->untyped_pointers.pNext;
      chain->untyped_pointers = *src;
      chain->untyped_pointers.sType =
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR;
      chain->untyped_pointers.pNext = keep;
      break;
    }
    default:
      break;
  }
}

[[nodiscard]] auto BuildDeviceFeatureChain(DeviceFeatureChain* chain,
                                           Capability requested,
                                           const DeviceExtras& extras,
                                           bool log) noexcept
    -> Result<ExtrasSplice> {
  APERTURE_ASSERT(chain != nullptr);

  chain->v14.pNext = &chain->v13;
  chain->v13.pNext = &chain->v12;
  chain->v12.pNext = &chain->v11;
  chain->v11.pNext = &chain->robustness2;
  chain->features2.pNext = &chain->v14;

  chain->robustness2.pNext = &chain->heap_features;
  chain->heap_features.pNext = nullptr;
  chain->mesh_features.pNext = nullptr;
  chain->address_commands.pNext = nullptr;
  chain->untyped_pointers.pNext = nullptr;

  void** tail = &chain->heap_features.pNext;
  if (HasAll(requested, Capability::MeshShading)) {
    *tail = &chain->mesh_features;
    tail = &chain->mesh_features.pNext;
  }
  *tail = &chain->address_commands;
  tail = &chain->address_commands.pNext;
  *tail = &chain->untyped_pointers;
  tail = &chain->untyped_pointers.pNext;

  std::array<VkBaseOutStructure*, ExtrasSplice::MAX_LINKS> unknowns = {};
  uint32_t unknown_count = 0;
  if (extras.features != nullptr) {
    chain->features2.features = extras.features->features;
    auto* node = static_cast<VkBaseOutStructure*>(extras.features->pNext);
    while (node != nullptr) {
      VkBaseOutStructure* next = node->pNext;
      if (IsOwnedFeatureStruct(node->sType)) {
        AdoptFeatureStruct(chain, node);
      } else if (unknown_count == unknowns.size()) [[unlikely]] {
        LogFailed(log, "DeviceExtras feature chain is too long ({})!",
                  ToString(Error::Invalid));
        return std::unexpected(Error::Invalid);
      } else {
        unknowns[unknown_count++] = node;
      }
      node = next;
    }
  }

  EnableRequiredFeatures(chain, requested);
  if (HasAll(requested, Capability::MeshShading)) {
    chain->mesh_features.meshShader = VK_TRUE;
    chain->mesh_features.taskShader = VK_TRUE;
  }

  ExtrasSplice splice;
  for (uint32_t i = 0; i < unknown_count; ++i) {
    splice.links[i].node = unknowns[i];
    splice.links[i].p_next = unknowns[i]->pNext;
    *tail = unknowns[i];
    tail = reinterpret_cast<void**>(&unknowns[i]->pNext);
  }
  if (unknown_count > 0) {
    unknowns[unknown_count - 1]->pNext = nullptr;
  }
  splice.count = unknown_count;
  return std::move(splice);
}

[[nodiscard]] auto CreateVmaAllocator(VkInstance instance, VkDevice device,
                                      VkPhysicalDevice physical,
                                      VmaAllocator* allocator) noexcept
    -> Result<void> {
  APERTURE_ASSERT(allocator != nullptr);
  VmaVulkanFunctions vma_fns{};
  vma_fns.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vma_fns.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
  VmaAllocatorCreateInfo vma_ci{};
  vma_ci.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT |
                 VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
  vma_ci.physicalDevice = physical;
  vma_ci.device = device;
  vma_ci.instance = instance;
  vma_ci.vulkanApiVersion = VK_API_VERSION_1_4;
  vma_ci.pVulkanFunctions = &vma_fns;
  if (vmaCreateAllocator(&vma_ci, allocator) != VK_SUCCESS) {
    return std::unexpected(Error::OutOfMemory);
  }
  return {};
}

void InitDeviceQueues(Device* device, VkDevice vk_device,
                      const QueueSelection& queues) noexcept {
  APERTURE_ASSERT(device != nullptr);
  device->graphics.device = device;
  device->graphics.family = queues.graphics_family;
  device->graphics.usage = QueueUsage::Graphics;
  vkGetDeviceQueue(vk_device, queues.graphics_family, 0,
                   &device->graphics.queue);
  if (queues.want_compute) {
    device->has_compute = true;
    device->compute.device = device;
    device->compute.family = queues.compute_family;
    device->compute.usage = QueueUsage::Compute;
    vkGetDeviceQueue(vk_device, queues.compute_family, 0,
                     &device->compute.queue);
  }
  if (queues.want_copy) {
    device->has_copy = true;
    device->copy.device = device;
    device->copy.family = queues.copy_family;
    device->copy.usage = QueueUsage::Copy;
    vkGetDeviceQueue(vk_device, queues.copy_family, 0, &device->copy.queue);
  }
}

void FillDeviceInfo(Device* device, const aperture::Adapter& info,
                    const DeviceDesc& desc, uint32_t push_bytes,
                    const Adapter& record,
                    const QueueSelection& queues) noexcept {
  APERTURE_ASSERT(device != nullptr);
  device->info.profile = AddressingProfile::Pointer;
  device->info.texture_heap_alignment = info.texture_heap_alignment;
  device->info.timestamp_period_ns = info.timestamp_period_ns;
  device->info.texture_descriptor_stride = info.texture_descriptor_stride;
  device->info.sampler_descriptor_stride = info.sampler_descriptor_stride;
  device->info.texture_heap_slots = desc.texture_heap_slots;
  device->info.sampler_heap_slots = desc.sampler_heap_slots;
  device->info.null_texture_slot = 0;
  device->info.null_sampler_slot = 0;
  device->info.max_cpu_root_bytes = push_bytes;
  device->info.copy_texture_granularity = info.copy_texture_granularity;

  const auto timestamped = [&record](uint32_t family) noexcept {
    return family < record.queue_families.size() &&
           record.queue_families[family].timestampValidBits != 0;
  };
  device->info.graphics_timestamps = timestamped(queues.graphics_family);
  device->info.compute_timestamps =
      queues.want_compute && timestamped(queues.compute_family);
  device->info.copy_timestamps =
      queues.want_copy && timestamped(queues.copy_family);
}

void DestroyPartialDevice(Device* device) noexcept {
  if (device == nullptr) {
    return;
  }

  if (device->memory != nullptr) {
    DestroyMemory(device);
  }
  if (device->allocator != VK_NULL_HANDLE) {
    vmaDestroyAllocator(device->allocator);
  }
  if (device->device != VK_NULL_HANDLE) {
    vkDestroyDevice(device->device, nullptr);
  }
  delete device;
}

}  // namespace

auto CreateDevice(Instance* instance, const DeviceDesc& desc) noexcept
    -> Result<Device*> {
  return CreateDevice(instance, desc, DeviceExtras{});
}

auto CreateDevice(Instance* instance, const DeviceDesc& desc,
                  const DeviceExtras& extras) noexcept -> Result<Device*> {
  APERTURE_ASSERT(instance != nullptr, "Null instance!");
  const bool log = instance->log_failed_results;

  Adapter* record = nullptr;
  const aperture::Adapter* info = nullptr;
  if (auto valid = ValidateDeviceRequest(instance, desc, &record, &info);
      !valid) [[unlikely]] {
    return std::unexpected(valid.error());
  }

  QueueSelection queues;
  if (auto selected =
          SelectDeviceQueues(*record, desc.capabilities, log, &queues);
      !selected) [[unlikely]] {
    return std::unexpected(selected.error());
  }

  Scratch scratch;
  std::pmr::vector<const char*> device_exts{&scratch.resource};
  if (auto collected =
          CollectDeviceExtensions(*record, desc, extras, log, &device_exts);
      !collected) [[unlikely]] {
    return std::unexpected(collected.error());
  }
  const auto ext_count = ToU32(device_exts.size());
  if (!ext_count) [[unlikely]] {
    return std::unexpected(ext_count.error());
  }

  DeviceFeatureChain chain;
  auto feature_chain =
      BuildDeviceFeatureChain(&chain, desc.capabilities, extras, log);
  if (!feature_chain) [[unlikely]] {
    return std::unexpected(feature_chain.error());
  }
  ExtrasSplice splice = std::move(*feature_chain);
  if (record->features12.bufferDeviceAddressCaptureReplay == VK_TRUE) {
    chain.v12.bufferDeviceAddressCaptureReplay = VK_TRUE;
  }

  VkDeviceCreateInfo device_ci{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &chain.features2,
      .queueCreateInfoCount = queues.create_info_count,
      .pQueueCreateInfos = queues.create_infos.data(),
      .enabledExtensionCount = *ext_count,
      .ppEnabledExtensionNames = device_exts.data(),
  };

  VkDevice vk_device = VK_NULL_HANDLE;
  const VkResult created =
      vkCreateDevice(record->physical_device, &device_ci, nullptr, &vk_device);
  if (created != VK_SUCCESS) [[unlikely]] {
    LogFailed(log, "vkCreateDevice failed: {}!", ToString(created));
    return std::unexpected(MapVkResult(created));
  }
  volkLoadDevice(vk_device);

  auto* impl = new Device{};
  impl->parent = instance;
  impl->instance = instance->instance;
  impl->device = vk_device;
  impl->physical_device = record->physical_device;
  impl->log_failed_results = log;
  impl->enabled = desc.capabilities;
  impl->capture_replay =
      record->features12.bufferDeviceAddressCaptureReplay == VK_TRUE;

  InitDeviceQueues(impl, vk_device, queues);

  if (auto allocator =
          CreateVmaAllocator(instance->instance, vk_device,
                             record->physical_device, &impl->allocator);
      !allocator) [[unlikely]] {
    DestroyPartialDevice(impl);
    LogFailed(log, "vmaCreateAllocator failed!");
    return std::unexpected(allocator.error());
  }
  InitMemory(impl);

  const auto push_bytes = std::min(
      static_cast<uint32_t>(record->descriptor_heap_props.maxPushDataSize),
      256U);

  FillDeviceInfo(impl, *info, desc, push_bytes, *record, queues);
  return impl;
}

void Destroy(Device* device) noexcept {
  DestroyPartialDevice(device);
}

Device& GetNative(aperture::Device device) noexcept {
  APERTURE_ASSERT(device.ptr != nullptr);
  auto* impl = static_cast<Device*>(device.ptr);
  APERTURE_ASSERT(impl->backend == Backend::Vulkan);
  return *impl;
}

}  // namespace aperture::vk
