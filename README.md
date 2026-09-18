# aperture

Thin render interface, following the No Graphics API philosophy.

A LunarG [Vulkan SDK](https://vulkan.lunarg.com/) is used when `VULKAN_SDK` is
set. If the SDK is missing or disabled, dependencies are taken from
`find_package` or fetched with CPM.
