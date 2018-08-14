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

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//
//

#include <yxml.h>

//
//
//

#include "svg_doc.h"
#include "common/macros.h"

//
//
//

static bool message_quiet = false;

//
//                 |  path  | raster | layer  |
// -NAMES----------+--------+--------+--------+
//                 |        |        |        |
// id              |   X    |   X    |   X    |
//                 |        |        |        |
// -CONTAINERS-----+--------+--------+--------+
//                 |        |        |        |
// svg             |   X    |   X    |   X    |
// g               |        |        |        |
//                 |        |        |        |
// -P ELEMENTS-----+--------+--------+--------+
//                 |        |        |        |
// circle          |        |        |        |
// ellipse         |        |        |        |
// line            |        |        |        |
// path            |        |        |        |
// polygon         |        |        |        |
// polyline        |        |        |        |
// rect            |        |        |        |
//                 |        |        |        |
// -P ATTRIBUTES---+--------+--------+--------+
//                 |        |        |        |
// r               |        |        |        | circle
// cx              |        |        |        | circle, ellipse
// cy              |        |        |        | circle, ellipse
// rx              |        |        |        | rect,   ellipse
// ry              |        |        |        | rect,   ellipse
// x               |        |        |        | rect
// y               |        |        |        | rect
// width           |        |        |        | rect
// height          |        |        |        | rect
// x1              |        |        |        | line
// y1              |        |        |        | line
// x2              |        |        |        | line
// y2              |        |        |        | line
//                 |        |        |        |
// d               |        |        |        | path
// points          |        |        |        | polygon, polyline
//                 |        |        |        |
// -R ATTRIBUTES---+--------+--------+--------+
//                 |        |        |        |
// transform    <> |   X    |   X    |   o    |
// fill|stroke  <> |   X    |   X    |   o    | <-- defined by change to either paint-ops or paint-op colors
// stroke-width <> |   X    |   X    |   o    |
//                 |        |        |        |
// -L ATTRIBUTES---+--------+--------+--------+
//                 |        |        |        |
// opacity         |   X    |   X    |   X    |
// fill-rule       |   X    |   X    |   X    |
// fill-color   <> |   X    |   X    |   X    |
// fill-opacity    |   X    |   X    |   X    |
// stroke-color <> |   X    |   X    |   X    |
// stroke-opacity  |   X    |   X    |   X    |
//                 |        |        |        |
// ----------------+--------+--------+--------+
//

/*
  NAME
  - starting indices in dictionaries
  - base indices for path/raster/layer arrays

  PATH
  - create path
  - ( path commands )+
  - seal path @ pathId array index

  RASTER
  - ( push raster attributes )* <-- most likely hanging off of svg/g/* container
  - create raster
  - (
  -   ( push raster attributes               )*
  -   ( add filled/stroke pathId array index )*
  -   ( pop raster attributes                )*
  - )+
  - seal raster @ rasterId array index

  LAYER
  - ( push layer attributes )*  <-- most likely hanging off of svg/g/* container
  - using current layer
  - (
  -   ( push layer attributes      )*
  -   ( place rasterId array index )*
  -   ( pop layer attributes       )*
  - )+
  - increment current layer index

*/

//
// paths only need to be pulled once and can be reused
// only need to split off rasters if rerasterizing
// top-level "use" routine will rerasterize as needed
//
// acquire doc resources
// release doc resources
//
// acquire def resources
// release def resources
//
// render doc
// render def
//
// count paths
// pull all paths
// dispose all paths
// pull path by name
// dispose path by name
//
// count rasters
// rasterize all paths
// dispose all rasters
// rasterize by name
// dispose raster by name
//
// count layers
// acquire all layers
// release all layers
// acquire layer by name
// release layer by name
//

//
// TYPES
//

struct svg_color_name { char * name;  svg_color_t color; };

#define SVG_RGB(r,g,b)   (((r)<<16)|((g)<<8)|(b))

#include "svg_color_names.h"

//
//
//

struct lookup_cmd { char * name; uint32_t size; } lookup_cmd;

#define SVG_LOOKUP_CMD(e,s) { ""#e , sizeof(s) }

static struct lookup_cmd const path_lookup_cmds[] = {
  SVG_LOOKUP_CMD(PATH_CMD_BEGIN                 , struct path_cmd_begin                ),
  SVG_LOOKUP_CMD(PATH_CMD_END                   , struct path_cmd_end                  ),

  SVG_LOOKUP_CMD(PATH_CMD_CIRCLE                , struct path_cmd_circle               ),
  SVG_LOOKUP_CMD(PATH_CMD_ELLIPSE               , struct path_cmd_ellipse              ),
  SVG_LOOKUP_CMD(PATH_CMD_LINE                  , struct path_cmd_line                 ),
  SVG_LOOKUP_CMD(PATH_CMD_POLYGON               , struct path_cmd_polygon              ),
  SVG_LOOKUP_CMD(PATH_CMD_POLYLINE              , struct path_cmd_polyline             ),
  SVG_LOOKUP_CMD(PATH_CMD_RECT                  , struct path_cmd_rect                 ),

  SVG_LOOKUP_CMD(PATH_CMD_POLY_POINT            , struct path_cmd_poly_point           ),
  SVG_LOOKUP_CMD(PATH_CMD_POLY_END              , struct path_cmd_poly_end             ),

  SVG_LOOKUP_CMD(PATH_CMD_PATH_BEGIN            , struct path_cmd_path_begin           ),
  SVG_LOOKUP_CMD(PATH_CMD_PATH_END              , struct path_cmd_path_end             ),

  SVG_LOOKUP_CMD(PATH_CMD_MOVE_TO               , struct path_cmd_move_to              ),
  SVG_LOOKUP_CMD(PATH_CMD_MOVE_TO_REL           , struct path_cmd_move_to              ),
  SVG_LOOKUP_CMD(PATH_CMD_CLOSE_UPPER           , struct path_cmd_close                ),
  SVG_LOOKUP_CMD(PATH_CMD_CLOSE                 , struct path_cmd_close                ),
  SVG_LOOKUP_CMD(PATH_CMD_LINE_TO               , struct path_cmd_line_to              ),
  SVG_LOOKUP_CMD(PATH_CMD_LINE_TO_REL           , struct path_cmd_line_to              ),
  SVG_LOOKUP_CMD(PATH_CMD_HLINE_TO              , struct path_cmd_hline_to             ),
  SVG_LOOKUP_CMD(PATH_CMD_HLINE_TO_REL          , struct path_cmd_hline_to             ),
  SVG_LOOKUP_CMD(PATH_CMD_VLINE_TO              , struct path_cmd_vline_to             ),
  SVG_LOOKUP_CMD(PATH_CMD_VLINE_TO_REL          , struct path_cmd_vline_to             ),
  SVG_LOOKUP_CMD(PATH_CMD_CUBIC_TO              , struct path_cmd_cubic_to             ),
  SVG_LOOKUP_CMD(PATH_CMD_CUBIC_TO_REL          , struct path_cmd_cubic_to             ),
  SVG_LOOKUP_CMD(PATH_CMD_CUBIC_SMOOTH_TO       , struct path_cmd_cubic_smooth_to      ),
  SVG_LOOKUP_CMD(PATH_CMD_CUBIC_SMOOTH_TO_REL   , struct path_cmd_cubic_smooth_to      ),
  SVG_LOOKUP_CMD(PATH_CMD_QUAD_TO               , struct path_cmd_quad_to              ),
  SVG_LOOKUP_CMD(PATH_CMD_QUAD_TO_REL           , struct path_cmd_quad_to              ),
  SVG_LOOKUP_CMD(PATH_CMD_QUAD_SMOOTH_TO        , struct path_cmd_quad_smooth_to       ),
  SVG_LOOKUP_CMD(PATH_CMD_QUAD_SMOOTH_TO_REL    , struct path_cmd_quad_smooth_to       ),
  SVG_LOOKUP_CMD(PATH_CMD_ARC_TO                , struct path_cmd_arc_to               ),
  SVG_LOOKUP_CMD(PATH_CMD_ARC_TO_REL            , struct path_cmd_arc_to               )
};

static struct lookup_cmd const raster_lookup_cmds[] = {
  SVG_LOOKUP_CMD(RASTER_CMD_BEGIN               , struct raster_cmd_begin               ),
  SVG_LOOKUP_CMD(RASTER_CMD_END                 , struct raster_cmd_end                 ),

  SVG_LOOKUP_CMD(RASTER_CMD_FILL                , struct raster_cmd_fill                ),
  SVG_LOOKUP_CMD(RASTER_CMD_STROKE              , struct raster_cmd_stroke              ),
  SVG_LOOKUP_CMD(RASTER_CMD_MARKER              , struct raster_cmd_marker              ),

  SVG_LOOKUP_CMD(RASTER_CMD_STROKE_WIDTH        , struct raster_cmd_stroke_width        ),

  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_MATRIX    , struct raster_cmd_transform_matrix    ),
  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_TRANSLATE , struct raster_cmd_transform_translate ),
  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_SCALE     , struct raster_cmd_transform_scale     ),
  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_ROTATE    , struct raster_cmd_transform_rotate    ),
  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_SKEW_X    , struct raster_cmd_transform_skew_x    ),
  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_SKEW_Y    , struct raster_cmd_transform_skew_y    ),
  SVG_LOOKUP_CMD(RASTER_CMD_TRANSFORM_DROP      , struct raster_cmd_transform_drop      )
};

