/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

//
// C++
//

#include "SkDevice_Compute.h"
#include "SkPM4f.h"

//
//
//

#if SK_SUPPORT_GPU_COMPUTE

//
// C++
//

#include "SkImageInfo.h"
#include "SkDraw.h"
#include "SkMatrix.h"
#include "SkPath.h"

//
// C
//

#ifdef __cplusplus
extern "C" {
#endif

#include "../spinel/spinel/color.h"
#include "../compute/skc/skc.h"

#ifdef __cplusplus
}
#endif

//
//
//

SkDevice_Compute::SkDevice_Compute(sk_sp<SkContext_Compute> compute, int w, int h)
    : SkClipStackDevice(SkImageInfo::MakeN32Premul(w,h), SkSurfaceProps(0,kUnknown_SkPixelGeometry))
    , fCompute(std::move(compute))
{
  fTopCTM = this->ctm();
  fTransformWeakref = SKC_WEAKREF_INVALID;

  fClipWeakref = SKC_WEAKREF_INVALID;

  skc_err err;

  //
  // create a composition
  //
#define LAYER_COUNT (1<<14)

  err = skc_composition_create(fCompute->context, &fComposition);
  SKC_ERR_CHECK(err);

    // Is this correct?
    int clipRect[] = { 0, 0, w - 1, h - 1 };
  err = skc_composition_clip_set(fComposition, clipRect);
  SKC_ERR_CHECK(err);

  //
  // create styling
  //
  err = skc_styling_create(fCompute->context,
                           LAYER_COUNT,
                           10,
                           2 * 1024 * 1024,
                           &fStyling);

  //
  // create a path builder
  //
  err = skc_path_builder_create(fCompute->context, &fPB);
  SKC_ERR_CHECK(err);

  //
  // create a raster builder
  //
  err = skc_raster_builder_create(fCompute->context, &fRB);
  SKC_ERR_CHECK(err);

  //
  // create the simplest styling group that encloses all layers
  //
  styling_group_init();
}


//
//
//

SkDevice_Compute::~SkDevice_Compute() {
    skc_err err;

    err = skc_raster_builder_release(fRB);
    SKC_ERR_CHECK(err);

    err = skc_path_builder_release(fPB);
    SKC_ERR_CHECK(err);

    err = skc_styling_dispose(fStyling);
    SKC_ERR_CHECK(err);

    err = skc_composition_dispose(fComposition);
    SKC_ERR_CHECK(err);
}

//
//
//

void SkDevice_Compute::flush() {
    //
    // seal the styling and composition objects
    //
    skc_err err;

    err = skc_composition_seal(fComposition);
    SKC_ERR_CHECK(err);

    err = skc_styling_seal(fStyling);
    SKC_ERR_CHECK(err);

    //
    // we're going to block here -- API mismatch
    //

    //
    // render to surface
    //
    // note this implicitly seals composition and styling
    //
    err = skc_surface_render(fCompute->surface, fComposition, fStyling);
    SKC_ERR_CHECK(err);

    //
    // kick off pipeline and wait here -- not needed since skc_surface_reset() blocks
    //
    err = skc_surface_wait(fCompute->surface);
    SKC_ERR_CHECK(err);

    //
    // reset the surface -- implicitly waits for render to finish -- FIXME -- composition might be released too early
    //
    err = skc_surface_reset(fCompute->surface);
    SKC_ERR_CHECK(err);

    //
    // reset composition and styling
    //
    err = skc_composition_reset(fComposition);
    SKC_ERR_CHECK(err);

    err = skc_styling_reset(fStyling);
    SKC_ERR_CHECK(err);

    //
    //
    //
    styling_group_init();
}

//
//
//

#define SKC_STYLING_CMDS(...) SK_ARRAY_COUNT(__VA_ARGS__),__VA_ARGS__
#define SKC_GROUP_IDS(...)    SK_ARRAY_COUNT(__VA_ARGS__),__VA_ARGS__

