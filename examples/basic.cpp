#include <aperture/aperture.hpp>
#include <aperture/utils/defer.hpp>
#include <common/window.hpp>

#include <algorithm>
#include <cstdint>

int main() {
  using namespace aperture;

  auto window_result = examples::CreateWindow({
      .title = "aperture basic",
      .width = 1280,
      .height = 720,
  });
  if (!window_result) {
    log::Error("Create window failed ({})!", ToString(window_result.error()));
    return 1;
  }

  examples::Window window = *window_result;
  APERTURE_DEFER {
    examples::Destroy(window);
  };

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

  log::Info("compiled backends: {}", CompiledBackends().size());
  log::Info("available backends: {}", AvailableBackends().size());
  log::Info("preferred backend: {} (available={})", ToString(preferred),
            Available(preferred) ? "yes" : "no");

  auto instance_result = CreateInstance({
      .log_failed_results = true,
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
              adapter.conformant ? " [conformant]" : " [non-conformant]");
    if (!adapter.conformant && !adapter.conformance_reason.empty()) {
      log::Info("  reason: {}", adapter.conformance_reason);
    }
    log::Info("  discrete={} local_memory={} queues g/c/c={}/{}/{}",
              adapter.discrete ? "yes" : "no", adapter.local_memory_bytes,
              adapter.graphics_queue_count, adapter.compute_queue_count,
              adapter.copy_queue_count);
    log::Info("  texture_slots={} sampler_slots={}",
              adapter.max_texture_heap_slots, adapter.max_sampler_heap_slots);
  }

  const auto chosen_adapter = std::ranges::find_if(
      adapters,
      [](const Adapter& adapter) noexcept { return adapter.conformant; });
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
  log::Info("supports fifo: {}",
            SupportsPresent(adapter, PresentMode::Fifo) ? "yes" : "no");
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
                                 .adapter = adapter.index,
                                 .texture_heap_slots = texture_slots,
                                 .sampler_heap_slots = sampler_slots,
                                 .capabilities = requested,
                             });
  if (!device_result) {
    log::Error("CreateDevice failed ({})!", ToString(device_result.error()));
    return 1;
  }

  Device device = *device_result;
  APERTURE_DEFER {
    Destroy(device);
  };

  const DeviceInfo& info = Info(device);
  log::Info("device: profile={} texture_slots={} sampler_slots={} push_data={}",
            ToString(info.profile), info.texture_heap_slots,
            info.sampler_heap_slots, info.max_cpu_root_bytes);
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

  return 0;
}
