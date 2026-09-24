#pragma once

#include <aperture/adapter.h>
#include <aperture/device.h>
#include <aperture/instance.h>
#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/surface.h>
#include <aperture/types.h>
#include <aperture/version.h>

#include <aperture/adapter.hpp>
#include <aperture/capability.hpp>
#include <aperture/device.hpp>
#include <aperture/instance.hpp>
#include <aperture/pipeline.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/surface.hpp>
#include <aperture/types.hpp>
#include <aperture/version.hpp>

#include <cstring>

namespace aperture::cbind {

[[nodiscard]] constexpr Version ToCpp(ApertureVersion version) noexcept {
  return {
      .major = version.major,
      .minor = version.minor,
      .patch = version.patch,
  };
}

[[nodiscard]] constexpr ApertureVersion ToC(Version version) noexcept {
  return {
      .major = version.major,
      .minor = version.minor,
      .patch = version.patch,
  };
}

[[nodiscard]] constexpr Backend ToCppBackend(ApertureBackend backend) noexcept {
  return static_cast<Backend>(backend);
}

[[nodiscard]] constexpr ApertureBackend ToCBackend(Backend backend) noexcept {
  return static_cast<ApertureBackend>(backend);
}

[[nodiscard]] constexpr Error ToCppError(ApertureError error) noexcept {
  return static_cast<Error>(error);
}

[[nodiscard]] constexpr ApertureError ToCError(Error error) noexcept {
  return static_cast<ApertureError>(error);
}

[[nodiscard]] inline Instance ToCpp(ApertureInstance instance) noexcept {
  return {.ptr = instance};
}

[[nodiscard]] inline ApertureInstance ToC(Instance instance) noexcept {
  return static_cast<ApertureInstance>(instance.ptr);
}

[[nodiscard]] inline Device ToCpp(ApertureDevice device) noexcept {
  return {.ptr = device};
}

[[nodiscard]] inline ApertureDevice ToC(Device device) noexcept {
  return static_cast<ApertureDevice>(device.ptr);
}

[[nodiscard]] inline Queue ToCpp(ApertureQueue queue) noexcept {
  return {.ptr = queue};
}

[[nodiscard]] inline ApertureQueue ToC(Queue queue) noexcept {
  return static_cast<ApertureQueue>(queue.ptr);
}

[[nodiscard]] constexpr auto ToCpp(ApertureGpuPtr ptr) noexcept
    -> GpuPtr<std::byte> {
  return {.addr = ptr.addr};
}

[[nodiscard]] constexpr ApertureGpuPtr ToC(GpuPtr<std::byte> ptr) noexcept {
  return {.addr = ptr.addr};
}

[[nodiscard]] constexpr auto ToCpp(ApertureDualPtr ptr) noexcept
    -> DualPtr<std::byte> {
  return {
      .host = static_cast<std::byte*>(ptr.host),
      .device = {.addr = ptr.device.addr},
  };
}

[[nodiscard]] constexpr ApertureDualPtr ToC(DualPtr<std::byte> ptr) noexcept {
  return {
      .host = ptr.host,
      .device = {.addr = ptr.device.addr},
  };
}

[[nodiscard]] constexpr InstanceDesc ToCpp(
    const ApertureInstanceDesc& desc) noexcept {
  return {
      .header_version = ToCpp(desc.header_version),
      .log_failed_results = desc.log_failed_results,
      .enable_validation = desc.enable_validation,
      .validation_fatal = desc.validation_fatal,
  };
}

[[nodiscard]] constexpr DeviceDesc ToCpp(
    const ApertureDeviceDesc& desc) noexcept {
  return {
      .capabilities = static_cast<Capability>(desc.capabilities),
      .adapter = desc.adapter,
      .texture_heap_slots = desc.texture_heap_slots,
      .sampler_heap_slots = desc.sampler_heap_slots,
      .pipeline_policy = static_cast<PipelinePolicy>(desc.pipeline_policy),
  };
}

[[nodiscard]] constexpr ApertureDeviceInfo ToC(
    const DeviceInfo& info) noexcept {
  return {
      .texture_heap_device = ToC(info.texture_heap_device),
      .sampler_heap_device = ToC(info.sampler_heap_device),
      .texture_heap_alignment = info.texture_heap_alignment,
      .timestamps =
          {
              .period_ns = info.timestamps.period_ns,
              .graphics = info.timestamps.graphics,
              .compute = info.timestamps.compute,
              .copy = info.timestamps.copy,
          },
      .texture_descriptor_stride = info.texture_descriptor_stride,
      .sampler_descriptor_stride = info.sampler_descriptor_stride,
      .texture_heap_slots = info.texture_heap_slots,
      .sampler_heap_slots = info.sampler_heap_slots,
      .null_texture_slot = info.null_texture_slot,
      .null_sampler_slot = info.null_sampler_slot,
      .max_cpu_root_bytes = info.max_cpu_root_bytes,
      .copy_texture_granularity =
          {
              .x = info.copy_texture_granularity.x,
              .y = info.copy_texture_granularity.y,
              .z = info.copy_texture_granularity.z,
          },
      .profile = static_cast<ApertureAddressingProfile>(info.profile),
  };
}

[[nodiscard]] inline Surface ToCpp(const ApertureSurface& surface) noexcept {
  Surface out;
  switch (surface.platform_surface.kind) {
    case APERTURE_SURFACE_KIND_WIN32:
      out.platform_surface = Win32Surface{
          .hwnd = surface.platform_surface.u.win32.hwnd,
          .hinstance = surface.platform_surface.u.win32.hinstance,
      };
      break;
    case APERTURE_SURFACE_KIND_XLIB:
      out.platform_surface = XlibSurface{
          .display = surface.platform_surface.u.xlib.display,
          .window = surface.platform_surface.u.xlib.window,
      };
      break;
    case APERTURE_SURFACE_KIND_XCB:
      out.platform_surface = XcbSurface{
          .connection = surface.platform_surface.u.xcb.connection,
          .window = surface.platform_surface.u.xcb.window,
      };
      break;
    case APERTURE_SURFACE_KIND_WAYLAND:
      out.platform_surface = WaylandSurface{
          .display = surface.platform_surface.u.wayland.display,
          .surface = surface.platform_surface.u.wayland.surface,
      };
      break;
    case APERTURE_SURFACE_KIND_COCOA:
      out.platform_surface = CocoaSurface{
          .layer = surface.platform_surface.u.cocoa.layer,
      };
      break;
    case APERTURE_SURFACE_KIND_ANDROID:
      out.platform_surface = AndroidSurface{
          .window = surface.platform_surface.u.android.window,
      };
      break;
    default:
      out.platform_surface = Win32Surface{};
      break;
  }
  return out;
}

[[nodiscard]] inline ApertureAdapter ToC(const Adapter& adapter) noexcept {
  ApertureAdapter out{};
  out.impl = adapter.impl;
  out.local_memory_bytes = adapter.local_memory_bytes;
  out.shared_memory_bytes = adapter.shared_memory_bytes;
  out.mapped_default_capacity = adapter.mapped_default_capacity;
  out.texture_heap_alignment = adapter.texture_heap_alignment;
  out.name = adapter.name.c_str();
  out.conformance_reason = adapter.conformance_reason.c_str();
  out.name_size = adapter.name.size();
  out.conformance_reason_size = adapter.conformance_reason.size();
  out.index = adapter.index;
  out.vendor_id = adapter.vendor_id;
  out.device_id = adapter.device_id;
  out.driver_version = adapter.driver_version;
  out.graphics_queue_count = adapter.graphics_queue_count;
  out.compute_queue_count = adapter.compute_queue_count;
  out.copy_queue_count = adapter.copy_queue_count;
  out.texture_descriptor_stride = adapter.texture_descriptor_stride;
  out.sampler_descriptor_stride = adapter.sampler_descriptor_stride;
  out.max_texture_heap_slots = adapter.max_texture_heap_slots;
  out.max_sampler_heap_slots = adapter.max_sampler_heap_slots;
  out.max_texture_dimension_2d = adapter.max_texture_dimension_2d;
  out.subgroup_size = adapter.subgroup_size;
  out.timestamps.period_ns = adapter.timestamps.period_ns;
  out.timestamps.graphics = adapter.timestamps.graphics;
  out.timestamps.compute = adapter.timestamps.compute;
  out.timestamps.copy = adapter.timestamps.copy;
  out.copy_texture_granularity = {
      .x = adapter.copy_texture_granularity.x,
      .y = adapter.copy_texture_granularity.y,
      .z = adapter.copy_texture_granularity.z,
  };
  std::memcpy(out.uuid, adapter.uuid, sizeof(out.uuid));
  std::memcpy(out.luid, adapter.luid, sizeof(out.luid));
  out.capabilities = static_cast<ApertureCapability>(adapter.capabilities);
  out.present_modes = static_cast<AperturePresentMode>(adapter.present_modes);
  out.conformance = ToCError(adapter.conformance);
  out.discrete = adapter.discrete;
  out.luid_valid = adapter.luid_valid;
  return out;
}

}  // namespace aperture::cbind