void SkDevice_Compute::styling_group_init() {
    skc_styling_group_alloc(fStyling, &fGroupID);
    fParents.push_back(fGroupID);

    // ENTER
    skc_styling_cmd_t const styling_cmds_enter[] = {
        SKC_STYLING_CMD_OP_COVER_ZERO_ACC,
        SKC_STYLING_CMD_OP_COLOR_ZERO_ACC | SKC_STYLING_CMD_OP_IS_FINAL
    };
    skc_styling_group_enter(fStyling, fGroupID, SKC_STYLING_CMDS(styling_cmds_enter));

    skc_group_id const group_id_parents[] = { fGroupID };
    skc_styling_group_parents(fStyling, fGroupID, SKC_GROUP_IDS(group_id_parents));

    // RANGE
    skc_styling_group_range_lo(fStyling, fGroupID, 0);
    skc_styling_group_range_hi(fStyling, fGroupID, LAYER_COUNT-1);

    // LEAVE
    skc_styling_cmd_t const styling_cmds_leave[] = {
        SKC_STYLING_CMD_OP_SURFACE_COMPOSITE | SKC_STYLING_CMD_OP_IS_FINAL
    };
    skc_styling_group_leave(fStyling, fGroupID, SKC_STYLING_CMDS(styling_cmds_leave));

    // START
    fGroupLayerID = LAYER_COUNT-1;
}

//
//
//

#define SK_SCALE_F32      (1.0f/255.0f)
#define SK_TO_RGBA_F32(c) { SK_SCALE_F32 * SkColorGetR(c),      \
                            SK_SCALE_F32 * SkColorGetG(c),      \
                            SK_SCALE_F32 * SkColorGetB(c),      \
                            SK_SCALE_F32 * SkColorGetA(c) }
//
//
//

void SkDevice_Compute::path_rasterize_and_place(const SkPaint&   paint,
                                                const skc_path_t path,
                                                const SkMatrix*  prePathMatrix) {
    float transform[9];
    const SkMatrix& ctm = fTopCTM;
    SkMatrix tmp;

    if (prePathMatrix) {
        tmp.setConcat(ctm, *prePathMatrix);
    }
    transform[0] = tmp.get(SkMatrix::kMScaleX);
    transform[1] = tmp.get(SkMatrix::kMSkewX );
    transform[2] = tmp.get(SkMatrix::kMTransX);
    transform[3] = tmp.get(SkMatrix::kMSkewY );
    transform[4] = tmp.get(SkMatrix::kMScaleY);
    transform[5] = tmp.get(SkMatrix::kMTransY);
    transform[6] = tmp.get(SkMatrix::kMPersp0);
    transform[7] = tmp.get(SkMatrix::kMPersp1);
    transform[8] = tmp.get(SkMatrix::kMPersp2);

    skc_transform_weakref_t& transform_weakref = fTransformWeakref;
    //

    // always invalid for now
    //
    skc_raster_clip_weakref_t clip_weakref = fClipWeakref;

    // TODO Support arbitrary path clip?
    SkRect devClip = SkRect::MakeFromIRect(this->devClipBounds());
    const float clip[] = { devClip.fLeft, devClip.fTop, devClip.fRight, devClip.fBottom };

    //
    //
    //
    skc_err      err;
    skc_raster_t raster;

    err = skc_raster_begin(fRB);
    err = skc_raster_add_filled(fRB, path, &transform_weakref, transform, &clip_weakref, clip);
    err = skc_raster_end(fRB, &raster);

    //
    // can release path handle now because it is being referenced by raster
    //
    err = skc_path_release(fCompute->context, path);

    //
    // style the path
    //
    skc_styling_cmd_t cmds[1 + 3 + 1];

    cmds[0]                      = SKC_STYLING_CMD_OP_COVER_NONZERO;
    cmds[SK_ARRAY_COUNT(cmds)-1] = SKC_STYLING_CMD_OP_BLEND_OVER | SKC_STYLING_CMD_OP_IS_FINAL;

    {
        SkPM4f rgba = SkColor4f::FromColor(paint.getColor()).premul();

        skc_styling_layer_fill_solid_encoder(cmds+1, rgba.fVec);

        skc_styling_group_layer(fStyling, fGroupID, fGroupLayerID, SKC_STYLING_CMDS(cmds));
    }

    err = skc_composition_place(fComposition, fGroupLayerID, raster, 0, 0);

    //
    // can release raster handle now because it is being referenced by composition
    //
    err = skc_raster_release(fCompute->context, raster);

    SkASSERT(err == SKC_ERR_SUCCESS);

    fGroupLayerID -= 1;
}

