
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

void SkMergeImageFilter::initAllocModes() {
    int inputCount = countInputs();
    if (inputCount) {
        size_t size = sizeof(uint8_t) * inputCount;
        if (size <= sizeof(fStorage)) {
            fModes = SkTCast<uint8_t*>(fStorage);
        } else {
            fModes = SkTCast<uint8_t*>(sk_malloc_throw(size));
        }
    } else {
        fModes = NULL;
    }
}

void SkMergeImageFilter::initModes(const SkXfermode::Mode modes[]) {
    if (modes) {
        this->initAllocModes();
        int inputCount = countInputs();
        for (int i = 0; i < inputCount; ++i) {
            fModes[i] = SkToU8(modes[i]);
        }
    } else {
        fModes = NULL;
    }
}

SkMergeImageFilter::SkMergeImageFilter(SkImageFilter* first, SkImageFilter* second,
                                       SkXfermode::Mode mode) : INHERITED(first, second) {
    if (SkXfermode::kSrcOver_Mode != mode) {
        SkXfermode::Mode modes[] = { mode, mode };
        this->initModes(modes);
    } else {
        fModes = NULL;
    }
}

SkMergeImageFilter::SkMergeImageFilter(SkImageFilter* filters[], int count,
                                       const SkXfermode::Mode modes[]) : INHERITED(count, filters) {
    this->initModes(modes);
}

SkMergeImageFilter::~SkMergeImageFilter() {

    if (fModes != SkTCast<uint8_t*>(fStorage)) {
        sk_free(fModes);
    }
}

bool SkMergeImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                        SkIRect* dst) {
    if (countInputs() < 1) {
        return false;
    }

    SkIRect totalBounds;

    int inputCount = countInputs();
    for (int i = 0; i < inputCount; ++i) {
        SkImageFilter* filter = getInput(i);
        SkIRect r;
        if (filter) {
            if (!filter->filterBounds(src, ctm, &r)) {
                return false;
            }
        } else {
            r = src;
        }
        if (0 == i) {
            totalBounds = r;
        } else {
            totalBounds.join(r);
        }
    }

    // don't modify dst until now, so we don't accidentally change it in the
    // loop, but then return false on the next filter.
    *dst = totalBounds;
    return true;
}

bool SkMergeImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                       const SkMatrix& ctm,
                                       SkBitmap* result, SkIPoint* loc) {
    if (countInputs() < 1) {
        return false;
    }

    const SkIRect srcBounds = SkIRect::MakeXYWH(loc->x(), loc->y(),
                                                src.width(), src.height());
    SkIRect bounds;
    if (!this->filterBounds(srcBounds, ctm, &bounds)) {
        return false;
    }

    const int x0 = bounds.left();
    const int y0 = bounds.top();

    SkDevice* dst = proxy->createDevice(bounds.width(), bounds.height());
    if (NULL == dst) {
        return false;
    }
    OwnDeviceCanvas canvas(dst);
    SkPaint paint;

    int inputCount = countInputs();
    for (int i = 0; i < inputCount; ++i) {
        SkBitmap tmp;
        const SkBitmap* srcPtr;
        SkIPoint pos = *loc;
        SkImageFilter* filter = getInput(i);
        if (filter) {
            if (!filter->filterImage(proxy, src, ctm, &tmp, &pos)) {
                return false;
            }
            srcPtr = &tmp;
        } else {
            srcPtr = &src;
        }

        if (fModes) {
            paint.setXfermodeMode((SkXfermode::Mode)fModes[i]);
        } else {
            paint.setXfermode(NULL);
        }
        canvas.drawSprite(*srcPtr, pos.x() - x0, pos.y() - y0, &paint);
    }

    loc->set(bounds.left(), bounds.top());
    *result = dst->accessBitmap(false);
    return true;
}

void SkMergeImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);

    buffer.writeBool(fModes != NULL);
    if (fModes) {
        buffer.writeByteArray(fModes, countInputs() * sizeof(fModes[0]));
    }
}

SkMergeImageFilter::SkMergeImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    bool hasModes = buffer.readBool();
    if (hasModes) {
        this->initAllocModes();
        SkASSERT(buffer.getArrayCount() == countInputs() * sizeof(fModes[0]));
        buffer.readByteArray(fModes);
    } else {
        fModes = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool SkDownSampleImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                            const SkMatrix& matrix,
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
