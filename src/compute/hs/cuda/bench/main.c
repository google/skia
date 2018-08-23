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
#include <float.h>
#include <stdbool.h>

//
//
//

#include <cuda_runtime.h>

//
//
//

#include "common/cuda/assert_cuda.h"
#include "common/macros.h"

//
//
//

#include "hs/cuda/sm_35/u32/hs_cuda.h"
#include "hs/cuda/sm_35/u64/hs_cuda.h"

//
// PFNs to select between different key widths
//

typedef void (*hs_cuda_info_pfn)(uint32_t * const key_words,
                                 uint32_t * const val_words,
                                 uint32_t * const slab_height,
                                 uint32_t * const slab_width_log2);

typedef void (*hs_cuda_pad_pfn)(uint32_t   const count,
                                uint32_t * const count_padded_in,
                                uint32_t * const count_padded_out);

typedef void (*hs_cuda_sort_pfn)(void *   const vin,
                                 void *   const vout,
                                 uint32_t const count,
                                 uint32_t const count_padded_in,
                                 uint32_t const count_padded_out,
                                 bool     const linearize,
                                 cudaStream_t   stream0,
                                 cudaStream_t   stream1,
                                 cudaStream_t   stream2);

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
bool
hs_verify_linear(uint32_t const hs_words,
                 void   *       sorted_h,
                 void   *       vout_h,
                 uint32_t const count)
{
  return memcmp(sorted_h, vout_h, sizeof(uint32_t) * hs_words * count) == 0;
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

static
void
hs_bench(hs_cuda_pad_pfn               hs_pad,
         hs_cuda_sort_pfn              hs_sort,
         cudaStream_t                  stream0,
         cudaStream_t                  stream1,
         cudaStream_t                  stream2,
         struct cudaDeviceProp const * props,
         int                   const   driver_version,
         uint32_t              const   hs_words,
         uint32_t              const   hs_height,
         uint32_t              const   hs_width,
         uint32_t              const   count_lo,
         uint32_t              const   count_hi,
         uint32_t              const   count_step,
         uint32_t              const   loops,
         uint32_t              const   warmup,
         bool                  const   linearize,
         bool                  const   verify)
{
  //
  // return if nothing to do
  //
  if (count_hi <= 1)
    return;

  //
  // size for the largest array
  //
  uint32_t count_hi_padded_in, count_hi_padded_out;

  hs_pad(count_hi,&count_hi_padded_in,&count_hi_padded_out);

  size_t const key_size    = sizeof(uint32_t)    * hs_words;
  size_t const size_hi_in  = count_hi_padded_in  * key_size;
  size_t const size_hi_out = count_hi_padded_out * key_size;

  //
  // allocate device extents
  //
  void * random_d;
  void * vin_d;
  void * vout_d;

  cuda(Malloc(&random_d,size_hi_in));
  cuda(Malloc(&vin_d,   size_hi_in));
  cuda(Malloc(&vout_d,  size_hi_out));

  //
  // initialize device random extent
  //
  void * random_h = malloc(size_hi_in);

  // fill with random numbers
  hs_fill_rand(random_h,count_hi,hs_words);

  // copy to device
  cuda(Memcpy(random_d,random_h,size_hi_in,cudaMemcpyHostToDevice));

  free(random_h);

  //
  // allocate host result extent
  //
  void * sorted_h = malloc(size_hi_in);
  void * vout_h   = malloc(size_hi_in);

  //
  // LABELS
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
          "CPU Algorithm, "
          "CPU Msecs, "
          "CPU Mkeys/s, "
          "Trials, "
          "Avg. Msecs, "
          "Min Msecs, "
          "Max Msecs, "
          "Avg. Mkeys/s, "
          "Max. Mkeys/s\n");
  //
  // BENCHMARK
  //
  cudaEvent_t start, end;

  cuda(EventCreate(&start));
  cuda(EventCreate(&end));

  for (uint32_t count=count_lo; count<=count_hi; count+=count_step)
    {
      // compute padding before sorting
      uint32_t count_padded_in, count_padded_out;

      hs_pad(count,&count_padded_in,&count_padded_out);

      cuda(Memcpy(vin_d,random_d,count*key_size,cudaMemcpyDeviceToDevice));

      float elapsed_ms_min = FLT_MAX;
      float elapsed_ms_max = 0.0f;
      float elapsed_ms_sum = 0.0f;

      for (uint32_t ii=0; ii<warmup+loops; ii++)
        {
          if (ii == warmup)
            {
              elapsed_ms_min = FLT_MAX;
              elapsed_ms_max = 0.0f;
              elapsed_ms_sum = 0.0f;
            }

          //
          // sort vin/vout
          //
          cuda(EventRecord(start,stream0));
          cuda(StreamWaitEvent(stream1,start,0));
          cuda(StreamWaitEvent(stream2,start,0));

          hs_sort(vin_d,
                  vout_d,
                  count,
                  count_padded_in,
                  count_padded_out,
                  linearize,
                  stream0,
                  stream1,
                  stream2);

          cuda(EventRecord(end,stream0));

          cuda(EventSynchronize(end));

          float elapsed;

          cuda(EventElapsedTime(&elapsed,start,end));

          elapsed_ms_min  = MIN_MACRO(elapsed_ms_min,elapsed);
          elapsed_ms_max  = MAX_MACRO(elapsed_ms_max,elapsed);
          elapsed_ms_sum += elapsed;
        }

      //
      // verify
      //
      char const * cpu_algo = NULL;
      double       cpu_ns   = 1.0;
      bool         verified = false;

      if (verify)
        {
	  //
	  // copy back the results
	  //
	  size_t const size_padded_in = count_padded_in * key_size;

	  cuda(Memcpy(sorted_h,vin_d, size_padded_in,cudaMemcpyDeviceToHost));
	  cuda(Memcpy(vout_h,  vout_d,size_padded_in,cudaMemcpyDeviceToHost));

	  //
	  // sort the input with another algorithm
	  //
	  cpu_algo = hs_cpu_sort(sorted_h,hs_words,count_padded_in,&cpu_ns);

	  // transpose the cpu sorted slabs before comparison
	  if (!linearize) {
	    hs_transpose_slabs(hs_words,hs_width,hs_height,vout_h,count_padded_in);
	  }

	  verified = hs_verify_linear(hs_words,sorted_h,vout_h,count_padded_in);

#ifndef NDEBUG
	  if (!verified)
	    {
	      if (hs_words == 1) {
		hs_debug_u32(hs_width,hs_height,vout_h,  count);
		hs_debug_u32(hs_width,hs_height,sorted_h,count);
	      } else { // ulong
		hs_debug_u64(hs_width,hs_height,vout_h,  count);
		hs_debug_u64(hs_width,hs_height,sorted_h,count);
	      }
	    }
#endif
	}

      //
      // REPORT
      //
      fprintf(stdout,"%s, %u, %s, %s, %s, %8u, %8u, %8u, CPU, %s, %9.2f, %6.2f, GPU, %9u, %7.3f, %7.3f, %7.3f, %6.2f, %6.2f\n",
              props->name,
              driver_version,
              (hs_words == 1) ? "uint32_t" : "uint64_t",
              linearize       ? "linear"   : "slab",
              verify ? (verified ? "  OK  " : "*FAIL*") : "UNVERIFIED",
              count,
              count_padded_in,
              count_padded_out,
              // CPU
              verify ? cpu_algo : "UNVERIFIED",
              verify ? (cpu_ns / 1000000.0)      : 0.0,             // milliseconds
              verify ? (1000.0 * count / cpu_ns) : 0.0,             // mkeys / sec
              // GPU
              loops,
              elapsed_ms_sum / loops,                               // avg msecs
              elapsed_ms_min,                                       // min msecs
              elapsed_ms_max,                                       // max msecs
              (double)(count * loops) / (1000.0 * elapsed_ms_sum),  // mkeys / sec - avg
              (double) count          / (1000.0 * elapsed_ms_min)); // mkeys / sec - max

      // quit early if not verified
      if (verify && !verified)
        break;
    }

  //
  // dispose
  //
  cuda(EventDestroy(start));
  cuda(EventDestroy(end));

  free(sorted_h);
  free(vout_h);

  cuda(Free(random_d));
  cuda(Free(vin_d));
  cuda(Free(vout_d));
}

