/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

//
//

#include "common/macros.h"
#include "common/vk/assert_vk.h"
#include "common/vk/host_alloc.h"
#include "common/vk/cache_vk.h"

//
//
//

#include "hs_vk.h"

//
// Compile-time images of HotSort targets
//

#include "hs/vk/intel/gen8/u32/hs_target.h"
#include "hs/vk/intel/gen8/u64/hs_target.h"

#include "hs/vk/nvidia/sm_35/u32/hs_target.h"
#include "hs/vk/nvidia/sm_35/u64/hs_target.h"

#include "hs/vk/amd/gcn/u32/hs_target.h"
#include "hs/vk/amd/gcn/u64/hs_target.h"

//
//
//

char const * hs_cpu_sort_u32(uint32_t * a, uint32_t const count, double * const cpu_ns);
char const * hs_cpu_sort_u64(uint64_t * a, uint32_t const count, double * const cpu_ns);

//
//
//

static
char const *
hs_cpu_sort(void     *       sorted_h,
            uint32_t   const hs_words,
            uint32_t   const count,
            double   * const cpu_ns)
{
  if (hs_words == 1)
    return hs_cpu_sort_u32(sorted_h,count,cpu_ns);
  else
    return hs_cpu_sort_u64(sorted_h,count,cpu_ns);
}

static
void
hs_transpose_slabs_u32(uint32_t const hs_words,
                       uint32_t const hs_width,
                       uint32_t const hs_height,
                       uint32_t *     vout_h,
                       uint32_t const count)
{
  uint32_t   const slab_keys  = hs_width * hs_height;
  size_t     const slab_size  = sizeof(uint32_t) * hs_words * slab_keys;
  uint32_t * const slab       = ALLOCA_MACRO(slab_size);
  uint32_t         slab_count = count / slab_keys;

  while (slab_count-- > 0)
    {
      memcpy(slab,vout_h,slab_size);

      for (uint32_t row=0; row<hs_height; row++)
        for (uint32_t col=0; col<hs_width; col++)
          vout_h[col * hs_height + row] = slab[row * hs_width + col];

      vout_h += slab_keys;
    }
}

static
void
hs_transpose_slabs_u64(uint32_t const hs_words,
                       uint32_t const hs_width,
                       uint32_t const hs_height,
                       uint64_t *     vout_h,
                       uint32_t const count)
{
  uint32_t   const slab_keys  = hs_width * hs_height;
  size_t     const slab_size  = sizeof(uint32_t) * hs_words * slab_keys;
  uint64_t * const slab       = ALLOCA_MACRO(slab_size);
  uint32_t         slab_count = count / slab_keys;

  while (slab_count-- > 0)
    {
      memcpy(slab,vout_h,slab_size);

      for (uint32_t row=0; row<hs_height; row++)
        for (uint32_t col=0; col<hs_width; col++)
          vout_h[col * hs_height + row] = slab[row * hs_width + col];

      vout_h += slab_keys;
    }
}

static
void
hs_transpose_slabs(uint32_t const hs_words,
                   uint32_t const hs_width,
                   uint32_t const hs_height,
                   void   *       vout_h,
                   uint32_t const count)
{
  if (hs_words == 1)
    hs_transpose_slabs_u32(hs_words,hs_width,hs_height,vout_h,count);
  else
    hs_transpose_slabs_u64(hs_words,hs_width,hs_height,vout_h,count);
}

//
//
//

#ifndef NDEBUG