static struct lookup_cmd const layer_lookup_cmds[] = {
  SVG_LOOKUP_CMD(LAYER_CMD_BEGIN                , struct layer_cmd_begin          ),
  SVG_LOOKUP_CMD(LAYER_CMD_END                  , struct layer_cmd_end            ),

  SVG_LOOKUP_CMD(LAYER_CMD_PLACE                , struct layer_cmd_place          ),

  SVG_LOOKUP_CMD(LAYER_CMD_OPACITY              , struct layer_cmd_opacity        ),

  SVG_LOOKUP_CMD(LAYER_CMD_FILL_RULE            , struct layer_cmd_fill_rule      ),
  SVG_LOOKUP_CMD(LAYER_CMD_FILL_COLOR           , struct layer_cmd_fill_color     ),
  SVG_LOOKUP_CMD(LAYER_CMD_FILL_OPACITY         , struct layer_cmd_fill_opacity   ),

  SVG_LOOKUP_CMD(LAYER_CMD_STROKE_COLOR         , struct layer_cmd_stroke_color   ),
  SVG_LOOKUP_CMD(LAYER_CMD_STROKE_OPACITY       , struct layer_cmd_stroke_opacity )
};

//
//
//

typedef enum paint_op {
  PAINT_OP_NONE,
  PAINT_OP_COLOR,
  PAINT_OP_INHERIT
} paint_op;

typedef enum marker_op {
  MARKER_OP_FALSE,        // no arg
  MARKER_OP_TRUE          // no arg
} marker_op;

//
//
//

typedef enum attrib_type {

  //
  // SCALARS
  //
  ATTRIB_TYPE_ELEM_COUNT,

  ATTRIB_TYPE_OPACITY,

  ATTRIB_TYPE_FILL_OP,
  ATTRIB_TYPE_FILL_COLOR,
  ATTRIB_TYPE_FILL_OPACITY,
  ATTRIB_TYPE_FILL_RULE,

  ATTRIB_TYPE_STROKE_OP,
  ATTRIB_TYPE_STROKE_COLOR,
  ATTRIB_TYPE_STROKE_OPACITY,
  ATTRIB_TYPE_STROKE_WIDTH,

  ATTRIB_TYPE_MARKER_OP,
  ATTRIB_TYPE_MARKER_COLOR,

  ATTRIB_TYPE_SCALAR_COUNT,  // NUMBER OF SCALAR ATTRIBS

  //
  // STACKS
  //
  ATTRIB_TYPE_TRANSFORM,     // drop from transform stack
  ATTRIB_TYPE_ID             // drop id stack

} attrib_type;


#define ATTRIB_TYPE_TO_MASK(t)      (1u<<t)

#define ATTRIB_TYPE_TO_MASK_BIT(t)  ATTRIB_TYPE_TO_MASK(t)
#define ATTRIB_TYPE_TO_MASK_OFF(t)  0

//
// STACK STRUCTURE
//

struct stack_entry
{
  uint32_t idx;
  uint32_t len;
};


struct stack
{
  struct stack_entry* entries;
  uint32_t            entry_max;
  uint32_t            entry_count;

  void*               buf;
  uint32_t            buf_max;
  uint32_t            buf_count;
};

//
//
//

typedef void (*on_stack_drop)(void* v, uint32_t len, void* extra);
typedef void (*on_stack_push)(void* v, uint32_t len, void* extra);

//
//
//

static
void
stack_reset(struct stack * s)
{
  s->entry_count = 0;
  s->buf_count   = 0;
}

static
struct stack *
stack_alloc(uint32_t const entry_max, uint32_t const buf_max)
{
  struct stack * s = malloc(sizeof(*s));

  s->entries       = malloc(sizeof(*s->entries) * entry_max);
  s->entry_max     = entry_max;

  s->buf           = malloc(buf_max);
  s->buf_max       = buf_max;

  return s;
}

static
struct stack *
stack_create()
{
  struct stack * s = stack_alloc(4096/sizeof(struct stack_entry),4096);

  stack_reset(s);

  return s;
}

static
void
stack_dispose(struct stack * s)
{
  if (s == NULL)
    return;

  free(s->entries);
  free(s->buf);
  free(s);
}

//
//
//

static
uint32_t
stack_entry_count(struct stack * s)
{
  return s->entry_count;
}

static
uint32_t
stack_buf_count(struct stack * s)
{
  return s->buf_count;
}

/*
  static void
  stack_append(struct stack* to, stack* from)
  {
  //
  // append struct_entry records
  //
  memcpy(to->entries + to->entry_count,
  from->entries,
  from->entry_count * sizeof(struct stack_entry));

  const uint32_t old_count = to->entry_count;

  to->entry_count += from->entry_count;

  //
  // append buf
  //
  memcpy((void*)((uintptr_t)to->buf + to->buf_count),
  from->buf,
  from->buf_count);

  const uint32_t old_end = to->buf_count;

  to->buf_count += from->buf_count;

  //
  // adjust entries
  //
  for (uint32_t ii=old_count; ii<to->entry_count; ii++)
  {
  to->entries[ii].idx += old_end;
  }
  }
*/

//
//
//

static void
stack_ensure(struct stack * s, uint32_t entry_inc, unsigned buf_inc)
{
  uint32_t const new_entry_count = s->entry_count + entry_inc;

  if (new_entry_count > s->entry_max)
    {
      do {
        s->entry_max *= 2;
      } while (new_entry_count > s->entry_max);

      s->entries = (struct stack_entry *)realloc(s->entries,sizeof(struct stack_entry)*s->entry_max);
    }

  uint32_t const new_buf_count = s->buf_count + buf_inc;

  if (new_buf_count > s->buf_max)
    {
      do {
        s->buf_max *= 2;
      } while (new_buf_count > s->buf_max);

      s->buf = realloc(s->buf,s->buf_max);
    }
}

//
//
//

static void
stack_push(struct stack * s, void * v, uint32_t const len)
{
  stack_ensure(s,1,len);

  s->entries[s->entry_count].idx = s->buf_count;
  s->entries[s->entry_count].len = len;

  memcpy((void*)((uintptr_t)s->buf+s->buf_count),v,len);

  s->entry_count += 1;
  s->buf_count   += len;
}

static void
stack_entry_get(struct stack * s, uint32_t idx, void ** v, uint32_t * len)
{
  struct stack_entry* e = s->entries + idx;

  *v   = (void*)((uintptr_t)s->buf + e->idx);
  *len = e->len;
}

static void *
stack_tos(struct stack * s)
{
  struct stack_entry * e = s->entries + s->entry_count - 1;

  return (void*)((uintptr_t)s->buf + e->idx);
}

static void
stack_tos_append(struct stack * s, void * v, uint32_t len)
{
  stack_ensure(s,0,len);

  void* tos = (void*)((uintptr_t)s->buf + s->buf_count);

  memcpy(tos,v,len);

  s->buf_count += len;

  struct stack_entry * e = s->entries + s->entry_count - 1;

  e->len += len;
}

static void
stack_tos_copy(struct stack * to, struct stack * from)
{
  stack_tos_append(to,from->buf,from->buf_count);
}

static bool
stack_entry_not_equal(struct stack * s, struct stack * t, uint32_t idx)
{
  struct stack_entry* s_entry = s->entries + idx;
  struct stack_entry* t_entry = t->entries + idx;

  if (s_entry->len != t_entry->len)
    return true;

  return memcmp((void*)((uintptr_t)s->buf+s_entry->idx),
                (void*)((uintptr_t)t->buf+t_entry->idx),
                s_entry->len) != 0;
}

static bool
stack_equal(struct stack * s, struct stack * t)
{
  if ((s->entry_count != t->entry_count) || (s->buf_count != t->buf_count))
    return false;

  return memcmp(s->buf,t->buf,s->buf_count) == 0;
}

static void
stack_drop(struct stack * s)
{
  if (s->entry_count == 0)
    return;

  s->entry_count       -= 1;

  struct stack_entry* e = s->entries + s->entry_count;

  s->buf_count         -= e->len;
}

static void
stack_pop(struct stack * s, void ** v, uint32_t * len)
{
  if (s->entry_count == 0)
    return;

  s->entry_count -= 1;

  stack_entry_get(s,s->entry_count,v,len);

  s->buf_count   -= *len;
}

static void
stack_pop_all(struct stack * s, on_stack_drop on_drop, void * extra)
{
  while (s->entry_count > 0)
    {
      void*    v;
      uint32_t len;

      stack_pop(s,&v,&len);

      on_drop(v,len,extra);
    }
}

static void
stack_diff(struct stack * prev, struct stack * curr, 
           on_stack_drop on_drop, 
           on_stack_push on_push, 
           void * extra)
{
  while (prev->entry_count > curr->entry_count)
    {
      void *   v;
      uint32_t len;

      stack_pop(prev,&v,&len);

      on_drop(v,len,extra);
    }

  uint32_t const m        = MIN_MACRO(prev->entry_count,curr->entry_count);
  uint32_t       curr_idx = m;

  for (uint32_t prev_idx=0; prev_idx<m; prev_idx++)
    {
      if (stack_entry_not_equal(prev,curr,prev_idx))
        {
          curr_idx = prev_idx; // start work on b from the point of mismatch

          while (prev_idx < prev->entry_count)
            {
              void *   v;
              uint32_t len;

              stack_entry_get(prev,prev_idx,&v,&len);

              on_drop(v,len,extra);

              prev->buf_count -= len;   // adjust end

              prev_idx += 1;
            }

          prev->entry_count = curr_idx; // adjust count

          break;
        }
    }

  while (curr_idx < curr->entry_count)
    {
      void *   v;
      uint32_t len;

      stack_entry_get(curr,curr_idx,&v,&len);

      stack_push(prev,v,len);

      on_push(v,len,extra);

      curr_idx += 1;
    }
}

