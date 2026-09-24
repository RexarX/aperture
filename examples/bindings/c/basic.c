#include <common/window.h>

#include <aperture/aperture.h>
#ifdef APERTURE_HAS_VULKAN
#include <aperture/vulkan.h>
#endif

#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

static void log_version(void) {
  const ApertureVersion header = aperture_header_version();
  const ApertureVersion linked = aperture_linked_version();
  aperture_log_infof("headers %u.%u.%u linked %u.%u.%u compatible=%s",
                     header.major, header.minor, header.patch, linked.major,
                     linked.minor, linked.patch,
                     aperture_compatible_with_headers() ? "yes" : "no");
}

static void log_adapter_gpu_fields(const ApertureAdapter* adapter) {
  aperture_log_infof(
      "  heap_align=%llu timestamp_period_ns=%g copy_gran=%u/%u/%u "
      "timestamps g/c/c=%s/%s/%s",
      (unsigned long long)adapter->texture_heap_alignment,
      (double)adapter->timestamp_period_ns, adapter->copy_texture_granularity.x,
      adapter->copy_texture_granularity.y, adapter->copy_texture_granularity.z,
      adapter->graphics_timestamps ? "yes" : "no",
      adapter->compute_timestamps ? "yes" : "no",
      adapter->copy_timestamps ? "yes" : "no");
  aperture_log_infof("  descriptor_stride tex/samp=%u/%u",
                     adapter->texture_descriptor_stride,
                     adapter->sampler_descriptor_stride);
}

static void log_adapter_capabilities(const ApertureAdapter* adapter) {
  aperture_log_infof(
      "  Supports(SplitBarriers)=%s",
      aperture_adapter_supports(adapter, APERTURE_CAPABILITY_SPLIT_BARRIERS)
          ? "yes"
          : "no");
  aperture_log_infof(
      "  Supports(HostImageCopy)=%s",
      aperture_adapter_supports(adapter, APERTURE_CAPABILITY_HOST_IMAGE_COPY)
          ? "yes"
          : "no");
  aperture_log_infof("  Supports(UnifiedImageLayouts)=%s",
                     aperture_adapter_supports(
                         adapter, APERTURE_CAPABILITY_UNIFIED_IMAGE_LAYOUTS)
                         ? "yes"
                         : "no");
}

static void log_present_modes(const ApertureAdapter* adapter) {
  aperture_log_infof(
      "supports fifo: %s",
      aperture_adapter_supports_present(adapter, APERTURE_PRESENT_MODE_FIFO)
          ? "yes"
          : "no");
  aperture_log_infof("supports immediate: %s",
                     aperture_adapter_supports_present(
                         adapter, APERTURE_PRESENT_MODE_IMMEDIATE)
                         ? "yes"
                         : "no");
  aperture_log_infof(
      "supports mailbox: %s",
      aperture_adapter_supports_present(adapter, APERTURE_PRESENT_MODE_MAILBOX)
          ? "yes"
          : "no");
}

static void log_device_info(const ApertureDeviceInfo* info) {
  aperture_log_infof(
      "device: profile=%s texture_slots=%u sampler_slots=%u push_data=%u "
      "heap_align=%llu timestamp_period_ns=%g copy_gran=%u/%u/%u",
      aperture_addressing_profile_to_string(info->profile),
      info->texture_heap_slots, info->sampler_heap_slots,
      info->max_cpu_root_bytes,
      (unsigned long long)info->texture_heap_alignment,
      (double)info->timestamp_period_ns, info->copy_texture_granularity.x,
      info->copy_texture_granularity.y, info->copy_texture_granularity.z);
  aperture_log_infof(
      "  descriptor_stride tex/samp=%u/%u null_slots tex/samp=%u/%u",
      info->texture_descriptor_stride, info->sampler_descriptor_stride,
      info->null_texture_slot, info->null_sampler_slot);
  aperture_log_infof("  timestamps g/c/c=%s/%s/%s",
                     info->graphics_timestamps ? "yes" : "no",
                     info->compute_timestamps ? "yes" : "no",
                     info->copy_timestamps ? "yes" : "no");
  aperture_log_infof("  heap_device tex=0x%llx samp=0x%llx",
                     (unsigned long long)info->texture_heap_device.addr,
                     (unsigned long long)info->sampler_heap_device.addr);
}

