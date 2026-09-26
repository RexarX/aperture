#include <aperture/vulkan/instance.h>

#include <aperture/instance.h>
#include <aperture/result.h>
#include <aperture/assert.hpp>
#include <aperture/instance.hpp>
#include <aperture/result.hpp>
#include <aperture/vulkan/instance.hpp>

#include "convert.hpp"

#include <cstdbool>
#include <cstddef>

extern "C" {

ApertureError aperture_vk_create_instance(
    const ApertureInstanceDesc* desc, const ApertureVkInstanceExtras* extras,
    ApertureInstance* out) noexcept {
  APERTURE_ASSERT(desc != nullptr);
  APERTURE_ASSERT(out != nullptr);
  *out = nullptr;

  aperture::Result<aperture::vk::Instance*> created;
  if (extras == nullptr) {
    created = aperture::vk::CreateInstance(aperture::cbind::ToCpp(*desc));
  } else {
    const aperture::vk::InstanceExtras cpp_extras{
        .layers = {extras->layers, extras->layer_count},
        .extensions = {extras->extensions, extras->extension_count},
    };
    created =
        aperture::vk::CreateInstance(aperture::cbind::ToCpp(*desc), cpp_extras);
  }
  if (!created) [[unlikely]] {
    return aperture::cbind::ToCError(created.error());
  }
  *out = aperture::cbind::ToC(aperture::Instance{.ptr = *created});
  return APERTURE_ERROR_OK;
}

VkInstance aperture_vk_instance(ApertureInstance instance) noexcept {
  return aperture::vk::GetNative(aperture::cbind::ToCpp(instance)).instance;
}

void aperture_vk_instance_extensions(const VkExtensionProperties** data,
                                     size_t* size) noexcept {
  APERTURE_ASSERT(data != nullptr);
  APERTURE_ASSERT(size != nullptr);
  const auto extensions = aperture::vk::InstanceExtensions();
  *data = extensions.data();
  *size = extensions.size();
}

void aperture_vk_instance_layers(const VkLayerProperties** data,
                                 size_t* size) noexcept {
  APERTURE_ASSERT(data != nullptr);
  APERTURE_ASSERT(size != nullptr);
  const auto layers = aperture::vk::InstanceLayers();
  *data = layers.data();
  *size = layers.size();
}

bool aperture_vk_has_instance_extension(const char* name) noexcept {
  APERTURE_ASSERT(name != nullptr);
  return aperture::vk::HasInstanceExtension(name);
}
}
