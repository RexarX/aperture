#include <pch.hpp>

#include <aperture/vulkan/command/pool.hpp>

#include <aperture/assert.hpp>
#include <aperture/command/pool.hpp>
#include <aperture/result.hpp>
#include <aperture/types.hpp>
#include <aperture/vulkan/device.hpp>
#include <aperture/vulkan/queue.hpp>
#include <aperture/vulkan/sync.hpp>

#include "command/state.hpp"
#include "internal.hpp"

#include <aperture/vulkan/header.hpp>

#include <atomic>
#include <expected>
#include <vector>

namespace aperture::vk {

namespace {

constexpr void Unregister(CommandState* commands, CommandPool* pool) noexcept {
  APERTURE_ASSERT(commands != nullptr);
  std::erase(commands->pools, pool);
}

}  // namespace

void InitCommands(Device* device) noexcept {
  APERTURE_ASSERT(device != nullptr);
  APERTURE_ASSERT(device->commands == nullptr);
  device->commands = new CommandState{};
}

void DestroyCommands(Device* device) noexcept {
  APERTURE_ASSERT(device != nullptr);
  CommandState* state = device->commands;
  if (state == nullptr) {
    return;
  }

  while (!state->pools.empty()) {
    Destroy(state->pools.back());
  }
  while (!state->timelines.empty()) {
    Destroy(state->timelines.back());
  }
  delete state;
  device->commands = nullptr;
}

auto CreateCommandPool(Queue* queue,
                       const aperture::CommandPoolDesc& desc) noexcept
    -> Result<CommandPool*> {
  APERTURE_ASSERT(queue != nullptr);
  APERTURE_ASSERT(queue->device != nullptr);
  APERTURE_ASSERT(queue->device->commands != nullptr);

  const bool log = queue->device->log_failed_results;

  auto* pool = new CommandPool{};
  pool->device = queue->device;
  pool->queue = queue;
  pool->state = new CommandPoolState{};
  pool->family = queue->family;
  pool->max_timestamps = desc.max_timestamps;
  pool->usage = queue->usage;

  const VkCommandPoolCreateInfo create{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = queue->family,
  };
  const VkResult created =
      vkCreateCommandPool(queue->device->device, &create, nullptr, &pool->pool);
  if (created != VK_SUCCESS) [[unlikely]] {
    delete pool->state;
    delete pool;
    LogFailed(log, "vkCreateCommandPool failed: {} ({})!", ToString(created),
              ToString(MapVkResult(created)));
    return std::unexpected(MapVkResult(created));
  }

  if (desc.max_timestamps > 0) {
    const VkQueryPoolCreateInfo queries{
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = desc.max_timestamps,
    };
    const VkResult query = vkCreateQueryPool(queue->device->device, &queries,
                                             nullptr, &pool->state->queries);
    if (query != VK_SUCCESS) [[unlikely]] {
      vkDestroyCommandPool(queue->device->device, pool->pool, nullptr);
      delete pool->state;
      delete pool;
      LogFailed(log, "vkCreateQueryPool failed: {} ({})!", ToString(query),
                ToString(MapVkResult(query)));
      return std::unexpected(MapVkResult(query));
    }
  }

  queue->device->commands->pools.push_back(pool);
  return pool;
}

void Reset(CommandPool* pool) noexcept {
  if (pool == nullptr) {
    return;
  }
  APERTURE_ASSERT(pool->state != nullptr);
  APERTURE_ASSERT(pool->device != nullptr);

  PoolGuard guard(pool->state);
  APERTURE_ASSERT(pool->state->live.load(std::memory_order_acquire) == 0,
                  "Reset while a command buffer is still recording!");
  vkResetCommandPool(pool->device->device, pool->pool, 0);
  if (pool->state->queries != VK_NULL_HANDLE) {
    vkResetQueryPool(pool->device->device, pool->state->queries, 0,
                     pool->max_timestamps);
  }
  pool->state->reusable = pool->state->all;
}

void Destroy(CommandPool* pool) noexcept {
  if (pool == nullptr) {
    return;
  }
  APERTURE_ASSERT(pool->state != nullptr);
  {
    PoolGuard guard(pool->state);
    APERTURE_ASSERT(pool->state->live.load(std::memory_order_acquire) == 0,
                    "Destroy while a command buffer is still recording!");
  }

  if (pool->device != nullptr && pool->device->commands != nullptr) {
    Unregister(pool->device->commands, pool);
  }
  if (pool->device != nullptr && pool->pool != VK_NULL_HANDLE) {
    if (pool->state->queries != VK_NULL_HANDLE) {
      vkDestroyQueryPool(pool->device->device, pool->state->queries, nullptr);
    }
    vkDestroyCommandPool(pool->device->device, pool->pool, nullptr);
  }
  delete pool->state;
  delete pool;
}

CommandPool& GetNative(aperture::CommandPool pool) noexcept {
  APERTURE_ASSERT(pool.ptr != nullptr);
  auto& impl = *static_cast<CommandPool*>(pool.ptr);
  APERTURE_ASSERT(impl.backend == Backend::Vulkan);
  return impl;
}

}  // namespace aperture::vk
