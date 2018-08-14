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
//#include <malloc.h>

//#include <color.h>
//#include <skc_x16.h>

#include <color/color.h>

#include "transform_stack.h"
#include "svg2skc.h"

//
//
//

#ifndef M_PI

#define M_PI 3.14159265358979323846

#endif

//
//
//

void
svg_doc_paths_release(struct svg_doc * const sd, 
                      skc_path_t     * const paths,
                      skc_context_t          context)
{
  uint32_t const n = svg_doc_path_count(sd);

  skc_path_release(context,paths,n);

  free(paths);
}


void
svg_doc_rasters_release(struct svg_doc * const sd, 
                        skc_raster_t   * const rasters,
                        skc_context_t          context)
{
  uint32_t const n = svg_doc_raster_count(sd);

  skc_raster_release(context,rasters,n);

  free(rasters);
}

//
//
//

void
svg_doc_paths_flush(struct svg_doc * const sd, 
                    skc_path_t     * const paths,
                    skc_context_t          context)
{
  uint32_t const n = svg_doc_path_count(sd);

  skc_path_flush(context,paths,n);
}


void
svg_doc_rasters_flush(struct svg_doc * const sd, 
                      skc_raster_t   * const rasters,
                      skc_context_t          context)
{
  uint32_t const n = svg_doc_raster_count(sd);

  skc_raster_flush(context,rasters,n);
}

//
//
//

static
void
svg_doc_poly_read(struct svg_doc * sd, skc_path_builder_t pb, bool const close)
{
  union path_cmd* cmd;
  float           x0,y0;
  bool            lineto = false;

  while (svg_doc_path_next(sd,&cmd))
    {
      if (cmd->type != PATH_CMD_POLY_POINT)
        break;

      if (lineto)
        {
          skc_path_line_to(pb,cmd->poly_point.x,cmd->poly_point.y);
        }
      else
        {
          skc_path_move_to(pb,x0=cmd->poly_point.x,y0=cmd->poly_point.y);
          lineto = true;
        }
    }

  if (close && lineto)
    skc_path_line_to(pb,x0,y0);
}

//
//
//

static
void
svg_implicit_close_filled_path(skc_path_builder_t pb, float x0, float y0, float x, float y)
{
  if ((x != x0) || (y != y0))
    skc_path_line_to(pb,x0,y0);
}

//
//
//

#ifdef SKC_USE_ELLIPTICAL_ARC

#include <elliptical_arc.h>

static 
void
svg_doc_arc_decode(bool const                           is_relative, 
                   struct path_cmd_arc_to const * const cmd_arc_to, 
                   float                        * const x,
                   float                        * const y,
                   skc_path_builder_t                   pb) 
{
  float x1 = cmd_arc_to->x;
  float y1 = cmd_arc_to->y;

  if (is_relative)
    {
      x1 += *x;
      y1 += *y;
    }

  struct maisonobe_elliptical_arc mea;

  maisonobe_elliptical_arc_init(&mea,
                                (double)*x,
                                (double)*y,
                                (double)x1,
                                (double)y1,
                                (double)cmd_arc_to->rx,
                                (double)cmd_arc_to->ry,
                                (double)cmd_arc_to->x_axis_rotation,
                                cmd_arc_to->large_arc_flag != 0.0f,
                                cmd_arc_to->sweep_flag     != 0.0f);

  if (mea.n == 0)
    {
      skc_path_line_to(pb,x1,y1);
    }
  else
    {
      float* cp = _alloca(sizeof(float)*(mea.n*6+2));

      maisonobe_iterate(&mea,cp+2);

      if (mea.ccw)
        {
          cp[0] = x1;
          cp[1] = y1;
          cp   += (mea.n - 1) * 6;
                    
          for (; mea.n > 0; mea.n--,cp-=6)
            {
              skc_path_cubic_to(pb,
                                cp[4],cp[5],
                                cp[2],cp[3],
                                cp[0],cp[1]);
            }
        }
      else
        {
          cp[mea.n*6+0] = x1;
          cp[mea.n*6+1] = y1;
          cp           += 2;
                    
          for (; mea.n > 0; mea.n--,cp+=6)
            {
              skc_path_cubic_to(pb,
                                cp[0],cp[1],
                                cp[2],cp[3],
                                cp[4],cp[5]);
            }
        }
    }

  *x = x1;
  *y = y1;
}

#endif

//
//
//

