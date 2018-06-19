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

#include <stdio.h>
#include <string.h>
#include <time.h>

//
// CPU bench
//

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

//
// squelch OpenCL 1.2 deprecation warning
//

#ifndef CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#endif

#include "macros.h"
#include "assert_cl.h"
#include "find_cl.h"
#include "hs_cl_launcher.h"

//
// The quality of the RNG doesn't matter.  The same number of
// instructions will be run no matter what the key distribution looks
// like.  So here is something small and fast.
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

char const * hs_cpu_sort_u32(uint32_t * a, uint32_t const count);
char const * hs_cpu_sort_u64(uint64_t * a, uint32_t const count);

//
//
//

static
char const *
hs_cpu_sort(void                *  sorted_h,
            uint32_t               const count,
            struct hs_info const * const info,
            double               * const cpu_ns)
{
  char const * algo;

  LARGE_INTEGER t0,t1,freq;

  QueryPerformanceCounter(&t0);

  if (info->words == 1)
    algo = hs_cpu_sort_u32(sorted_h,count);
  else
    algo = hs_cpu_sort_u64(sorted_h,count);

  QueryPerformanceCounter(&t1);
  QueryPerformanceFrequency(&freq);

  // nanoseconds
  *cpu_ns = (double)((t1.QuadPart - t0.QuadPart) * 1000 * 1000 * 1000) / (double)freq.QuadPart;

  return algo;
}

static
bool
hs_verify_linear(void * sorted_h, void * vout_h, const uint32_t count, struct hs_info const * const info)
{
  return memcmp(sorted_h, vout_h, sizeof(uint32_t) * info->words * count) == 0;
}

static
void
hs_transpose_slabs_u32(uint32_t * vout_h, const uint32_t count, struct hs_info const * const info)
{
  uint32_t   const slab_keys  = info->keys * info->lanes;
  size_t     const slab_size  = sizeof(uint32_t) * info->words * slab_keys;
  uint32_t * const slab       = _alloca(slab_size);
  uint32_t         slab_count = count / slab_keys;

  while (slab_count-- > 0)
    {
      memcpy(slab,vout_h,slab_size);

      for (uint32_t row=0; row<info->keys; row++)
        for (uint32_t col=0; col<info->lanes; col++)
          vout_h[col * info->keys + row] = slab[row * info->lanes + col];

      vout_h += slab_keys;
    }
}

static
void
hs_transpose_slabs_u64(uint64_t * vout_h, const uint32_t count, struct hs_info const * const info)
{
  uint32_t   const slab_keys  = info->keys * info->lanes;
  size_t     const slab_size  = sizeof(uint32_t) * info->words * slab_keys;
  uint64_t * const slab       = _alloca(slab_size);
  uint32_t         slab_count = count / slab_keys;

  while (slab_count-- > 0)
    {
      memcpy(slab,vout_h,slab_size);

      for (uint32_t row=0; row<info->keys; row++)
        for (uint32_t col=0; col<info->lanes; col++)
          vout_h[col * info->keys + row] = slab[row * info->lanes + col];

      vout_h += slab_keys;
    }
}

static
void
hs_transpose_slabs(void * vout_h, const uint32_t count, struct hs_info const * const info)
{
  if (info->words == 1)
    hs_transpose_slabs_u32(vout_h,count,info);
  else
    hs_transpose_slabs_u64(vout_h,count,info);
}

//
//
//

static
void
hs_debug_u32(
             uint32_t       const *       vout_h,
             uint32_t               const count,
             struct hs_info const * const info)
{
  uint32_t const slab  = info->keys * info->lanes;
  uint32_t const slabs = (count + slab - 1) / slab;

  for (uint32_t ss=0; ss<slabs; ss++) {
    fprintf(stderr,"%u\n",ss);
    for (uint32_t cc=0; cc<info->keys; cc++) {
      for (uint32_t rr=0; rr<info->lanes; rr++)
        fprintf(stderr,"%8X ",*vout_h++);
      fprintf(stderr,"\n");
    }
  }
}

static
void
hs_debug_u64(uint64_t       const *       vout_h,
             uint32_t               const count,
             struct hs_info const * const info)
{
  uint32_t const slab  = info->keys * info->lanes;
  uint32_t const slabs = (count + slab - 1) / slab;