//
//
//

struct attribs {
  //
  // fixed-length state
  //
  union {

    struct {

      uint32_t       elem_count;        // total # of elems

      float          opacity;           // global opacity

      paint_op       fill_op;           // fill enabled?
      svg_color_t    fill_color;        // fill rgb
      float          fill_opacity;      // fill opacity
      fill_rule_op   fill_rule;         // even-odd or non-zero

      paint_op       stroke_op;         // stroke enabled?
      svg_color_t    stroke_color;      // stroke rgb
      float          stroke_opacity;    // stroke opacity
      float          stroke_width;      // stroke width

      paint_op       marker_op;         // marker enabled?
      svg_color_t    marker_color;      // stroke rgb
    };

    uint32_t         u32[ATTRIB_TYPE_SCALAR_COUNT];
    float            f32[ATTRIB_TYPE_SCALAR_COUNT];
  };

  //
  // variable-length state
  //
  struct stack*      transforms;
  struct stack*      ids;
};

//
//
//

static
struct attribs*
attribs_create()
{
  struct attribs* a = malloc(sizeof(*a));

  a->elem_count     = 0;

  a->opacity        = 1.0f;

  a->fill_op        = PAINT_OP_COLOR;
  a->fill_color     = 0x000000;
  a->fill_opacity   = 1.0f;
  a->fill_rule      = FILL_RULE_OP_NONZERO;

  a->stroke_op      = PAINT_OP_NONE;
  a->stroke_color   = 0x000000;
  a->stroke_opacity = 1.0f;
  a->stroke_width   = 1.0f;

  a->marker_op      = PAINT_OP_NONE;
  a->marker_color   = 0x000000;

  a->transforms     = stack_create(); // transform commands and implicit drops
  a->ids            = stack_create(); // id_end + id_begin commands

  return a;
}

static
void
attribs_dispose(struct attribs* a)
{
  stack_dispose(a->transforms);
  stack_dispose(a->ids);

  free(a);
}

//
//
//

struct svg_parser
{
  struct stack   * p;            // path   dictionary
  struct stack   * r;            // raster dictionary
  struct stack   * l;            // layer  dictionary

  struct attribs * prev;         // previous   render state
  struct attribs * curr;         // cumulative render state

  struct stack   * paths;        // stack of parsed paths

  struct stack   * undo;         // commands executed upon element close
  uint32_t         undo_count;   // number of commands in current element

  char           * attr_buf;
  uint32_t         attr_max;
  uint32_t         attr_count;
};

//
//
//

static
struct svg_parser *
svg_parser_create()
{
  struct svg_parser * sp = malloc(sizeof(*sp));

  sp->p          = stack_create();
  sp->r          = stack_create();
  sp->l          = stack_create();

  sp->prev       = attribs_create();
  sp->curr       = attribs_create();

  sp->paths      = stack_create();

  sp->undo       = stack_create();
  sp->undo_count = 0;

  stack_push(sp->undo,&sp->undo_count,sizeof(sp->undo_count));

  sp->attr_max   = 4096 * 4;
  sp->attr_buf   = malloc(sp->attr_max);
  sp->attr_count = 0;

  return sp;
}

static
void
svg_parser_dispose(struct svg_parser* sp)
{
  stack_dispose(sp->p);
  stack_dispose(sp->r);
  stack_dispose(sp->l);

  attribs_dispose(sp->prev);
  attribs_dispose(sp->curr);

  stack_dispose(sp->paths);

  stack_dispose(sp->undo);

  free(sp->attr_buf);
}

//
//
//

struct svg_doc
{
  struct stack* p;       // path   dictionary
  struct stack* r;       // raster dictionary
  struct stack* l;       // layer  dictionary

  uint32_t      p_idx;
  uint32_t      r_idx;
  uint32_t      l_idx;
};

//
//
//

void
svg_doc_rewind(struct svg_doc * const sd)
{
  sd->p_idx = 0;
  sd->r_idx = 0;
  sd->l_idx = 0;
}

//
//
//

uint32_t
svg_doc_path_count(struct svg_doc const * const sd)
{
  return stack_entry_count(sd->p);
}

uint32_t
svg_doc_raster_count(struct svg_doc const * const sd)
{
  return stack_entry_count(sd->r);
}

uint32_t
svg_doc_layer_count(struct svg_doc const * const sd)
{
  return stack_entry_count(sd->l);
}

//
//
//

static
struct svg_doc *
svg_doc_create(struct svg_parser* sp)
{
  struct svg_doc * sd = malloc(sizeof(*sd));

  sd->p = sp->p;
  sd->r = sp->r;
  sd->l = sp->l;

  svg_doc_rewind(sd);

  // steal stacks from svg_parser
  sp->p = NULL;
  sp->r = NULL;
  sp->l = NULL;

  return sd;
}

//
//
//

void
svg_doc_dispose(struct svg_doc * const sd)
{
  stack_dispose(sd->p);
  stack_dispose(sd->r);
  stack_dispose(sd->l);

  free(sd);
}

bool
svg_doc_path_next(struct svg_doc * const sd, union path_cmd** cmd)
{
  if (sd->p_idx >= sd->p->buf_count)
    {
      sd->p_idx = 0;

      return false;
    }

  // set cmd
  *cmd = (union path_cmd*)((uintptr_t)sd->p->buf + sd->p_idx);

  // update index
  struct lookup_cmd const * lookup = path_lookup_cmds + (*cmd)->type;

  sd->p_idx += lookup->size;

  // fprintf(stderr,"%s\n",lookup->name);

  return true;
}

bool
svg_doc_raster_next(struct svg_doc * const sd, union raster_cmd * * cmd)
{
  if (sd->r_idx >= sd->r->buf_count)
    {
      sd->r_idx = 0;

      return false;
    }

  // set cmd
  *cmd = (union raster_cmd*)((uintptr_t)sd->r->buf + sd->r_idx);

  // update index
  struct lookup_cmd const * lookup = raster_lookup_cmds + (*cmd)->type;

  sd->r_idx += lookup->size;

  // fprintf(stderr,"%s\n",lookup->name);

  return true;
}

bool
svg_doc_layer_next(struct svg_doc * const sd, union layer_cmd * * cmd)
{
  if (sd->l_idx >= sd->l->buf_count)
    {
      sd->l_idx = 0;

      return false;
    }

  // set cmd
  *cmd = (union layer_cmd*)((uintptr_t)sd->l->buf + sd->l_idx);

  // update index
  struct lookup_cmd const * lookup = layer_lookup_cmds + (*cmd)->type;

  sd->l_idx += lookup->size;

  // fprintf(stderr,"%s\n",lookup->name);

  return true;
}

//
// Threaded SVG representation
//
// element attributes : id
// container elements : svg, g
// path elements      : circle, ellipse, line, path, polygon, polyline, rect
// raster attributes  : transform, fill|stroke|marker, style props (*)
// layer attributes   : fill-rule, opacities, colors or gradient references, style props (*)
//
//   (*) --> raster and layer attributes also appear in style properties
//
//   1. decode path elements into path dictionary and save reference.
//      either create a sealed path for every element or seal the path
//      only when there is a new name, transform or fill^stroke change,
//      color change or stroke-width.
//
//   2. decode raster attributes into raster dictionary and save
//      reference. transform ops followed by stroked/filled path
//      head. append raster reference to current layer. either create
//      a new raster and a new layer for every path or "compact" and
//      create a new raster when there is a new name, new transform,
//      fill^stroke change, color change or stroke-width change. And
//      for compacted layers, create a new layer whenever there is a
//      fill^stroke change or color change.
//
//      - *begin*
//      - # of rasters before (start index)
//      - # of sub rasters    (can be zero!)
//      - (*transform*)?
//      - (*fill-or-stroke*)
//      - (*path*)
//      - *end*  -> restores stack
//
//   3. add layer attributes into layer dictionary:
//
//      - *begin*
//      - *layer*
//      - *color* | *gradient* | *texture* | ...
//      - fill rule
//      - { *place*, raster, tx, ty }+
//      - *end*
//

//
// ONLY VS2013 SUPPORTS strtof()
//

#if defined(_MSC_VER) && (_MSC_VER <= 1700) // MSVC 2012

static float
strtof(char const * nptr, char * * endptr)
{
  return (float)strtod(nptr,endptr);
}

#endif

//
// UNDO STACK
//

struct attrib_restore
{
  attrib_type type;
  uint32_t    value;
};


static void
attribs_undo(struct svg_parser* sp)
{
  while (sp->undo_count > 0)
    {
      sp->undo_count -= 1;

      void*    v;
      uint32_t len;

      stack_pop(sp->undo,&v,&len);

      struct attrib_restore const * r = (struct attrib_restore*)v;

      if (r->type < ATTRIB_TYPE_SCALAR_COUNT)
        {
          sp->curr->u32[r->type] = r->value;
        }
      else if (r->type == ATTRIB_TYPE_TRANSFORM)
        {
          stack_drop(sp->curr->transforms);
        }
    }
}

static void
attrib_save(struct svg_parser* sp, attrib_type t, uint32_t s)
{
  sp->undo_count += 1;

  struct attrib_restore r = { t, s };

  stack_push(sp->undo,&r,sizeof(r));
}

static void
attrib_save_scalar(struct svg_parser* sp, attrib_type t)
{
  assert(t < ATTRIB_TYPE_SCALAR_COUNT);

  attrib_save(sp,t,sp->curr->u32[t]);
}

