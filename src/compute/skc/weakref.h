/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#include "skc.h"
#include "types.h"

//
//
//

typedef skc_ulong skc_epoch_t;

//
//
//

void
skc_weakref_epoch_init(skc_epoch_t * const epoch);

void
skc_weakref_epoch_inc(skc_epoch_t * const epoch);

void
skc_weakref_init(skc_weakref_t * const weakref,
                 skc_epoch_t   * const epoch,
                 skc_uint        const index);

bool
skc_weakref_is_invalid(skc_weakref_t const * const weakref,
                       skc_epoch_t   const * const epoch);

skc_uint
skc_weakref_index(skc_weakref_t const * const weakref);

//
//
//
