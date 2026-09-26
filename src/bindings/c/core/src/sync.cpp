#include <aperture/sync.h>

#include <aperture/command/buffer.h>
#include <aperture/device.h>
#include <aperture/queue.h>
#include <aperture/result.h>
#include <aperture/assert.hpp>
#include <aperture/command/buffer.hpp>
#include <aperture/sync.hpp>

#include "convert.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" {

ApertureError aperture_create_timeline(ApertureDevice device, uint64_t initial,
                                       ApertureTimeline* out) noexcept {
  APERTURE_ASSERT(out != nullptr);

  auto created =
      aperture::CreateTimeline(aperture::cbind::ToCpp(device), initial);
  if (!created) [[unlikely]] {
    *out = nullptr;
    return aperture::cbind::ToCError(created.error());
  }
  *out = static_cast<ApertureTimeline>(created->ptr);
  return APERTURE_ERROR_OK;
}

void aperture_wait_timeline(ApertureTimeline timeline,
                            uint64_t value) noexcept {
  aperture::Wait(aperture::Timeline{.ptr = timeline}, value);
}

ApertureError aperture_wait_timeline_timeout(ApertureTimeline timeline,
                                             uint64_t value,
                                             uint64_t timeout_ns) noexcept {
  auto waited =
      aperture::Wait(aperture::Timeline{.ptr = timeline}, value, timeout_ns);
  if (!waited) [[unlikely]] {
    return aperture::cbind::ToCError(waited.error());
  }
  return APERTURE_ERROR_OK;
}

uint64_t aperture_timeline_current_value(ApertureTimeline timeline) noexcept {
  return aperture::CurrentValue(aperture::Timeline{.ptr = timeline});
}

void aperture_destroy_timeline(ApertureTimeline timeline) noexcept {
  aperture::Destroy(aperture::Timeline{.ptr = timeline});
}

ApertureError aperture_submit(ApertureQueue queue,
                              const ApertureSubmitDesc* desc) noexcept {
  APERTURE_ASSERT(desc != nullptr);
  APERTURE_ASSERT(desc->buffer_count == 0 || desc->buffers != nullptr);
  APERTURE_ASSERT(desc->wait_count == 0 || desc->waits != nullptr);
  APERTURE_ASSERT(desc->signal_count == 0 || desc->signals != nullptr);

  std::vector<aperture::CommandBuffer> buffers(desc->buffer_count);
  for (size_t index = 0; index < desc->buffer_count; ++index) {
    buffers[index].ptr = desc->buffers[index];
  }

  std::vector<aperture::TimelineWait> waits(desc->wait_count);
  for (size_t index = 0; index < desc->wait_count; ++index) {
    waits[index] = aperture::TimelineWait{
        .timeline = {.ptr = desc->waits[index].timeline},
        .value = desc->waits[index].value,
        .wait_before =
            static_cast<aperture::Stage>(desc->waits[index].wait_before),
    };
  }

  std::vector<aperture::TimelineSignal> signals(desc->signal_count);
  for (size_t index = 0; index < desc->signal_count; ++index) {
    signals[index] = aperture::TimelineSignal{
        .timeline = {.ptr = desc->signals[index].timeline},
        .value = desc->signals[index].value,
    };
  }

  const aperture::SubmitDesc submit{
      .buffers = buffers,
      .waits = waits,
      .signals = signals,
  };
  auto submitted = aperture::Submit(aperture::cbind::ToCpp(queue), submit);
  for (size_t index = 0; index < desc->buffer_count; ++index) {
    desc->buffers[index] =
        static_cast<ApertureCommandBuffer>(buffers[index].ptr);
  }
  if (!submitted) [[unlikely]] {
    return aperture::cbind::ToCError(submitted.error());
  }
  return APERTURE_ERROR_OK;
}
}