//
//
//

void SkDevice_Compute::path_add(const SkPaint&  paint,
                                const SkPath&   path,
                                const SkMatrix* prePathMatrix) {
  skc_err err;

  err = skc_path_begin(fPB);

#if 0
  SkPath::Iter    pi(path,false);
#else
  SkPath::RawIter pi(path); // this seems to work fine for now
#endif

  SkPoint xy0;

  //
  // build path
  //
  while (true)
    {
      SkPoint            pts[4];
      SkPath::Verb const verb = pi.next(pts);

      switch (verb)
        {
        case SkPath::kMove_Verb:
	  xy0 = pts[0];
          err = skc_path_move_to(fPB,
                                 pts[0].x(),pts[0].y());
          continue;

        case SkPath::kLine_Verb:
          err = skc_path_line_to(fPB,
                                 pts[1].x(),pts[1].y());
          continue;

        case SkPath::kQuad_Verb:
          err = skc_path_quad_to(fPB,
                                 pts[1].x(),pts[1].y(),
                                 pts[2].x(),pts[2].y());
          continue;

        case SkPath::kConic_Verb: // <--------------------- FIXME
          err = skc_path_line_to(fPB,
                                 pts[2].x(),pts[2].y());
          continue;

        case SkPath::kCubic_Verb:
          err = skc_path_cubic_to(fPB,
                                  pts[1].x(),pts[1].y(),
                                  pts[2].x(),pts[2].y(),
                                  pts[3].x(),pts[3].y());
          continue;

        case SkPath::kClose_Verb:
          err = skc_path_line_to(fPB,xy0.x(),xy0.y());
          continue;

        case SkPath::kDone_Verb:
          break;
        }

      //
      // otherwise, kDone_Verb breaks out of while loop
      //
      break;
    }

  //
  // seal the path
  //
  skc_path_t skc_path;

  err = skc_path_end(fPB,&skc_path);

  //
  // rasterize the path and place it in a composition
  //
  path_rasterize_and_place(paint,skc_path,prePathMatrix);

  SkASSERT(err == SKC_ERR_SUCCESS);
}

//
//
//

