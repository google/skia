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

#include <stdint.h>
#include <stdbool.h>

//
//
//

struct svg_doc;

union  path_cmd;
union  raster_cmd;
union  layer_cmd;

//
//
//

struct svg_doc *
svg_doc_parse       (char const * const filename, bool const quiet);

void
svg_doc_rewind      (struct svg_doc * const sd);

void
svg_doc_dispose     (struct svg_doc * const sd);

uint32_t
svg_doc_path_count  (struct svg_doc const * const sd);

uint32_t
svg_doc_raster_count(struct svg_doc const * const sd);

uint32_t
svg_doc_layer_count (struct svg_doc const * const sd);

bool
svg_doc_path_next   (struct svg_doc * const sd, union path_cmd * * cmd);

bool
svg_doc_raster_next (struct svg_doc * const sd, union raster_cmd * * cmd);

bool
svg_doc_layer_next  (struct svg_doc * const sd, union layer_cmd * * cmd);

//
//
//

typedef uint32_t svg_color_t;

//
// PATH COMMAND TYPES
//

typedef enum path_cmd_type {
  //
  PATH_CMD_BEGIN,
  PATH_CMD_END,

  //
  PATH_CMD_CIRCLE,
  PATH_CMD_ELLIPSE,
  PATH_CMD_LINE,
  PATH_CMD_POLYGON,
  PATH_CMD_POLYLINE,
  PATH_CMD_RECT,

  // POLY POINT
  PATH_CMD_POLY_POINT,
  PATH_CMD_POLY_END,

  // PATH COMMANDS
  PATH_CMD_PATH_BEGIN,
  PATH_CMD_PATH_END,

  PATH_CMD_MOVE_TO,
  PATH_CMD_MOVE_TO_REL,

  PATH_CMD_CLOSE_UPPER,
  PATH_CMD_CLOSE,

  PATH_CMD_LINE_TO,
  PATH_CMD_LINE_TO_REL,

  PATH_CMD_HLINE_TO,
  PATH_CMD_HLINE_TO_REL,

  PATH_CMD_VLINE_TO,
  PATH_CMD_VLINE_TO_REL,

  PATH_CMD_CUBIC_TO,
  PATH_CMD_CUBIC_TO_REL,

  PATH_CMD_CUBIC_SMOOTH_TO,
  PATH_CMD_CUBIC_SMOOTH_TO_REL,

  PATH_CMD_QUAD_TO,
  PATH_CMD_QUAD_TO_REL,

  PATH_CMD_QUAD_SMOOTH_TO,
  PATH_CMD_QUAD_SMOOTH_TO_REL,

  PATH_CMD_ARC_TO,
  PATH_CMD_ARC_TO_REL,
} path_cmd_type;

//
// PATH COMMAND STRUCTS
//

struct path_point { float x; float y; };

//

struct path_cmd_begin      { path_cmd_type type;                                         };
struct path_cmd_end        { path_cmd_type type; uint32_t path_index;                    };

struct path_cmd_circle     { path_cmd_type type; float cx; float cy; float r;            };
struct path_cmd_ellipse    { path_cmd_type type; float cx; float cy; float rx; float ry; };

struct path_cmd_line       { path_cmd_type type; float x1; float y1; float x2; float y2; };

struct path_cmd_polygon    { path_cmd_type type;                                         };
struct path_cmd_polyline   { path_cmd_type type;                                         };

struct path_cmd_poly_point { path_cmd_type type; float x; float y;                       };
struct path_cmd_poly_end   { path_cmd_type type;                                         };

struct path_cmd_rect
{
  path_cmd_type type;
  float         x;
  float         y;
  float         width;
  float         height;
  float         rx;
  float         ry;
};

struct path_cmd_path_begin { path_cmd_type type; };
struct path_cmd_path_end   { path_cmd_type type; };

struct path_cmd_move_to    { path_cmd_type type; float x; float y; };
struct path_cmd_close      { path_cmd_type type; };

struct path_cmd_line_to    { path_cmd_type type; float x; float y; };

struct path_cmd_coord_to   { path_cmd_type type; float c; };
struct path_cmd_hline_to   { path_cmd_type type; float c; };
struct path_cmd_vline_to   { path_cmd_type type; float c; };

struct path_cmd_cubic_to
{
  path_cmd_type type;
  float x1; float y1;
  float x2; float y2;
  float x;  float y;
};

struct path_cmd_cubic_smooth_to
{
  path_cmd_type type;
  float x2; float y2;
  float x;  float y;
};

struct path_cmd_quad_to
{
  path_cmd_type type;
  float x1; float y1;
  float x;  float y;
};

struct path_cmd_quad_smooth_to
{
  path_cmd_type type;
  float x;  float y;
};

struct path_cmd_arc_to
{
  path_cmd_type type;
  float rx; float ry;
  float x_axis_rotation;
  float large_arc_flag;
  float sweep_flag;
  float x;  float y;
};

//
//
//

union path_cmd
{
  path_cmd_type                   type;

  struct path_cmd_begin           begin;
  struct path_cmd_end             end;

  struct path_cmd_circle          circle;
  struct path_cmd_ellipse         ellipse;
  struct path_cmd_line            line;

  struct path_cmd_polygon         polygon;
  struct path_cmd_polyline        polyline;

  struct path_cmd_poly_point      poly_point;

  struct path_cmd_rect            rect;

  struct path_cmd_path_begin      path_begin;
  struct path_cmd_path_end        path_end;

  struct path_cmd_move_to         move_to;
  struct path_cmd_close           close;

  struct path_cmd_line_to         line_to;
  struct path_cmd_hline_to        hline_to;
  struct path_cmd_vline_to        vline_to;

  struct path_cmd_cubic_to        cubic_to;
  struct path_cmd_cubic_smooth_to cubic_smooth_to;

