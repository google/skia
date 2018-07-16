/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#include <stdio.h>
#include <stdlib.h>

//
//
//

#include "gen.h"

//
//
//

#define HSG_INDENT  2

//
//
//

struct hsg_target_state
{
  FILE * txt;
};

//
//
//

void
hsg_target_indent(struct hsg_target * const target, uint32_t const depth)
{
  fprintf(target->state->txt,
          "%*s",
          depth*HSG_INDENT,"");
}

void
hsg_target_debug(struct hsg_target       * const target,
                 struct hsg_config const * const config,
                 struct hsg_merge  const * const merge,
                 struct hsg_op     const * const ops,
                 uint32_t                  const depth)
{
  if (ops->type == HSG_OP_TYPE_TARGET_BEGIN)
    {
      target->state = malloc(sizeof(*target->state));
      fopen_s(&target->state->txt,"hs_debug.txt","wb");
    }

  hsg_target_indent(target,depth);

  fprintf(target->state->txt,
          "%s\n",
          hsg_op_type_string[ops->type]);

  if (ops->type == HSG_OP_TYPE_TARGET_END)
    {
      fclose(target->state->txt);
      free(target->state);
    }
}

//
//
//