static void
attribs_restore_undo_count(struct svg_parser* sp)
{
  void*    v;
  uint32_t len;

  stack_pop(sp->undo,&v,&len);

  sp->undo_count = *(uint32_t*)v;
}

static void
attribs_update(struct svg_parser* sp)
{
  for (uint32_t ii=0; ii<ATTRIB_TYPE_SCALAR_COUNT; ii++)
    sp->prev->u32[ii] = sp->curr->u32[ii];
}

static uint32_t
attrib_changes(struct svg_parser* sp)
{
  uint32_t changes = 0;

  for (uint32_t ii=0; ii<ATTRIB_TYPE_SCALAR_COUNT; ii++)
    {
      if (sp->prev->u32[ii] != sp->curr->u32[ii])
        changes |= ATTRIB_TYPE_TO_MASK(ii);
    }

  // special case: even-odd fills can't be accumulated like non-zero
  if (sp->curr->u32[ATTRIB_TYPE_FILL_RULE] == FILL_RULE_OP_EVENODD)
    changes |= ATTRIB_TYPE_TO_MASK(ATTRIB_TYPE_FILL_RULE);

  // changes |= ATTRIB_TYPE_TO_MASK(ATTRIB_TYPE_FILL_RULE);

  // transform change?
  if (!stack_equal(sp->prev->transforms,sp->curr->transforms))
    changes |= 1u << ATTRIB_TYPE_TRANSFORM;

  /*
    if (!stack_equal(sp->prev->ids,sp->curr->ids))
    changes |= 1u << ATTRIB_TYPE_ID;
  */

  return changes;
}

static bool
attrib_changed(uint32_t changes, attrib_type type)
{
  return (changes & ATTRIB_TYPE_TO_MASK_BIT(type)) != 0;
}

//
// DEFINE CHANGE MASKS
//

#define CHANGE_MASK_NEW_PATH                                            \
  (                                                                     \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_ELEM_COUNT      ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_OPACITY         ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_OP         ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_RULE       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_COLOR      ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_OPACITY    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_OP       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_COLOR    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_OPACITY  ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_WIDTH    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_MARKER_OP       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_MARKER_COLOR    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_TRANSFORM       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_ID              )              \
                                                                )

#define CHANGE_MASK_NEW_RASTER                                  \
  (                                                             \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_ELEM_COUNT      ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_OPACITY         ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_OP         ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_RULE       ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_COLOR      ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_OPACITY    ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_OP       ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_COLOR    ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_OPACITY  ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_WIDTH    ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_MARKER_OP       ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_MARKER_COLOR    ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_TRANSFORM       ) |    \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_ID              )      \
   )

#define CHANGE_MASK_NEW_LAYER                                           \
  (                                                                     \
   ATTRIB_TYPE_TO_MASK_OFF(  ATTRIB_TYPE_ELEM_COUNT      ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_OPACITY         ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_OP         ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_RULE       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_COLOR      ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_FILL_OPACITY    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_OP       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_COLOR    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_OPACITY  ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_STROKE_WIDTH    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_MARKER_OP       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_MARKER_COLOR    ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_TRANSFORM       ) |            \
   ATTRIB_TYPE_TO_MASK_BIT(  ATTRIB_TYPE_ID              )              \
                                                                )

//
//
//

#define PAINT_OP_BITS_IDX    4
#define PAINT_OP_MASK_IDX    ((1<<PAINT_OP_BITS_IDX)-1)

#define PAINT_OP_BITS_CMD    4
#define PAINT_OP_MASK_CMD    ((1<<PAINT_OP_BITS_CMD)-1)

#define PAINT_OP_BITS_TOTAL  (PAINT_OP_BITS_IDX + PAINT_OP_BITS_CMD)
#define PAINT_OP_MASK_TOTAL  ((1<<PAINT_OP_BITS_TOTAL)-1)


#define ATTRIB_PAINT_OPS_ANY                                            \
  ((((RASTER_CMD_FILL   << PAINT_OP_BITS_CMD) | ATTRIB_TYPE_FILL_OP)   << (0 * PAINT_OP_BITS_TOTAL)) | \
   (((RASTER_CMD_STROKE << PAINT_OP_BITS_CMD) | ATTRIB_TYPE_STROKE_OP) << (1 * PAINT_OP_BITS_TOTAL)) | \
   (((RASTER_CMD_MARKER << PAINT_OP_BITS_CMD) | ATTRIB_TYPE_MARKER_OP) << (2 * PAINT_OP_BITS_TOTAL)))

//
//
//

uint32_t
attrib_paint_op_first_idx(uint32_t const ops)
{
  return ops & PAINT_OP_MASK_IDX;
}

raster_cmd_type
attrib_paint_op_first_cmd(uint32_t const ops)
{
  return (raster_cmd_type)((ops >> PAINT_OP_BITS_IDX) & PAINT_OP_MASK_CMD);
}

uint32_t
attrib_paint_op_drop(uint32_t const ops)
{
  return ops >> PAINT_OP_BITS_TOTAL;
}

uint32_t
paint_enabled_first(struct svg_parser* sp, uint32_t ops)
{
  while (ops != 0)
    {
      if (sp->curr->u32[attrib_paint_op_first_idx(ops)] == PAINT_OP_COLOR)
        return ops;

      ops = attrib_paint_op_drop(ops);
    }

  return ops;
}

static
bool
paint_enabled_any(struct attribs const * const a, uint32_t ops)
{
  while (ops != 0)
    {
      if (a->u32[attrib_paint_op_first_idx(ops)] == PAINT_OP_COLOR)
        return true;

      ops = attrib_paint_op_drop(ops);
    }

  return false;
}

static bool
paint_was_enabled(struct svg_parser* sp)
{
  return paint_enabled_any(sp->prev,ATTRIB_PAINT_OPS_ANY);
}

static bool
paint_is_enabled(struct svg_parser* sp)
{
  return paint_enabled_any(sp->curr,ATTRIB_PAINT_OPS_ANY);
}

//
//
//

static void
compile_end(struct svg_parser* sp)
{
  //
  // ALWAYS END THE CURRENT PATH CLAUSE
  //
  uint32_t const p = stack_entry_count(sp->p);

  if (p > 0)
    {
      struct path_cmd_end pce = { PATH_CMD_END, p-1 };
      stack_tos_append(sp->p,&pce,sizeof(pce));
    }

  //
  // IF THERE WAS A PAINT IN PROGRESS THEN:
  // - END THE RASTER
  // - PLACE THE RASTER ON THE WIP LAYER
  // - END THE LAYER
  //
  uint32_t const r_idx    = stack_entry_count(sp->r);
  bool     const first_rl = r_idx == 0;

  if (!first_rl && paint_was_enabled(sp))
    {
      struct raster_cmd_end  rce = { RASTER_CMD_END, r_idx-1};
      stack_tos_append(sp->r,&rce,sizeof(rce));

      struct layer_cmd_place lcp = { LAYER_CMD_PLACE, r_idx-1 , 0, 0 };
      stack_tos_append(sp->l,&lcp,sizeof(lcp));

      struct layer_cmd_end   lce = { LAYER_CMD_END };
      stack_tos_append(sp->l,&lce,sizeof(lce));
    }
}

//
//
//

static void
raster_add_path(struct svg_parser* sp, uint32_t ops)
{
  if ((ops = paint_enabled_first(sp,ops)) == 0)
    return;

  while (true)
    {
      struct raster_cmd_fsm fsm = { attrib_paint_op_first_cmd(ops), stack_entry_count(sp->p)-1 };

      stack_tos_append(sp->r,&fsm,sizeof(fsm));

      ops = attrib_paint_op_drop(ops);

      if ((ops = paint_enabled_first(sp,ops)) == 0)
        return;

      //
      // otherwise, end this raster and start another
      //
      uint32_t const rid = stack_entry_count(sp->r) - 1;

      struct raster_cmd_end   rce = { RASTER_CMD_END, rid };
      stack_tos_append(sp->r,&rce,sizeof(rce));

      struct layer_cmd_place  lcp = { LAYER_CMD_PLACE, rid , 0, 0 };
      stack_tos_append(sp->l,&lcp,sizeof(lcp));

      struct layer_cmd_end    lce = { LAYER_CMD_END };
      stack_tos_append(sp->l,&lce,sizeof(lce));

      struct raster_cmd_begin rcb = { RASTER_CMD_BEGIN };
      stack_push(sp->r,&rcb,sizeof(rcb));

      struct layer_cmd_begin  lcb = { LAYER_CMD_BEGIN , stack_entry_count(sp->l) };
      stack_push(sp->l,&lcb,sizeof(lcb));
    }
}

//
//
//

static void
transform_on_drop(void* v, uint32_t len, void* extra)
{
  struct svg_parser*               sp  = extra;
  struct raster_cmd_transform_drop cmd = { RASTER_CMD_TRANSFORM_DROP };

  stack_tos_append(sp->r,&cmd,sizeof(cmd));
}

static void
transform_on_push(void* v, uint32_t len, void* extra)
{
  struct svg_parser* sp = extra;

  stack_tos_append(sp->r,v,len);
}

//
// PROCESS PATH ELEMENTS
//
// Path elements trigger compilation of paths, rasters and layers.
//