  struct path_cmd_quad_to         quad_to;
  struct path_cmd_quad_smooth_to  quad_smooth_to;

  struct path_cmd_arc_to          arc_to;
};

//
// RASTER COMMAND TYPES
//

typedef enum raster_cmd_type {

  //
  RASTER_CMD_BEGIN,
  RASTER_CMD_END,

  // RASTERIZE PATH
  RASTER_CMD_FILL,
  RASTER_CMD_STROKE,
  RASTER_CMD_MARKER,

  //
  RASTER_CMD_STROKE_WIDTH,

  // TRANSFORM PATH BEFORE RASTERIZING
  RASTER_CMD_TRANSFORM_MATRIX,    // 6 float
  RASTER_CMD_TRANSFORM_TRANSLATE, // 2 float
  RASTER_CMD_TRANSFORM_SCALE,     // 2 float
  RASTER_CMD_TRANSFORM_ROTATE,    // 3 float
  RASTER_CMD_TRANSFORM_SKEW_X,    // 1 float
  RASTER_CMD_TRANSFORM_SKEW_Y,    // 1 float

  // DROP TRANSFORM FROM HOST'S TRANSFORM STACK
  RASTER_CMD_TRANSFORM_DROP

} raster_cmd_type;

//
// RASTER COMMAND STRUCTS
//

struct raster_cmd_begin         { raster_cmd_type type;                              };
struct raster_cmd_end           { raster_cmd_type type; uint32_t raster_index;       };

struct raster_cmd_fsm           { raster_cmd_type type; uint32_t path_index;         };
struct raster_cmd_fill          { raster_cmd_type type; uint32_t path_index;         };
struct raster_cmd_stroke        { raster_cmd_type type; uint32_t path_index;         };
struct raster_cmd_marker        { raster_cmd_type type; uint32_t path_index;         };

struct raster_cmd_stroke_width  { raster_cmd_type type; float stroke_width;          };

struct raster_cmd_transform_matrix
{
  raster_cmd_type type;
  float sx;  float shy;
  float shx; float sy;
  float tx;  float ty;
};

struct raster_cmd_transform_translate { raster_cmd_type type;          float tx; float ty; };
struct raster_cmd_transform_scale     { raster_cmd_type type;          float sx; float sy; };
struct raster_cmd_transform_rotate    { raster_cmd_type type; float d; float cx; float cy; };
struct raster_cmd_transform_skew_x    { raster_cmd_type type; float d;                     };
struct raster_cmd_transform_skew_y    { raster_cmd_type type; float d;                     };

struct raster_cmd_transform_drop      { raster_cmd_type type;                              };

//
//
//

union raster_cmd
{
  raster_cmd_type                          type;

  struct raster_cmd_begin                  begin;
  struct raster_cmd_end                    end;

  struct raster_cmd_fsm                    fsm;
  struct raster_cmd_fill                   fill;
  struct raster_cmd_stroke                 stroke;
  struct raster_cmd_marker                 marker;

  struct raster_cmd_stroke_width           stroke_width; // stroke_* attributes

  struct raster_cmd_transform_matrix       matrix;

  struct raster_cmd_transform_translate    translate;
  struct raster_cmd_transform_scale        scale;
  struct raster_cmd_transform_rotate       rotate;
  struct raster_cmd_transform_skew_x       skew_x;
  struct raster_cmd_transform_skew_y       skew_y;

  struct raster_cmd_transform_drop         drop;
};

//
// LAYER COMMAND TYPES
//

typedef enum fill_rule_op {
  FILL_RULE_OP_EVENODD,
  FILL_RULE_OP_NONZERO
} fill_rule_op;

//

typedef enum layer_cmd_type {

  //
  LAYER_CMD_BEGIN,
  LAYER_CMD_END,

  // PLACE RASTER ON LAYER
  LAYER_CMD_PLACE, // { rasterId index tx ty }

  // LAYER PAINT SETTINGS
  LAYER_CMD_OPACITY,

  LAYER_CMD_FILL_RULE,
  LAYER_CMD_FILL_COLOR,
  LAYER_CMD_FILL_OPACITY,

  LAYER_CMD_STROKE_COLOR,
  LAYER_CMD_STROKE_OPACITY,

} layer_cmd_type;

//
// LAYER COMMAND STRUCTS
//

struct layer_cmd_begin { layer_cmd_type type; uint32_t layer_index;    };
struct layer_cmd_end   { layer_cmd_type type;                          };

struct layer_cmd_place
{
  layer_cmd_type type;
  uint32_t       raster_index;
  int            tx;
  int            ty;
};

struct layer_cmd_opacity        { layer_cmd_type type; float        opacity;        };

struct layer_cmd_fill_rule      { layer_cmd_type type; fill_rule_op fill_rule;      };
struct layer_cmd_fill_color     { layer_cmd_type type; svg_color_t  fill_color;     };
struct layer_cmd_fill_opacity   { layer_cmd_type type; float        fill_opacity;   };

struct layer_cmd_stroke_color   { layer_cmd_type type; svg_color_t  stroke_color;   };
struct layer_cmd_stroke_opacity { layer_cmd_type type; float        stroke_opacity; };

//
//
//

union layer_cmd
{
  layer_cmd_type                   type;

  struct layer_cmd_begin           begin;
  struct layer_cmd_end             end;

  struct layer_cmd_place           place;

  struct layer_cmd_opacity         opacity;

  struct layer_cmd_fill_rule       fill_rule;
  struct layer_cmd_fill_color      fill_color;
  struct layer_cmd_fill_opacity    fill_opacity;

  struct layer_cmd_stroke_color    stroke_color;
  struct layer_cmd_stroke_opacity  stroke_opacity;
};

//
//
//
