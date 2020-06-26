void SkSurface_Base::onAsyncRescaleAndReadPixels(const SkImageInfo& info, const SkIRect& srcRect,
                                                 SkSurface::RescaleGamma rescaleGamma,
                                                 SkFilterQuality rescaleQuality,
                                                 SkSurface::ReadPixelsCallback callback,
                                                 SkSurface::ReadPixelsContext context) {
    int srcW = srcRect.width();
    int srcH = srcRect.height();
    float sx = (float)info.width() / srcW;
    float sy = (float)info.height() / srcH;
    // How many bilerp/bicubic steps to do in X and Y. + means upscaling, - means downscaling.
    int stepsX;
    int stepsY;
    if (rescaleQuality > kNone_SkFilterQuality) {
        stepsX = static_cast<int>((sx > 1.f) ? std::ceil(std::log2f(sx))
                                             : std::floor(std::log2f(sx)));
        stepsY = static_cast<int>((sy > 1.f) ? std::ceil(std::log2f(sy))
                                             : std::floor(std::log2f(sy)));
    } else {
        stepsX = sx != 1.f;
        stepsY = sy != 1.f;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    if (stepsX < 0 || stepsY < 0) {
        // Don't trigger MIP generation. We don't currently have a way to trigger bicubic for
        // downscaling draws.
        rescaleQuality = std::min(rescaleQuality, kLow_SkFilterQuality);
    }
    paint.setFilterQuality(rescaleQuality);
    sk_sp<SkSurface> src(SkRef(this));
    int srcX = srcRect.fLeft;
    int srcY = srcRect.fTop;
    SkCanvas::SrcRectConstraint constraint = SkCanvas::kStrict_SrcRectConstraint;
    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == SkSurface::RescaleGamma::kLinear &&
        this->getCanvas()->imageInfo().colorSpace() &&
        !this->getCanvas()->imageInfo().colorSpace()->gammaIsLinear()) {
        auto cs = this->getCanvas()->imageInfo().colorSpace()->makeLinearGamma();
        // Promote to F16 color type to preserve precision.
        auto ii = SkImageInfo::Make(srcW, srcH, kRGBA_F16_SkColorType,
                                    this->getCanvas()->imageInfo().alphaType(), std::move(cs));
        auto linearSurf = this->makeSurface(ii);
        if (!linearSurf) {
            // Maybe F16 isn't supported? Try again with original color type.
            ii = ii.makeColorType(this->getCanvas()->imageInfo().colorType());
            linearSurf = this->makeSurface(ii);
            if (!linearSurf) {
                callback(context, nullptr);
                return;
            }
        }
        this->draw(linearSurf->getCanvas(), -srcX, -srcY, &paint);
        src = std::move(linearSurf);
        srcX = 0;
        srcY = 0;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }
    while (stepsX || stepsY) {
        int nextW = info.width();
        int nextH = info.height();
        if (stepsX < 0) {
            nextW = info.width() << (-stepsX - 1);
            stepsX++;
        } else if (stepsX != 0) {
            if (stepsX > 1) {
                nextW = srcW * 2;
            }
            --stepsX;
        }
        if (stepsY < 0) {
            nextH = info.height() << (-stepsY - 1);
            stepsY++;
        } else if (stepsY != 0) {
            if (stepsY > 1) {
                nextH = srcH * 2;
            }
            --stepsY;
        }
        auto ii = src->getCanvas()->imageInfo().makeWH(nextW, nextH);
        if (!stepsX && !stepsY) {
            // Might as well fold conversion to final info in the last step.
            ii = info;
        }
        auto next = this->makeSurface(ii);
        if (!next) {
            callback(context, nullptr);
            return;
        }
        next->getCanvas()->drawImageRect(
                src->makeImageSnapshot(), SkIRect::MakeXYWH(srcX, srcY, srcW, srcH),
                SkRect::MakeWH((float)nextW, (float)nextH), &paint, constraint);
        src = std::move(next);
        srcX = srcY = 0;
        srcW = nextW;
        srcH = nextH;
        constraint = SkCanvas::kFast_SrcRectConstraint;
    }

    size_t rowBytes = info.minRowBytes();
    std::unique_ptr<char[]> data(new char[info.height() * rowBytes]);
    SkPixmap pm(info, data.get(), rowBytes);
    if (src->readPixels(pm, srcX, srcY)) {
        class Result : public AsyncReadResult {
        public:
            Result(std::unique_ptr<const char[]> data, size_t rowBytes)
                    : fData(std::move(data)), fRowBytes(rowBytes) {}
            int count() const override { return 1; }
            const void* data(int i) const override { return fData.get(); }
            size_t rowBytes(int i) const override { return fRowBytes; }

        private:
            std::unique_ptr<const char[]> fData;
            size_t fRowBytes;
        };
        callback(context, std::make_unique<Result>(std::move(data), rowBytes));
    } else {
        callback(context, nullptr);
    }
}