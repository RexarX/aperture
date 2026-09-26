#include <pch.hpp>

#include <aperture/vulkan/command/buffer.hpp>

#include <aperture/assert.hpp>
#include <aperture/capability.hpp>
#include <aperture/command/buffer.hpp>
#include <aperture/queue.hpp>
#include <aperture/result.hpp>
#include <aperture/sync.hpp>
#include <aperture/types.hpp>
#include <aperture/utils/flags.hpp>
#include <aperture/vulkan/queue.hpp>
#include <aperture/vulkan/sync.hpp>

#include "command/state.hpp"
#include "internal.hpp"
#include "memory/storage.hpp"

#include <aperture/vulkan/header.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>
#include <vector>

namespace aperture::vk {

namespace {

struct Scope {
  VkPipelineStageFlags2 stages = 0;
  VkAccessFlags2 access = 0;
};

constexpr Stage STAGE_ORDER[] = {
    Stage::Host,           Stage::Copy,   Stage::Compute,
    Stage::Indirect,       Stage::Vertex, Stage::Pixel,
    Stage::Mesh,           Stage::Task,   Stage::RasterColorOut,
    Stage::RasterDepthOut, Stage::Ray,
};
constexpr int STAGE_ORDER_COUNT =
    static_cast<int>(sizeof(STAGE_ORDER) / sizeof(STAGE_ORDER[0]));

constexpr VkAccessFlags2 SHADER_WRITE = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
constexpr VkAccessFlags2 SHADER_READ =
    VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

[[nodiscard]] constexpr bool StageEnabled(Stage stage,
                                          Capability enabled) noexcept {
  switch (stage) {
    using enum Stage;
    case Mesh:
    case Task:
      return HasAll(enabled, Capability::MeshShading);
    case Ray:
      return HasAll(enabled, Capability::RayTracingPipeline) ||
             HasAll(enabled, Capability::RayQuery);
    default:
      return true;
  }
}

constexpr void AddStage(Scope* scope, Stage stage, bool writes,
                        Capability enabled) noexcept {
  APERTURE_ASSERT(scope != nullptr);
  if (!StageEnabled(stage, enabled)) {
    return;
  }

  switch (stage) {
    using enum Stage;
    case Host:
      scope->stages |= VK_PIPELINE_STAGE_2_HOST_BIT;
      scope->access |=
          writes ? VK_ACCESS_2_HOST_WRITE_BIT : VK_ACCESS_2_HOST_READ_BIT;
      break;
    case Copy:
      scope->stages |= VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
      scope->access |= writes ? VK_ACCESS_2_TRANSFER_WRITE_BIT
                              : VK_ACCESS_2_TRANSFER_READ_BIT;
      break;
    case Compute:
      scope->stages |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
      scope->access |= writes ? SHADER_WRITE : SHADER_READ;
      break;
    case Indirect:
      scope->stages |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
      if (!writes) {
        scope->access |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
      }
      break;
    case Vertex:
      scope->stages |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
      scope->access |= writes ? SHADER_WRITE : SHADER_READ;
      break;
    case Pixel:
      scope->stages |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
      scope->access |= writes ? SHADER_WRITE : SHADER_READ;
      break;
    case Mesh:
#ifdef VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT
      scope->stages |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
      scope->access |= writes ? SHADER_WRITE : SHADER_READ;
#endif
      break;
    case Task:
#ifdef VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT
      scope->stages |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
      scope->access |= writes ? SHADER_WRITE : SHADER_READ;
#endif
      break;
    case RasterColorOut:
      scope->stages |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
      scope->access |= writes ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
                              : VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
      break;
    case RasterDepthOut:
      scope->stages |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                       VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
      scope->access |= writes ? VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
                              : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
      break;
    case Ray:
#ifdef VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR
      scope->stages |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
      scope->access |= writes ? SHADER_WRITE : SHADER_READ;
#endif
      break;
    case None:
    case All:
      break;
  }
}

[[nodiscard]] constexpr Stage ExpandSrc(Stage src) noexcept {
  if (HasAll(src, Stage::All)) {
    auto all = Stage::None;
    for (const Stage stage : STAGE_ORDER) {
      all |= stage;
    }
    return all;
  }

  const auto bits = std::to_underlying(src);
  int highest = -1;
  for (int index = 0; index < STAGE_ORDER_COUNT; ++index) {
    if ((bits & std::to_underlying(STAGE_ORDER[index])) != 0) {
      highest = index;
    }
  }

  auto out = Stage::None;
  for (int index = 0; index <= highest; ++index) {
    out |= STAGE_ORDER[index];
  }
  return out;
}

[[nodiscard]] constexpr Stage ExpandDst(Stage dst) noexcept {
  if (HasAll(dst, Stage::All)) {
    return ExpandSrc(Stage::All);
  }

  const auto bits = std::to_underlying(dst);
  int lowest = -1;
  for (int index = 0; index < STAGE_ORDER_COUNT; ++index) {
    if ((bits & std::to_underlying(STAGE_ORDER[index])) != 0) {
      lowest = index;
      break;
    }
  }

  auto out = Stage::None;
  if (lowest < 0) {
    return out;
  }
  for (int index = lowest; index < STAGE_ORDER_COUNT; ++index) {
    out |= STAGE_ORDER[index];
  }
  return out;
}

[[nodiscard]] constexpr Scope ScopeOf(Stage expanded, bool writes,
                                      Capability enabled) noexcept {
  Scope scope{};
  const auto bits = std::to_underlying(expanded);
  for (const Stage stage : STAGE_ORDER) {
    if ((bits & std::to_underlying(stage)) != 0) {
      AddStage(&scope, stage, writes, enabled);
    }
  }
  return scope;
}

[[nodiscard]] constexpr VkAccessFlags2 DescriptorReadAccess() noexcept {
#ifdef VK_ACCESS_2_DESCRIPTOR_HEAP_READ_BIT_EXT
  return VK_ACCESS_2_DESCRIPTOR_HEAP_READ_BIT_EXT;
#elifdef VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT
  return VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT;
#else
  return SHADER_READ;
#endif
}

constexpr void AddHazards(Scope* dst, Hazard hazards,
                          Capability enabled) noexcept {
  APERTURE_ASSERT(dst != nullptr);

  if (HasAll(hazards, Hazard::DrawArguments)) {
    dst->stages |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
    dst->access |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
  }
  if (HasAll(hazards, Hazard::IndexBuffer)) {
    dst->stages |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
    dst->access |= VK_ACCESS_2_INDEX_READ_BIT;
  }
  if (HasAll(hazards, Hazard::DepthStencil)) {
    dst->stages |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                   VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    dst->access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
  }
  if (HasAll(hazards, Hazard::Descriptors)) {
    dst->stages |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
                   VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                   VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    dst->access |= DescriptorReadAccess();
    if (HasAll(enabled, Capability::MeshShading)) {
      AddStage(dst, Stage::Mesh, false, enabled);
      AddStage(dst, Stage::Task, false, enabled);
    }
  }
}

[[nodiscard]] constexpr VkPipelineStageFlags2 WaitStages(
    Stage stage, Capability enabled) noexcept {
  if (HasAll(stage, Stage::All)) {
    return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  }
  return ScopeOf(stage, false, enabled).stages;
}

[[nodiscard]] constexpr bool Overlaps(uint64_t lhs, uint64_t lhs_size,
                                      uint64_t rhs,
                                      uint64_t rhs_size) noexcept {
  return lhs < rhs + rhs_size && rhs < lhs + lhs_size;
}

constexpr void ReleaseLive(CommandBuffer* buffer) noexcept {
  APERTURE_ASSERT(buffer != nullptr);
  APERTURE_ASSERT(buffer->pool != nullptr);
  APERTURE_ASSERT(buffer->pool->state != nullptr);

  const uint32_t previous =
      buffer->pool->state->live.fetch_sub(1, std::memory_order_acq_rel);
  APERTURE_ASSERT(previous > 0);
}

void Consume(aperture::CommandBuffer* token) noexcept {
  APERTURE_ASSERT(token != nullptr);
  APERTURE_ASSERT(token->ptr != nullptr);

  auto* impl = static_cast<CommandBuffer*>(token->ptr);
  impl->recording = false;
  ReleaseLive(impl);
  delete impl;
  token->ptr = nullptr;
}

}  // namespace

auto Begin(CommandPool* pool) noexcept -> Result<CommandBuffer*> {
  APERTURE_ASSERT(pool != nullptr);
  APERTURE_ASSERT(pool->state != nullptr);
  APERTURE_ASSERT(pool->device != nullptr);

  const bool log = pool->device->log_failed_results;
  PoolGuard guard(pool->state);

  VkCommandBuffer vk_buffer = VK_NULL_HANDLE;
  if (!pool->state->reusable.empty()) {
    vk_buffer = pool->state->reusable.back();
    pool->state->reusable.pop_back();
  } else {
    const VkCommandBufferAllocateInfo allocate{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool->pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    const VkResult allocated =
        vkAllocateCommandBuffers(pool->device->device, &allocate, &vk_buffer);
    if (allocated != VK_SUCCESS) [[unlikely]] {
      LogFailed(log, "vkAllocateCommandBuffers failed: {} ({})!",
                ToString(allocated), ToString(MapVkResult(allocated)));
      return std::unexpected(MapVkResult(allocated));
    }
    pool->state->all.push_back(vk_buffer);
  }

  const VkCommandBufferBeginInfo begin{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  const VkResult begun = vkBeginCommandBuffer(vk_buffer, &begin);
  if (begun != VK_SUCCESS) [[unlikely]] {
    pool->state->reusable.push_back(vk_buffer);
    LogFailed(log, "vkBeginCommandBuffer failed: {} ({})!", ToString(begun),
              ToString(MapVkResult(begun)));
    return std::unexpected(MapVkResult(begun));
  }

  auto* token = new CommandBuffer{
      .pool = pool,
      .buffer = vk_buffer,
      .recording = true,
  };
  pool->state->live.fetch_add(1, std::memory_order_acq_rel);
  return token;
}

void Barrier(CommandBuffer* buffer, Stage src, Stage dst,
             Hazard hazards) noexcept {
  APERTURE_ASSERT(buffer != nullptr);
  APERTURE_ASSERT(buffer->recording);
  APERTURE_ASSERT(buffer->pool != nullptr);
  APERTURE_ASSERT(buffer->pool->device != nullptr);
  APERTURE_ASSERT(src != Stage::None);
  APERTURE_ASSERT(dst != Stage::None);

  const Capability enabled = buffer->pool->device->enabled;
  Scope source = ScopeOf(ExpandSrc(src), true, enabled);
  Scope destination = ScopeOf(ExpandDst(dst), false, enabled);
  AddHazards(&destination, hazards, enabled);
  APERTURE_ASSERT(source.stages != 0);
  APERTURE_ASSERT(destination.stages != 0);

  const VkMemoryBarrier2 barrier{
      .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
      .srcStageMask = source.stages,
      .srcAccessMask = source.access,
      .dstStageMask = destination.stages,
      .dstAccessMask = destination.access,
  };
  const VkDependencyInfo dependency{
      .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .memoryBarrierCount = 1,
      .pMemoryBarriers = &barrier,
  };
  vkCmdPipelineBarrier2(buffer->buffer, &dependency);
}

void Copy(CommandBuffer* buffer, GpuRange dst, GpuRange src) noexcept {
  APERTURE_ASSERT(buffer != nullptr);
  APERTURE_ASSERT(buffer->recording);
  APERTURE_ASSERT(buffer->pool != nullptr);
  APERTURE_ASSERT(buffer->pool->device != nullptr);
  APERTURE_ASSERT(dst.size != 0);
  APERTURE_ASSERT(src.size != 0);

  const ResolvedRange dst_range = FindRange(buffer->pool->device, dst.gpu);
  const ResolvedRange src_range = FindRange(buffer->pool->device, src.gpu);
  APERTURE_ASSERT(HasAll(dst_range.usage, buffer->pool->usage),
                  "Copy destination is not visible to the pool queue!");
  APERTURE_ASSERT(HasAll(src_range.usage, buffer->pool->usage),
                  "Copy source is not visible to the pool queue!");
  APERTURE_ASSERT(dst.size <= dst_range.remaining);
  APERTURE_ASSERT(src.size <= src_range.remaining);
  if (dst_range.buffer == src_range.buffer) {
    APERTURE_ASSERT(
        !Overlaps(dst_range.offset, dst.size, src_range.offset, src.size),
        "Copy ranges overlap!");
  }

  const VkBufferCopy region{
      .srcOffset = src_range.offset,
      .dstOffset = dst_range.offset,
      .size = dst.size,
  };
  APERTURE_ASSERT(src.size == dst.size, "Copy ranges differ in size!");
  vkCmdCopyBuffer(buffer->buffer, src_range.buffer, dst_range.buffer, 1,
                  &region);
}

auto Submit(Queue* queue, const SubmitDesc& desc) noexcept -> Result<void> {
  APERTURE_ASSERT(queue != nullptr);
  APERTURE_ASSERT(queue->device != nullptr);
  APERTURE_ASSERT(queue->device->commands != nullptr);

  const bool log = queue->device->log_failed_results;

  if (desc.buffers.empty()) [[unlikely]] {
    LogFailed(log, "Submit with no command buffers ({})!",
              ToString(Error::Invalid));
    return std::unexpected(Error::Invalid);
  }

  for (const aperture::CommandBuffer& token : desc.buffers) {
    APERTURE_ASSERT(token.ptr != nullptr);
    const auto* impl = static_cast<const CommandBuffer*>(token.ptr);
    APERTURE_ASSERT(impl->backend == Backend::Vulkan);
    APERTURE_ASSERT(impl->recording);
    APERTURE_ASSERT(impl->pool != nullptr);
    if (impl->pool->queue != queue) [[unlikely]] {
      LogFailed(log, "Command buffer queue does not match Submit ({})!",
                ToString(Error::Invalid));
      return std::unexpected(Error::Invalid);
    }
  }

  for (const TimelineWait& wait : desc.waits) {
    APERTURE_ASSERT(wait.timeline.ptr != nullptr);
    const auto* timeline = static_cast<const Timeline*>(wait.timeline.ptr);
    if (timeline->device != queue->device ||
        WaitStages(wait.wait_before, queue->device->enabled) == 0)
        [[unlikely]] {
      LogFailed(log, "Timeline wait is invalid ({})!",
                ToString(Error::Invalid));
      return std::unexpected(Error::Invalid);
    }
  }
  for (const TimelineSignal& signal : desc.signals) {
    APERTURE_ASSERT(signal.timeline.ptr != nullptr);
    const auto* timeline = static_cast<const Timeline*>(signal.timeline.ptr);
    if (timeline->device != queue->device) [[unlikely]] {
      LogFailed(log, "Timeline signal is invalid ({})!",
                ToString(Error::Invalid));
      return std::unexpected(Error::Invalid);
    }
  }

#ifdef APERTURE_ENABLE_VALIDATION_SUPPORT
  std::atomic_flag* flag = &queue->device->commands->submit_graphics;
  if (queue->usage == QueueUsage::Compute) {
    flag = &queue->device->commands->submit_compute;
  } else if (queue->usage == QueueUsage::Copy) {
    flag = &queue->device->commands->submit_copy;
  }
  SubmitGuard guard(flag);
#else
  SubmitGuard guard(nullptr);
#endif

  for (aperture::CommandBuffer& token : desc.buffers) {
    auto* impl = static_cast<CommandBuffer*>(token.ptr);
    const VkResult ended = vkEndCommandBuffer(impl->buffer);
    if (ended != VK_SUCCESS) [[unlikely]] {
      LogFailed(log, "vkEndCommandBuffer failed: {} ({})!", ToString(ended),
                ToString(MapVkResult(ended)));
      return std::unexpected(MapVkResult(ended));
    }
    impl->recording = false;
  }

  std::vector<VkCommandBufferSubmitInfo> buffers(desc.buffers.size());
  for (size_t index = 0; index < desc.buffers.size(); ++index) {
    const auto* impl =
        static_cast<const CommandBuffer*>(desc.buffers[index].ptr);
    buffers[index] = VkCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = impl->buffer,
    };
  }

  std::vector<VkSemaphoreSubmitInfo> waits(desc.waits.size());
  for (size_t index = 0; index < desc.waits.size(); ++index) {
    const TimelineWait& wait = desc.waits[index];
    const auto* timeline = static_cast<const Timeline*>(wait.timeline.ptr);
    waits[index] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = timeline->semaphore,
        .value = wait.value,
        .stageMask = WaitStages(wait.wait_before, queue->device->enabled),
    };
  }

  std::vector<VkSemaphoreSubmitInfo> signals(desc.signals.size());
  for (size_t index = 0; index < desc.signals.size(); ++index) {
    const TimelineSignal& signal = desc.signals[index];
    const auto* timeline = static_cast<const Timeline*>(signal.timeline.ptr);
    signals[index] = VkSemaphoreSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = timeline->semaphore,
        .value = signal.value,
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
    };
  }

