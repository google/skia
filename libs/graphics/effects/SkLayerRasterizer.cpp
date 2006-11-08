/* libs/graphics/effects/SkLayerRasterizer.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkLayerRasterizer.h"
#include "SkBuffer.h"
#include "SkDraw.h"
#include "SkMask.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkXfermode.h"

struct SkLayerRasterizer_Rec {
    SkPaint     fPaint;
    SkVector    fOffset;
};

SkLayerRasterizer::SkLayerRasterizer() : fLayers(sizeof(SkLayerRasterizer_Rec))
{
}

SkLayerRasterizer::~SkLayerRasterizer()
{
    SkDeque::Iter           iter(fLayers);
    SkLayerRasterizer_Rec*  rec;

    while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != NULL)
        rec->fPaint.~SkPaint();
}

void SkLayerRasterizer::addLayer(const SkPaint& paint, SkScalar dx, SkScalar dy)
{
    SkLayerRasterizer_Rec* rec = (SkLayerRasterizer_Rec*)fLayers.push_back();

    new (&rec->fPaint) SkPaint(paint);
    rec->fOffset.set(dx, dy);
}

static bool compute_bounds(const SkDeque& layers, const SkPath& path, const SkMatrix& matrix,
                           const SkRect16* clipBounds, SkRect16* bounds)
{
    SkDeque::Iter           iter(layers);
    SkLayerRasterizer_Rec*  rec;

    bounds->set(SK_MaxS16, SK_MaxS16, SK_MinS16, SK_MinS16);
    
    while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != NULL)
    {
        const SkPaint&  paint = rec->fPaint;
        SkPath          fillPath, devPath;
        const SkPath*   p = &path;

        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style)
        {
            paint.getFillPath(path, &fillPath);
            p = &fillPath;
        }
        if (p->isEmpty())
            continue;

        // apply the matrix and offset
        {
            SkMatrix m = matrix;
            m.preTranslate(rec->fOffset.fX, rec->fOffset.fY);
            p->transform(m, &devPath);
        }

        SkMask  mask;
        if (!SkDraw::DrawToMask(devPath, clipBounds, paint.getMaskFilter(), &matrix,
                                &mask, SkMask::kJustComputeBounds_CreateMode))
            return false;

        bounds->join(mask.fBounds);
    }
    return true;
}

bool SkLayerRasterizer::onRasterize(const SkPath& path, const SkMatrix& matrix,
                                    const SkRect16* clipBounds,
                                    SkMask* mask, SkMask::CreateMode mode)
{
    if (fLayers.empty())
        return false;

    if (SkMask::kJustRenderImage_CreateMode != mode)
    {
        if (!compute_bounds(fLayers, path, matrix, clipBounds, &mask->fBounds))
            return false;
    }

    if (SkMask::kComputeBoundsAndRenderImage_CreateMode == mode)
    {
        mask->fFormat   = SkMask::kA8_Format;
        mask->fRowBytes = SkToU16(mask->fBounds.width());
        mask->fImage = SkMask::AllocImage(mask->computeImageSize());
        memset(mask->fImage, 0, mask->computeImageSize());
    }

    if (SkMask::kJustComputeBounds_CreateMode != mode)
    {    
        SkBitmap device;
        SkDraw   draw;
        SkMatrix translatedMatrix;  // this translates us to our local pixels
        SkMatrix drawMatrix;        // this translates the path by each layer's offset
        SkRegion rectClip;
        
        rectClip.setRect(0, 0, mask->fBounds.width(), mask->fBounds.height());

        translatedMatrix = matrix;
        translatedMatrix.postTranslate(-SkIntToScalar(mask->fBounds.fLeft),
                                       -SkIntToScalar(mask->fBounds.fTop));

        device.setConfig(SkBitmap::kA8_Config, mask->fBounds.width(), mask->fBounds.height(), mask->fRowBytes);
        device.setPixels(mask->fImage);

        draw.fDevice    = &device;
        draw.fMatrix    = &drawMatrix;
        draw.fClip      = &rectClip;
        // we set the matrixproc in the loop, as the matrix changes each time (potentially)
        draw.fBounder   = NULL;
        
        SkDeque::Iter           iter(fLayers);
        SkLayerRasterizer_Rec*  rec;

        while ((rec = (SkLayerRasterizer_Rec*)iter.next()) != NULL)
        {
            drawMatrix = translatedMatrix;
            drawMatrix.preTranslate(rec->fOffset.fX, rec->fOffset.fY);     
            draw.fMapPtProc = drawMatrix.getMapPtProc();

            draw.drawPath(path, rec->fPaint);
        }
    }
    return true;
}

/////////// Routines for flattening /////////////////

static SkFlattenable* load_flattenable(SkRBuffer& buffer)
{
    SkFlattenable::Factory fact = (SkFlattenable::Factory)buffer.readPtr();
    return fact ? fact(buffer) : NULL;
}

static void write_flattenable(SkFlattenable* obj, SkWBuffer& buffer)
{
    if (NULL == obj)
        buffer.writePtr(NULL);
    else
    {
        SkFlattenable::Factory fact = obj->getFactory();
        SkASSERT(fact);
        
        buffer.writePtr((void*)fact);
        obj->flatten(buffer);
    }
}

static void paint_read(SkPaint* paint, SkRBuffer& buffer)
{
    paint->setAntiAliasOn(buffer.readBool());
    paint->setStyle((SkPaint::Style)buffer.readU8());
    paint->setAlpha(buffer.readU8());
    
    if (paint->getStyle() != SkPaint::kFill_Style)
    {
        paint->setStrokeWidth(buffer.readScalar());
        paint->setStrokeMiter(buffer.readScalar());
        paint->setStrokeCap((SkPaint::Cap)buffer.readU8());
        paint->setStrokeJoin((SkPaint::Join)buffer.readU8());
    }

    buffer.skipToAlign4();

    paint->setMaskFilter((SkMaskFilter*)load_flattenable(buffer));
    paint->setPathEffect((SkPathEffect*)load_flattenable(buffer));
    paint->setRasterizer((SkRasterizer*)load_flattenable(buffer));
    paint->setXfermode((SkXfermode*)load_flattenable(buffer));
}

static void paint_write(const SkPaint& paint, SkWBuffer& buffer)
{
    buffer.writeBool(paint.isAntiAliasOn());
    buffer.write8(paint.getStyle());
    buffer.write8(paint.getAlpha());
    
    if (paint.getStyle() != SkPaint::kFill_Style)
    {
        buffer.writeScalar(paint.getStrokeWidth());
        buffer.writeScalar(paint.getStrokeMiter());
        buffer.write8(paint.getStrokeCap());
        buffer.write8(paint.getStrokeJoin());
    }
    
    buffer.padToAlign4();

    write_flattenable(paint.getMaskFilter(), buffer);
    write_flattenable(paint.getPathEffect(), buffer);
    write_flattenable(paint.getRasterizer(), buffer);
    write_flattenable(paint.getXfermode(), buffer);
}

SkLayerRasterizer::SkLayerRasterizer(SkRBuffer& buffer)
    : SkRasterizer(buffer), fLayers(sizeof(SkLayerRasterizer_Rec))
{
    int count = buffer.readS32();
    
    for (int i = 0; i < count; i++)
    {
        SkLayerRasterizer_Rec* rec = (SkLayerRasterizer_Rec*)fLayers.push_back();
    
#if 0
        new (&rec->fPaint) SkPaint(buffer);
#else
        new (&rec->fPaint) SkPaint;
        paint_read(&rec->fPaint, buffer);
#endif
        rec->fOffset.fX = buffer.readScalar();
        rec->fOffset.fY = buffer.readScalar();
    }
}

void SkLayerRasterizer::flatten(SkWBuffer& buffer)
{
    this->INHERITED::flatten(buffer);

    buffer.write32(fLayers.count());

    SkDeque::Iter                   iter(fLayers);
    const SkLayerRasterizer_Rec*    rec;

    while ((rec = (const SkLayerRasterizer_Rec*)iter.next()) != NULL)
    {
#if 0
        rec->fPaint.flatten(buffer);
#else
        paint_write(rec->fPaint, buffer);
#endif
        buffer.writeScalar(rec->fOffset.fX);
        buffer.writeScalar(rec->fOffset.fY);
    }
}

SkFlattenable* SkLayerRasterizer::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkLayerRasterizer, (buffer));
}

SkFlattenable::Factory SkLayerRasterizer::getFactory()
{
    return CreateProc;
}