  for (uint32_t ss=0; ss<slabs; ss++) {
    fprintf(stderr,"%u\n",ss);
    for (uint32_t cc=0; cc<info->keys; cc++) {
      for (uint32_t rr=0; rr<info->lanes; rr++)
        fprintf(stderr,"%16llX ",*vout_h++);
      fprintf(stderr,"\n");
    }
  }
}

//
// Used for benchmarking on out-of-order queues.  Attaching an event
// to a kernel on an OOQ with profiling enabled will result in a
// synchronization point and block concurrent execution of kernels.
//
// The workaround that enables measuring the entire runtime of the
// sort is to launch a dummy kernel with an event, a barrier without
// an event, then the call to hs_sort(), followed by a final dummy
// kernel with an event.
//
// The end time of the first dummy and start time of the second dummy
// will provide a conservative estimate of the total execution time of
// the hs_sort() routine.
//
// Note that once kernels are enqueued they are scheduled with only
// microseconds between them so this should only be a small number of
// microseconds longer than the true hs_sort() execution time.
//

#define HS_DUMMY_KERNEL_PROGRAM "kernel void hs_dummy_kernel() { ; }"

static cl_kernel hs_dummy_kernel;

static
void
hs_dummy_kernel_create(cl_context context, cl_device_id device_id)
{
  cl_int err;

  char   const * strings[]        = { HS_DUMMY_KERNEL_PROGRAM };
  size_t const   strings_sizeof[] = { sizeof(HS_DUMMY_KERNEL_PROGRAM) + 1 };

  cl_program program = clCreateProgramWithSource(context,
                                                 1,
                                                 strings,
                                                 strings_sizeof,
                                                 &err); cl_ok(err);
  cl(BuildProgram(program,
                  1,
                  &device_id,
                  NULL,
                  NULL,
                  NULL));

  hs_dummy_kernel = clCreateKernel(program,"hs_dummy_kernel",&err); cl_ok(err);

  cl(ReleaseProgram(program));
}

static
void
hs_dummy_kernel_release()
{
  cl(ReleaseKernel(hs_dummy_kernel));
}

static
void
hs_dummy_kernel_enqueue(cl_command_queue cq, cl_event * const event)
{
  size_t const global_work_size = 1;

  cl(EnqueueNDRangeKernel(cq,
                          hs_dummy_kernel,
                          1,
                          NULL,
                          &global_work_size,
                          NULL,
                          0,
                          NULL,
                          event));
}

//
//
//