void
SkDevice_Compute::circles_add(
			      const SkPaint  & paint,
			      const SkPoint    points[],
			      int32_t  const   count,
			      SkScalar const   radius)
{
#define CIRCLE_KAPPA    0.55228474983079339840f // moar digits!

#define CIRCLE_RADIUS_X radius
#define CIRCLE_RADIUS_Y radius

#define CIRCLE_KAPPA_X  (CIRCLE_RADIUS_X * CIRCLE_KAPPA)
#define CIRCLE_KAPPA_Y  (CIRCLE_RADIUS_Y * CIRCLE_KAPPA)

  //
  // use a 4 Bezier approximation
  //
  float const circle[] =
    {
      0.0f,             +CIRCLE_RADIUS_Y,   // move_to

      +CIRCLE_KAPPA_X,  +CIRCLE_RADIUS_Y,   // cubic_to
      +CIRCLE_RADIUS_X, +CIRCLE_KAPPA_Y,
      +CIRCLE_RADIUS_X,  0.0f,

      +CIRCLE_RADIUS_X, -CIRCLE_KAPPA_Y,    // cubic_to
      +CIRCLE_KAPPA_X,  -CIRCLE_RADIUS_Y,
      0.0f,             -CIRCLE_RADIUS_Y,

      -CIRCLE_KAPPA_X,  -CIRCLE_RADIUS_Y,   // cubic_to
      -CIRCLE_RADIUS_X, -CIRCLE_KAPPA_Y,
      -CIRCLE_RADIUS_X, 0.0f,

      -CIRCLE_RADIUS_X, +CIRCLE_KAPPA_Y,    // cubic_to
      -CIRCLE_KAPPA_X,  +CIRCLE_RADIUS_Y,
      0.0f,             +CIRCLE_RADIUS_Y
    };

#define CXLAT(x,y,t) circle[x]+t.fX,circle[y]+t.fY

  //
  //
  //

  skc_err err;

  err = skc_path_begin(fPB);

  //
  //
  //
  for (int32_t ii=0; ii<count; ii++)
    {
      SkPoint const p = points[ii];

      err = skc_path_move_to(fPB,
			     CXLAT(0,1,p));

      err = skc_path_cubic_to(fPB,
			      CXLAT(2,3,p),
			      CXLAT(4,5,p),
			      CXLAT(6,7,p));

      err = skc_path_cubic_to(fPB,
			      CXLAT(8, 9,p),
			      CXLAT(10,11,p),
			      CXLAT(12,13,p));

      err = skc_path_cubic_to(fPB,
			      CXLAT(14,15,p),
			      CXLAT(16,17,p),
			      CXLAT(18,19,p));

      err = skc_path_cubic_to(fPB,
			      CXLAT(20,21,p),
			      CXLAT(22,23,p),
			      CXLAT(24,25,p));
    }

  //
  // seal the path
  //
  skc_path_t skc_path;

  err = skc_path_end(fPB,&skc_path);

  //
  // rasterize the path and place it in a composition
  //
  path_rasterize_and_place(paint,skc_path,NULL);

  SkASSERT(err == SKC_ERR_SUCCESS);
}

//
//
//

void
SkDevice_Compute::squares_add(
			      const SkPaint  & paint,
			      const SkPoint    points[],
			      int32_t  const   count,
			      SkScalar const   radius)
{
  float const square[] =
    {
      -radius,+radius, // move_to
      +radius,+radius, // line_to
      +radius,-radius, // line_to
      -radius,-radius, // line_to
      -radius,+radius  // line_to
    };

#define SXLAT(x,y,t) square[x]+t.fX,square[y]+t.fY

  //
  //
  //

  skc_err err;

  err = skc_path_begin(fPB);

  //
  //
  //
  for (int32_t ii=0; ii<count; ii++)
    {
      SkPoint const p = points[ii];

      err = skc_path_move_to(fPB,SXLAT(0,1,p));
      err = skc_path_line_to(fPB,SXLAT(2,3,p));
      err = skc_path_line_to(fPB,SXLAT(4,5,p));
      err = skc_path_line_to(fPB,SXLAT(6,7,p));
      err = skc_path_line_to(fPB,SXLAT(8,9,p));
    }

  //
  // seal the path
  //
  skc_path_t skc_path;

  err = skc_path_end(fPB,&skc_path);

  //
  // rasterize the path and place it in a composition
  //
  path_rasterize_and_place(paint,skc_path,NULL);

  SkASSERT(err == SKC_ERR_SUCCESS);
}

//
// FIXME -- THIS IS NOT CORRECT
//
// Need to implement butt, round, square caps
//

