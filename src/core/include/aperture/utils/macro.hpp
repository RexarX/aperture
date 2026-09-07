#pragma once

// Stringify macros
#define APERTURE_STRINGIFY_IMPL(x) #x
#define APERTURE_STRINGIFY(x) APERTURE_STRINGIFY_IMPL(x)

// Concatenation macros
#define APERTURE_CONCAT_IMPL(a, b) a##b
#define APERTURE_CONCAT(a, b) APERTURE_CONCAT_IMPL(a, b)

// Anonymous variable generation
#define APERTURE_ANONYMOUS_VAR(prefix) APERTURE_CONCAT(prefix, __LINE__)
