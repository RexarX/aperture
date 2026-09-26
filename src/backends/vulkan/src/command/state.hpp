#pragma once

#include <aperture/assert.hpp>
#include <aperture/vulkan/command/pool.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/sync.hpp>

#include <aperture/vulkan/header.hpp>

#include <atomic>
#include <cstdint>
#include <vector>

namespace aperture::vk {

struct CommandPoolState {
  std::vector<VkCommandBuffer> all;
  std::vector<VkCommandBuffer> reusable;
  VkQueryPool queries = VK_NULL_HANDLE;
  std::atomic<uint32_t> live{0};
#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
  std::atomic_flag busy;
#endif
};

struct CommandState {
  std::vector<CommandPool*> pools;
  std::vector<Timeline*> timelines;
#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
  std::atomic_flag submit_graphics;
  std::atomic_flag submit_compute;
  std::atomic_flag submit_copy;
#endif
};

void InitCommands(Device* device) noexcept;
void DestroyCommands(Device* device) noexcept;

#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
/// @brief Asserts if another op is already in progress on `state`.
/// @details Not a lock: release builds with validation OFF compile this away.
struct PoolGuard {
  CommandPoolState* state = nullptr;

  explicit PoolGuard(CommandPoolState* pool) noexcept : state(pool) {
    APERTURE_ASSERT(state != nullptr);
    const bool busy = state->busy.test_and_set(std::memory_order_acquire);
    APERTURE_ASSERT(!busy,
                    "Concurrent Begin/Reset/Destroy on one CommandPool!");
    if (busy) {
      state = nullptr;
    }
  }
  PoolGuard(const PoolGuard&) = delete;
  PoolGuard(PoolGuard&&) = delete;
  ~PoolGuard() noexcept {
    if (state != nullptr) {
      state->busy.clear(std::memory_order_release);
    }
  }

  PoolGuard& operator=(const PoolGuard&) = delete;
  PoolGuard& operator=(PoolGuard&&) = delete;
};

struct SubmitGuard {
  std::atomic_flag* flag = nullptr;

  explicit SubmitGuard(std::atomic_flag* submit) noexcept : flag(submit) {
    APERTURE_ASSERT(flag != nullptr);
    const bool busy = flag->test_and_set(std::memory_order_acquire);
    APERTURE_ASSERT(!busy, "Concurrent Submit on one Queue!");
    if (busy) {
      flag = nullptr;
    }
  }
  SubmitGuard(const SubmitGuard&) = delete;
  SubmitGuard(SubmitGuard&&) = delete;
  ~SubmitGuard() noexcept {
    if (flag != nullptr) {
      flag->clear(std::memory_order_release);
    }
  }

  SubmitGuard& operator=(const SubmitGuard&) = delete;
  SubmitGuard& operator=(SubmitGuard&&) = delete;
};
#else
struct PoolGuard {
  explicit PoolGuard(CommandPoolState*) noexcept {}
};

struct SubmitGuard {
  explicit SubmitGuard(std::atomic_flag*) noexcept {}
};
#endif

}  // namespace aperture::vk
