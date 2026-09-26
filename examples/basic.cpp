#include <aperture/aperture.hpp>
#include <aperture/utils/defer.hpp>
#include <aperture/vulkan.hpp>

#ifdef CreateWindow
#undef CreateWindow
#endif

#include <common/window.hpp>

#include <algorithm>
#include <cstdint>

using namespace aperture;

namespace {

void LogVersion() {
  const Version header = HeaderVersion();
  const Version linked = LinkedVersion();
  log::Info("headers {}.{}.{} linked {}.{}.{} compatible={}", header.major,
            header.minor, header.patch, linked.major, linked.minor,
            linked.patch, CompatibleWithHeaders() ? "yes" : "no");
}

void LogAdapterGpuFields(const aperture::Adapter& adapter) {
  log::Info(
      "  heap_align={} timestamp_period_ns={} copy_gran={}/{}/{} "
      "timestamps g/c/c={}/{}/{}",
      adapter.texture_heap_alignment, adapter.timestamps.period_ns,
      adapter.copy_texture_granularity.x, adapter.copy_texture_granularity.y,
      adapter.copy_texture_granularity.z, adapter.timestamps.graphics,
      adapter.timestamps.compute, adapter.timestamps.copy);
  log::Info("  descriptor_stride tex/samp={}/{}",
            adapter.texture_descriptor_stride,
            adapter.sampler_descriptor_stride);
}

void LogAdapterCapabilities(const aperture::Adapter& adapter) {
  log::Info("  Supports(SplitBarriers)={}",
            Supports(adapter, Capability::SplitBarriers) ? "yes" : "no");
  log::Info("  Supports(HostImageCopy)={}",
            Supports(adapter, Capability::HostImageCopy) ? "yes" : "no");
  log::Info("  Supports(UnifiedImageLayouts)={}",
            Supports(adapter, Capability::UnifiedImageLayouts) ? "yes" : "no");
}

void LogPresentModes(const aperture::Adapter& adapter) {
  log::Info("supports fifo: {}",
            SupportsPresent(adapter, PresentMode::Fifo) ? "yes" : "no");
  log::Info("supports immediate: {}",
            SupportsPresent(adapter, PresentMode::Immediate) ? "yes" : "no");
  log::Info("supports mailbox: {}",
            SupportsPresent(adapter, PresentMode::Mailbox) ? "yes" : "no");
}

void LogDeviceInfo(const aperture::DeviceInfo& info) {
  log::Info(
      "device: profile={} texture_slots={} sampler_slots={} push_data={} "
      "heap_align={} timestamp_period_ns={} copy_gran={}/{}/{}",
      ToString(info.profile), info.texture_heap_slots, info.sampler_heap_slots,
      info.max_cpu_root_bytes, info.texture_heap_alignment,
      info.timestamps.period_ns, info.copy_texture_granularity.x,
      info.copy_texture_granularity.y, info.copy_texture_granularity.z);
  log::Info("  descriptor_stride tex/samp={}/{} null_slots tex/samp={}/{}",
            info.texture_descriptor_stride, info.sampler_descriptor_stride,
            info.null_texture_slot, info.null_sampler_slot);
  log::Info("  timestamps g/c/c={}/{}/{}", info.timestamps.graphics,
            info.timestamps.compute, info.timestamps.copy);
  log::Info("  heap_device tex={:#x} samp={:#x}", info.texture_heap_device.addr,
            info.sampler_heap_device.addr);
}

void LogShaderAbi(aperture::Backend backend) {
  log::Info("native shader format: {}", ToString(NativeShaderFormat(backend)));
  log::Info("spirv compatible: {}",
            Compatible(ShaderFormat::Spirv, backend) ? "yes" : "no");
  log::Info("texture_slot {} sampler_slot {} {}", ToString(TextureSlot::Null),
            ToString(SamplerSlot::LinearClamp),
            ToString(SamplerSlot::FirstUser));
}

}  // namespace

