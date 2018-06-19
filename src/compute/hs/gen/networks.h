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

#pragma once

//
//
//

#include "gen.h"

//
//
//

struct hsg_network
{
  uint32_t      const   length;
  struct hsg_op const * network;
};

//
//
//

extern struct hsg_network const hsg_networks_sorting[];
extern struct hsg_network const hsg_networks_merging[];

//
//
//