static
void
hs_bench(cl_context                   context,
         cl_command_queue             cq,
         char           const * const device_name,
         struct hs_info const * const info,
         uint32_t               const count_lo,
         uint32_t               const count_hi,
         uint32_t               const count_step,
         uint32_t               const loops,
         uint32_t               const warmup,
         bool                   const linearize)
{
  //
  // return if nothing to do
  //
  if (count_hi <= 1)
    return;

  //
  // size the arrays
  //
  uint32_t count_hi_padded_in, count_hi_padded_out;

  hs_pad(count_hi,&count_hi_padded_in,&count_hi_padded_out);

  //
  // SIZE
  //
  size_t const key_size    = sizeof(uint32_t)    * info->words;

  size_t const size_hi     = count_hi            * key_size;
  size_t const size_hi_in  = count_hi_padded_in  * key_size;
  size_t const size_hi_out = count_hi_padded_out * key_size;

  //
  // ALLOCATE
  //
  cl_int cl_err;

  void * sorted_h = malloc(size_hi_in);

  cl_mem random   = clCreateBuffer(context,
                                   CL_MEM_READ_ONLY  | CL_MEM_ALLOC_HOST_PTR,
                                   size_hi_in,
                                   NULL,&cl_err); cl_ok(cl_err);

  cl_mem vin      = clCreateBuffer(context,
                                   CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                   size_hi_in,
                                   NULL,&cl_err); cl_ok(cl_err);

  cl_mem vout     = clCreateBuffer(context,
                                   CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                                   size_hi_out,
                                   NULL,&cl_err); cl_ok(cl_err);
  //
  // BLOCKING MAP AND INIT KEYS
  //
  {
    void * random_h = clEnqueueMapBuffer(cq,
                                         random,
                                         CL_TRUE,
                                         CL_MAP_WRITE_INVALIDATE_REGION,
                                         0,size_hi_in,
                                         0,NULL,NULL,
                                         &cl_err); cl_ok(cl_err);

    // fill with random numbers
    hs_fill_rand(random_h,count_hi,info->words);

    //
    // UNMAP
    //
    cl(EnqueueUnmapMemObject(cq,random,random_h,0,NULL,NULL));
  }

  //
  // BENCHMARK
  //
  for (uint32_t count=count_lo; count<=count_hi; count+=count_step)
    {
      // compute padding before sorting
      uint32_t count_padded_in, count_padded_out;

      hs_pad(count,&count_padded_in,&count_padded_out);

      cl_ulong elapsed_ns_min = ULONG_MAX;
      cl_ulong elapsed_ns_max = 0;
      cl_ulong elapsed_ns_sum = 0;

#if 1
          cl(EnqueueCopyBuffer(cq,random,vin,0,0,count * key_size,0,NULL,NULL));
          cl(Finish(cq));
#endif

      for (uint32_t ii=0; ii<warmup+loops; ii++)
        {
          if (ii == warmup)
            {
              elapsed_ns_min = ULONG_MAX;
              elapsed_ns_max = 0;
              elapsed_ns_sum = 0;
            }

          //
          // initialize vin on every loop -- shouldn't need to do this
          //
#if 0
          cl(EnqueueCopyBuffer(cq,random,vin,0,0,count * key_size,0,NULL,NULL));
          cl(Finish(cq));
#endif

          //
          // sort vin
          //
          cl_event start, end;

          hs_dummy_kernel_enqueue(cq,&start);

          cl(EnqueueBarrierWithWaitList(cq,0,NULL,NULL));

          // note hs_sort enqueues a final barrier
          hs_sort(cq,
                  vin,vout,
                  count,
                  count_padded_in,
                  count_padded_out,
                  linearize);

          hs_dummy_kernel_enqueue(cq,&end);

          cl(Finish(cq));

          //
          // measure duration
          //
          cl_ulong t_start=0, t_end=0;

          // start
          cl(GetEventProfilingInfo(start,
                                   CL_PROFILING_COMMAND_END,
                                   sizeof(cl_ulong),
                                   &t_start,
                                   NULL));
          cl(ReleaseEvent(start));

          // end
          cl(GetEventProfilingInfo(end,
                                   CL_PROFILING_COMMAND_START,
                                   sizeof(cl_ulong),
                                   &t_end,
                                   NULL));
          cl(ReleaseEvent(end));

          cl_ulong const t = t_end - t_start;

          elapsed_ns_min  = MIN_MACRO(elapsed_ns_min,t);
          elapsed_ns_max  = MAX_MACRO(elapsed_ns_max,t);
          elapsed_ns_sum += t;
        }

      //
      // COPY KEYS BACK FOR VERIFICATION
      //
      size_t const size_padded_in = count_padded_in * key_size;

      void * vin_h = clEnqueueMapBuffer(cq,
                                        vin,
                                        CL_FALSE,
                                        CL_MAP_READ,
                                        0,size_padded_in,
                                        0,NULL,NULL,
                                        &cl_err); cl_ok(cl_err);

      void * vout_h = clEnqueueMapBuffer(cq,
                                         vout,
                                         CL_FALSE,
                                         CL_MAP_READ,
                                         0,size_padded_in,
                                         0,NULL,NULL,
                                         &cl_err); cl_ok(cl_err);
      cl(Finish(cq));

      //
      // SORT THE UNTOUCHED RANDOM INPUT
      //
      memcpy(sorted_h,vin_h,size_padded_in);

      double cpu_ns;

      char const * const algo = hs_cpu_sort(sorted_h,count_padded_in,info,&cpu_ns);

      //
      // EXPLICITLY TRANSPOSE THE CPU SORTED SLABS IF NOT LINEARIZING
      //
      if (!linearize) {
        hs_transpose_slabs(vout_h,count_padded_in,info);
      }

      //
      // VERIFY
      //
      bool const verified = hs_verify_linear(sorted_h,vout_h,count_padded_in,info);

#ifndef NDEBUG
      if (!verified)
        {
          if (info->words == 1)
            hs_debug_u32(vout_h,count,info);
          else // ulong
            hs_debug_u64(vout_h,count,info);
        }
#endif

      cl(EnqueueUnmapMemObject(cq,vin, vin_h, 0,NULL,NULL));
      cl(EnqueueUnmapMemObject(cq,vout,vout_h,0,NULL,NULL));

      cl(Finish(cq));

      //
      // REPORT
      //
      fprintf(stdout,"%s, %s, %s, %s, %8u, %8u, %8u, CPU, %s, %9.2f, %6.2f, GPU, %9u, %7.3f, %7.3f, %7.3f, %6.2f, %6.2f\n",
              device_name,
              (info->words == 1) ? "uint" : "ulong",
              linearize ? "linear" : "slab",
              verified ? "  OK  " : "*FAIL*",
              count,
              count_padded_in,
              count_padded_out,
              // CPU
              algo,
              cpu_ns / 1000000.0,                       // milliseconds
              1000.0 * count / cpu_ns,                  // mkeys / sec
              // GPU
              loops,
              elapsed_ns_sum / 1000000.0 / loops,       // avg msecs
              elapsed_ns_min / 1000000.0,               // min msecs
              elapsed_ns_max / 1000000.0,               // max msecs
              1000.0 * count * loops / elapsed_ns_sum,  // mkeys / sec - avg
              1000.0 * count         / elapsed_ns_min); // mkeys / sec - max

      // quit early if not verified
      if (!verified)
        break;
    }

  //
  // dispose
  //
  cl(ReleaseMemObject(vout));
  cl(ReleaseMemObject(vin));
  cl(ReleaseMemObject(random));
  free(sorted_h);
}

