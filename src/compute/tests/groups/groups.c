/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
//
//

#include <stdio.h>
#include <stdlib.h>

#include "color/color.h"
#include "ts/transform_stack.h"

#include "groups.h"

/*
Group {
    children: [
        Layer {
            rasters: [
                SkcRaster {
                    private: 2147745810
                }
            ],
            cmds: [
                CoverNonzero,
                CoverWipMoveToMask
            ]
        },
        Layer {
            rasters: [
                SkcRaster {
                    private: 2147745805
                }
            ],
            cmds: [
                CoverNonzero,
                CoverMask,
                FillColor(
                    [
                        1.0,
                        1.0,
                        1.0,
                        1.0
                    ]
                ),
                BlendOver
            ]
        }
    ],
    enter_cmds: [
        CoverMaskZero
    ],
    leave_cmds: []
}
*/

static bool is_mask_invertible = false;

void
groups_toggle()
{
  is_mask_invertible = !is_mask_invertible;
}

//
//
//

skc_path_t *
groups_paths_decode(skc_path_builder_t pb)
{
  skc_path_t * paths = malloc(sizeof(*paths) * 6);

  for (uint32_t ii=0; ii<3; ii++)
    {
      float const xy = (float)ii * 2.0f + 2.0f;

      skc_path_begin(pb);
      skc_path_ellipse(pb,xy,xy,0.5f,1.5f);
      skc_path_end(pb,paths+ii*2);

      skc_path_begin(pb);
      skc_path_ellipse(pb,xy,xy,1.0f,1.0f);
      skc_path_end(pb,paths+ii*2+1);
    }

  return paths;
}

void
groups_paths_release(skc_context_t      context,
                     skc_path_t * const paths)
{
  skc_path_release(context,paths,6);

  free(paths);
}

//
//
//

skc_raster_t *
groups_rasters_decode(struct ts_transform_stack * const ts,
                      skc_path_t const          * const paths,
                      skc_raster_builder_t              rb)
{
  static float const raster_clip[] = { 0.0f, 0.0f, 0.0f, 0.0f };

  skc_raster_t * rasters = malloc(sizeof(*rasters) * 6);

  uint32_t const ts_restore = ts_transform_stack_save(ts);

  ts_transform_stack_push_translate(ts,800,800);
  ts_transform_stack_concat(ts);

  for (uint32_t ii=0; ii<6; ii++)
    {
      skc_raster_begin(rb);
      skc_raster_add_filled(rb,
                            paths[ii],
                            ts_transform_stack_top_weakref(ts),
                            ts_transform_stack_top_transform(ts),
                            NULL,
                            raster_clip);
      skc_raster_end(rb,rasters+ii);
    }

  // restore stack depth
  ts_transform_stack_restore(ts,ts_restore);

  return rasters;
}

void
groups_rasters_release(skc_context_t        context,
                       skc_raster_t * const rasters)
{
  skc_raster_release(context,rasters,6);

  free(rasters);
}

//
//
//

void
groups_rewind()
{
  ;
}

//
//
//

uint32_t
groups_layer_count()
{
  return 6;
}

void
groups_layers_decode(skc_raster_t const * const rasters,
                     skc_composition_t          composition,
                     skc_styling_t              styling,
                     bool const                 is_srgb)
{
  skc_group_id group_ids[2];

  {
    skc_styling_group_alloc(styling,group_ids+0);

    // this is the root group
    skc_styling_group_parents(styling,group_ids[0],0,NULL);

    // the range of the root group is maximal
    skc_styling_group_range_lo(styling,group_ids[0],0);
    skc_styling_group_range_hi(styling,group_ids[0],5);

    skc_styling_group_enter(styling,
                            group_ids[0],
                            1,
                            (skc_styling_cmd_t[])
                            { SKC_STYLING_OPCODE_COLOR_ACC_ZERO |
                              SKC_STYLING_OPCODE_IS_FINAL });

    skc_styling_cmd_t cmds[3 + 1];

    float background[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    skc_styling_background_over_encoder(cmds,background);

    cmds[3] = SKC_STYLING_OPCODE_SURFACE_COMPOSITE | SKC_STYLING_OPCODE_IS_FINAL;

    skc_styling_group_leave(styling,
                            group_ids[0],
                            SKC_STYLING_CMDS(cmds));
  }

  //
  // for all layers...
  //
  for (uint32_t ii=0; ii<3; ii++)
    {
      skc_styling_group_alloc(styling,group_ids+1);

      // this is the root group
      skc_styling_group_parents(styling,group_ids[1],1,group_ids);

      // the range of the root group is maximal
      skc_styling_group_range_lo(styling,group_ids[1],ii*2);
      skc_styling_group_range_hi(styling,group_ids[1],ii*2+1);

      skc_styling_group_enter(styling,
                              group_ids[1],
                              1,
                              (skc_styling_cmd_t[])
                              { is_mask_invertible
                                  ? (SKC_STYLING_OPCODE_COVER_MASK_ONE  | SKC_STYLING_OPCODE_IS_FINAL)
                                  : (SKC_STYLING_OPCODE_COVER_MASK_ZERO | SKC_STYLING_OPCODE_IS_FINAL) });

      skc_styling_group_leave(styling,
                              group_ids[1],
                              1,
                              (skc_styling_cmd_t[])
                              { SKC_STYLING_OPCODE_NOOP |
                                SKC_STYLING_OPCODE_IS_FINAL });

      //
      //
      //

      {
        float const rgba[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

        skc_styling_cmd_t cmds[1 + 1 + 1];

        cmds[0] = SKC_STYLING_OPCODE_COVER_NONZERO;

        if (is_mask_invertible)
          {
            cmds[1] = SKC_STYLING_OPCODE_COVER_WIP_MOVE_TO_MASK;
            cmds[2] = SKC_STYLING_OPCODE_COVER_MASK_INVERT | SKC_STYLING_OPCODE_IS_FINAL;
          }
        else
          {
            cmds[1] = SKC_STYLING_OPCODE_COVER_WIP_MOVE_TO_MASK | SKC_STYLING_OPCODE_IS_FINAL;
          }

        skc_styling_group_layer(styling,
                                group_ids[1],
                                ii*2,
                                _countof(cmds),cmds);
      }
      {
        float rgba[4];

        color_rgb32_to_rgba_f32(rgba,0xFF<<ii*8,1.0f);

        skc_styling_cmd_t cmds[1 + 1 + 3 + 1];

        cmds[0]                = SKC_STYLING_OPCODE_COVER_NONZERO;
        cmds[1]                = SKC_STYLING_OPCODE_COVER_MASK;

        skc_styling_layer_fill_rgba_encoder(cmds+2,rgba);

        cmds[_countof(cmds)-1] = SKC_STYLING_OPCODE_BLEND_OVER | SKC_STYLING_OPCODE_IS_FINAL;

        skc_styling_group_layer(styling,
                                group_ids[1],
                                ii*2+1,
                                _countof(cmds),cmds);
      }
    }

  //
  // for all rasters...
  //
  for (uint32_t ii=0; ii<6; ii++)
    {
      skc_composition_place(composition,
                            rasters+ii,
                            &ii,
                            NULL, // &cmd->place.tx,
                            NULL, // &cmd->place.ty,
                            1);
    }
}

//
//
//