static void
compile(struct svg_parser* sp)
{
  //
  // if there are no paths then return and continue to record attrib
  // changes
  //
  bool const paths_empty = stack_entry_count(sp->paths) == 0;

  if (paths_empty)
    return;

  //
  // compute changes
  //
  uint32_t const changes = attrib_changes(sp);

  //
  // compile paths
  //
  // FIXME -- FORCE IF PATHS=0 / EMPTY
  //
  uint32_t const pc           = stack_entry_count(sp->p);
  bool     const pc_is_empty  = pc == 0;
  bool     const path_changed = pc_is_empty || ((changes & CHANGE_MASK_NEW_PATH) != 0);

  if (path_changed)
    {
      if (pc > 0)
        {
          struct path_cmd_end pce = { PATH_CMD_END, pc-1 };
          stack_tos_append(sp->p,&pce,sizeof(pce));
        }

      struct path_cmd_begin pcb = { PATH_CMD_BEGIN };
      stack_push(sp->p,&pcb,sizeof(pcb));
    }

  // append path commands
  stack_tos_copy(sp->p,sp->paths);

  // reset the path stack
  stack_reset(sp->paths);

  // return if path is unchanged
  if (!path_changed)
    return;

  //
  // 0. if path unchanged then return
  //
  // 1. if !first_rl && raster changed && any fill/stroke/marker *was* enabled
  //          - RASTER_END
  //          - if layer changed then LAYER_END
  //
  // 2. if any fill/stroke/marker paint *is* enabled
  //      - if first_rl || raster changed
  //          - RASTER_BEGIN
  //      - if first_rl || layer changed
  //          - LAYER_BEGIN
  //      - compile transform and stroke-width raster changes
  //      - compile opacity/color/fill-rule layer changes
  //
  // 3. if fill *is* enabled
  //      - add filled path
  //      - if stroke or marker enabled
  //          - RASTER_END
  //          - add RASTER to layer
  //          - LAYER_END
  //          - RASTER_BEGIN
  //          - LAYER_BEGIN
  // 4. if stroke *is* enabled
  //      - add stroked path
  //      - if marker enabled
  //          - RASTER_END
  //          - add RASTER to layer
  //          - LAYER_END
  //          - RASTER_BEGIN
  //          - LAYER_BEGIN
  // 5. if marker *is* enabled
  //      - add marker path
  //

  uint32_t const r_idx          = stack_entry_count(sp->r);
  bool     const first_rl       = r_idx == 0;
  bool     const raster_changed = (changes & CHANGE_MASK_NEW_RASTER) != 0;
  bool     const layer_changed  = (changes & CHANGE_MASK_NEW_LAYER)  != 0;

  // RASTER WAS ENABLED
  if (!first_rl && raster_changed && paint_was_enabled(sp))
    {
      struct raster_cmd_end  rce = { RASTER_CMD_END, r_idx-1};
      stack_tos_append(sp->r,&rce,sizeof(rce));

      struct layer_cmd_place lcp = { LAYER_CMD_PLACE, r_idx-1 , 0, 0 };
      stack_tos_append(sp->l,&lcp,sizeof(lcp));

      if (layer_changed)
        {
          struct layer_cmd_end lce = { LAYER_CMD_END };
          stack_tos_append(sp->l,&lce,sizeof(lce));
        }
    }

  if (paint_is_enabled(sp))
    {
      if (first_rl || raster_changed)
        {
          struct raster_cmd_begin rcb = { RASTER_CMD_BEGIN };
          stack_push(sp->r,&rcb,sizeof(rcb));
        }

      if (first_rl || layer_changed)
        {
          struct layer_cmd_begin lcb = { LAYER_CMD_BEGIN, stack_entry_count(sp->l) };
          stack_push(sp->l,&lcb,sizeof(lcb));
        }

      //
      // IT SHOULD BE OK TO FRONTLOAD ALL THESE CHANGES SINCE IN THE
      // WORST CASE THEY'LL BE BRACKETED BY ID NAMES
      //
      // compile raster changes... transforms always first
      //
      if (attrib_changed(changes,ATTRIB_TYPE_TRANSFORM))
        {
          stack_diff(sp->prev->transforms,
                     sp->curr->transforms,
                     transform_on_drop,
                     transform_on_push,
                     sp);
        }

      if (attrib_changed(changes,ATTRIB_TYPE_STROKE_WIDTH))
        {
          ; //
        }

      if (layer_changed) // not checking r_is_empty here (?)
        {
          // compile layer changes: opacity, color, fill-rule changes

          if (attrib_changed(changes,ATTRIB_TYPE_OPACITY))
            {
              struct layer_cmd_opacity cmd = { LAYER_CMD_OPACITY, sp->curr->opacity };
              stack_tos_append(sp->l,&cmd,sizeof(cmd));
            }

          if (attrib_changed(changes,ATTRIB_TYPE_FILL_RULE))
            {
              struct layer_cmd_fill_rule cmd = { LAYER_CMD_FILL_RULE, sp->curr->fill_rule };
              stack_tos_append(sp->l,&cmd,sizeof(cmd));
            }

          if (attrib_changed(changes,ATTRIB_TYPE_FILL_COLOR))
            {
              struct layer_cmd_fill_color cmd = { LAYER_CMD_FILL_COLOR, sp->curr->fill_color };
              stack_tos_append(sp->l,&cmd,sizeof(cmd));
            }

          if (attrib_changed(changes,ATTRIB_TYPE_FILL_OPACITY))
            {
              struct layer_cmd_fill_opacity cmd = { LAYER_CMD_FILL_OPACITY, sp->curr->fill_opacity };
              stack_tos_append(sp->l,&cmd,sizeof(cmd));
            }

          if (attrib_changed(changes,ATTRIB_TYPE_STROKE_COLOR))
            {
              struct layer_cmd_stroke_color cmd = { LAYER_CMD_STROKE_COLOR, sp->curr->stroke_color };
              stack_tos_append(sp->l,&cmd,sizeof(cmd));
            }

          if (attrib_changed(changes,ATTRIB_TYPE_STROKE_OPACITY))
            {
              struct layer_cmd_stroke_opacity cmd = { LAYER_CMD_STROKE_OPACITY, sp->curr->stroke_opacity };
              stack_tos_append(sp->l,&cmd,sizeof(cmd));
            }
        }
    }

  //
  // append path and/or create new rasters and layers
  //
  raster_add_path(sp,ATTRIB_PAINT_OPS_ANY);

  //
  // copy curr attribs to prev attribs
  //
  attribs_update(sp);
}

//
//
//

static void
warning(yxml_t * ys, char const * condition, char const * name)
{
  if (!message_quiet)
    {
      fprintf(stderr,"Warning: %s at line %u, column %llu --> \"%s\"\n",
              condition,ys->line,ys->byte,name);
    }
}

static void
attrib_ignore(yxml_t * ys, char const * name)
{
  warning(ys,"ignoring attribute",name);
}

static void
transform_ignore(yxml_t * ys, char const * name)
{
  warning(ys,"ignoring transform",name);
}

//
// PARSE CONTAINERS
//

static void
parse_elem_svg(struct svg_parser* sp, yxml_t* ys)
{
  ;
}

static void
parse_elem_g(struct svg_parser* sp, yxml_t* ys)
{
  ;
}

//
// PARSE SHAPES
//