//
//
//

int
main(int argc, char** argv)
{
  char const * const target_platform_substring = "Intel";
  char const * const target_device_substring   = "Graphics";

  //
  // find platform and device ids
  //
  cl_platform_id platform_id;
  cl_device_id   device_id;

#define HS_DEVICE_NAME_SIZE  64

  char   device_name[HS_DEVICE_NAME_SIZE];
  size_t device_name_size;

  cl(FindIdsByName(target_platform_substring,
                   target_device_substring,
                   &platform_id,
                   &device_id,
                   HS_DEVICE_NAME_SIZE,
                   device_name,
                   &device_name_size,
                   true));
  //
  // create context
  //
  cl_context_properties context_properties[] =
    {
      CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id,
      0
    };

  cl_int     cl_err;
  cl_context context = clCreateContext(context_properties,
                                       1,
                                       &device_id,
                                       NULL,
                                       NULL,
                                       &cl_err);
  cl_ok(cl_err);

  //
  // create command queue
  //
  cl_command_queue_properties const props = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;

#if 0 // OPENCL 2.0
  cl_queue_properties queue_properties[] =
    {
      CL_QUEUE_PROPERTIES, (cl_queue_properties)props,
      0
    };

  cl_command_queue cq = clCreateCommandQueueWithProperties(context,
                                                           device_id,
                                                           queue_properties,
                                                           &cl_err); cl_ok(cl_err);
#else // OPENCL 1.2
  cl_command_queue cq = clCreateCommandQueue(context,
                                             device_id,
                                             props,
                                             &cl_err); cl_ok(cl_err);
#endif

  //
  // create kernels
  //
  fprintf(stdout,"Creating... ");

  struct hs_info info;

  hs_create(context,device_id,&info);

  fprintf(stdout,"done.\n");

  //
  // create dummy kernel for profiling
  //
  hs_dummy_kernel_create(context,device_id);

  //
  //
  //
#ifdef NDEBUG
#define HS_BENCH_LOOPS   50
#define HS_BENCH_WARMUP  10
#else
#define HS_BENCH_LOOPS   1
#define HS_BENCH_WARMUP  0
#endif

  //
  // sort sizes and loops
  //
  uint32_t const kpb        = info.keys * info.lanes;

  uint32_t const count_lo   = (argc <= 1) ? kpb             : strtoul(argv[1],NULL,0);
  uint32_t const count_hi   = (argc <= 2) ? count_lo        : strtoul(argv[2],NULL,0);
  uint32_t const count_step = (argc <= 3) ? count_lo        : strtoul(argv[3],NULL,0);
  uint32_t const loops      = (argc <= 4) ? HS_BENCH_LOOPS  : strtoul(argv[4],NULL,0);
  uint32_t const warmup     = (argc <= 5) ? HS_BENCH_WARMUP : strtoul(argv[5],NULL,0);
  bool     const linearize  = (argc <= 6) ? true            : strtoul(argv[6],NULL,0);

  //
  // benchmark
  //
  hs_bench(context,cq,device_name,&info,count_lo,count_hi,count_step,loops,warmup,linearize);

  //
  // release everything
  //
  hs_dummy_kernel_release();
  hs_release();

  cl(ReleaseCommandQueue(cq));
  cl(ReleaseContext(context));

  return 0;
}
