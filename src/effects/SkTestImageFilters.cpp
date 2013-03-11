
#include "SkTestImageFilters.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkFlattenableBuffers.h"

// Simple helper canvas that "takes ownership" of the provided device, so that
// when this canvas goes out of scope, so will its device. Could be replaced
// with the following:
//
//  SkCanvas canvas(device);
//  SkAutoTUnref<SkDevice> aur(device);
//
class OwnDeviceCanvas : public SkCanvas {
public:
    OwnDeviceCanvas(SkDevice* device) : SkCanvas(device) {
        SkSafeUnref(device);
    }
};

///////////////////////////////////////////////////////////////////////////////

SkComposeImageFilter::~SkComposeImageFilter() {
}

bool SkComposeImageFilter::onFilterImage(Proxy* proxy,
                                         const SkBitmap& src,
                                         const SkMatrix& ctm,
                                         SkBitmap* result,
                                         SkIPoint* loc) {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    if (!outer && !inner) {
        return false;
    }

    if (!outer || !inner) {
        return (outer ? outer : inner)->filterImage(proxy, src, ctm, result, loc);
    }

    SkBitmap tmp;
    return inner->filterImage(proxy, src, ctm, &tmp, loc) &&
           outer->filterImage(proxy, tmp, ctm, result, loc);
}

bool SkComposeImageFilter::onFilterBounds(const SkIRect& src,
                                          const SkMatrix& ctm,
                                          SkIRect* dst) {
    SkImageFilter* outer = getInput(0);
    SkImageFilter* inner = getInput(1);

    if (!outer && !inner) {
        return false;
    }

    if (!outer || !inner) {
        return (outer ? outer : inner)->filterBounds(src, ctm, dst);
    }

    SkIRect tmp;
    return inner->filterBounds(src, ctm, &tmp) &&
           outer->filterBounds(tmp, ctm, dst);
}

SkComposeImageFilter::SkComposeImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
}

///////////////////////////////////////////////////////////////////////////////

bool SkDownSampleImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                            const SkMatrix&,
                                            SkBitmap* result, SkIPoint*) {
    SkScalar scale = fScale;
    if (scale > SK_Scalar1 || scale <= 0) {
        return false;
    }

    int dstW = SkScalarRoundToInt(src.width() * scale);
    int dstH = SkScalarRoundToInt(src.height() * scale);
    if (dstW < 1) {
        dstW = 1;
    }
    if (dstH < 1) {
        dstH = 1;
    }

    SkBitmap tmp;

    // downsample
    {
        SkDevice* dev = proxy->createDevice(dstW, dstH);
        if (NULL == dev) {
            return false;
        }
        OwnDeviceCanvas canvas(dev);
        SkPaint paint;

        paint.setFilterBitmap(true);
        canvas.scale(scale, scale);
        canvas.drawBitmap(src, 0, 0, &paint);
        tmp = dev->accessBitmap(false);
    }

    // upscale
    {
        SkDevice* dev = proxy->createDevice(src.width(), src.height());
        if (NULL == dev) {
            return false;
        }
        OwnDeviceCanvas canvas(dev);

        SkRect r = SkRect::MakeWH(SkIntToScalar(src.width()),
                                  SkIntToScalar(src.height()));
        canvas.drawBitmapRect(tmp, NULL, r, NULL);
        *result = dev->accessBitmap(false);
    }
    return true;
}

void SkDownSampleImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeScalar(fScale);
}

SkDownSampleImageFilter::SkDownSampleImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fScale = buffer.readScalar();
}
