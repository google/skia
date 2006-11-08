/* libs/graphics/sgl/SkBlitter.h
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

#ifndef SkBlitter_DEFINED
#define SkBlitter_DEFINED

#include "SkBitmap.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkMask.h"

class SkBlitter {
public:
    virtual ~SkBlitter();
    virtual void    blitH(int x, int y, int width);
    virtual void    blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
    virtual void    blitV(int x, int y, int height, SkAlpha alpha);
    virtual void    blitRect(int x, int y, int width, int height);
    virtual void    blitMask(const SkMask&, const SkRect16& clip);

    // not virtual, just a helper
    void blitMaskRegion(const SkMask& mask, const SkRegion& clip);

    static SkBlitter* Choose(const SkBitmap& device,
                             const SkMatrix& matrix,
                             const SkPaint& paint)
    {
        return Choose(device, matrix, paint, nil, 0);
    }

    static SkBlitter* Choose(const SkBitmap& device,
                             const SkMatrix& matrix,
                             const SkPaint& paint,
                             void* storage, size_t storageSize);

    static SkBlitter* ChooseSprite(const SkBitmap& device,
                             const SkPaint&,
                             const SkBitmap& src,
                             int left, int top,
                             void* storage, size_t storageSize);

private:
};

class SkNullBlitter : public SkBlitter {
public:
    virtual void    blitH(int x, int y, int width);
    virtual void    blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
    virtual void    blitV(int x, int y, int height, SkAlpha alpha);
    virtual void    blitRect(int x, int y, int width, int height);
};

class SkRectClipBlitter : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkRect16& clipRect)
    {
        SkASSERT(!clipRect.isEmpty());
        fBlitter = blitter;
        fClipRect = clipRect;
    }

    // overrides
    virtual void    blitH(int x, int y, int width);
    virtual void    blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
    virtual void    blitV(int x, int y, int height, SkAlpha alpha);
    virtual void    blitRect(int x, int y, int width, int height);

private:
    SkBlitter*  fBlitter;
    SkRect16    fClipRect;
};

class SkRgnClipBlitter : public SkBlitter {
public:
    void init(SkBlitter* blitter, const SkRegion* rgn)
    {
        SkASSERT(rgn && !rgn->isEmpty());
        fBlitter = blitter;
        fRgn = rgn;
    }

    // overrides
    virtual void    blitH(int x, int y, int width);
    virtual void    blitAntiH(int x, int y, const SkAlpha antialias[], const S16 runs[]);
    virtual void    blitV(int x, int y, int height, SkAlpha alpha);
    virtual void    blitRect(int x, int y, int width, int height);

private:
    SkBlitter*      fBlitter;
    const SkRegion* fRgn;
};

class SkBlitterClipper {
public:
    SkBlitter*  apply(SkBlitter* blitter, const SkRegion* clip, const SkRect16* bounds = nil);

private:
    SkNullBlitter       fNullBlitter;
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
};

#endif