skc_path_t *
svg_doc_paths_decode(struct svg_doc * const sd, skc_path_builder_t pb)
{
  skc_path_t* const paths = malloc(sizeof(*paths) * svg_doc_path_count(sd));

  union path_cmd* cmd;

  float x0,y0,x,y;

  while (svg_doc_path_next(sd,&cmd))
    {
      switch (cmd->type)
        {
        case PATH_CMD_BEGIN:
          skc_path_begin(pb);
          break;

        case PATH_CMD_END:
          skc_path_end(pb,paths+cmd->end.path_index);
          break;

        case PATH_CMD_CIRCLE:
          skc_path_ellipse(pb,
                           cmd->circle.cx,cmd->circle.cy,
                           cmd->circle.r, cmd->circle.r);
          break;

        case PATH_CMD_ELLIPSE:
          skc_path_ellipse(pb,
                           cmd->ellipse.cx,cmd->ellipse.cy,
                           cmd->ellipse.rx,cmd->ellipse.ry);
          break;

        case PATH_CMD_LINE:
          skc_path_move_to(pb,cmd->line.x1,cmd->line.y1);
          skc_path_line_to(pb,cmd->line.x2,cmd->line.y2);
          break;

        case PATH_CMD_POLYGON:
          svg_doc_poly_read(sd,pb,true);
          break;

        case PATH_CMD_POLYLINE:
          svg_doc_poly_read(sd,pb,false);
          break;

        case PATH_CMD_RECT:
          skc_path_move_to(pb,cmd->rect.x,                cmd->rect.y);
          skc_path_line_to(pb,cmd->rect.x+cmd->rect.width,cmd->rect.y);
          skc_path_line_to(pb,cmd->rect.x+cmd->rect.width,cmd->rect.y+cmd->rect.height);
          skc_path_line_to(pb,cmd->rect.x                ,cmd->rect.y+cmd->rect.height);
          skc_path_line_to(pb,cmd->rect.x                ,cmd->rect.y);
          break;

        case PATH_CMD_PATH_BEGIN:
          x = x0 = 0.0f;
          y = y0 = 0.0f;
          break;

        case PATH_CMD_PATH_END:
          svg_implicit_close_filled_path(pb,x0,y0,x,y);
          break;

        case PATH_CMD_MOVE_TO:
          svg_implicit_close_filled_path(pb,x0,y0,x,y);
          skc_path_move_to(pb,x0=x=cmd->move_to.x,y0=y=cmd->move_to.y);
          break;

        case PATH_CMD_MOVE_TO_REL:
          svg_implicit_close_filled_path(pb,x0,y0,x,y);
          skc_path_move_to(pb,x0=(x+=cmd->move_to.x),y0=(y+=cmd->move_to.y));
          break;

        case PATH_CMD_CLOSE_UPPER:
        case PATH_CMD_CLOSE:
          svg_implicit_close_filled_path(pb,x0,y0,x,y);
          skc_path_move_to(pb,x=x0,y=y0); // reset to initial point
          break;

        case PATH_CMD_LINE_TO:
          skc_path_line_to(pb,x=cmd->line_to.x,y=cmd->line_to.y);
          break;

        case PATH_CMD_LINE_TO_REL:
          skc_path_line_to(pb,x+=cmd->line_to.x,y+=cmd->line_to.y);
          break;

        case PATH_CMD_HLINE_TO:
          skc_path_line_to(pb,x=cmd->hline_to.c,y);
          break;

        case PATH_CMD_HLINE_TO_REL:
          skc_path_line_to(pb,x+=cmd->hline_to.c,y);
          break;

        case PATH_CMD_VLINE_TO:
          skc_path_line_to(pb,x,y=cmd->vline_to.c);
          break;

        case PATH_CMD_VLINE_TO_REL:
          skc_path_line_to(pb,x,y+=cmd->vline_to.c);
          break;

        case PATH_CMD_CUBIC_TO:
          skc_path_cubic_to(pb,
                            cmd->cubic_to.x1, cmd->cubic_to.y1,
                            cmd->cubic_to.x2, cmd->cubic_to.y2,
                            x=cmd->cubic_to.x,y=cmd->cubic_to.y);
          break;

        case PATH_CMD_CUBIC_TO_REL:
          skc_path_cubic_to(pb,
                            x+cmd->cubic_to.x1,y+cmd->cubic_to.y1,
                            x+cmd->cubic_to.x2,y+cmd->cubic_to.y2,
                            x+cmd->cubic_to.x, y+cmd->cubic_to.y);
          x+=cmd->cubic_to.x;
          y+=cmd->cubic_to.y;
          break;

        case PATH_CMD_CUBIC_SMOOTH_TO:
          skc_path_cubic_smooth_to(pb,
                                   cmd->cubic_smooth_to.x2, cmd->cubic_smooth_to.y2,
                                   x=cmd->cubic_smooth_to.x,y=cmd->cubic_smooth_to.y);
          break;

        case PATH_CMD_CUBIC_SMOOTH_TO_REL:
          skc_path_cubic_smooth_to(pb,
                                   x+cmd->cubic_smooth_to.x2,y+cmd->cubic_smooth_to.y2,
                                   x+cmd->cubic_smooth_to.x, y+cmd->cubic_smooth_to.y);
          x+=cmd->cubic_smooth_to.x;
          y+=cmd->cubic_smooth_to.y;
          break;

        case PATH_CMD_QUAD_TO:
          skc_path_quad_to(pb,
                           cmd->quad_to.x1, cmd->quad_to.y1,
                           x=cmd->quad_to.x,y=cmd->quad_to.y);
          break;

        case PATH_CMD_QUAD_TO_REL:
          skc_path_quad_to(pb,
                           x+cmd->quad_to.x1,y+cmd->quad_to.y1,
                           x+cmd->quad_to.x, y+cmd->quad_to.y);
          x+=cmd->quad_to.x;
          y+=cmd->quad_to.y;
          break;

        case PATH_CMD_QUAD_SMOOTH_TO:
          skc_path_quad_smooth_to(pb,
                                  x=cmd->quad_smooth_to.x,y=cmd->quad_smooth_to.y);
          break;

        case PATH_CMD_QUAD_SMOOTH_TO_REL:
          skc_path_quad_smooth_to(pb,
                                  x+cmd->quad_smooth_to.x,y+cmd->quad_smooth_to.y);
          x+=cmd->quad_smooth_to.x;
          y+=cmd->quad_smooth_to.y;
          break;

        case PATH_CMD_ARC_TO:
#if SKC_USE_ELLIPTICAL_ARC
          svg_doc_arc_decode(false,&cmd->arc_to,&x,&y,pb);          
#else
          skc_path_line_to(pb,cmd->arc_to.x,cmd->arc_to.y);
          fprintf(stderr,"arc_to() support disabled\n");
#endif
          break;

        case PATH_CMD_ARC_TO_REL:
#if SKC_USE_ELLIPTICAL_ARC
          svg_doc_arc_decode(true,&cmd->arc_to,&x,&y,pb);
#else
          skc_path_line_to(pb,x+cmd->arc_to.x,y+cmd->arc_to.y);
          fprintf(stderr,"arc_to_rel() support disabled\n");
#endif
          break;

        default:
          fprintf(stderr,"unhandled path type: %u\n",cmd->type);
          exit(-1);
        }
    }

  return paths;
}

