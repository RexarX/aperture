#include <aperture/instance.h>

#include <aperture/result.h>
#include <aperture/types.h>
#include <aperture/assert.hpp>
#include <aperture/instance.hpp>
#include <aperture/types.hpp>

#include "convert.hpp"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <span>

extern "C" {

void aperture_compiled_backends(const ApertureBackend** data,
                                size_t* size) noexcept {
  APERTURE_ASSERT(data != nullptr);
  APERTURE_ASSERT(size != nullptr);
  const std::span<const aperture::Backend> backends =
      aperture::CompiledBackends();
  *data = reinterpret_cast<const ApertureBackend*>(backends.data());
  *size = backends.size();
}

void aperture_available_backends(const ApertureBackend** data,
                                 size_t* size) noexcept {
  APERTURE_ASSERT(data != nullptr);
  APERTURE_ASSERT(size != nullptr);
  const std::span<const aperture::Backend> backends =
      aperture::AvailableBackends();
  *data = reinterpret_cast<const ApertureBackend*>(backends.data());
  *size = backends.size();
}

bool aperture_available(ApertureBackend backend) noexcept {
  return aperture::Available(aperture::cbind::ToCppBackend(backend));
}

ApertureError aperture_preferred_backend(ApertureBackend* out) noexcept {
  APERTURE_ASSERT(out != nullptr);
  auto result = aperture::PreferredBackend();
  if (!result) [[unlikely]] {
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToCBackend(*result);
  return APERTURE_ERROR_OK;
}

ApertureBackend aperture_backend_of_instance(
    ApertureInstance instance) noexcept {
  return aperture::cbind::ToCBackend(
      aperture::BackendOf(aperture::cbind::ToCpp(instance)));
}

ApertureError aperture_create_instance(const ApertureInstanceDesc* desc,
                                       ApertureInstance* out) noexcept {
  APERTURE_ASSERT(desc != nullptr);
  APERTURE_ASSERT(out != nullptr);
  *out = nullptr;
  auto result = aperture::CreateInstance(aperture::cbind::ToCpp(*desc));
  if (!result) [[unlikely]] {
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}

ApertureError aperture_create_instance_for(ApertureBackend backend,
                                           const ApertureInstanceDesc* desc,
                                           ApertureInstance* out) noexcept {
  APERTURE_ASSERT(desc != nullptr);
  APERTURE_ASSERT(out != nullptr);
  *out = nullptr;
  auto result = aperture::CreateInstance(aperture::cbind::ToCppBackend(backend),
                                         aperture::cbind::ToCpp(*desc));
  if (!result) [[unlikely]] {
    return aperture::cbind::ToCError(result.error());
  }
  *out = aperture::cbind::ToC(*result);
  return APERTURE_ERROR_OK;
}

void aperture_destroy_instance(ApertureInstance instance) noexcept {
  aperture::Destroy(aperture::cbind::ToCpp(instance));
}

size_t aperture_adapter_count(ApertureInstance instance) noexcept {
  return aperture::Adapters(aperture::cbind::ToCpp(instance)).size();
}

ApertureAdapter aperture_adapter(ApertureInstance instance,
                                 uint32_t index) noexcept {
  const auto adapters = aperture::Adapters(aperture::cbind::ToCpp(instance));
  APERTURE_ASSERT(index < adapters.size());
  return aperture::cbind::ToC(adapters[index]);
}
}