static void
parse_elem_circle(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_circle cmd;

  cmd.type = PATH_CMD_CIRCLE;
  cmd.cx   = 0.0f;
  cmd.cy   = 0.0f;
  cmd.r    = 0.0f;

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

static void
parse_elem_ellipse(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_ellipse cmd;

  cmd.type = PATH_CMD_ELLIPSE;
  cmd.cx   = 0.0f;
  cmd.cy   = 0.0f;
  cmd.rx   = 0.0f;
  cmd.ry   = 0.0f;

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

static void
parse_elem_line(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_line cmd;

  cmd.type = PATH_CMD_LINE;
  cmd.x1   = 0.0f;
  cmd.y1   = 0.0f;
  cmd.x2   = 0.0f;
  cmd.y2   = 0.0f;

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

static void
parse_elem_path(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_path_begin cmd;

  cmd.type = PATH_CMD_PATH_BEGIN;

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

static void
parse_elem_polygon(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_polygon cmd = { PATH_CMD_POLYGON };

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

static void
parse_elem_polyline(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_polyline cmd = { PATH_CMD_POLYLINE };

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

static void
parse_elem_rect(struct svg_parser* sp, yxml_t* ys)
{
  struct path_cmd_rect cmd;

  cmd.type   = PATH_CMD_RECT;
  cmd.x      = 0.0f;
  cmd.y      = 0.0f;
  cmd.width  = 0.0f;
  cmd.height = 0.0f;
  cmd.rx     = 0.0f;
  cmd.ry     = 0.0f;

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

//
// MODIFY OR APPEND TO PATH ELEMENT
//

static bool
paths_empty(struct svg_parser* sp)
{
  return stack_entry_count(sp->paths) == 0;
}

static union path_cmd*
paths_tos(struct svg_parser* sp)
{
  return (union path_cmd*)stack_tos(sp->paths);
}

static void
paths_tos_append(struct svg_parser* sp, void* v, uint32_t len)
{
  stack_tos_append(sp->paths,v,len);
}

//
//
//

static void
invalid_attrib(struct svg_parser* sp, yxml_t* ys, char const * val)
{
  fprintf(stderr,"Error: invalid attribute at "
          "line %u, column %llu --> \"%s\"\n",
          ys->line,ys->byte,val);

  exit(-1);
}

//
//
//

static float
parse_number(struct svg_parser* sp, yxml_t* ys, char* val)
{
  char* stop = val;

  float f = strtof(val,&stop);

  if (stop == val)
    invalid_attrib(sp,ys,val);

  return f;
}

static uint32_t
parse_numbers(struct svg_parser* sp,
              yxml_t*            ys,
              char*              val,
              uint32_t           len,
              float*             array,
              uint32_t           array_count,
              uint32_t*          parse_count)
{
  *parse_count = 0;

  char* next = val;

  while (*next != 0)
    {
      char* stop = next;

      *array = strtof(next,&stop);

      if (stop == next)
        break;

      next = stop;

      *parse_count += 1;

      // eat trailing whitespace... but let calling routine handle
      // inter-sequence commas

      while (isspace(*next))
        next += 1;

      if (*next == ',') // eat up to one comma
        next += 1;

      if (*parse_count == array_count)
        break;

      array += 1;
    }

  return (uint32_t)(next - val);
}

//
// PARSE ATTRIBUTES
//

static void
parse_attrib_id(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  stack_push(sp->curr->ids,val,len+1); // push the symbol name
}

static void
parse_attrib_r(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_CIRCLE)
    invalid_attrib(sp,ys,val);

  cmd->circle.r = parse_number(sp,ys,val);
}

static void
parse_attrib_cx(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type == PATH_CMD_CIRCLE)
    {
      cmd->circle.cx = parse_number(sp,ys,val);
    }
  else if (cmd->type == PATH_CMD_ELLIPSE)
    {
      cmd->ellipse.cx = parse_number(sp,ys,val);
    }
  else
    {
      invalid_attrib(sp,ys,val);
    }
}

static void
parse_attrib_cy(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type == PATH_CMD_CIRCLE)
    {
      cmd->circle.cy = parse_number(sp,ys,val);
    }
  else if (cmd->type == PATH_CMD_ELLIPSE)
    {
      cmd->ellipse.cy = parse_number(sp,ys,val);
    }
  else
    {
      invalid_attrib(sp,ys,val);
    }
}

static void
parse_attrib_rx(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type == PATH_CMD_ELLIPSE)
    {
      cmd->ellipse.rx = parse_number(sp,ys,val);
    }
  else if (cmd->type == PATH_CMD_RECT)
    {
      cmd->rect.rx = parse_number(sp,ys,val);
    }
  else
    {
      invalid_attrib(sp,ys,val);
    }
}

static void
parse_attrib_ry(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type == PATH_CMD_ELLIPSE)
    {
      cmd->ellipse.ry = parse_number(sp,ys,val);
    }
  else if (cmd->type == PATH_CMD_RECT)
    {
      cmd->rect.ry = parse_number(sp,ys,val);
    }
  else
    {
      invalid_attrib(sp,ys,val);
    }
}

static void
parse_attrib_x(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  if (paths_empty(sp)) {
    attrib_ignore(ys,"x");
    return;
  }

  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_RECT)
    invalid_attrib(sp,ys,"x");

  cmd->rect.x = parse_number(sp,ys,val);
}

static void
parse_attrib_y(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  if (paths_empty(sp)) {
    attrib_ignore(ys,"y");
    return;
  }

  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_RECT)
    invalid_attrib(sp,ys,"y");

  cmd->rect.y = parse_number(sp,ys,val);
}

static void
parse_attrib_width(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  if (paths_empty(sp)) {
    attrib_ignore(ys,"width");
    return;
  }

  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_RECT)
    {
      // invalid_attrib(sp,ys,val);
      attrib_ignore(ys,"width");
    }

  cmd->rect.width = parse_number(sp,ys,val);
}

static void
parse_attrib_height(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  if (paths_empty(sp)) {
    attrib_ignore(ys,"height");
    return;
  }

  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_RECT)
    {
      // invalid_attrib(sp,ys,val);
      attrib_ignore(ys,"height");
    }

  cmd->rect.height = parse_number(sp,ys,val);
}

static void
parse_attrib_x1(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_LINE)
    invalid_attrib(sp,ys,val);

  cmd->line.x1 = parse_number(sp,ys,val);
}

static void
parse_attrib_y1(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_LINE)
    invalid_attrib(sp,ys,val);

  cmd->line.y1 = parse_number(sp,ys,val);
}

static void
parse_attrib_x2(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_LINE)
    invalid_attrib(sp,ys,val);

  cmd->line.x2 = parse_number(sp,ys,val);
}

static void
parse_attrib_y2(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if (cmd->type != PATH_CMD_LINE)
    invalid_attrib(sp,ys,val);

  cmd->line.y2 = parse_number(sp,ys,val);
}

//
//
//

static void
parse_points(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  {
    struct path_cmd_poly_point cmd = { PATH_CMD_POLY_POINT };

    bool first = true;

    do {
      uint32_t parse_count;

      uint32_t n = parse_numbers(sp,ys,val,len,&cmd.x,2,&parse_count);

      if (parse_count != 2)
        invalid_attrib(sp,ys,val);

      paths_tos_append(sp,&cmd,sizeof(cmd));

      val += n;
      len -= n;

      first = false;

    } while (len > 0);
  }

  {
    struct path_cmd_poly_end cmd = { PATH_CMD_POLY_END };

    paths_tos_append(sp,&cmd,sizeof(cmd));
  }
}

//
//
//

static void
parse_attrib_points(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  union path_cmd* cmd = paths_tos(sp);

  if ((cmd->type != PATH_CMD_POLYGON) && (cmd->type != PATH_CMD_POLYLINE))
    invalid_attrib(sp,ys,val);

  parse_points(sp,ys,val,len);
}

//
//
//

static int
parse_path_coord_sequence(struct svg_parser* sp,
                          yxml_t*            ys,
                          char*              val,
                          uint32_t           len,
                          void*              cmd,
                          uint32_t           cmd_size,
                          float*             cmd_coords,
                          uint32_t const     cmd_coord_count,
                          bool     const     optional)
{
  int  t     = 0;
  bool first = true;

  do {
    uint32_t parse_count;

    uint32_t n = parse_numbers(sp,ys,val,len,
                               cmd_coords,
                               cmd_coord_count,
                               &parse_count);

    if ((parse_count == 0) && (!first || optional))
      break;

    if (parse_count != cmd_coord_count)
      invalid_attrib(sp,ys,val);

    stack_push(sp->paths,cmd,cmd_size);

    first  = false;
    t     += n;

    val   += n;
    len   -= n;

  } while (len > 0);

  return t;
}

//
//
//

static int
parse_path_move_to(struct svg_parser * sp,
                   yxml_t *            ys,
                   char *              val,
                   uint32_t            len,
                   bool          const first_cmd,
                   path_cmd_type const type)
{
  struct path_cmd_move_to cmd = { type };

  uint32_t                parse_count;
  uint32_t                n   = parse_numbers(sp,ys,val,len,&cmd.x,2,&parse_count);

  if (parse_count != 2)
    invalid_attrib(sp,ys,val);

  stack_push(sp->paths,&cmd,sizeof(cmd));

  return n;
}

static int
parse_path_close(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct path_cmd_close cmd = { PATH_CMD_CLOSE };

  stack_push(sp->paths,&cmd,sizeof(cmd));

  return 0;
}

static int
parse_path_line_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type, const bool optional)
{
  struct path_cmd_line_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.x,2,optional);
}

static int
parse_path_hv_line_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type)
{
  struct path_cmd_coord_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.c,1,false);
}

static int
parse_path_cubic_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type)
{
  struct path_cmd_cubic_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.x1,6,false);
}

static int
parse_path_cubic_smooth_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type)
{
  struct path_cmd_cubic_smooth_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.x2,4,false);
}

static int
parse_path_quad_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type)
{
  struct path_cmd_quad_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.x1,4,false);
}

static int
parse_path_quad_smooth_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type)
{
  struct path_cmd_quad_smooth_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.x,2,false);
}

static int
parse_path_arc_to(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len, path_cmd_type const type)
{
  struct path_cmd_arc_to cmd = { type };

  return parse_path_coord_sequence(sp,ys,val,len,
                                   &cmd,sizeof(cmd),
                                   &cmd.rx,7,false);
}

//
//
//