void
SkDevice_Compute::line_stroked_butt(SkPoint  const xy0,
				    SkPoint  const xy1,
				    SkScalar const radius)
{
  float const dx    = xy1.fX - xy0.fX;
  float const dy    = xy1.fY - xy0.fY;

  float const hypot = hypotf(dx,dy);

  // FIXME -- what's practical here?
  if (hypot == 0.0f)
    return;

  float const scale = radius / hypot;

  float const rx    = dy * scale;
  float const ry    = dx * scale;

  skc_err err;

  err = skc_path_move_to(fPB,xy0.fX-rx,xy0.fY+ry);
  err = skc_path_line_to(fPB,xy1.fX-rx,xy1.fY+ry);
  err = skc_path_line_to(fPB,xy1.fX+rx,xy1.fY-ry);
  err = skc_path_line_to(fPB,xy0.fX+rx,xy0.fY-ry);
  err = skc_path_line_to(fPB,xy0.fX-rx,xy0.fY+ry);

  SkASSERT(err == SKC_ERR_SUCCESS);
}

void
SkDevice_Compute::lines_stroked_add(
				    const SkPaint  & paint,
				    const SkPoint    points[],
				    int32_t  const   count,
				    SkScalar const   radius)
{
  skc_err err;

  err = skc_path_begin(fPB);

  //
  //
  //
  for (int32_t ii=0; ii<count; ii+=2)
    line_stroked_butt(points[ii],points[ii+1],radius);

  //
  // seal the path
  //
  skc_path_t skc_path;

  err = skc_path_end(fPB,&skc_path);

  //
  // rasterize the path and place it in a composition
  //
  path_rasterize_and_place(paint,skc_path,NULL);

  SkASSERT(err == SKC_ERR_SUCCESS);
}


//
//
//

//  drawPaint is really just a short-cut for drawRect(wide_open, paint)
//  so we have to respect everything (but stroking and maskfilter) in the paint
//  - color | shader
//  - colorFilter
//  - blendmode
//  - etc.
void SkDevice_Compute::drawPaint(const SkPaint& paint) {
  //
  // clear the surface -- will be postponed until render is complete
  //
  SkColor const c       = paint.getColor();
  float         rgba[4] = SK_TO_RGBA_F32(c);

  skc_surface_clear(fCompute->surface,rgba);
}

void SkDevice_Compute::drawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint points[],
                                  const SkPaint& paint) {
    if (count == 0) {
        return;
    }

    const SkScalar radius = paint.getStrokeWidth() * 0.5f;

    /*
     *  drawPoints draws each element (point, line) separately. This means our bulk-adding into the
     *  same raster is not valid for most blendmodes.
     */
    switch (mode) {
        case SkCanvas::kPoints_PointMode: {
            if (paint.getStrokeCap() == SkPaint::kRound_Cap) {
                circles_add(paint, points, (int32_t)count, radius);
            } else {
                squares_add(paint, points,(int32_t)count, radius);
            }
        } break;

        case SkCanvas::kLines_PointMode: {
            if (count <= 1) {
                return;
            }
            lines_stroked_add(paint, points, (int32_t)count & ~1, radius);
        } break;

        case SkCanvas::kPolygon_PointMode: {
            SkPoint xy0 = points[0];
            skc_err err = skc_path_begin(fPB);

            for (size_t i = 0; i < count; ++i) {
                const SkPoint xy1 = points[i];
                line_stroked_butt(xy0, xy1, radius);
                xy0 = xy1;
            }

            //
            // seal the path
            //
            skc_path_t skc_path;
            err = skc_path_end(fPB, &skc_path);

            //
            // rasterize the path and place it in a composition
            //
            path_rasterize_and_place(paint, skc_path, nullptr);

            SkASSERT(err == SKC_ERR_SUCCESS);
        } break;

        default:
            break;
    }
}

void SkDevice_Compute::drawRect(const SkRect& rect, const SkPaint& paint) {
    SkPath path;

    path.addRect(rect);
    this->drawPath(path, paint, nullptr, true);
}

