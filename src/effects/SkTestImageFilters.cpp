
#include "SkTestImageFilters.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

// Simple helper canvas that "takes ownership" of the provided device, so that
// when this canvas goes out of scope, so will its device. Could be replaced
// with the following:
//
//  SkCanvas canvas(device);
//  SkAutoTUnref<SkBaseDevice> aur(device);
//
class OwnDeviceCanvas : public SkCanvas {
public:
    OwnDeviceCanvas(SkBaseDevice* device) : SkCanvas(device) {
        SkSafeUnref(device);
    }
};

///////////////////////////////////////////////////////////////////////////////

bool SkDownSampleImageFilter::onFilterImage(Proxy* proxy, const SkBitmap& src,
                                            const Context&,
                                            SkBitmap* result, SkIPoint*) const {
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
        SkBaseDevice* dev = proxy->createDevice(dstW, dstH);
        if (NULL == dev) {
            return false;
        }
        OwnDeviceCanvas canvas(dev);
        SkPaint paint;

        paint.setFilterLevel(SkPaint::kLow_FilterLevel);
        canvas.scale(scale, scale);
        canvas.drawBitmap(src, 0, 0, &paint);
        tmp = dev->accessBitmap(false);
    }

    // upscale
    {
        SkBaseDevice* dev = proxy->createDevice(src.width(), src.height());
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

SkFlattenable* SkDownSampleImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    return Create(buffer.readScalar(), common.getInput(0));
}

void SkDownSampleImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fScale);
}

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkDownSampleImageFilter::SkDownSampleImageFilter(SkReadBuffer& buffer)
  : INHERITED(1, buffer) {
    fScale = buffer.readScalar();
    buffer.validate(SkScalarIsFinite(fScale));
}
#endif