static void log_shader_abi(ApertureBackend backend) {
  aperture_log_infof(
      "native shader format: %s",
      aperture_shader_format_to_string(aperture_native_shader_format(backend)));
  aperture_log_infof(
      "spirv compatible: %s",
      aperture_shader_format_compatible(APERTURE_SHADER_FORMAT_SPIRV, backend)
          ? "yes"
          : "no");
  aperture_log_infof(
      "texture_slot Null sampler_slot LinearClamp=%u FirstUser=%u",
      (uint32_t)APERTURE_SAMPLER_SLOT_LINEAR_CLAMP,
      (uint32_t)APERTURE_SAMPLER_SLOT_FIRST_USER);
}

int main() {
  aperture_log_info(aperture_version_string());
  log_version();
  if (!aperture_compatible_with_headers()) {
    aperture_log_error("Header/linked version mismatch!");
    return 1;
  }

  ApertureExamplesCWindowDesc window_desc = {
      .title = "aperture basic (C)",
      .width = 1280,
      .height = 720,
      .resizable = true,
      .visible = true,
  };
  ApertureExamplesCWindow window = {0};
  ApertureError error =
      aperture_examples_c_create_window(&window_desc, &window);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("Create window failed (%s)!",
                        aperture_error_to_string(error));
    return 1;
  }

  aperture_examples_c_poll_window_events();
  uint32_t client_w = 0;
  uint32_t client_h = 0;
  uint32_t fb_w = 0;
  uint32_t fb_h = 0;
  aperture_examples_c_window_size(window, &client_w, &client_h);
  aperture_examples_c_window_framebuffer_size(window, &fb_w, &fb_h);
  aperture_log_infof("window client=%ux%u framebuffer=%ux%u", client_w,
                     client_h, fb_w, fb_h);

  ApertureSurface surface = {0};
  error = aperture_examples_c_window_surface(window, &surface);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("Window surface failed (%s)!",
                        aperture_error_to_string(error));
    aperture_examples_c_destroy_window(window);
    return 1;
  }

  ApertureBackend preferred = 0;
  error = aperture_preferred_backend(&preferred);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("PreferredBackend failed (%s)!",
                        aperture_error_to_string(error));
    aperture_examples_c_destroy_window(window);
    return 1;
  }

  const ApertureBackend* compiled = NULL;
  size_t compiled_count = 0;
  aperture_compiled_backends(&compiled, &compiled_count);
  for (size_t i = 0; i < compiled_count; ++i) {
    aperture_log_infof("compiled backend: %s available=%s",
                       aperture_backend_to_string(compiled[i]),
                       aperture_available(compiled[i]) ? "yes" : "no");
  }

  const ApertureBackend* available = NULL;
  size_t available_count = 0;
  aperture_available_backends(&available, &available_count);
  aperture_log_infof("available backends: %zu", available_count);
  aperture_log_infof("preferred backend: %s (available=%s)",
                     aperture_backend_to_string(preferred),
                     aperture_available(preferred) ? "yes" : "no");

  ApertureInstanceDesc instance_desc = aperture_instance_desc();
  instance_desc.enable_validation = true;
  instance_desc.validation_fatal = true;

  ApertureInstance instance = NULL;
  error = aperture_create_instance_for(preferred, &instance_desc, &instance);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("CreateInstance failed (%s)!",
                        aperture_error_to_string(error));
    aperture_examples_c_destroy_window(window);
    return 1;
  }

  aperture_log_infof(
      "instance backend: %s",
      aperture_backend_to_string(aperture_backend_of_instance(instance)));

  const size_t adapter_count = aperture_adapter_count(instance);
  if (adapter_count == 0) {
    aperture_log_error("No adapters!");
    aperture_destroy_instance(instance);
    aperture_examples_c_destroy_window(window);
    return 1;
  }

  int chosen = -1;
  for (size_t i = 0; i < adapter_count; ++i) {
    const ApertureAdapter adapter = aperture_adapter(instance, (uint32_t)i);
    aperture_log_infof(
        "adapter %u: %.*s%s", adapter.index, (int)adapter.name_size,
        adapter.name,
        adapter.conformant ? " [conformant]" : " [non-conformant]");
    if (!adapter.conformant && adapter.conformance_reason_size > 0) {
      aperture_log_infof("  reason: %.*s", (int)adapter.conformance_reason_size,
                         adapter.conformance_reason);
    }
    aperture_log_infof(
        "  discrete=%s local_memory=%llu shared_memory=%llu queues "
        "g/c/c=%u/%u/%u",
        adapter.discrete ? "yes" : "no",
        (unsigned long long)adapter.local_memory_bytes,
        (unsigned long long)adapter.shared_memory_bytes,
        adapter.graphics_queue_count, adapter.compute_queue_count,
        adapter.copy_queue_count);
    aperture_log_infof("  texture_slots=%u sampler_slots=%u max_tex_2d=%u",
                       adapter.max_texture_heap_slots,
                       adapter.max_sampler_heap_slots,
                       adapter.max_texture_dimension_2d);
    log_adapter_gpu_fields(&adapter);
    log_adapter_capabilities(&adapter);
    if (chosen < 0 && adapter.conformant) {
      chosen = (int)i;
    }
  }

  if (chosen < 0) {
    aperture_log_error("No conformant adapter!");
    aperture_destroy_instance(instance);
    aperture_examples_c_destroy_window(window);
    return 1;
  }

  const ApertureAdapter adapter = aperture_adapter(instance, (uint32_t)chosen);

  const ApertureTextureFormat* formats = NULL;
  size_t format_count = 0;
  aperture_presentable_formats(&adapter, &surface, &formats, &format_count);
  aperture_log_infof("presentable formats (%zu):", format_count);
  for (size_t i = 0; i < format_count; ++i) {
    aperture_log_infof("  %s", aperture_texture_format_to_string(formats[i]));
  }
  log_present_modes(&adapter);
  aperture_log_infof("supports R8G8B8A8Unorm color+sample: %s",
                     aperture_supports_format(
                         &adapter, APERTURE_TEXTURE_FORMAT_R8G8B8A8_UNORM,
                         (ApertureFormatUsage)(APERTURE_FORMAT_USAGE_COLOR |
                                               APERTURE_FORMAT_USAGE_SAMPLE))
                         ? "yes"
                         : "no");

  ApertureCapability requested = APERTURE_CAPABILITY_NONE;
  if (aperture_adapter_supports(&adapter, APERTURE_CAPABILITY_ASYNC_COMPUTE)) {
    requested =
        (ApertureCapability)(requested | APERTURE_CAPABILITY_ASYNC_COMPUTE);
  }
  if (aperture_adapter_supports(&adapter, APERTURE_CAPABILITY_ASYNC_COPY)) {
    requested =
        (ApertureCapability)(requested | APERTURE_CAPABILITY_ASYNC_COPY);
  }

  uint32_t texture_slots = adapter.max_texture_heap_slots;
  if (texture_slots > 1024U) {
    texture_slots = 1024U;
  }
  uint32_t sampler_slots = adapter.max_sampler_heap_slots;
  if (sampler_slots > 32U) {
    sampler_slots = 32U;
  }

  ApertureDeviceDesc device_desc = {
      .capabilities = requested,
      .adapter = adapter.index,
      .texture_heap_slots = texture_slots,
      .sampler_heap_slots = sampler_slots,
      .pipeline_policy = APERTURE_PIPELINE_POLICY_FAIL_ON_MISS,
  };

  ApertureDevice device = NULL;
  ApertureDualPtr numbers = {0};
  ApertureDualPtr readback = {0};
  ApertureGpuPtr gpu_only = {0};
  ApertureGpuPtr gpu_dedicated = {0};
  ApertureDualPtr bump_storage = {0};
  ApertureBumpAllocator bump = NULL;
  ApertureOffsetAllocator offsets = NULL;
  ApertureDualPtr dedicated = {0};
  ApertureBumpAllocator dedicated_bump = NULL;

  error = aperture_create_device(instance, &device_desc, &device);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("CreateDevice failed (%s)!",
                        aperture_error_to_string(error));
    aperture_destroy_instance(instance);
    aperture_examples_c_destroy_window(window);
    return 1;
  }

  aperture_log_infof(
      "device backend: %s",
      aperture_backend_to_string(aperture_backend_of_device(device)));
  const ApertureDeviceInfo info = aperture_device_info(device);
  log_device_info(&info);
  log_shader_abi(aperture_backend_of_device(device));
  aperture_log_infof(
      "Has(MeshShading)=%s",
      aperture_device_has(device, APERTURE_CAPABILITY_MESH_SHADING) ? "yes"
                                                                    : "no");
  aperture_log_infof(
      "Has(AsyncCompute)=%s Has(AsyncCopy)=%s",
      aperture_device_has(device, APERTURE_CAPABILITY_ASYNC_COMPUTE) ? "yes"
                                                                     : "no",
      aperture_device_has(device, APERTURE_CAPABILITY_ASYNC_COPY) ? "yes"
                                                                  : "no");

  const ApertureQueue graphics = aperture_graphics_queue(device);
  aperture_log_infof("graphics queue: %s", graphics != NULL ? "yes" : "no");

  ApertureQueue compute = NULL;
  error = aperture_compute_queue(device, &compute);
  if (error == APERTURE_ERROR_OK) {
    aperture_log_info("compute queue: yes");
  } else {
    aperture_log_infof("compute queue: no (%s)",
                       aperture_error_to_string(error));
  }

  ApertureQueue copy = NULL;
  error = aperture_copy_queue(device, &copy);
  if (error == APERTURE_ERROR_OK) {
    aperture_log_info("copy queue: yes");
  } else {
    aperture_log_infof("copy queue: no (%s)", aperture_error_to_string(error));
  }

  ApertureQueueUsage shared_usage = APERTURE_QUEUE_USAGE_GRAPHICS;
  if (compute != NULL) {
    shared_usage =
        (ApertureQueueUsage)(shared_usage | APERTURE_QUEUE_USAGE_COMPUTE);
  }

  error = aperture_malloc(device, 1024U * sizeof(uint32_t), alignof(uint32_t),
                          APERTURE_MEMORY_DEFAULT, shared_usage, &numbers);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("Malloc failed (%s)!", aperture_error_to_string(error));
    goto cleanup;
  }

  uint32_t* host_numbers = (uint32_t*)numbers.host;
  host_numbers[0] = 42;
  host_numbers[1023] = 7;
  const ApertureGpuPtr looked_up =
      aperture_device_address_of(device, numbers.host);
  aperture_log_infof("malloc count=1024 host[0]=%u gpu=0x%llx lookup=0x%llx",
                     host_numbers[0], (unsigned long long)numbers.device.addr,
                     (unsigned long long)looked_up.addr);
  if (looked_up.addr != numbers.device.addr) {
    aperture_log_error("DeviceAddressOf mismatch!");
    error = APERTURE_ERROR_INVALID;
    goto cleanup;
  }

  const ApertureGpuRange number_range =
      aperture_gpu_range_from_dual(numbers, 16U, sizeof(uint32_t));
  aperture_log_infof("gpu_range_from count=16 bytes=%llu",
                     (unsigned long long)number_range.size);
  const ApertureDualRange dual_range =
      aperture_dual_range_from(numbers, 4U, sizeof(uint32_t));
  ApertureDualRange tail = dual_range;
  tail.ptr.host = (uint8_t*)tail.ptr.host + sizeof(uint32_t);
  tail.ptr.device.addr += sizeof(uint32_t);
  tail.size = sizeof(uint32_t);
  const ApertureGpuRange tail_gpu = aperture_dual_range_device(tail);
  aperture_log_infof("dual_range slice device=0x%llx bytes=%llu",
                     (unsigned long long)tail_gpu.gpu.addr,
                     (unsigned long long)tail_gpu.size);

  error = aperture_malloc(device, 16U, alignof(uint8_t),
                          APERTURE_MEMORY_READBACK, shared_usage, &readback);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("Malloc Readback failed (%s)!",
                        aperture_error_to_string(error));
    goto cleanup;
  }
  ((uint8_t*)readback.host)[0] = 0xAB;
  ((uint8_t*)readback.host)[15] = 0xCD;
  aperture_log_infof("readback host[0]=0x%02x host[15]=0x%02x",
                     ((uint8_t*)readback.host)[0],
                     ((uint8_t*)readback.host)[15]);

  error = aperture_malloc_gpu(device, 256U * sizeof(uint32_t),
                              alignof(uint32_t), shared_usage, &gpu_only);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("MallocGpu failed (%s)!",
                        aperture_error_to_string(error));
    goto cleanup;
  }
  aperture_log_infof("malloc_gpu count=256 gpu=0x%llx",
                     (unsigned long long)gpu_only.addr);

  error = aperture_malloc_gpu_dedicated(device, 64U * sizeof(uint32_t),
                                        alignof(uint32_t), shared_usage,
                                        &gpu_dedicated);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("MallocGpuDedicated failed (%s)!",
                        aperture_error_to_string(error));
    goto cleanup;
  }
  aperture_log_infof("malloc_gpu_dedicated count=64 gpu=0x%llx",
                     (unsigned long long)gpu_dedicated.addr);

  const size_t bump_bytes = 64U * 1024U;
  error = aperture_malloc(device, bump_bytes, 16, APERTURE_MEMORY_DEFAULT,
                          shared_usage, &bump_storage);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("Bump storage Malloc failed (%s)!",
                        aperture_error_to_string(error));
    goto cleanup;
  }

  bump = aperture_bump_allocator_create(bump_bytes);
  if (bump == NULL) {
    aperture_log_error("BumpAllocator create failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  ApertureBumpAllocation tmp_span = aperture_bump_allocator_allocate(
      bump, 16U * sizeof(float), alignof(float));
  if (!aperture_bump_allocation_ok(tmp_span)) {
    aperture_log_error("BumpAllocator Allocate failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  float* floats =
      (float*)((uint8_t*)bump_storage.host + (size_t)tmp_span.offset);
  floats[0] = 1.0F;
  floats[15] = 2.0F;
  aperture_log_infof(
      "bump alloc count=16 host[0]=%g gpu=0x%llx used=%llu free=%llu",
      (double)floats[0],
      (unsigned long long)(bump_storage.device.addr + tmp_span.offset),
      (unsigned long long)aperture_bump_allocator_used(bump),
      (unsigned long long)aperture_bump_allocator_free_bytes(bump));

  aperture_bump_allocator_reset(bump);
  ApertureBumpAllocation tmp2_span = aperture_bump_allocator_allocate(
      bump, 16U * sizeof(float), alignof(float));
  if (!aperture_bump_allocation_ok(tmp2_span)) {
    aperture_log_error("BumpAllocator Allocate after Reset failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  aperture_log_infof(
      "bump reset+alloc gpu=0x%llx empty=%s",
      (unsigned long long)(bump_storage.device.addr + tmp2_span.offset),
      aperture_bump_allocator_empty(bump) ? "yes" : "no");

  offsets = aperture_offset_allocator_create(
      (uint32_t)bump_bytes, APERTURE_OFFSET_ALLOCATOR_DEFAULT_MAX_ALLOCS);
  if (offsets == NULL) {
    aperture_log_error("OffsetAllocator create failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  const ApertureOffsetAllocation offset_a =
      aperture_offset_allocator_allocate(offsets, 128U);
  const ApertureOffsetAllocation offset_b =
      aperture_offset_allocator_allocate(offsets, 256U);
  if (!aperture_offset_allocation_ok(offset_a) ||
      !aperture_offset_allocation_ok(offset_b)) {
    aperture_log_error("OffsetAllocator Allocate failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  aperture_offset_allocator_free(offsets, offset_a);
  const ApertureOffsetStorageReport offset_report =
      aperture_offset_allocator_report(offsets);
  aperture_log_infof("offset_allocator free=%u largest=%u",
                     offset_report.free_bytes,
                     offset_report.largest_free_region);

  const size_t dedicated_bytes = 4U * 1024U;
  error = aperture_malloc_dedicated(device, dedicated_bytes, 16,
                                    APERTURE_MEMORY_DEFAULT, shared_usage,
                                    &dedicated);
  if (error != APERTURE_ERROR_OK) {
    aperture_log_errorf("MallocDedicated failed (%s)!",
                        aperture_error_to_string(error));
    goto cleanup;
  }

  dedicated_bump = aperture_bump_allocator_create(dedicated_bytes);
  if (dedicated_bump == NULL) {
    aperture_log_error("BumpAllocator over MallocDedicated create failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  ApertureBumpAllocation dedicated_span = aperture_bump_allocator_allocate(
      dedicated_bump, 8U * sizeof(uint32_t), alignof(uint32_t));
  if (!aperture_bump_allocation_ok(dedicated_span)) {
    aperture_log_error("BumpAllocator over MallocDedicated failed!");
    error = APERTURE_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  uint32_t* words =
      (uint32_t*)((uint8_t*)dedicated.host + (size_t)dedicated_span.offset);
  words[0] = 11;
  words[7] = 99;
  aperture_log_infof(
      "malloc_dedicated bytes=%zu host[0]=%u gpu=0x%llx", dedicated_bytes,
      words[0],
      (unsigned long long)(dedicated.device.addr + dedicated_span.offset));

#ifdef APERTURE_HAS_VULKAN
  if (aperture_backend_of_instance(instance) == APERTURE_BACKEND_VULKAN) {
    const ApertureVkBuffer buf = aperture_vk_buffer(device, numbers.device);
    aperture_log_infof("vk_buffer offset=%llu size=%llu addr=0x%llx",
                       (unsigned long long)buf.offset,
                       (unsigned long long)buf.size,
                       (unsigned long long)buf.address);
  }
#endif

  error = APERTURE_ERROR_OK;

cleanup:
  if (dedicated_bump != NULL) {
    aperture_bump_allocator_destroy(dedicated_bump);
  }
  if (dedicated.host != NULL) {
    aperture_free(device, dedicated);
  }
  if (offsets != NULL) {
    aperture_offset_allocator_destroy(offsets);
  }
  if (bump != NULL) {
    aperture_bump_allocator_destroy(bump);
  }
  if (bump_storage.host != NULL) {
    aperture_free(device, bump_storage);
  }
  if (gpu_dedicated.addr != 0) {
    aperture_free_gpu(device, gpu_dedicated);
  }
  if (gpu_only.addr != 0) {
    aperture_free_gpu(device, gpu_only);
  }
  if (readback.host != NULL) {
    aperture_free(device, readback);
  }
  if (numbers.host != NULL) {
    aperture_free(device, numbers);
  }
  if (device != NULL) {
    aperture_destroy_device(device);
  }
  if (instance != NULL) {
    aperture_destroy_instance(instance);
  }
  aperture_examples_c_destroy_window(window);

  return error == APERTURE_ERROR_OK ? 0 : 1;
}