void SkDevice_Compute::drawOval(const SkRect& oval, const SkPaint& paint) {
    SkPath path;

    path.addOval(oval);
    this->drawPath(path, paint, nullptr, true);
}

void SkDevice_Compute::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    SkPath path;

    path.addRRect(rrect);
    this->drawPath(path, paint, nullptr, true);
}

void SkDevice_Compute::drawPath(const SkPath& path, const SkPaint& paint,
                                const SkMatrix* prePathMatrix, bool pathIsMutable) {
  if (paint.getStyle() == SkPaint::kFill_Style) {
      path_add(paint,path,prePathMatrix);
  } else {
      SkPath stroked;

#define SK_MAGIC_RES_SCALE 1024

        paint.getFillPath(path, &stroked, nullptr, SK_MAGIC_RES_SCALE);
        this->path_add(paint, stroked, prePathMatrix);
    }
}

void SkDevice_Compute::drawText(const void*    text,
                                size_t         length,
                                SkScalar       x,
                                SkScalar       y,
                                const SkPaint& paint) {
    SkPath outline;

    paint.getTextPath(text,length,x,y,&outline);
    this->drawPath(outline, paint, nullptr, true);
}

void
SkDevice_Compute::drawPosText(const void     * text,
                              size_t           length,
                              const SkScalar   pos[],
                              int              scalarsPerPos,
                              const SkPoint  & offset,
                              const SkPaint  & paint)
{
#if 0
  draw.drawPosText_asPaths((const char *)text,length,
			   pos,scalarsPerPos,offset,paint);
#endif
}

SkBaseDevice* SkDevice_Compute::onCreateDevice(const CreateInfo& cinfo, const SkPaint* paint) {
#ifdef SK_USE_COMPUTE_LAYER_GROUP
    return this->createLayerGroup(cinfo, paint);
#else
    // TODO return a new SkDevice_Compute when SkDevice_ComputeLayerGroup doesn't work
    return nullptr;
#endif
}

void SkDevice_Compute::drawDevice(SkBaseDevice* device, int left, int top, const SkPaint& paint) {
    // It seems that we won't support image filter until snapSpecial and drawSpecial are implemented
    // (SkCanvas.cpp will call drawSpecial when the paint has an image filter).
    SkASSERT(!paint.getImageFilter());

#ifdef SK_USE_COMPUTE_LAYER_GROUP
    // In case of SkDevice_ComputeLayerGroup, we close the group
    SkDevice_ComputeLayerGroup* layerDevice = static_cast<SkDevice_ComputeLayerGroup*>(device);
    SkASSERT(layerDevice->fRoot == this); // the layerDevice should belong to this root device
    SkASSERT(layerDevice->fGroupID == fGroupID); // the layerDevice should be the top device

    // left, top should be the same as the origin,
    // and we can ignore them because we have no offscreen buffer.
    SkASSERT(SkIPoint::Make(left, top) == device->getOrigin());

    // close the group and pop the top device
    skc_styling_group_range_lo(fStyling, fGroupID, fGroupLayerID + 1);
    fGroupID = fParents.back();
    fParents.pop_back();
#else
    // TODO handle the case where the device is a SkDevice_Compute rather than
    // SkDevice_ComputeLayerGroup (in which case an offscreen buffer is created).
#endif
}

#ifdef SK_USE_COMPUTE_LAYER_GROUP

SkDevice_ComputeLayerGroup* SkDevice_Compute::createLayerGroup(const CreateInfo& cinfo,
        const SkPaint* paint) {
    return new SkDevice_ComputeLayerGroup(this, cinfo, paint);
}

void SkDevice_Compute::onCtmChanged() {
    fTopCTM = this->ctm();
    fTransformWeakref = SKC_WEAKREF_INVALID;
}

