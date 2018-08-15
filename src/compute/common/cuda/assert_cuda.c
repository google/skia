/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include <stdlib.h>
#include <stdio.h>

//
//
//

#include <cuda_runtime_api.h>

//
//
//

#include "assert_cuda.h"

//
//
//

cudaError_t
assert_cuda(cudaError_t  const code,
            char const * const file,
            int          const line,
            bool         const abort)
{
  if (code != cudaSuccess)
    {
      const char* const cuda_err_str = cudaGetErrorString(code);

      fprintf(stderr,
              "\"%s\", line %d: assert_cuda ( %d ) = \"%s\"",
              file,line,code,cuda_err_str);

      if (abort)
        {
          cudaDeviceReset();
          exit(code);
        }
    }

  return code;
}

//
//
//