static void
parse_attrib_d(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  if (paths_tos(sp)->type != PATH_CMD_PATH_BEGIN)
    invalid_attrib(sp,ys,val);

  bool first_cmd = true;

  do {
    int  n = len;
    char t[2];

    int const err = sscanf_s(val," %1[ACHLMQSTVZachlmqstvz]%n",t,2,&n);

    if (err != 1)
      break;

    val += n;
    len -= n;

    // fprintf(stderr,"%s\n",t);

    switch (t[0])
      {
        //
        // ABSOLUTE
        //
      case 'A':
        n = parse_path_arc_to(sp,ys,val,len,PATH_CMD_ARC_TO);
        break;

      case 'C':
        n = parse_path_cubic_to(sp,ys,val,len,PATH_CMD_CUBIC_TO);
        break;

      case 'H':
        n = parse_path_hv_line_to(sp,ys,val,len,PATH_CMD_HLINE_TO);
        break;

      case 'L':
        n = parse_path_line_to(sp,ys,val,len,PATH_CMD_LINE_TO,false);
        break;

      case 'M':
        // parse move to and optional line to's
        n    = parse_path_move_to(sp,ys,val,len,first_cmd,PATH_CMD_MOVE_TO);
        val += n;
        len -= n;
        n    = parse_path_line_to(sp,ys,val,len,PATH_CMD_LINE_TO,true);
        // reset first
        first_cmd = false;
        break;

      case 'Q':
        n = parse_path_quad_to(sp,ys,val,len,PATH_CMD_QUAD_TO);
        break;

      case 'S':
        n = parse_path_cubic_smooth_to(sp,ys,val,len,PATH_CMD_CUBIC_SMOOTH_TO);
        break;

      case 'T':
        n = parse_path_quad_smooth_to(sp,ys,val,len,PATH_CMD_QUAD_SMOOTH_TO);
        break;

      case 'V':
        n = parse_path_hv_line_to(sp,ys,val,len,PATH_CMD_VLINE_TO);
        break;

      case 'Z':
        n = parse_path_close(sp,ys,val,len);
        break;

        //
        // RELATIVE
        //
      case 'a':
        n = parse_path_arc_to(sp,ys,val,len,PATH_CMD_ARC_TO_REL);
        break;

      case 'c':
        n = parse_path_cubic_to(sp,ys,val,len,PATH_CMD_CUBIC_TO_REL);
        break;

      case 'h':
        n = parse_path_hv_line_to(sp,ys,val,len,PATH_CMD_HLINE_TO_REL);
        break;

      case 'l':
        n = parse_path_line_to(sp,ys,val,len,PATH_CMD_LINE_TO_REL,false);
        break;

      case 'm':
        // if relative move_to is first command in path then force to absolute
        n    = parse_path_move_to(sp,ys,val,len,first_cmd,first_cmd ? PATH_CMD_MOVE_TO : PATH_CMD_MOVE_TO_REL);
        val += n;
        len -= n;
        n    = parse_path_line_to(sp,ys,val,len,PATH_CMD_LINE_TO_REL,true);
        // reset first
        first_cmd = false;
        break;

      case 'q':
        n = parse_path_quad_to(sp,ys,val,len,PATH_CMD_QUAD_TO_REL);
        break;

      case 's':
        n = parse_path_cubic_smooth_to(sp,ys,val,len,PATH_CMD_CUBIC_SMOOTH_TO_REL);
        break;

      case 't':
        n = parse_path_quad_smooth_to(sp,ys,val,len,PATH_CMD_QUAD_SMOOTH_TO_REL);
        break;

      case 'v':
        n = parse_path_hv_line_to(sp,ys,val,len,PATH_CMD_VLINE_TO_REL);
        break;

      case 'z':
        n = parse_path_close(sp,ys,val,len);
        break;

      default:
        invalid_attrib(sp,ys,t);
      }

    val += n;
    len -= n;

  } while (len > 0);

  //
  //
  //

  struct path_cmd_path_end cmd = { PATH_CMD_PATH_END };

  stack_push(sp->paths,&cmd,sizeof(cmd));
}

//
// PARSE RENDER STATE ATTRIBS -- VARIABLE LENGTH
//

static void
parse_transform_matrix(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct raster_cmd_transform_matrix cmd = { RASTER_CMD_TRANSFORM_MATRIX };

  uint32_t parse_count;

  parse_numbers(sp,ys,val,len,&cmd.sx,6,&parse_count);

  if (parse_count != 6)
    invalid_attrib(sp,ys,val);

  stack_push(sp->curr->transforms,&cmd,sizeof(cmd));
}

static void
parse_transform_translate(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct raster_cmd_transform_translate cmd = { RASTER_CMD_TRANSFORM_TRANSLATE, 0.0f, 0.0f };

  uint32_t parse_count;

  parse_numbers(sp,ys,val,len,&cmd.tx,2,&parse_count);

  if (parse_count < 1)
    invalid_attrib(sp,ys,val);

  stack_push(sp->curr->transforms,&cmd,sizeof(cmd));
}

static void
parse_transform_scale(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct raster_cmd_transform_scale cmd = { RASTER_CMD_TRANSFORM_SCALE };

  uint32_t parse_count;

  parse_numbers(sp,ys,val,len,&cmd.sx,2,&parse_count);

  if (parse_count < 1)
    invalid_attrib(sp,ys,val);

  if (parse_count == 1)
    cmd.sy = cmd.sx;

  stack_push(sp->curr->transforms,&cmd,sizeof(cmd));
}

static void
parse_transform_rotate(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct raster_cmd_transform_rotate cmd = { RASTER_CMD_TRANSFORM_ROTATE, 0.0f, 0.0f, 0.0f };

  uint32_t parse_count;

  parse_numbers(sp,ys,val,len,&cmd.d,3,&parse_count);

  if (parse_count < 1)
    invalid_attrib(sp,ys,val);

  stack_push(sp->curr->transforms,&cmd,sizeof(cmd));
}

static void
parse_transform_skewX(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct raster_cmd_transform_skew_x cmd = { RASTER_CMD_TRANSFORM_SKEW_X };

  cmd.d = parse_number(sp,ys,val);

  stack_push(sp->curr->transforms,&cmd,sizeof(cmd));
}

static void
parse_transform_skewY(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  struct raster_cmd_transform_skew_y cmd = { RASTER_CMD_TRANSFORM_SKEW_Y };

  cmd.d = parse_number(sp,ys,val);

  stack_push(sp->curr->transforms,&cmd,sizeof(cmd));
}

//
// PARSE RENDER STATE ATTRIBS -- FIXED LENGTH
//

static void
parse_attrib_opacity(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  attrib_save_scalar(sp,ATTRIB_TYPE_OPACITY);

  sp->curr->opacity = parse_number(sp,ys,val);
}

static void
parse_attrib_fill_rule(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  attrib_save_scalar(sp,ATTRIB_TYPE_FILL_RULE);

  if      (strcmp(val,"evenodd") == 0)
    sp->curr->fill_rule = FILL_RULE_OP_EVENODD;
  else if (strcmp(val,"nonzero") == 0)
    sp->curr->fill_rule = FILL_RULE_OP_NONZERO;
}

static void
parse_color(struct svg_parser* sp, yxml_t* ys, paint_op* op, svg_color_t* color, char* val, uint32_t len)
{
  if (strcmp(val,"inherit") == 0)
    return;               // don't touch op or color

  if (strcmp(val,"none") == 0) {
    *op = PAINT_OP_NONE;  // don't touch color
    return;
  }

  // otherwise, parse it
  if (sscanf_s(val,"#%6x",color) == 1) {
    *op = PAINT_OP_COLOR; // update op and color

    if (len == 4)
      {
        uint32_t const r = (*color >> 8) & 0xF;
        uint32_t const g = (*color >> 4) & 0xF;
        uint32_t const b = (*color >> 0) & 0xF;

        *color = SVG_RGB((r<<4)|r,(g<<4)|g,(b<<4)|b);
      }
    return;
  }

  {
    uint32_t r,g,b;
    if (sscanf_s(val,"rgb( %u , %u , %u )",&r,&g,&b) == 3) {
      *op    = PAINT_OP_COLOR; // update op and color
      *color = SVG_RGB(r,g,b);
      return;
    }
  }

  {
    float r,g,b;
    if (sscanf_s(val,"rgb( %f%% , %f%% , %f%% )",&r,&g,&b) == 3) {
      *op    = PAINT_OP_COLOR; // update op and color
      *color = SVG_RGB((uint32_t)(0xFF*r),
                       (uint32_t)(0xFF*g),
                       (uint32_t)(0xFF*b));
      return;
    }
  }

  // svg color keyword?
  struct svg_color_name const * cn = svg_color_name_lookup(val,len);

  if (cn != NULL)
    {
      *op    = PAINT_OP_COLOR;
      *color = cn->color;
      return;
    }

  //
  // FIXME -- parse floating point format
  //

  // otherwise this is an error
  invalid_attrib(sp,ys,val);
}

static void
parse_attrib_fill_color(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  // save even though it might not be changed
  attrib_save_scalar(sp,ATTRIB_TYPE_FILL_OP);
  attrib_save_scalar(sp,ATTRIB_TYPE_FILL_COLOR);

  //
  // parse color
  //
  // if "none"    then set flag to false
  // if "inherit" then do nothing
  // if color     then set flag to true and set color
  // else         error
  //
  parse_color(sp,ys,&sp->curr->fill_op,&sp->curr->fill_color,val,len);
}

static void
parse_attrib_fill_opacity(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  attrib_save_scalar(sp,ATTRIB_TYPE_FILL_OPACITY);

  sp->curr->fill_opacity = parse_number(sp,ys,val);
}

static void
parse_attrib_stroke_color(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  // save even though it might not be changed
  attrib_save_scalar(sp,ATTRIB_TYPE_STROKE_OP);
  attrib_save_scalar(sp,ATTRIB_TYPE_STROKE_COLOR);

  //
  // parse color
  //
  // if "none"    then set flag to false
  // if "inherit" then do nothing
  // if color     then set flag to true and set color
  // else         error
  //
  parse_color(sp,ys,&sp->curr->stroke_op,&sp->curr->stroke_color,val,len);
}

static void
parse_attrib_stroke_opacity(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  attrib_save_scalar(sp,ATTRIB_TYPE_STROKE_OPACITY);

  sp->curr->stroke_opacity = parse_number(sp,ys,val);
}

static void
parse_attrib_stroke_width(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
  attrib_save_scalar(sp,ATTRIB_TYPE_STROKE_WIDTH);

  sp->curr->stroke_width = parse_number(sp,ys,val);
}

//
//
//

static void
parse_attrib_style(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len);

static void
parse_attrib_transform(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len);

//
//
//

typedef void (*parse_elem_fp     )(struct svg_parser* sp, yxml_t* ys);
typedef void (*parse_attrib_fp   )(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t const len);
typedef void (*parse_transform_fp)(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t const len);

struct svg_elem       { char* name;  parse_elem_fp      fp; };
struct svg_attrib     { char* name;  parse_attrib_fp    fp; };
struct svg_transform  { char* name;  parse_transform_fp fp; };