int main() {
  log::Info("aperture {}", VersionString());
  LogVersion();
  if (!CompatibleWithHeaders()) {
    log::Error("Header/linked version mismatch!");
    return 1;
  }

  auto window_result = examples::CreateWindow({
      .width = 1280,
      .height = 720,
      .title = "aperture basic",
  });
  if (!window_result) {
    log::Error("Create window failed ({})!", ToString(window_result.error()));
    return 1;
  }

  examples::Window window = *window_result;
  APERTURE_DEFER {
    examples::Destroy(window);
  };

  examples::PollWindowEvents();
  const uint2 client = examples::Size(window);
  const uint2 framebuffer = examples::FramebufferSize(window);
  log::Info("window client={}x{} framebuffer={}x{}", client[0], client[1],
            framebuffer[0], framebuffer[1]);

  auto surface_result = examples::Surface(window);
  if (!surface_result) {
    log::Error("Window surface failed ({})!", ToString(surface_result.error()));
    return 1;
  }

  Surface surface = *surface_result;

  auto preferred_result = PreferredBackend();
  if (!preferred_result) {
    log::Error("PreferredBackend failed ({})!",
               ToString(preferred_result.error()));
    return 1;
  }

  const Backend preferred = *preferred_result;

  for (const Backend backend : CompiledBackends()) {
    log::Info("compiled backend: {} available={}", ToString(backend),
              Available(backend) ? "yes" : "no");
  }

  log::Info("preferred backend: {} (available={})", ToString(preferred),
            Available(preferred) ? "yes" : "no");

  auto instance_result =
      CreateInstance(preferred, {
                                    .log_failed_results = true,
                                    .enable_validation = true,
                                    .validation_fatal = true,
                                });
  if (!instance_result) {
    log::Error("CreateInstance failed ({})!",
               ToString(instance_result.error()));
    return 1;
  }

  Instance instance = *instance_result;
  log::Info("instance backend: {}", ToString(BackendOf(instance)));
  APERTURE_DEFER {
    Destroy(instance);
  };

  const auto adapters = Adapters(instance);
  if (adapters.empty()) {
    log::Error("No adapters!");
    return 1;
  }

  for (const Adapter& adapter : adapters) {
    log::Info("adapter {}: {}{}", adapter.index, adapter.name,
              Conformant(adapter) ? " [conformant]" : " [non-conformant]");
    if (!Conformant(adapter) && !adapter.conformance_reason.empty()) {
      log::Info("  reason: {}", adapter.conformance_reason);
    }
    log::Info(
        "  discrete={} local_memory={} shared_memory={} queues g/c/c={}/{}/{}",
        adapter.discrete ? "yes" : "no", adapter.local_memory_bytes,
        adapter.shared_memory_bytes, adapter.graphics_queue_count,
        adapter.compute_queue_count, adapter.copy_queue_count);
    log::Info("  texture_slots={} sampler_slots={} max_tex_2d={}",
              adapter.max_texture_heap_slots, adapter.max_sampler_heap_slots,
              adapter.max_texture_dimension_2d);
    LogAdapterGpuFields(adapter);
    LogAdapterCapabilities(adapter);
  }

  const auto chosen_adapter = std::ranges::find_if(
      adapters,
      [](const Adapter& adapter) noexcept { return Conformant(adapter); });
  if (chosen_adapter == adapters.end()) {
    log::Error("No conformant adapter!");
    return 1;
  }

  const Adapter& adapter = *chosen_adapter;

  const auto presentable = PresentableFormats(adapter, surface);
  log::Info("presentable formats ({}):", presentable.size());
  for (const TextureFormat format : presentable) {
    log::Info("  {}", ToString(format));
  }
  LogPresentModes(adapter);
  log::Info("supports R8G8B8A8Unorm color+sample: {}",
            SupportsFormat(adapter, TextureFormat::R8G8B8A8Unorm,
                           FormatUsage::Color | FormatUsage::Sample)
                ? "yes"
                : "no");

  auto requested = Capability::None;
  if (Supports(adapter, Capability::AsyncCompute)) {
    requested |= Capability::AsyncCompute;
  }
  if (Supports(adapter, Capability::AsyncCopy)) {
    requested |= Capability::AsyncCopy;
  }

  const uint32_t texture_slots =
      std::min(adapter.max_texture_heap_slots, 1024U);
  const uint32_t sampler_slots = std::min(adapter.max_sampler_heap_slots, 32U);

  auto device_result =
      CreateDevice(instance, {
                                 .capabilities = requested,
                                 .adapter = adapter.index,
                                 .texture_heap_slots = texture_slots,
                                 .sampler_heap_slots = sampler_slots,
                                 .pipeline_policy = PipelinePolicy::FailOnMiss,
                             });
  if (!device_result) {
    log::Error("CreateDevice failed ({})!", ToString(device_result.error()));
    return 1;
  }

  Device device = *device_result;
  APERTURE_DEFER {
    Destroy(device);
  };

  log::Info("device backend: {}", ToString(BackendOf(device)));
  LogDeviceInfo(Info(device));
  LogShaderAbi(BackendOf(device));
  log::Info("Has(MeshShading)={}",
            Has(device, Capability::MeshShading) ? "yes" : "no");
  log::Info("Has(AsyncCompute)={} Has(AsyncCopy)={}",
            Has(device, Capability::AsyncCompute) ? "yes" : "no",
            Has(device, Capability::AsyncCopy) ? "yes" : "no");

  const Queue graphics = GraphicsQueue(device);
  log::Info("graphics queue: {}", graphics ? "yes" : "no");

  auto compute = ComputeQueue(device);
  if (compute) {
    log::Info("compute queue: yes");
  } else {
    log::Info("compute queue: no ({})", ToString(compute.error()));
  }

  auto copy = CopyQueue(device);
  if (copy) {
    log::Info("copy queue: yes");
  } else {
    log::Info("copy queue: no ({})", ToString(copy.error()));
  }

  QueueUsage shared_usage = QueueUsage::Graphics;
  if (compute) {
    shared_usage = shared_usage | QueueUsage::Compute;
  }

  auto numbers_result =
      Malloc<uint32_t>(device, 1024, Memory::Default, shared_usage);
  if (!numbers_result) {
    log::Error("Malloc failed ({})!", ToString(numbers_result.error()));
    return 1;
  }
  DualPtr<uint32_t> numbers = *numbers_result;
  APERTURE_DEFER {
    Free(device, numbers);
  };
  numbers.host[0] = 42;
  numbers.host[1023] = 7;
  const GpuPtr<std::byte> looked_up = DeviceAddressOf(device, numbers.host);
  log::Info("malloc count=1024 host[0]={} gpu={:#x} lookup={:#x}",
            numbers.host[0], numbers.device.addr, looked_up.addr);
  if (looked_up.addr != numbers.device.addr) {
    log::Error("DeviceAddressOf mismatch!");
    return 1;
  }

  const GpuRange number_range = GpuRangeFrom(numbers, 16);
  log::Info("gpu_range_from count=16 bytes={}", number_range.size);
  const DualRange dual_range = DualRangeFrom(numbers, 4);
  const DualRange tail = Slice(dual_range, sizeof(uint32_t), sizeof(uint32_t));
  log::Info("dual_range slice device={:#x} bytes={}", tail.ptr.device.addr,
            tail.size);
  const GpuPtr<uint32_t> at_one = numbers.device + 1;
  log::Info("gpu_ptr+1 addr={:#x}", at_one.addr);

  auto readback_result = Malloc<uint8_t>(device, 16, Memory::Readback);
  if (!readback_result) {
    log::Error("Malloc Readback failed ({})!",
               ToString(readback_result.error()));
    return 1;
  }
  DualPtr<uint8_t> readback = *readback_result;
  APERTURE_DEFER {
    Free(device, readback);
  };
  readback.host[0] = 0xAB;
  readback.host[15] = 0xCD;
  log::Info("readback host[0]={:#02x} host[15]={:#02x}", readback.host[0],
            readback.host[15]);

  auto gpu_result = MallocGpu<uint32_t>(device, 256, shared_usage);
  if (!gpu_result) {
    log::Error("MallocGpu failed ({})!", ToString(gpu_result.error()));
    return 1;
  }
  GpuPtr<uint32_t> gpu_only = *gpu_result;
  APERTURE_DEFER {
    Free(device, gpu_only);
  };
  log::Info("malloc_gpu count=256 gpu={:#x}", gpu_only.addr);

  auto gpu_dedicated_result =
      MallocGpu<uint32_t>(device, 64, shared_usage, MallocFlags::Dedicated);
  if (!gpu_dedicated_result) {
    log::Error("MallocGpu Dedicated failed ({})!",
               ToString(gpu_dedicated_result.error()));
    return 1;
  }
  GpuPtr<uint32_t> gpu_dedicated = *gpu_dedicated_result;
  APERTURE_DEFER {
    Free(device, gpu_dedicated);
  };
  log::Info("malloc_gpu_dedicated count=64 gpu={:#x}", gpu_dedicated.addr);

  constexpr size_t bump_bytes = 64U * 1024U;
  auto bump_storage_result = Malloc(device, bump_bytes, 16);
  if (!bump_storage_result) {
    log::Error("Bump storage Malloc failed ({})!",
               ToString(bump_storage_result.error()));
    return 1;
  }
  DualPtr<std::byte> bump_storage = *bump_storage_result;
  APERTURE_DEFER {
    Free(device, bump_storage);
  };

  BumpAllocator bump(bump_bytes);
  const auto tmp_span = bump.Allocate<float>(16);
  if (!tmp_span) {
    log::Error("BumpAllocator Allocate failed!");
    return 1;
  }
  const DualRange tmp = Slice(bump_storage, tmp_span);
  auto* floats = reinterpret_cast<float*>(tmp.ptr.host);
  floats[0] = 1.0F;
  floats[15] = 2.0F;
  log::Info("bump alloc count=16 host[0]={} gpu={:#x} used={} free={}",
            floats[0], tmp.ptr.device.addr, bump.Used(), bump.FreeBytes());

  bump.Reset();
  const auto tmp2_span = bump.Allocate<float>(16);
  if (!tmp2_span) {
    log::Error("BumpAllocator Allocate after Reset failed!");
    return 1;
  }
  log::Info("bump reset+alloc gpu={:#x} empty={}",
            bump_storage.device.addr + tmp2_span.offset,
            bump.Empty() ? "yes" : "no");

  OffsetAllocator offsets(static_cast<uint32_t>(bump_bytes));
  const auto offset_a = offsets.Allocate(128);
  const auto offset_b = offsets.Allocate(256);
  if (!offset_a || !offset_b) {
    log::Error("OffsetAllocator Allocate failed!");
    return 1;
  }
  offsets.Free(offset_a);
  const OffsetAllocator::StorageReport report = offsets.Report();
  log::Info("offset_allocator free={} largest={}", report.free_bytes,
            report.largest_free_region);

  constexpr size_t dedicated_bytes = 4U * 1024U;
  auto dedicated_result = Malloc(device, dedicated_bytes, 16, Memory::Default,
                                 QueueUsage::Graphics, MallocFlags::Dedicated);
  if (!dedicated_result) {
    log::Error("Malloc Dedicated failed ({})!",
               ToString(dedicated_result.error()));
    return 1;
  }
  DualPtr<std::byte> dedicated = *dedicated_result;
  APERTURE_DEFER {
    Free(device, dedicated);
  };
  BumpAllocator dedicated_bump(dedicated_bytes);
  const auto dedicated_span = dedicated_bump.Allocate<uint32_t>(8);
  if (!dedicated_span) {
    log::Error("BumpAllocator over dedicated Malloc failed!");
    return 1;
  }
  const DualRange dedicated_range = Slice(dedicated, dedicated_span);
  auto* words = reinterpret_cast<uint32_t*>(dedicated_range.ptr.host);
  words[0] = 11;
  words[7] = 99;
  log::Info("malloc_dedicated bytes={} host[0]={} gpu={:#x}", dedicated_bytes,
            words[0], dedicated_range.ptr.device.addr);

  auto timeline_result = CreateTimeline(device);
  if (!timeline_result) {
    log::Error("CreateTimeline failed ({})!",
               ToString(timeline_result.error()));
    return 1;
  }
  Timeline timeline = *timeline_result;
  APERTURE_DEFER {
    Destroy(timeline);
  };

  auto pool_result = CreateCommandPool(graphics);
  if (!pool_result) {
    log::Error("CreateCommandPool failed ({})!", ToString(pool_result.error()));
    return 1;
  }
  CommandPool pool = *pool_result;
  APERTURE_DEFER {
    Destroy(pool);
  };

  constexpr uint32_t copy_count = 4;
  auto copy_src_result = Malloc<uint32_t>(device, copy_count);
  if (!copy_src_result) {
    log::Error("Copy source Malloc failed ({})!",
               ToString(copy_src_result.error()));
    return 1;
  }
  DualPtr<uint32_t> copy_src = *copy_src_result;
  APERTURE_DEFER {
    Free(device, copy_src);
  };
  auto copy_dst_result = Malloc<uint32_t>(device, copy_count, Memory::Readback);
  if (!copy_dst_result) {
    log::Error("Copy destination Malloc failed ({})!",
               ToString(copy_dst_result.error()));
    return 1;
  }
  DualPtr<uint32_t> copy_dst = *copy_dst_result;
  APERTURE_DEFER {
    Free(device, copy_dst);
  };

  copy_src.host[0] = 11;
  copy_src.host[1] = 22;
  copy_src.host[2] = 33;
  copy_src.host[3] = 44;

  auto begun = Begin(pool);
  if (!begun) {
    log::Error("Begin failed ({})!", ToString(begun.error()));
    return 1;
  }
  CommandBuffer command = *begun;
  Barrier(&command, Stage::Host, Stage::Copy);
  Copy(&command, GpuRangeFrom(copy_dst, copy_count),
       GpuRangeFrom(copy_src, copy_count));

  CommandBuffer submit_buffers[] = {command};
  const TimelineSignal signals[] = {{
      .timeline = timeline,
      .value = 1,
  }};
  auto submitted = Submit(graphics, {
                                        .buffers = submit_buffers,
                                        .signals = signals,
                                    });
  if (!submitted) {
    log::Error("Submit failed ({})!", ToString(submitted.error()));
    return 1;
  }
  Wait(timeline, 1);
  const uint64_t signaled = CurrentValue(timeline);
  log::Info("copy timeline={} dst={}/{}/{}/{}", signaled, copy_dst.host[0],
            copy_dst.host[1], copy_dst.host[2], copy_dst.host[3]);
  if (signaled != 1 || copy_dst.host[0] != 11 || copy_dst.host[1] != 22 ||
      copy_dst.host[2] != 33 || copy_dst.host[3] != 44) {
    log::Error("Copy readback mismatch!");
    return 1;
  }

  return 0;
}
