/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "allocator_host.h"

//
// HOST MEMORY ALLOCATION
//

void *
skc_runtime_host_perm_alloc(struct skc_runtime * const runtime,
                            skc_mem_flags_e      const flags,
                            size_t               const size);

void
skc_runtime_host_perm_free(struct skc_runtime * const runtime,
                           void               * const mem);

void *
skc_runtime_host_temp_alloc(struct skc_runtime * const runtime,
                            skc_mem_flags_e      const flags,
                            size_t               const size,
                            skc_subbuf_id_t    * const subbuf_id,
                            size_t             * const subbuf_size);

void
skc_runtime_host_temp_free(struct skc_runtime * const runtime,
                           void               * const mem,
                           skc_subbuf_id_t      const subbuf_id);


//
//
//