//
// These are generated by the gperf utility:
//
// - uint32_t svg_elem_hash  (register const char *str, register uint32_t len);
// - uint32_t svg_attrib_hash(register const char *str, register uint32_t len);
//
// - const struct svg_elem*   svg_elem_lookup  (register const char *str, register uint32_t len);
// - const struct svg_attrib* svg_attrib_lookup(register const char *str, register uint32_t len);
//

#include "svg_elems.h"
#include "svg_attribs.h"
#include "svg_transforms.h"

//
//
//

static void
attrib_dispatch(struct svg_parser* sp,
                yxml_t*            ys,
                const char*        attrName,
                const size_t       attrLen,
                char*              attrVal,
                const size_t       attrValLen)
{
  const struct svg_attrib* r = svg_attrib_lookup(attrName,(uint32_t)attrLen);

  //
  // warn or ignore
  //
  if (r == NULL)
    {
      attrib_ignore(ys,attrName);
      return;
    }

  //
  // otherwise, process attribute
  //
  r->fp(sp,ys,attrVal,(uint32_t)attrValLen);
}

//
//
//

static void
transform_dispatch(struct svg_parser* sp,
                   yxml_t*            ys,
                   const char*        attrName,
                   const size_t       attrLen,
                   char*              attrVal,
                   const size_t       attrValLen)
{
  const struct svg_transform* r = svg_transform_lookup(attrName,(uint32_t)attrLen);

  //
  // warn or ignore
  //
  if (r == NULL)
    {
      transform_ignore(ys,attrName); // should never happen
      return;
    }

  //
  // otherwise, process attribute
  //
  r->fp(sp,ys,attrVal,(uint32_t)attrValLen);

  //
  // save transform stack drop
  //
  attrib_save(sp,ATTRIB_TYPE_TRANSFORM,0);
}

//
//
//

static void
parse_attrib_style(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
#define STYLE_NAME_LEN    33
#define STYLE_VALUE_LEN   65

  char name[STYLE_NAME_LEN], value[STYLE_VALUE_LEN];

  do {
    int n = len;

    const int err = sscanf_s(val," %32[^: \t\n] : %64[^;] %*[;]%n",
                             name, STYLE_NAME_LEN,
                             value,STYLE_VALUE_LEN,
                             &n);


    if (err < 2)
      invalid_attrib(sp,ys,val);

    val += n;
    len -= n;

    attrib_dispatch(sp,ys,name,
                    strlen(name),
                    value,
                    strlen(value));

  } while (len > 0);
}

static void
parse_attrib_transform(struct svg_parser* sp, yxml_t* ys, char* val, uint32_t len)
{
#define TRANSFORM_NAME_LEN   11
#define TRANSFORM_VALS_LEN   121

  char name[TRANSFORM_NAME_LEN], vals[TRANSFORM_VALS_LEN];

  do {
    int n = len;

    const int err = sscanf_s(val," %10[^( \t\n] ( %120[^)] ) %n",
                             name,TRANSFORM_NAME_LEN,
                             vals,TRANSFORM_VALS_LEN,
                             &n);


    if (err < 2)
      invalid_attrib(sp,ys,val);

    val += n;
    len -= n;

    // fprintf(stderr,"%s ( %s )\n",name,vals);

    transform_dispatch(sp,ys,name,
                       strlen(name),
                       vals,
                       strlen(vals));

  } while (len > 0);
}

//
//
//

static void
elem_begin(struct svg_parser* sp, yxml_t* ys)
{
  //
  // save undo count
  //
  stack_push(sp->undo,&sp->undo_count,sizeof(sp->undo_count));

  //
  // reset count
  //
  sp->undo_count = 0;

  //
  // lookup element by name
  //
  const struct svg_elem* r = svg_elem_lookup(ys->elem,(uint32_t)yxml_symlen(ys,ys->elem));

  //
  // warn or ignore
  //
  if (r == NULL)
    {
      warning(ys,"unhandled element",ys->elem);
      return;
    }

  //
  // increment element count
  //
  sp->curr->elem_count += 1; // attrib_save_scalar(sp,ATTRIB_TYPE_ELEM_COUNT);

  //
  // otherwise, process element
  //
  r->fp(sp,ys);
}

static void
elem_end(struct svg_parser* sp)
{
  //
  // if necessary, compile any outstanding paths
  //
  compile(sp);

  //
  // apply undo stack for this element
  //
  attribs_undo(sp);

  //
  // restore previous undo count
  //
  attribs_restore_undo_count(sp);
}

//
//
//

static void
y_attr_copy(struct svg_parser* sp, const char* from)
{
  if (sp->attr_count + 8 > sp->attr_max) // at most a few chars
    {
      sp->attr_max *= 2;
      sp->attr_buf  = (char*)realloc(sp->attr_buf,sp->attr_max);
    }

  char c;

  while (c = *from++)
    sp->attr_buf[sp->attr_count++] = c;
}

void
y_attr_null_terminate(struct svg_parser* sp)
{
  sp->attr_buf[sp->attr_count] = 0;
}

void
y_attr_reset(struct svg_parser* sp)
{
  sp->attr_count = 0;
}

//
//
//

static void
xml_parse(struct svg_parser* sp, yxml_t* ys, yxml_ret_t yr)
{
  static char*  attrCur;
  static size_t attrLen;

  switch(yr)
    {
    case YXML_OK:
      break;

    case YXML_ELEMSTART:
      // WRITE BEGIN COMMAND TO P/R/L DICTIONARIES
      elem_begin(sp,ys);
      break;

    case YXML_CONTENT:
      break;

    case YXML_ELEMEND:
      // POP AND WRITE END COMMANDS TO P/R/L DICTIONARIES
      elem_end(sp);
      break;

    case YXML_ATTRSTART:
      // SAVE ATTRIBUTE NAME
      attrLen = yxml_symlen(ys,ys->attr); // save attr name len
      attrCur = ys->attr;                 // save attr name
      y_attr_reset(sp);
      break;

    case YXML_ATTRVAL:
      y_attr_copy(sp,ys->data);
      break;

    case YXML_ATTREND:
      // PROCESS ATTRIBUTE AND WRITE BEGIN COMMANDS AND PUSH END
      // COMMANDS TO P/R/L STACKS
      y_attr_null_terminate(sp);
      attrib_dispatch(sp,ys,attrCur,attrLen,sp->attr_buf,sp->attr_count); // process attribute
      break;

    case YXML_PISTART:
    case YXML_PICONTENT:
    case YXML_PIEND:
      break;

    default:
      fprintf(stderr,"t%03llu l%03u b%03llu: error %d\n",ys->total,ys->line,ys->byte,yr);
      exit(yr);
    }
}

//
//
//

static char *
svg_doc_load(const char* const filename)
{
  // open file
  FILE*   f;
  errno_t err = fopen_s(&f,filename,"rb");

  if (err)
    {
      fprintf(stderr,"fopen() error: \"%s\"\n",filename);
      exit(-1);
    }

  // seek to end of file
  fseek(f,0L,SEEK_END);

  // get final position
  const long bytes = ftell(f);

  // rewind to start
  rewind(f);

  // allocate
  char * doc = malloc(bytes+1);

  // read it in
  fread(doc,bytes,1,f);

  // close file
  fclose(f);

  // zero-terminate
  doc[bytes] = '\0';

  return doc;
}


//
//
//

struct svg_doc *
svg_doc_parse(char const * const filename, const bool quiet)
{
  //
  // set flag
  //
  message_quiet = quiet;

  //
  // load SVG doc as image
  //
  char * const doc = svg_doc_load(filename);

  //
  // init svg parser
  //
  struct svg_parser * sp = svg_parser_create();

  //
  // init YXML parser
  //
#define YXML_BUFFER_SIZE  (1024 * 8)

  yxml_t * ys = malloc(sizeof(*ys) + YXML_BUFFER_SIZE);

  yxml_init(ys,ys+1,YXML_BUFFER_SIZE);

  //
  // parse SVG doc
  //
  for (char* c = doc; *c != 0; c++)
    {
      yxml_ret_t yr = yxml_parse(ys,*c);

      xml_parse(sp,ys,yr);
    }

  //
  // check for errors
  //
  yxml_ret_t yr = yxml_eof(ys);

  if (yr < 0)
    {
      fprintf(stderr,"t%03llu l%03u b%03llu: error %d\n",ys->total,ys->line,ys->byte,yr);
      exit(yr);
    }

  //
  // we're done with both the doc and the YXML parser
  //
  free(doc);
  free(ys);

  //
  // end any in-progress p/r/l's
  //
  compile_end(sp);

  //
  // create cmds struct
  //
  struct svg_doc * sd = svg_doc_create(sp);

  //
  // immediately dispose of parser
  //
  svg_parser_dispose(sp);

  //
  //
  //

  return sd;
}

//
//
//

#ifdef SVG_MAIN

#include <getopt.h>

int
main(int argc, char * argv[])
{
  //
  // defaults
  //
  bool quiet = false;

  //
  // process options
  //
  int opt;

  while ((opt = getopt(argc, argv, "hq")) != EOF)
    {
      switch (opt)
        {
        case 'h':
          fprintf(stderr,"Help goes here...\n");
          return -1;

        case 'q':
          quiet = true;
        }
    }

  if (optind >= argc)
    {
      fprintf(stderr,"-- missing filename\n");
      return -1; // no filename
    }

  //
  //
  //

  struct svg_doc * sd = svg_doc_parse(argv[optind],quiet);

  fprintf(stderr,"p/r/l = %u / %u / %u\n",
          svg_doc_path_count(sd),
          svg_doc_raster_count(sd),
          svg_doc_layer_count(sd));

  //
  //
  //

  svg_doc_dispose(sd);
}

#endif // SVG_MAIN

//
//
//