  const VkSubmitInfo2 submit{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
      .waitSemaphoreInfoCount = static_cast<uint32_t>(waits.size()),
      .pWaitSemaphoreInfos = waits.empty() ? nullptr : waits.data(),
      .commandBufferInfoCount = static_cast<uint32_t>(buffers.size()),
      .pCommandBufferInfos = buffers.data(),
      .signalSemaphoreInfoCount = static_cast<uint32_t>(signals.size()),
      .pSignalSemaphoreInfos = signals.empty() ? nullptr : signals.data(),
  };
  const VkResult submitted =
      vkQueueSubmit2(queue->queue, 1, &submit, VK_NULL_HANDLE);
  for (aperture::CommandBuffer& token : desc.buffers) {
    Consume(&token);
  }
  if (submitted != VK_SUCCESS) [[unlikely]] {
    LogFailed(log, "vkQueueSubmit2 failed: {} ({})!", ToString(submitted),
              ToString(MapVkResult(submitted)));
    return std::unexpected(MapVkResult(submitted));
  }
  return {};
}

CommandBuffer& GetNative(const aperture::CommandBuffer& buffer) noexcept {
  APERTURE_ASSERT(buffer.ptr != nullptr);
  auto& impl = *static_cast<CommandBuffer*>(buffer.ptr);
  APERTURE_ASSERT(impl.backend == Backend::Vulkan);
  return impl;
}

}  // namespace aperture::vk