SkDevice_ComputeLayerGroup::SkDevice_ComputeLayerGroup(SkDevice_Compute* root,
        const CreateInfo& cinfo, const SkPaint* paint)
        : SkBaseDevice(SkImageInfo::MakeN32Premul(cinfo.fInfo.width(), cinfo.fInfo.height()),
                       SkSurfaceProps(0,kUnknown_SkPixelGeometry)) {
    // TODO clip the group using cinfo; handle the paint's alpha and maybe color filter?

    // Create a new group. We'll restore the previous group during onRestore.
    skc_styling_group_alloc(fRoot->fStyling, &fRoot->fGroupID);
    fRoot->fParents.push_back(fRoot->fGroupID);
    fGroupID = fRoot->fGroupID;

    // ENTER
    skc_styling_cmd_t const styling_cmds_enter[] = {
        SKC_STYLING_CMD_OP_COVER_ZERO_ACC,
        SKC_STYLING_CMD_OP_COLOR_ZERO_ACC
    };
    skc_styling_group_enter(fRoot->fStyling, fRoot->fGroupID, SKC_STYLING_CMDS(styling_cmds_enter));

    skc_styling_group_parents(fRoot->fStyling, fRoot->fGroupID, fRoot->fParents.count(),
            fRoot->fParents.begin());

    // RANGE
    // We'll set range_lo at restore
    skc_styling_group_range_hi(fRoot->fStyling, fRoot->fGroupID, fRoot->fGroupLayerID);

    // LEAVE
    skc_styling_cmd_t const styling_cmds_leave[] = {
        SKC_STYLING_CMD_OP_SURFACE_COMPOSITE
    };
    skc_styling_group_leave(fRoot->fStyling, fRoot->fGroupID, SKC_STYLING_CMDS(styling_cmds_leave));
}

void SkDevice_ComputeLayerGroup::drawDevice(SkBaseDevice* device, int left, int top,
        const SkPaint& paint) {
    fRoot->drawDevice(device, left, top, paint); // the root will properly close the group
}

SkBaseDevice* SkDevice_ComputeLayerGroup::onCreateDevice(const CreateInfo& cinfo,
        const SkPaint* paint) {
    return fRoot->createLayerGroup(cinfo, paint);
}

void SkDevice_ComputeLayerGroup::onCtmChanged() {
    this->sanityCheck();

    // Cancels the translation as we're not using an offscreen buffer
    const SkIPoint& origin = this->getOrigin();
    fRoot->fTopCTM = this->ctm();
    fRoot->fTopCTM.postTranslate(SkIntToScalar(origin.fX), SkIntToScalar(origin.fY));
    fRoot->fTransformWeakref = SKC_WEAKREF_INVALID;
}

void SkDevice_ComputeLayerGroup::onSave() {
    this->sanityCheck();
    fRoot->onSave();
}

void SkDevice_ComputeLayerGroup::onRestore() {
    this->sanityCheck();
    fRoot->onRestore();
}

void SkDevice_ComputeLayerGroup::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
    this->sanityCheck();
    fRoot->fClipStack.clipRect(rect, fRoot->fTopCTM, op, aa);
    fRoot->fClipWeakref = SKC_WEAKREF_INVALID;
}

void SkDevice_ComputeLayerGroup::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    this->sanityCheck();
    fRoot->fClipStack.clipRRect(rrect, fRoot->fTopCTM, op, aa);
    fRoot->fClipWeakref = SKC_WEAKREF_INVALID;
}

void SkDevice_ComputeLayerGroup::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    this->sanityCheck();
    fRoot->fClipStack.clipPath(path, fRoot->fTopCTM, op, aa);
    fRoot->fClipWeakref = SKC_WEAKREF_INVALID;
}

void SkDevice_ComputeLayerGroup::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    this->sanityCheck();
    fRoot->onClipRegion(deviceRgn, op);
}

void SkDevice_ComputeLayerGroup::onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) {
    this->sanityCheck();
    fRoot->onSetDeviceClipRestriction(mutableClipRestriction);
}

#endif // SK_USE_COMPUTE_LAYER_GROUP

#endif
