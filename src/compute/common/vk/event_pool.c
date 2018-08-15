/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

//
//
//

#include "event_pool.h"
#include "host_alloc.h"
#include "assert_vk.h"

//
//
//

struct vk_event_pool
{
  VkEvent                     * events;
  VkAllocationCallbacks const * allocator;
  VkDevice                      device;
  uint32_t                      resize;
  uint32_t                      size;
  uint32_t                      next;
};

static
void
vk_event_pool_resize(struct vk_event_pool * const event_pool)
{
  static struct VkEventCreateInfo const eci = {
    .sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
    .pNext = NULL,
    .flags = 0
  };

  // FIXME -- respect allocator

  event_pool->size  += event_pool->resize;
  event_pool->events = vk_host_realloc(event_pool->allocator,
                                       event_pool->events,
                                       sizeof(VkEvent) * event_pool->size);

  for (uint32_t ii=event_pool->next; ii<event_pool->size; ii++)
    {
      vk(CreateEvent(event_pool->device,
                     &eci,
                     event_pool->allocator,
                     event_pool->events+ii));
    }
}

struct vk_event_pool *
vk_event_pool_create(VkDevice device, VkAllocationCallbacks const * allocator, uint32_t const resize)
{
  struct vk_event_pool * const event_pool = vk_host_alloc(allocator,sizeof(*event_pool));

  event_pool->events    = NULL;
  event_pool->allocator = allocator;
  event_pool->device    = device;
  event_pool->resize    = resize;
  event_pool->size      = 0;
  event_pool->next      = 0;

  vk_event_pool_resize(event_pool);

  return event_pool;
}

void
vk_event_pool_release(struct vk_event_pool * const event_pool)
{
  for (uint32_t ii=0; ii<event_pool->size; ii++)
    {
      vkDestroyEvent(event_pool->device,
                     event_pool->events[ii],
                     event_pool->allocator);
    }

  vk_host_free(event_pool->allocator,event_pool->events);
  vk_host_free(event_pool->allocator,event_pool);
}

void
vk_event_pool_reset(struct vk_event_pool * const event_pool)
{
  for (uint32_t ii=0; ii<event_pool->next; ii++)
    vk(ResetEvent(event_pool->device,event_pool->events[ii]));

  event_pool->next = 0;
}

VkEvent
vk_event_pool_acquire(struct vk_event_pool * const event_pool)
{
  if (event_pool->next == event_pool->size)
    vk_event_pool_resize(event_pool);

  return event_pool->events[event_pool->next++];
}

//
//
//