//
//
//

int
main(int argc, char const * argv[])
{
  //
  // which CUDA device?
  //
  const int32_t device = (argc == 1) ? 0 : atoi(argv[1]);

  struct cudaDeviceProp props;
  cuda(GetDeviceProperties(&props,device));

  cuda(SetDeviceFlags(cudaDeviceScheduleBlockingSync));
  cuda(SetDevice(device));

  int driver_version;

  cuda(DriverGetVersion(&driver_version));

#ifndef NDEBUG
  fprintf(stdout,"%s (%2d) : %u\n",
          props.name,
          props.multiProcessorCount,
          driver_version);
#endif

  //
  // create some streams
  //
  cudaStream_t stream0,stream1,stream2;

  cuda(StreamCreate(&stream0));
  cuda(StreamCreate(&stream1));
  cuda(StreamCreate(&stream2));

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
  // are we sorting 32-bit or 64-bit keys?
  //
  uint32_t const key_size = (argc <= 2) ? 2 : strtoul(argv[2],NULL,0);

  hs_cuda_info_pfn hs_info;
  hs_cuda_pad_pfn  hs_pad;
  hs_cuda_sort_pfn hs_sort;

  if (key_size == 1)
    {
      hs_info = hs_cuda_info_u32;
      hs_pad  = hs_cuda_pad_u32;
      hs_sort = hs_cuda_sort_u32;
    }
  else
    {
      hs_info = hs_cuda_info_u64;
      hs_pad  = hs_cuda_pad_u64;
      hs_sort = hs_cuda_sort_u64;
    }

  //
  // get some configuration info
  //
  uint32_t key_words, val_words, slab_height, slab_width_log2;

  hs_info(&key_words,&val_words,&slab_height,&slab_width_log2);

  //
  // sort sizes and loops
  //
  uint32_t const kpb        = slab_height << slab_width_log2;
  uint32_t const count_lo   = (argc <= 3) ? kpb             : strtoul(argv[3],NULL,0);
  uint32_t const count_hi   = (argc <= 4) ? count_lo        : strtoul(argv[4],NULL,0);
  uint32_t const count_step = (argc <= 5) ? count_lo        : strtoul(argv[5],NULL,0);
  uint32_t const loops      = (argc <= 6) ? HS_BENCH_LOOPS  : strtoul(argv[6],NULL,0);
  uint32_t const warmup     = (argc <= 7) ? HS_BENCH_WARMUP : strtoul(argv[7],NULL,0);
  bool     const linearize  = (argc <= 8) ? true            : strtoul(argv[8],NULL,0);
  bool     const verify     = (argc <= 9) ? true            : strtoul(argv[9],NULL,0);

  //
  // benchmark
  //
  hs_bench(hs_pad,
           hs_sort,
           stream0,
           stream1,
           stream2,
           &props,
           driver_version,
           key_words + val_words,
           slab_height,
           1 << slab_width_log2,
           count_lo,
           count_hi,
           count_step,
           loops,
           warmup,
           linearize,
	   verify);

  //
  // cleanup
  //
  cuda(StreamDestroy(stream0));
  cuda(StreamDestroy(stream1));
  cuda(StreamDestroy(stream2));

  cuda(DeviceReset());

  return EXIT_SUCCESS;
}