static
VkBool32
VKAPI_PTR
vk_debug_report_cb(VkDebugReportFlagsEXT      flags,
                   VkDebugReportObjectTypeEXT objectType,
                   uint64_t                   object,
                   size_t                     location,
                   int32_t                    messageCode,
                   const char*                pLayerPrefix,
                   const char*                pMessage,
                   void*                      pUserData)
{
  char const * flag_str = "";
  bool         is_error = false;

#define VK_FLAG_CASE_TO_STRING(c)               \
  case c:                                       \
    flag_str = #c;                              \
    is_error = true;                            \
    break

  switch (flags)
    {
      // VK_FLAG_CASE_TO_STRING(VK_DEBUG_REPORT_INFORMATION_BIT_EXT);
      VK_FLAG_CASE_TO_STRING(VK_DEBUG_REPORT_WARNING_BIT_EXT);
      VK_FLAG_CASE_TO_STRING(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
      VK_FLAG_CASE_TO_STRING(VK_DEBUG_REPORT_ERROR_BIT_EXT);
      VK_FLAG_CASE_TO_STRING(VK_DEBUG_REPORT_DEBUG_BIT_EXT);
    }

  if (is_error)
    {
      fprintf(stderr,"%s  %s  %s\n",
              flag_str,
              pLayerPrefix,
              pMessage);
    }

  return VK_FALSE;
}

#endif

//
//
//

static
uint32_t
hs_rand_u32()
{
  static uint32_t seed = 0xDEADBEEF;

  // Numerical Recipes
  seed = seed * 1664525 + 1013904223;

  return seed;
}

//
//
//

static
void
hs_fill_rand(uint32_t * vin_h, uint32_t const count, uint32_t const words)
{
#if   1
  for (uint32_t ii=0; ii<count*words; ii++)
    vin_h[ii] = hs_rand_u32();
#elif 0 // in-order
  memset(vin_h,0,count*words*sizeof(uint32_t));
  for (uint32_t ii=0; ii<count; ii++)
    vin_h[ii*words] = ii;
#else   // reverse order
  memset(vin_h,0,count*words*sizeof(uint32_t));
  for (uint32_t ii=0; ii<count; ii++)
    vin_h[ii*words] = count - 1 - ii;
#endif
}


//
//
//

static
void
hs_debug_u32(uint32_t const   hs_width,
             uint32_t const   hs_height,
             uint32_t const * vout_h,
             uint32_t const   count)
{
  uint32_t const slab_keys = hs_width * hs_height;
  uint32_t const slabs     = (count + slab_keys - 1) / slab_keys;

  for (uint32_t ss=0; ss<slabs; ss++) {
    fprintf(stderr,"%u\n",ss);
    for (uint32_t cc=0; cc<hs_height; cc++) {
      for (uint32_t rr=0; rr<hs_width; rr++)
        fprintf(stderr,"%8" PRIX32 " ",*vout_h++);
      fprintf(stderr,"\n");
    }
  }
}

static
void
hs_debug_u64(uint32_t const   hs_width,
             uint32_t const   hs_height,
             uint64_t const * vout_h,
             uint32_t const   count)
{
  uint32_t const slab_keys = hs_width * hs_height;
  uint32_t const slabs     = (count + slab_keys - 1) / slab_keys;

  for (uint32_t ss=0; ss<slabs; ss++) {
    fprintf(stderr,"%u\n",ss);
    for (uint32_t cc=0; cc<hs_height; cc++) {
      for (uint32_t rr=0; rr<hs_width; rr++)
        fprintf(stderr,"%16" PRIX64 " ",*vout_h++);
      fprintf(stderr,"\n");
    }
  }
}

//
//
//

bool
is_matching_device(VkPhysicalDeviceProperties const * const phy_device_props,
                   struct hs_vk_target const *      * const hs_target,
                   uint32_t                           const vendor_id,
                   uint32_t                           const device_id,
                   uint32_t                           const key_val_words)
{
  if ((phy_device_props->vendorID != vendor_id) || (phy_device_props->deviceID != device_id))
    return false;

  if (phy_device_props->vendorID == 0x10DE)
    {
      //
      // FIXME -- for now, the kernels in this app are targeting
      // sm_35+ devices.  You could add some rigorous rejection by
      // device id here...
      //
      if (key_val_words == 1)
        *hs_target = &hs_nvidia_sm35_u32;
      else
        *hs_target = &hs_nvidia_sm35_u64;
    }
  else if (phy_device_props->vendorID == 0x8086)
    {
      //
      // FIXME -- for now, the kernels in this app are targeting GEN8+
      // devices -- this does *not* include variants of GEN9LP+
      // "Apollo Lake" because that device has a different
      // architectural "shape" than GEN8 GTx.  You could add some
      // rigorous rejection by device id here...
      //
      if (key_val_words == 1)
        *hs_target = &hs_intel_gen8_u32;
      else
        *hs_target = &hs_intel_gen8_u64;
    }
  else if (phy_device_props->vendorID == 0x1002)
    {
      //
      // AMD GCN
      //
      if (key_val_words == 1)
        *hs_target = &hs_amd_gcn_u32;
      else
        *hs_target = &hs_amd_gcn_u64;
    }
  else
    {
      return false;
    }

  return true;
}

//
//
//

uint32_t
vk_find_mem_type_idx(VkPhysicalDeviceMemoryProperties const * phy_device_mem_props,
                     uint32_t                         const   compatible_mem_types,
                     VkMemoryPropertyFlags            const   required_mem_props,
                     bool                             const   abort)
{
  //
  // FIXME -- jump between indices in the memoryTypeBits mask
  //
  uint32_t const count = phy_device_mem_props->memoryTypeCount;

  for (uint32_t index=0; index<count; index++)
    {
      // acceptable memory type for this resource?
      if ((compatible_mem_types & (1<<index)) == 0)
        continue;

      // otherwise, find first match...
      VkMemoryPropertyFlags const common_props =
        phy_device_mem_props->memoryTypes[index].propertyFlags & required_mem_props;

      if (common_props == required_mem_props)
        return index;
    }

  if (abort)
    {
      fprintf(stderr,"Memory type not found: %X\n",required_mem_props);
      exit(EXIT_FAILURE);
    }

  return UINT32_MAX;
}

//
//
//

#ifdef NDEBUG
#define HS_BENCH_LOOPS   100
#define HS_BENCH_WARMUP  100
#else
#define HS_BENCH_LOOPS   1
#define HS_BENCH_WARMUP  0
#endif

//
//
//

int
main(int argc, char const * argv[])
{
  //
  // select the target by vendor and device id
  //
  uint32_t const vendor_id     = (argc <= 1) ? UINT32_MAX : strtoul(argv[1],NULL,16);
  uint32_t const device_id     = (argc <= 2) ? UINT32_MAX : strtoul(argv[2],NULL,16);
  uint32_t const key_val_words = (argc <= 3) ? 1          : strtoul(argv[3],NULL,0);

  if ((key_val_words != 1) && (key_val_words != 2))
    {
      fprintf(stderr,"Key/Val Words must be 1 or 2\n");
      exit(EXIT_FAILURE);
    }

  //
  // create a Vulkan instances
  //
  VkApplicationInfo const app_info = {
      .sType                 = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext                 = NULL,
      .pApplicationName      = "Google HotSort Bench",
      .applicationVersion    = 0,
      .pEngineName           = "Google HotSort Gen",
      .engineVersion         = 0,
      .apiVersion            = VK_API_VERSION_1_1
  };

  char const * const instance_enabled_layers[] = {
    "VK_LAYER_LUNARG_standard_validation"
  };

  char const * const instance_enabled_extensions[] = {
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };

  uint32_t const instance_enabled_layer_count =
#ifndef NDEBUG
    ARRAY_LENGTH_MACRO(instance_enabled_layers)
#else
    0
#endif
    ;

  uint32_t const instance_enabled_extension_count =
#ifndef NDEBUG
    ARRAY_LENGTH_MACRO(instance_enabled_extensions)
#else
    0
#endif
    ;

  VkInstanceCreateInfo const instance_info = {
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext                   = NULL,
    .flags                   = 0,
    .pApplicationInfo        = &app_info,
    .enabledLayerCount       = instance_enabled_layer_count,
    .ppEnabledLayerNames     = instance_enabled_layers,
    .enabledExtensionCount   = instance_enabled_extension_count,
    .ppEnabledExtensionNames = instance_enabled_extensions
  };

  VkInstance instance;

  vk(CreateInstance(&instance_info,NULL,&instance));

  //
  //
  //
#ifndef NDEBUG
  PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
    (PFN_vkCreateDebugReportCallbackEXT)
    vkGetInstanceProcAddr(instance,"vkCreateDebugReportCallbackEXT");

  PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
    (PFN_vkDestroyDebugReportCallbackEXT)
    vkGetInstanceProcAddr(instance,"vkDestroyDebugReportCallbackEXT");

  struct VkDebugReportCallbackCreateInfoEXT const drcci = {
    .sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
    .pNext       = NULL,
    .flags       = UINT32_MAX, // enable everything for now
    .pfnCallback = vk_debug_report_cb,
    .pUserData   = NULL
  };

  VkDebugReportCallbackEXT drc;

  vk(CreateDebugReportCallbackEXT(instance,
                                  &drcci,
                                  NULL,
                                  &drc));
#endif

  //
  // acquire all physical devices and select a match
  //
  uint32_t phy_device_count;

  vk(EnumeratePhysicalDevices(instance,
                              &phy_device_count,
                              NULL));

  VkPhysicalDevice * phy_devices = vk_host_alloc(NULL,phy_device_count * sizeof(*phy_devices));

  vk(EnumeratePhysicalDevices(instance,
                              &phy_device_count,
                              phy_devices));

  VkPhysicalDevice           phy_device = VK_NULL_HANDLE;
  VkPhysicalDeviceProperties phy_device_props;

  struct hs_vk_target const * hs_target;

  for (uint32_t ii=0; ii<phy_device_count; ii++)
    {
      VkPhysicalDeviceProperties tmp;

      vkGetPhysicalDeviceProperties(phy_devices[ii],&tmp);

      bool const is_match = is_matching_device(&tmp,
                                               &hs_target,
                                               vendor_id,
                                               device_id,
                                               key_val_words);

      fprintf(stdout,"%c %4X : %4X : %s\n",
              is_match ? '*' : ' ',
              tmp.vendorID,
              tmp.deviceID,
              tmp.deviceName);

      if (is_match)
        {
          phy_device = phy_devices[ii];
          memcpy(&phy_device_props,&tmp,sizeof(tmp));
        }

    }

  if (phy_device == VK_NULL_HANDLE)
    {
      fprintf(stderr,"Device %4X:%4X not found.\n",
              vendor_id & 0xFFFF,
              device_id & 0xFFFF);

      return EXIT_FAILURE;
    }

  vk_host_free(NULL,phy_devices);

  //
  // Get rest of command line
  //
  uint32_t const slab_size    = hs_target->config.slab.height << hs_target->config.slab.width_log2;

  uint32_t const count_lo     = (argc <=  4) ? slab_size       : strtoul(argv[ 4],NULL,0);
  uint32_t const count_hi     = (argc <=  5) ? count_lo        : strtoul(argv[ 5],NULL,0);
  uint32_t const count_step   = (argc <=  6) ? count_lo        : strtoul(argv[ 6],NULL,0);
  uint32_t const loops        = (argc <=  7) ? HS_BENCH_LOOPS  : strtoul(argv[ 7],NULL,0);
  uint32_t const warmup       = (argc <=  8) ? HS_BENCH_WARMUP : strtoul(argv[ 8],NULL,0);
  bool     const linearize    = (argc <=  9) ? true            : strtoul(argv[ 9],NULL,0) != 0;
  bool     const verify       = (argc <= 10) ? true            : strtoul(argv[10],NULL,0) != 0;

  //
  // get the physical device's memory props
  //
  VkPhysicalDeviceMemoryProperties phy_device_mem_props;

  vkGetPhysicalDeviceMemoryProperties(phy_device,&phy_device_mem_props);

  //
  // get queue properties
  //
  VkQueueFamilyProperties queue_fam_props[2];
  uint32_t                queue_fam_count = ARRAY_LENGTH_MACRO(queue_fam_props);

  vkGetPhysicalDeviceQueueFamilyProperties(phy_device,&queue_fam_count,queue_fam_props);

  //
  // create device
  //
  float const queue_priorities[] = { 1.0f };

  VkDeviceQueueCreateInfo const queue_info = {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext            = NULL,
    .flags            = 0,
    .queueFamilyIndex = 0,
    .queueCount       = 1,
    .pQueuePriorities = queue_priorities
  };

  //
  // clumsily enable AMD GCN shader info extension
  //
  char const * const device_enabled_extensions[] = {
#if defined( HS_VK_VERBOSE_STATISTICS_AMD ) || defined( HS_VK_VERBOSE_DISASSEMBLY_AMD )
    VK_AMD_SHADER_INFO_EXTENSION_NAME
#endif
  };

  uint32_t device_enabled_extension_count = 0;

#if defined( HS_VK_VERBOSE_STATISTICS_AMD ) || defined( HS_VK_VERBOSE_DISASSEMBLY_AMD )
  if (phy_device_props.vendorID == 0x1002)
    device_enabled_extension_count = 1;
#endif

  //
  //
  //
  VkPhysicalDeviceFeatures device_features = { false };

  if (key_val_words == 2)
    {
      device_features.shaderInt64 = true;
    }

  VkDeviceCreateInfo const device_info = {
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext                   = NULL,
    .flags                   = 0,
    .queueCreateInfoCount    = 1,
    .pQueueCreateInfos       = &queue_info,
    .enabledLayerCount       = 0,
    .ppEnabledLayerNames     = NULL,
    .enabledExtensionCount   = device_enabled_extension_count,
    .ppEnabledExtensionNames = device_enabled_extensions,
    .pEnabledFeatures        = &device_features
  };

  VkDevice device;

  vk(CreateDevice(phy_device,&device_info,NULL,&device));

  //
  // get a queue
  //
  VkQueue queue;

  vkGetDeviceQueue(device,0,0,&queue);

  //
  // get the pipeline cache
  //
  VkPipelineCache pipeline_cache;

  vk_pipeline_cache_create(device,NULL,".vk_cache",&pipeline_cache);

  //
  // create a descriptor set pool
  //
  VkDescriptorPoolSize const dps[] = {
    {
      .type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      .descriptorCount = 2
    }
  };

  VkDescriptorPoolCreateInfo const dpci = {
    .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .pNext         = NULL,
    .flags         = 0, // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
    .maxSets       = 1,
    .poolSizeCount = ARRAY_LENGTH_MACRO(dps),
    .pPoolSizes    = dps
  };

  VkDescriptorPool desc_pool;

  vk(CreateDescriptorPool(device,
                          &dpci,
                          NULL, // allocator
                          &desc_pool));

  //
  // create HotSort device instance
  //
  struct hs_vk * hs = hs_vk_create(hs_target,
                                   device,
                                   NULL,
                                   pipeline_cache);
  //
  // create a HotSort descriptor set for this thread
  //
  VkDescriptorSet hs_ds = hs_vk_ds_alloc(hs,desc_pool);

  //
  // create a command pool for this thread
  //
  VkCommandPoolCreateInfo const cmd_pool_info = {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = NULL,
    .flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = 0,
  };

  VkCommandPool cmd_pool;

  vk(CreateCommandPool(device,
                       &cmd_pool_info,
                       NULL,
                       &cmd_pool));

  //
  // create a query pool for benchmarking
  //
  static VkQueryPoolCreateInfo const query_pool_info = {
    .sType              = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
    .pNext              = NULL,
    .flags              = 0,
    .queryType          = VK_QUERY_TYPE_TIMESTAMP,
    .queryCount         = 4,
    .pipelineStatistics = 0
  };

  VkQueryPool query_pool;

  vk(CreateQueryPool(device,
                     &query_pool_info,
                     NULL,
                     &query_pool));

  //
  // create two big buffers -- buffer_out_count is always the largest
  //
  uint32_t buffer_in_count, buffer_out_count;

  hs_vk_pad(hs,count_hi,&buffer_in_count,&buffer_out_count);

  size_t const buffer_out_size = buffer_out_count * key_val_words * sizeof(uint32_t);

  VkBufferCreateInfo bci = {
    .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext                 = NULL,
    .flags                 = 0,
    .size                  = buffer_out_size,
    .usage                 = 0,
    .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices   = NULL
  };

  VkBuffer vin, vout, sorted, rand;

  bci.usage =
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
    VK_BUFFER_USAGE_TRANSFER_DST_BIT,

  vk(CreateBuffer(device,
                  &bci,
                  NULL,
                  &vin));

  vk(CreateBuffer(device,
                  &bci,
                  NULL,
                  &sorted));

  bci.usage =
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT   |
    VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  vk(CreateBuffer(device,
                  &bci,
                  NULL,
                  &vout));

  bci.usage =
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  vk(CreateBuffer(device,
                  &bci,
                  NULL,
                  &rand));

  //
  // get memory requirements for one of the buffers
  //
  VkMemoryRequirements mr_vin, mr_vout, mr_sorted, mr_rand;

  vkGetBufferMemoryRequirements(device,vin, &mr_vin);
  vkGetBufferMemoryRequirements(device,vout,&mr_vout);

  vkGetBufferMemoryRequirements(device,rand,&mr_sorted);
  vkGetBufferMemoryRequirements(device,rand,&mr_rand);

  //
  // allocate memory for the buffers
  //
  // for simplicity, all buffers are the same size
  //
  // vin and vout have the same usage
  //
  VkMemoryAllocateInfo const mai_vin_vout = {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext           = NULL,
    .allocationSize  = mr_vin.size,
    .memoryTypeIndex = vk_find_mem_type_idx(&phy_device_mem_props,
                                            mr_vin.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            true)
  };

  VkMemoryAllocateInfo const mai_sorted_rand = {
    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext           = NULL,
    .allocationSize  = mr_sorted.size,
    .memoryTypeIndex = vk_find_mem_type_idx(&phy_device_mem_props,
                                            mr_sorted.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                            true)
  };

  VkDeviceMemory mem_vin, mem_vout, mem_sorted, mem_rand;

  vk(AllocateMemory(device,
                    &mai_vin_vout,
                    NULL,
                    &mem_vin));

  vk(AllocateMemory(device,
                    &mai_vin_vout,
                    NULL,
                    &mem_vout));

  vk(AllocateMemory(device,
                    &mai_sorted_rand,
                    NULL,
                    &mem_sorted));

  vk(AllocateMemory(device,
                    &mai_sorted_rand,
                    NULL,
                    &mem_rand));

  //
  // bind backing memory to the virtual allocations
  //
  vk(BindBufferMemory(device,vin,   mem_vin,   0));
  vk(BindBufferMemory(device,vout,  mem_vout,  0));

  vk(BindBufferMemory(device,sorted,mem_sorted,0));
  vk(BindBufferMemory(device,rand,  mem_rand,  0));

  //
  // map and fill the rand buffer with random values
  //
  void * rand_h   = vk_host_alloc(NULL,buffer_out_size);
  void * sorted_h = vk_host_alloc(NULL,buffer_out_size);

  hs_fill_rand(rand_h,buffer_out_count,key_val_words);

  void * rand_map;

  vk(MapMemory(device,mem_rand,0,VK_WHOLE_SIZE,0,&rand_map));

  memcpy(rand_map,rand_h,buffer_out_size);

  vkUnmapMemory(device,mem_rand);

  //
  // create a single command buffer for this thread
  //
  VkCommandBufferAllocateInfo const cmd_buffer_info = {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext              = NULL,
    .commandPool        = cmd_pool,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
  };

  VkCommandBuffer cb;

  vk(AllocateCommandBuffers(device,
                            &cmd_buffer_info,
                            &cb));

  //
  //
  //
  static VkCommandBufferBeginInfo const cb_begin_info = {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext            = NULL,
    .flags            = 0, // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = NULL
  };

  struct VkSubmitInfo const submit_info = {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = NULL,
    .waitSemaphoreCount   = 0,
    .pWaitSemaphores      = NULL,
    .pWaitDstStageMask    = NULL,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &cb,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores    = NULL
  };

  //
  // labels
  //
  fprintf(stdout,
          "Device, "
          "Driver, "
          "Type, "
          "Slab/Linear, "
          "Verified?, "
          "Keys, "
          "Keys Padded In, "
          "Keys Padded Out, "
          "CPU, "
          "Algorithm, "
          "CPU Msecs, "
          "CPU Mkeys/s, "
          "GPU, "
          "Trials, "
          "Avg. Msecs, "
          "Min Msecs, "
          "Max Msecs, "
          "Avg. Mkeys/s, "
          "Max. Mkeys/s\n");

  //
  // test a range
  //
  for (uint32_t count=count_lo; count<=count_hi; count+=count_step)
    {
      //
      // size the vin and vout arrays
      //
      uint32_t count_padded_in, count_padded_out;

      hs_vk_pad(hs,count,&count_padded_in,&count_padded_out);

      //
      // initialize vin with 'count' random keys
      //
      vkBeginCommandBuffer(cb,&cb_begin_info);

      VkBufferCopy const copy_rand = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size      = count * key_val_words * sizeof(uint32_t)
      };

      vkCmdCopyBuffer(cb,
                      rand,
                      vin,
                      1,
                      &copy_rand);

      vk(EndCommandBuffer(cb));

      vk(QueueSubmit(queue,
                     1,
                     &submit_info,
                     VK_NULL_HANDLE)); // FIXME -- put a fence here

      // wait for queue to drain
      vk(QueueWaitIdle(queue));
      vk(ResetCommandBuffer(cb,0));

      //
      // build the sorting command buffer
      //
      vkBeginCommandBuffer(cb,&cb_begin_info);

      //
      // starting timestamp
      //
      vkCmdWriteTimestamp(cb,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,query_pool,0);

      //
      // bind the vin/vout buffers early
      //
      hs_vk_ds_bind(hs,hs_ds,cb,vin,vout);

      //
      // append sorting commands
      //
      hs_vk_sort(hs,
                 cb,
                 vin,0,0,
                 vout,0,0,
                 count,
                 count_padded_in,
                 count_padded_out,
                 linearize);

      //
      // end timestamp
      //
      vkCmdWriteTimestamp(cb,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,query_pool,1);

      //
      // end the command buffer
      //
      vk(EndCommandBuffer(cb));

      //
      // measure the min/max/avg execution time
      //
      uint64_t elapsed_ns_min = UINT64_MAX;
      uint64_t elapsed_ns_max = 0;
      uint64_t elapsed_ns_sum = 0;

      for (uint32_t ii=0; ii<warmup+loops; ii++)
        {
          if (ii == warmup)
            {
              elapsed_ns_min = UINT64_MAX;
              elapsed_ns_max = 0;
              elapsed_ns_sum = 0;
            }

          vk(QueueSubmit(queue,
                         1,
                         &submit_info,
                         VK_NULL_HANDLE)); // FIXME -- put a fence here

          // wait for queue to drain
          vk(QueueWaitIdle(queue));

          // get results
          uint64_t timestamps[2];

          vk(GetQueryPoolResults(device,query_pool,
                                 0,ARRAY_LENGTH_MACRO(timestamps),
                                 sizeof(timestamps),
                                 timestamps,
                                 sizeof(timestamps[0]),
                                 VK_QUERY_RESULT_64_BIT |
                                 VK_QUERY_RESULT_WAIT_BIT));

          uint64_t const t = timestamps[1] - timestamps[0];

          elapsed_ns_min  = MIN_MACRO(elapsed_ns_min,t);
          elapsed_ns_max  = MAX_MACRO(elapsed_ns_max,t);
          elapsed_ns_sum += t;
        }

      vk(ResetCommandBuffer(cb,0));

      //
      // copy the results back and, optionally, verify them
      //
      char const * cpu_algo = NULL;
      double       cpu_ns   = 0.0;
      bool         verified = false;

      if (verify)
        {
          size_t const size_padded_in = count_padded_in * key_val_words * sizeof(uint32_t);

          vkBeginCommandBuffer(cb,&cb_begin_info);

          VkBufferCopy const copy_vout = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = size_padded_in
          };

          vkCmdCopyBuffer(cb,
                          vout,
                          sorted,
                          1,
                          &copy_vout);

          vk(EndCommandBuffer(cb));

          vk(QueueSubmit(queue,
                         1,
                         &submit_info,
                         VK_NULL_HANDLE)); // FIXME -- put a fence here

          // wait for queue to drain
          vk(QueueWaitIdle(queue));
          vk(ResetCommandBuffer(cb,0));

          size_t const size_sorted_h = count * key_val_words * sizeof(uint32_t);

          // copy and sort random data
          memcpy(sorted_h,rand_h,size_sorted_h);
          memset((uint8_t*)sorted_h + size_sorted_h,-1,size_padded_in-size_sorted_h);

          cpu_algo = hs_cpu_sort(sorted_h,key_val_words,count_padded_in,&cpu_ns);

          void * sorted_map;

          vk(MapMemory(device,mem_sorted,0,VK_WHOLE_SIZE,0,&sorted_map));

          if (!linearize) {
            hs_transpose_slabs(key_val_words,
                               1u<<hs_target->config.slab.width_log2,
                               hs_target->config.slab.height,
                               sorted_map,
                               count_padded_in);
          }

          // verify
          verified = memcmp(sorted_h,sorted_map,size_padded_in) == 0;

#ifndef NDEBUG
          if (!verified)
            {
              if (key_val_words == 1)
                {
                  hs_debug_u32(1u<<hs_target->config.slab.width_log2,
                               hs_target->config.slab.height,
                               sorted_h,
                               count);

                  hs_debug_u32(1u<<hs_target->config.slab.width_log2,
                               hs_target->config.slab.height,
                               sorted_map,
                               count);
                }
              else // ulong
                {
                  hs_debug_u64(1u<<hs_target->config.slab.width_log2,
                               hs_target->config.slab.height,
                               sorted_h,
                               count);

                  hs_debug_u64(1u<<hs_target->config.slab.width_log2,
                               hs_target->config.slab.height,
                               sorted_map,
                               count);
                }
            }
#endif

          vkUnmapMemory(device,mem_sorted);
        }

      //
      // REPORT
      //
      float const timestamp_period = phy_device_props.limits.timestampPeriod;

      fprintf(stdout,"%s, %u.%u.%u.%u, %s, %s, %s, %8u, %8u, %8u, CPU, %s, %9.2f, %6.2f, GPU, %9u, %7.3f, %7.3f, %7.3f, %6.2f, %6.2f\n",
              phy_device_props.deviceName,
              (phy_device_props.driverVersion>>24)&0xFF,
              (phy_device_props.driverVersion>>16)&0xFF,
              (phy_device_props.driverVersion>> 8)&0xFF,
              (phy_device_props.driverVersion    )&0xFF,
              (key_val_words == 1) ? "uint" : "ulong",
              linearize ? "linear" : "slab",
              verify ? (verified ? "  OK  " : "*FAIL*") : "UNVERIFIED",
              count,
              count_padded_in,
              count_padded_out,
              // CPU
              verify ? cpu_algo : "UNVERIFIED",
              verify ? (cpu_ns / 1000000.0)      : 0.0,                      // milliseconds
              verify ? (1000.0 * count / cpu_ns) : 0.0,                      // mkeys / sec
              // GPU
              loops,
              timestamp_period * elapsed_ns_sum / 1e6 / loops,               // avg msecs
              timestamp_period * elapsed_ns_min / 1e6,                       // min msecs
              timestamp_period * elapsed_ns_max / 1e6,                       // max msecs
              1000.0 * count * loops / (timestamp_period * elapsed_ns_sum),  // mkeys / sec - avg
              1000.0 * count         / (timestamp_period * elapsed_ns_min)); // mkeys / sec - max
    }

  // reset the descriptor pool
  vk(ResetDescriptorPool(device,desc_pool,0));

  //
  // cleanup
  //

  // release shared HotSort state
  hs_vk_release(hs);

  // destroy the vin/vout buffers (before device memory)
  vkDestroyBuffer(device,vin,   NULL);
  vkDestroyBuffer(device,vout,  NULL);
  vkDestroyBuffer(device,sorted,NULL);
  vkDestroyBuffer(device,rand,  NULL);

  // free device memory
  vkFreeMemory(device,mem_vin,   NULL);
  vkFreeMemory(device,mem_vout,  NULL);
  vkFreeMemory(device,mem_sorted,NULL);
  vkFreeMemory(device,mem_rand,  NULL);

  // free host memory
  vk_host_free(NULL,rand_h);
  vk_host_free(NULL,sorted_h);

  // destroy the descriptor pool
  vkDestroyDescriptorPool(device,desc_pool,NULL);

  // destroy remaining...
  vkDestroyQueryPool(device,query_pool,NULL);
  vkFreeCommandBuffers(device,cmd_pool,1,&cb);
  vkDestroyCommandPool(device,cmd_pool,NULL);

  vk_pipeline_cache_destroy(device,NULL,".vk_cache",pipeline_cache);

  vkDestroyDevice(device,NULL);

#ifndef NDEBUG
  vkDestroyDebugReportCallbackEXT(instance,drc,NULL);
#endif

  vkDestroyInstance(instance,NULL);

  return EXIT_SUCCESS;
}

//
//
//