//
//
//

skc_raster_t *
svg_doc_rasters_decode(struct svg_doc             * const sd, 
                       struct skc_transform_stack * const ts,
                       skc_path_t const           * const paths, 
                       skc_raster_builder_t               rb)
{
  static float const raster_clip[] = { 0.0f, 0.0f, 0.0f, 0.0f };

  skc_path_t * const rasters    = malloc(sizeof(*rasters) * svg_doc_raster_count(sd));
  uint32_t     const ts_restore = skc_transform_stack_save(ts);

  union raster_cmd* cmd;

  while (svg_doc_raster_next(sd,&cmd))
    {
      switch (cmd->type)
        {
        case RASTER_CMD_BEGIN:
          skc_raster_begin(rb);
          break;

        case RASTER_CMD_END:
          skc_raster_end(rb,rasters+cmd->end.raster_index);
          break;

        case RASTER_CMD_FILL:
          skc_raster_add_filled(rb,
                                paths[cmd->fill.path_index],
                                skc_transform_stack_top_weakref(ts),
                                skc_transform_stack_top_transform(ts),
                                NULL,
                                raster_clip);
          break;

        case RASTER_CMD_STROKE:       // FIXME -- IGNORED
          break;

        case RASTER_CMD_MARKER:       // FIXME -- IGNORED
          break;

        case RASTER_CMD_STROKE_WIDTH: // FIXME -- IGNORED
          break;

        case RASTER_CMD_TRANSFORM_MATRIX:
          skc_transform_stack_push_affine(ts,
                                          cmd->matrix.sx, cmd->matrix.shx,cmd->matrix.tx,
                                          cmd->matrix.shy,cmd->matrix.sy, cmd->matrix.ty);
          skc_transform_stack_concat(ts);
          break;

        case RASTER_CMD_TRANSFORM_TRANSLATE:
          skc_transform_stack_push_translate(ts,
                                             cmd->translate.tx,cmd->translate.ty);
          skc_transform_stack_concat(ts);
          break;

        case RASTER_CMD_TRANSFORM_SCALE:
          skc_transform_stack_push_scale(ts,
                                         cmd->scale.sx,cmd->scale.sy);
          skc_transform_stack_concat(ts);
          break;

        case RASTER_CMD_TRANSFORM_ROTATE:
          skc_transform_stack_push_rotate_xy(ts,
                                             cmd->rotate.d * (float)(M_PI / 180.0),
                                             cmd->rotate.cx,cmd->rotate.cy);
          skc_transform_stack_concat(ts);
          break;

        case RASTER_CMD_TRANSFORM_SKEW_X:
          skc_transform_stack_push_skew_x(ts,
                                          cmd->skew_x.d);
          skc_transform_stack_concat(ts);
          break;

        case RASTER_CMD_TRANSFORM_SKEW_Y:
          skc_transform_stack_push_skew_y(ts,
                                          cmd->skew_y.d);
          skc_transform_stack_concat(ts);
          break;

        case RASTER_CMD_TRANSFORM_DROP:
          skc_transform_stack_drop(ts);
          break;
        }
    }

  // restore stack depth
  skc_transform_stack_restore(ts,ts_restore);

  return rasters;
}

//
//
//

void
skc_styling_background_over_encoder(skc_styling_cmd_t * const cmds,
                                    float               const colors[]);

//
//
//

void
svg_doc_layers_decode(struct svg_doc     * const sd,
                      skc_raster_t const * const rasters, 
                      skc_composition_t          composition, 
                      skc_styling_t              styling,
                      bool const                 is_srgb)
{
  //
  //
  //
  
  skc_group_id group_id;

  skc_styling_group_alloc(styling,&group_id);

  skc_styling_group_enter(styling,
                          group_id,
                          1,
                          (skc_styling_cmd_t[])
                          { SKC_STYLING_OPCODE_COLOR_ACC_ZERO | SKC_STYLING_OPCODE_IS_FINAL });

  skc_styling_group_parents(styling,
                            group_id,
                            1,
                            (skc_group_id[]){ group_id });
  
  skc_styling_group_range_lo(styling,group_id,0);
  skc_styling_group_range_hi(styling,group_id,(1<<18)-1);

  //
  //
  //

  union layer_cmd *    cmd;

  skc_layer_id         layer_id;

  skc_styling_cmd_t    fill_rule    = SKC_STYLING_OPCODE_COVER_NONZERO;
  skc_styling_cmd_t    blend_mode   = SKC_STYLING_OPCODE_BLEND_OVER;

  svg_color_t          rgb          = 0;
  float                opacity      = 1.0f;
  float                fill_opacity = 1.0f;

  uint32_t const       layer_count  = svg_doc_layer_count(sd);

  while (svg_doc_layer_next(sd,&cmd))
    {
      switch (cmd->type)
        {
        case LAYER_CMD_BEGIN:
          layer_id = layer_count - 1 - cmd->begin.layer_index;
          break;

        case LAYER_CMD_END:
           {
            float rgba[4];

            color_rgb32_to_rgba_f32(rgba,rgb,fill_opacity * opacity);

            if (is_srgb)
              color_srgb_to_linear_rgb_f32(rgba);

            color_premultiply_rgba_f32(rgba);

            skc_styling_cmd_t cmds[1 + 3 + 1];

            cmds[0]                = fill_rule;
            cmds[_countof(cmds)-1] = SKC_STYLING_OPCODE_BLEND_OVER | SKC_STYLING_OPCODE_IS_FINAL;
            
            skc_styling_layer_fill_rgba_encoder(cmds+1,rgba);

            skc_styling_group_layer(styling,
                                    group_id,
                                    layer_id,
                                    _countof(cmds),cmds);
          }
          break;

        case LAYER_CMD_PLACE:
          skc_composition_place(composition,
                                rasters+cmd->place.raster_index,
                                &layer_id,
                                NULL, // &cmd->place.tx,
                                NULL, // &cmd->place.ty,
                                1);
          // 16,-16);
          break;

        case LAYER_CMD_OPACITY:
          opacity = cmd->opacity.opacity;
          break;

        case LAYER_CMD_FILL_RULE:
          fill_rule = (cmd->fill_rule.fill_rule == FILL_RULE_OP_NONZERO)
            ? SKC_STYLING_OPCODE_COVER_NONZERO
            : SKC_STYLING_OPCODE_COVER_EVENODD;
          break;

        case LAYER_CMD_FILL_COLOR:
          rgb = cmd->fill_color.fill_color;
          break;

        case LAYER_CMD_FILL_OPACITY:
          fill_opacity = cmd->fill_opacity.fill_opacity;
          break;

        case LAYER_CMD_STROKE_COLOR:   // FIXME -- IGNORED
          break;

        case LAYER_CMD_STROKE_OPACITY: // FIXME -- IGNORED
          break;
        }
    }

  //
  //
  //

  {
    skc_styling_cmd_t cmds[3 + 1];

    float background[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    skc_styling_background_over_encoder(cmds,background);
    
    cmds[3] = SKC_STYLING_OPCODE_SURFACE_COMPOSITE | SKC_STYLING_OPCODE_IS_FINAL;

    skc_styling_group_leave(styling,
                            group_id,
                            SKC_STYLING_CMDS(cmds));
  }

  //
  //
  //
}

//
//
//
