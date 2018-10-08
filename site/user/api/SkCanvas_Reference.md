SkCanvas Reference
===

<a name='SkCanvas'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkCanvas'>SkCanvas</a> {
public:
    static std::unique_ptr<<a href='#SkCanvas'>SkCanvas</a>> <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels,
                                               size_t rowBytes,
                                               const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    static std::unique_ptr<<a href='#SkCanvas'>SkCanvas</a>> <a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a>(int width, int height, <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>* pixels,
                                                         size_t rowBytes);
    <a href='#SkCanvas_empty_constructor'>SkCanvas()</a>;
    <a href='#SkCanvas_int_int_const_SkSurfaceProps_star'>SkCanvas(int width, int height, const SkSurfaceProps* props = nullptr)</a>;
    explicit <a href='#SkCanvas_copy_SkBaseDevice'>SkCanvas(sk_sp<SkBaseDevice> device)</a>;
    explicit <a href='#SkCanvas_copy_const_SkBitmap'>SkCanvas(const SkBitmap& bitmap)</a>;

    enum class <a href='#SkCanvas_ColorBehavior'>ColorBehavior</a> {
        <a href='#SkCanvas_ColorBehavior_kLegacy'>kLegacy</a>,
    };

    <a href='#SkCanvas_const_SkBitmap'>SkCanvas(const SkBitmap& bitmap, ColorBehavior behavior)</a>;
    <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps'>SkCanvas(const SkBitmap& bitmap, const SkSurfaceProps& props)</a>;
    virtual <a href='#SkCanvas_destructor'>~SkCanvas()</a>;
    <a href='undocumented#SkMetaData'>SkMetaData</a>& <a href='#SkCanvas_getMetaData'>getMetaData</a>();
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>() const;
    bool <a href='#SkCanvas_getProps'>getProps</a>(<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props) const;
    void <a href='#SkCanvas_flush'>flush</a>();
    virtual <a href='undocumented#SkISize'>SkISize</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkCanvas_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    virtual <a href='undocumented#GrContext'>GrContext</a>* <a href='#SkCanvas_getGrContext'>getGrContext</a>();
    void* <a href='#SkCanvas_accessTopLayerPixels'>accessTopLayerPixels</a>(<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>* info, size_t* rowBytes, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* origin = nullptr);
    <a href='undocumented#SkRasterHandleAllocator_Handle'>SkRasterHandleAllocator::Handle</a> <a href='#SkCanvas_accessTopRasterHandle'>accessTopRasterHandle</a>() const;
    bool <a href='#SkCanvas_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* pixmap);
    bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);
    bool <a href='#SkCanvas_readPixels_2'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& pixmap, int srcX, int srcY);
    bool <a href='#SkCanvas_readPixels_3'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, int srcX, int srcY);
    bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* pixels, size_t rowBytes, int x, int y);
    bool <a href='#SkCanvas_writePixels_2'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, int x, int y);
    int <a href='#SkCanvas_save'>save</a>();
    int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    int <a href='#SkCanvas_saveLayer_2'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    int <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    int <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, <a href='undocumented#U8CPU'>U8CPU</a> alpha);

    enum <a href='#SkCanvas_SaveLayerFlagsSet'>SaveLayerFlagsSet</a> {
        <a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a> = 1 << 1,
        <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> = 1 << 2,
        <a href='#SkCanvas_kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag'>kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag</a> =
                                          1 << 3,
        <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>kDontClipToLayer_Legacy_SaveLayerFlag</a> =
           kDontClipToLayer_PrivateSaveLayerFlag,
    };

    typedef uint32_t <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a>;

    struct <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> {
        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>();
        <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags = 0);
        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint, const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* backdrop,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags);
        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint, const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* backdrop,
                     const <a href='SkImage_Reference#SkImage'>SkImage</a>* clipMask, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* clipMatrix,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags);
        const <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> = nullptr;
        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a> = nullptr;
        const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> = nullptr;
        const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> = nullptr;
        const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> = nullptr;
        <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a> = 0;
    };

    int <a href='#SkCanvas_saveLayer_3'>saveLayer</a>(const <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>& layerRec);
    void <a href='#SkCanvas_restore'>restore</a>();
    int <a href='#SkCanvas_getSaveCount'>getSaveCount</a>() const;
    void <a href='#SkCanvas_restoreToCount'>restoreToCount</a>(int saveCount);
    void <a href='#SkCanvas_translate'>translate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkCanvas_scale'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    void <a href='#SkCanvas_rotate'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees);
    void <a href='#SkCanvas_rotate_2'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkCanvas_skew'>skew</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    void <a href='#SkCanvas_concat'>concat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix);
    void <a href='#SkCanvas_setMatrix'>setMatrix</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix);
    void <a href='#SkCanvas_resetMatrix'>resetMatrix</a>();
    void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias);
    void <a href='#SkCanvas_clipRect_2'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkClipOp'>SkClipOp</a> op);
    void <a href='#SkCanvas_clipRect_3'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, bool doAntiAlias = false);
    void <a href='#SkCanvas_androidFramework_setDeviceClipRestriction'>androidFramework_setDeviceClipRestriction</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& rect);
    void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias);
    void <a href='#SkCanvas_clipRRect_2'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='undocumented#SkClipOp'>SkClipOp</a> op);
    void <a href='#SkCanvas_clipRRect_3'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, bool doAntiAlias = false);
    void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& path, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias);
    void <a href='#SkCanvas_clipPath_2'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& path, <a href='undocumented#SkClipOp'>SkClipOp</a> op);
    void <a href='#SkCanvas_clipPath_3'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& path, bool doAntiAlias = false);
    void <a href='#SkCanvas_setAllowSimplifyClip'>setAllowSimplifyClip</a>(bool allow);
    void <a href='#SkCanvas_clipRegion'>clipRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& deviceRgn, <a href='undocumented#SkClipOp'>SkClipOp</a> op = <a href='undocumented#SkClipOp_kIntersect'>SkClipOp::kIntersect</a>);
    bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect) const;
    bool <a href='#SkCanvas_quickReject_2'>quickReject</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& path) const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>() const;
    bool <a href='#SkCanvas_getLocalClipBounds_2'>getLocalClipBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>() const;
    bool <a href='#SkCanvas_getDeviceClipBounds_2'>getDeviceClipBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* bounds) const;
    void <a href='#SkCanvas_drawColor'>drawColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode = <a href='SkBlendMode_Reference#SkBlendMode_kSrcOver'>SkBlendMode::kSrcOver</a>);
    void <a href='#SkCanvas_clear'>clear</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color);
    void <a href='#SkCanvas_discard'>discard</a>();
    void <a href='#SkCanvas_drawPaint'>drawPaint</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);

    enum <a href='#SkCanvas_PointMode'>PointMode</a> {
        <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>,
        <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>,
        <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>,
    };

    void <a href='#SkCanvas_drawPoints'>drawPoints</a>(<a href='#SkCanvas_PointMode'>PointMode</a> mode, size_t count, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPoint_2'>drawPoint</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='undocumented#SkScalar'>SkScalar</a> x0, <a href='undocumented#SkScalar'>SkScalar</a> y0, <a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawLine_2'>drawLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p0, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p1, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawRect'>drawRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawIRect'>drawIRect</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& rect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawRegion'>drawRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& region, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawOval'>drawOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawRRect'>drawRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawDRRect'>drawDRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& outer, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& inner, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> cx, <a href='undocumented#SkScalar'>SkScalar</a> cy, <a href='undocumented#SkScalar'>SkScalar</a> radius, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawCircle_2'>drawCircle</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> center, <a href='undocumented#SkScalar'>SkScalar</a> radius, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawArc'>drawArc</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle,
                 bool useCenter, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPath'>drawPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& path, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* image, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);
    void <a href='#SkCanvas_drawImage_2'>drawImage</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& image, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);

    enum <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> {
        <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>,
        <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>,
    };

    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* image, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect_2'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* image, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect_3'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* image, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect_4'>drawImageRect</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& image, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect_5'>drawImageRect</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& image, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect_6'>drawImageRect</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& image, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* image, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);
    void <a href='#SkCanvas_drawImageNine_2'>drawImageNine</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& image, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);
    void <a href='#SkCanvas_drawBitmap'>drawBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
                    const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);
    void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawBitmapRect_2'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawBitmapRect_3'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);

    struct <a href='#SkCanvas_Lattice'>Lattice</a> {

        enum <a href='#SkCanvas_Lattice_RectType'>RectType</a> : uint8_t {
            <a href='#SkCanvas_Lattice_kDefault'>kDefault</a> = 0,
            <a href='#SkCanvas_Lattice_kTransparent'>kTransparent</a>,
            <a href='#SkCanvas_Lattice_kFixedColor'>kFixedColor</a>,
        };

        const int* <a href='#SkCanvas_Lattice_fXDivs'>fXDivs</a>;
        const int* <a href='#SkCanvas_Lattice_fYDivs'>fYDivs</a>;
        const <a href='#SkCanvas_Lattice_RectType'>RectType</a>* <a href='#SkCanvas_Lattice_fRectTypes'>fRectTypes</a>;
        int <a href='#SkCanvas_Lattice_fXCount'>fXCount</a>;
        int <a href='#SkCanvas_Lattice_fYCount'>fYCount</a>;
        const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='#SkCanvas_Lattice_fBounds'>fBounds</a>;
        const <a href='SkColor_Reference#SkColor'>SkColor</a>* <a href='#SkCanvas_Lattice_fColors'>fColors</a>;
    };

    void <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap, const <a href='#SkCanvas_Lattice'>Lattice</a>& lattice, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                           const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);
    void <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* image, const <a href='#SkCanvas_Lattice'>Lattice</a>& lattice, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                          const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint = nullptr);
    void <a href='#SkCanvas_drawText'>drawText</a>(const void* text, size_t byteLength, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                  const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawString'>drawString</a>(const char* string, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawString_2'>drawString</a>(const <a href='undocumented#SkString'>SkString</a>& string, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPosText'>drawPosText</a>(const void* text, size_t byteLength, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[],
                     const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>(const void* text, size_t byteLength, const <a href='undocumented#SkScalar'>SkScalar</a> xpos[], <a href='undocumented#SkScalar'>SkScalar</a> constY,
                      const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>(const void* text, size_t byteLength, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[],
                         const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* blob, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawTextBlob_2'>drawTextBlob</a>(const sk_sp<<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>>& blob, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* picture);
    void <a href='#SkCanvas_drawPicture_2'>drawPicture</a>(const sk_sp<<a href='SkPicture_Reference#SkPicture'>SkPicture</a>>& picture);
    void <a href='#SkCanvas_drawPicture_3'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* picture, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* matrix, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    void <a href='#SkCanvas_drawPicture_4'>drawPicture</a>(const sk_sp<<a href='SkPicture_Reference#SkPicture'>SkPicture</a>>& picture, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* matrix,
                     const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>* vertices, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawVertices_2'>drawVertices</a>(const sk_sp<<a href='undocumented#SkVertices'>SkVertices</a>>& vertices, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawVertices_3'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>* vertices, const <a href='undocumented#SkVertices_Bone'>SkVertices::Bone</a> bones[], int boneCount,
                      <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawVertices_4'>drawVertices</a>(const sk_sp<<a href='undocumented#SkVertices'>SkVertices</a>>& vertices, const <a href='undocumented#SkVertices_Bone'>SkVertices::Bone</a> bones[],
                      int boneCount, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> cubics[12], const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[4],
                   const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> texCoords[4], <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawPatch_2'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> cubics[12], const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[4],
                   const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> texCoords[4], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
                   const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[], int count, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    void <a href='#SkCanvas_drawAtlas_2'>drawAtlas</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
                   const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[], int count, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    void <a href='#SkCanvas_drawAtlas_3'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[], int count,
                   const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    void <a href='#SkCanvas_drawAtlas_4'>drawAtlas</a>(const sk_sp<<a href='SkImage_Reference#SkImage'>SkImage</a>>& atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
                   int count, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* drawable, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* matrix = nullptr);
    void <a href='#SkCanvas_drawDrawable_2'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* drawable, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y);
    void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, const char key[], <a href='undocumented#SkData'>SkData</a>* value);
    void <a href='#SkCanvas_drawAnnotation_2'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, const char key[], const sk_sp<<a href='undocumented#SkData'>SkData</a>>& value);
    virtual bool <a href='#SkCanvas_isClipEmpty'>isClipEmpty</a>() const;
    virtual bool <a href='#SkCanvas_isClipRect'>isClipRect</a>() const;
    const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a>() const;
};
</pre>

<a href='#Canvas'>Canvas</a> provides an interface for drawing

## <a name='Constructors'>Constructors</a>

Create the desired type of <a href='SkSurface_Reference#Surface'>Surface</a> to obtain its <a href='#Canvas'>Canvas</a> when possible

<a name='SkCanvas_MakeRasterDirect'></a>
## MakeRasterDirect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr
</pre>

Allocates raster <a href='#Canvas'>Canvas</a> that will draw directly into <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirect_info'><code><strong>info</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#Surface'>Surface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts</td>
  </tr>
</table>

### Return Value

<a href='#Canvas'>Canvas</a> if all parameters are valid

### Example

<div><fiddle-embed name="525285073aae7e53eb8f454a398f880c"><div>Allocates a three by three bitmap</div>

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a> <a href='SkSurface_Reference#SkSurface_MakeRasterDirect'>SkSurface::MakeRasterDirect</a>

---

<a name='SkCanvas_MakeRasterDirectN32'></a>
## MakeRasterDirectN32

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr
</pre>

Allocates raster <a href='#Canvas'>Canvas</a> specified by inline image specification

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_width'><code><strong>width</strong></code></a></td>
    <td>pixel column count on <a href='undocumented#Raster_Surface'>Raster Surface</a> created</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_height'><code><strong>height</strong></code></a></td>
    <td>pixel row count on <a href='undocumented#Raster_Surface'>Raster Surface</a> created</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#Surface'>Surface</a> row to the next</td>
  </tr>
</table>

### Return Value

<a href='#Canvas'>Canvas</a> if all parameters are valid

### Example

<div><fiddle-embed name="4cacf302830e644234d522f6e2f8f580"><div>Allocates a three by three bitmap</div>

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='SkSurface_Reference#SkSurface_MakeRasterDirect'>SkSurface::MakeRasterDirect</a> <a href='SkImageInfo_Reference#SkImageInfo_MakeN32Premul'>SkImageInfo::MakeN32Premul</a><sup><a href='SkImageInfo_Reference#SkImageInfo_MakeN32Premul_2'>[2]</a></sup>

---

<a name='SkCanvas_empty_constructor'></a>
## SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas'>SkCanvas</a>(
</pre>

Creates an empty <a href='#Canvas'>Canvas</a> with no backing device or pixels

### Return Value

empty <a href='#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="903451d6c93bf69e2833747a3e8cc8f7"><div>Passes a placeholder to a function that requires one</div>

#### Example Output

~~~~
paint draws text left to right
paint draws text top to bottom
paint draws text top to bottom
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkRasterHandleAllocator_MakeCanvas'>SkRasterHandleAllocator::MakeCanvas</a> <a href='SkSurface_Reference#SkSurface_getCanvas'>SkSurface::getCanvas</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

---

<a name='SkCanvas_int_int_const_SkSurfaceProps_star'></a>
## SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas'>SkCanvas</a>(int width
</pre>

Creates <a href='#Canvas'>Canvas</a> of the specified dimensions without a <a href='SkSurface_Reference#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_width'><code><strong>width</strong></code></a></td>
    <td>zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_height'><code><strong>height</strong></code></a></td>
    <td>zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts</td>
  </tr>
</table>

### Return Value

<a href='#Canvas'>Canvas</a> placeholder with dimensions

### Example

<div><fiddle-embed name="ce6a5ef2df447970b4453489d9d67930">

#### Example Output

~~~~
canvas is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkPixelGeometry'>SkPixelGeometry</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

---

<a name='SkCanvas_copy_SkBaseDevice'></a>
## SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
explicit <a href='#SkCanvas'>SkCanvas</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

To be deprecated soon.

---

<a name='SkCanvas_copy_const_SkBitmap'></a>
## SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
explicit <a href='#SkCanvas'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Constructs a canvas that draws into <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_copy_const_SkBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>width</td>
  </tr>
</table>

### Return Value

<a href='#Canvas'>Canvas</a> that can be used to draw into <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a>

### Example

<div><fiddle-embed name="dd92db963af190e849894038f39b598a"><div>The actual output depends on the installed fonts</div>

#### Example Output

~~~~
-----
---x-
---x-
---x-
---x-
---x-
---x-
-----
---x-
---x-
-----
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkRasterHandleAllocator_MakeCanvas'>SkRasterHandleAllocator::MakeCanvas</a> <a href='SkSurface_Reference#SkSurface_getCanvas'>SkSurface::getCanvas</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

---

## <a name='SkCanvas_ColorBehavior'>Enum SkCanvas::ColorBehavior</a>

Private: Android framework only.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum class <a href='#SkCanvas_ColorBehavior'>ColorBehavior</a> {
        <a href='#SkCanvas_ColorBehavior_kLegacy'>kLegacy</a>,
    };
</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_ColorBehavior_kLegacy'><code>SkCanvas::ColorBehavior::kLegacy</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Is a placeholder to allow specialized constructor</td>
  </tr>
</table>

<a name='SkCanvas_const_SkBitmap'></a>
## SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

For use by Android framework only

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>specifies a <a href='#SkCanvas_const_SkBitmap_bitmap'>bitmap</a> for the canvas to draw into</td>
  </tr>
  <tr>    <td><a name='SkCanvas_const_SkBitmap_behavior'><code><strong>behavior</strong></code></a></td>
    <td>specializes this constructor</td>
  </tr>
</table>

### Return Value

<a href='#Canvas'>Canvas</a> that can be used to draw into <a href='#SkCanvas_const_SkBitmap_bitmap'>bitmap</a>

---

<a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps'></a>
## SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Constructs a canvas that draws into <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'><code><strong>props</strong></code></a></td>
    <td>order and orientation of RGB striping</td>
  </tr>
</table>

### Return Value

<a href='#Canvas'>Canvas</a> that can be used to draw into <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a>

### Example

<div><fiddle-embed name="c26cfae4c42cb445240335cc12a50235"><div>The actual output depends on the installed fonts</div>

#### Example Output

~~~~
-----
---x-
---x-
---x-
---x-
---x-
---x-
-----
---x-
---x-
-----
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkRasterHandleAllocator_MakeCanvas'>SkRasterHandleAllocator::MakeCanvas</a> <a href='SkSurface_Reference#SkSurface_getCanvas'>SkSurface::getCanvas</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

---

<a name='SkCanvas_destructor'></a>
## ~SkCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='#SkCanvas_destructor'>~SkCanvas</a>(
</pre>

Draws saved <a href='#Layer'>Layers</a>

### Example

<div><fiddle-embed name="b7bc91ff16c9b9351b2a127f35394b82"><div><a href='#Canvas'>Canvas</a> <a href='#Layer'>Layer</a> draws into bitmap</div></fiddle-embed></div>

### See Also

<a href='#State_Stack'>State Stack</a>

---

## <a name='Property'>Property</a>

<a name='SkCanvas_getMetaData'></a>
## getMetaData

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMetaData'>SkMetaData</a>
</pre>

Returns storage to associate additional data with the canvas

### Return Value

storage that can be read from and written to

### Example

<div><fiddle-embed name="1598396056045e8d0c583b748293d652">

#### Example Output

~~~~
before: (null)
during: Hello!
after: (null)
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkMetaData'>SkMetaData</a>

---

<a name='SkCanvas_imageInfo'></a>
## imageInfo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>(
</pre>

Returns <a href='SkImageInfo_Reference#Image_Info'>Image Info</a> for <a href='#Canvas'>Canvas</a>

### Return Value

dimensions and <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> of <a href='#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="d93389d971f8084c4ccc7a66e4e157ee">

#### Example Output

~~~~
emptyInfo == canvasInfo
~~~~

</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkCanvas_makeSurface'>makeSurface</a>

---

<a name='SkCanvas_getProps'></a>
## getProps

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_getProps'>getProps</a>(<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>
</pre>

Copies <a href='undocumented#Surface_Properties'>Surface Properties</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getProps_props'><code><strong>props</strong></code></a></td>
    <td>storage for writable <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a></td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#Surface_Properties'>Surface Properties</a> was copied

### Example

<div><fiddle-embed name="0fbf2dedc2619bbfbf173c9e3bc1a508">

#### Example Output

~~~~
isRGB:0
isRGB:1
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='#SkCanvas_makeSurface'>makeSurface</a>

---

## <a name='Utility'>Utility</a>

<a name='SkCanvas_flush'></a>
## flush

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_flush'>flush</a>(
</pre>

Triggers the immediate execution of all pending draw operations

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='SkSurface_Reference#SkSurface_flush'>SkSurface::flush</a>(

---

<a name='SkCanvas_getBaseLayerSize'></a>
## getBaseLayerSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='undocumented#SkISize'>SkISize</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a>(
</pre>

Gets the size of the base or root <a href='#Layer'>Layer</a> in global canvas coordinates

### Return Value

integral width and height of base <a href='#Layer'>Layer</a>

### Example

<div><fiddle-embed name="374e245d91cd729eca48fd20e631fdf3">

#### Example Output

~~~~
clip=10,30
size=20,30
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a><sup><a href='#SkCanvas_getDeviceClipBounds_2'>[2]</a></sup>

---

<a name='SkCanvas_makeSurface'></a>
## makeSurface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='SkSurface_Reference#Surface'>Surface</a> matching <a href='#SkCanvas_makeSurface_info'>info</a> and <a href='#SkCanvas_makeSurface_props'>props</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_makeSurface_info'><code><strong>info</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkCanvas_makeSurface_props'><code><strong>props</strong></code></a></td>
    <td><a href='undocumented#Surface_Properties'>Surface Properties</a> to match</td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#Surface'>Surface</a> matching <a href='#SkCanvas_makeSurface_info'>info</a> and <a href='#SkCanvas_makeSurface_props'>props</a>

### Example

<div><fiddle-embed name="1ce28351444b41ab2b8e3128a4b9b9c2">

#### Example Output

~~~~
compatible != nullptr
size = 3, 4
~~~~

</fiddle-embed></div>

### See Also

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface_makeSurface'>SkSurface::makeSurface</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>

---

<a name='SkCanvas_getGrContext'></a>
## getGrContext

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='undocumented#GrContext'>GrContext</a>
</pre>

Returns <a href='undocumented#GPU_Context'>GPU Context</a> of the <a href='undocumented#GPU_Surface'>GPU Surface</a> associated with <a href='#Canvas'>Canvas</a>

### Return Value

<a href='undocumented#GPU_Context'>GPU Context</a>

### Example

<div><fiddle-embed name="c4ea949e5fa5a0630dcb6b0204bd498f"></fiddle-embed></div>

### See Also

<a href='undocumented#GrContext'>GrContext</a>

---

<a name='SkCanvas_accessTopLayerPixels'></a>
## accessTopLayerPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void
</pre>

Returns the pixel base address

### Parameters

<table>  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_info'><code><strong>info</strong></code></a></td>
    <td>storage for writable pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>storage for writable pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_origin'><code><strong>origin</strong></code></a></td>
    <td>storage for <a href='#Canvas'>Canvas</a> top <a href='#Layer'>Layer</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a></td>
  </tr>
</table>

### Return Value

address of pixels

### Example

<div><fiddle-embed name="38d0d6ca9bea146d31bcbec197856359"></fiddle-embed></div>

### Example

<div><fiddle-embed name="a7ac9c21bbabcdeeca00f72a61cd0f3e"><div>Draws</div></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>

---

<a name='SkCanvas_accessTopRasterHandle'></a>
## accessTopRasterHandle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkRasterHandleAllocator_Handle'>SkRasterHandleAllocator::Handle</a> <a href='#SkCanvas_accessTopRasterHandle'>accessTopRasterHandle</a>(
</pre>

Returns custom context that tracks the <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Return Value

context of custom allocation

### Example

<div><fiddle-embed name="4486d0c0b22ad2931db130f42da4c80c"><div></div>

#### Example Output

~~~~
context = skia
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>

---

## <a name='Pixels'>Pixels</a>

<a name='SkCanvas_peekPixels'></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Returns true if <a href='#Canvas'>Canvas</a> has direct access to its pixels

### Parameters

<table>  <tr>    <td><a name='SkCanvas_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for pixel state if pixels are readable</td>
  </tr>
</table>

### Return Value

true if <a href='#Canvas'>Canvas</a> has direct access to pixels

### Example

<div><fiddle-embed name="e9411d676d1fa13b46331abe9e14ad3e">

#### Example Output

~~~~
width=256 height=256
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a><sup><a href='#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_peekPixels'>SkBitmap::peekPixels</a> <a href='SkImage_Reference#SkImage_peekPixels'>SkImage::peekPixels</a> <a href='SkSurface_Reference#SkSurface_peekPixels'>SkSurface::peekPixels</a>

---

<a name='SkCanvas_readPixels'></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Canvas'>Canvas</a> into <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels could not be converted to <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a></td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels are not readable</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_dstRowBytes'>dstRowBytes</a> is too small to contain one row of pixels</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>size of one destination row</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="102d014d7f753db2a9b9ee08893aaf11"><div>A black circle drawn on a blue background provides an image to copy</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="481e990e923a0ed34654f4361b94f096"><div><a href='#Canvas'>Canvas</a> returned by <a href='undocumented#Raster_Surface'>Raster Surface</a> has <a href='undocumented#Premultiply'>Premultiplied</a> pixel values</div>

#### Example Output

~~~~
pixel = 802b5580
pixel = 8056a9ff
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='#SkCanvas_writePixels'>writePixels</a><sup><a href='#SkCanvas_writePixels_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkPixmap_Reference#SkPixmap_readPixels'>SkPixmap::readPixels</a><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_2'>[2]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_3'>[3]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_4'>[4]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkCanvas_readPixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Canvas'>Canvas</a> into <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels could not be converted to <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a></td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels are not readable</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> pixels could not be allocated</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_2_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for pixels copied from <a href='#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="85f199032943b6483722c34a91c4e20f"><div><a href='#SkCanvas_clear'>clear</a>(</div>

#### Example Output

~~~~
pixel = 802b5580
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='#SkCanvas_writePixels'>writePixels</a><sup><a href='#SkCanvas_writePixels_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkPixmap_Reference#SkPixmap_readPixels'>SkPixmap::readPixels</a><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_2'>[2]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_3'>[3]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_4'>[4]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkCanvas_readPixels_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Canvas'>Canvas</a> into <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels could not be converted to <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a></td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels are not readable</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a> pixels could not be allocated</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_3_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>storage for pixels copied from <a href='#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="af6dec8ef974aa67bf102f29915bcd6a"><div><a href='#SkCanvas_clear'>clear</a>(</div>

#### Example Output

~~~~
pixel = 802b5580
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='#SkCanvas_writePixels'>writePixels</a><sup><a href='#SkCanvas_writePixels_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkPixmap_Reference#SkPixmap_readPixels'>SkPixmap::readPixels</a><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_2'>[2]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_3'>[3]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_4'>[4]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkCanvas_writePixels'></a>
## writePixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> from <a href='#SkCanvas_writePixels_pixels'>pixels</a> to <a href='#Canvas'>Canvas</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_pixels'>pixels</a> could not be converted to <a href='#Canvas'>Canvas</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>(</td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> are not writable</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_rowBytes'>rowBytes</a> is too small to contain one row of <a href='#SkCanvas_writePixels_pixels'>pixels</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_writePixels_info'><code><strong>info</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td><a href='#SkCanvas_writePixels_pixels'>pixels</a> to copy</td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>size of one row of <a href='#SkCanvas_writePixels_pixels'>pixels</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='#Canvas'>Canvas</a> writable <a href='#SkCanvas_writePixels_pixels'>pixels</a> on <a href='#SkCanvas_writePixels_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='#Canvas'>Canvas</a> writable <a href='#SkCanvas_writePixels_pixels'>pixels</a> on <a href='#SkCanvas_writePixels_y'>y</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_writePixels_pixels'>pixels</a> were written to <a href='#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="29b98ebf58aa9fd1edfaabf9f4490b3a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a><sup><a href='#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='#SkCanvas_readPixels_3'>[3]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='SkBitmap_Reference#SkBitmap_writePixels'>SkBitmap::writePixels</a><sup><a href='SkBitmap_Reference#SkBitmap_writePixels_2'>[2]</a></sup>

---

<a name='SkCanvas_writePixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> from pixels to <a href='#Canvas'>Canvas</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> does not have allocated pixels</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> pixels could not be converted to <a href='#Canvas'>Canvas</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>(</td>
  </tr>  <tr>
    <td><a href='#Canvas'>Canvas</a> pixels are not writable</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> pixels are inaccessible</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_writePixels_2_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>contains pixels copied to <a href='#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_2_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='#Canvas'>Canvas</a> writable pixels in <a href='#SkCanvas_writePixels_2_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_2_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='#Canvas'>Canvas</a> writable pixels in <a href='#SkCanvas_writePixels_2_y'>y</a></td>
  </tr>
</table>

### Return Value

true if pixels were written to <a href='#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="8b128e067881f9251357653692fa28da"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a><sup><a href='#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='#SkCanvas_readPixels_3'>[3]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='SkBitmap_Reference#SkBitmap_writePixels'>SkBitmap::writePixels</a><sup><a href='SkBitmap_Reference#SkBitmap_writePixels_2'>[2]</a></sup>

---

## <a name='State_Stack'>State Stack</a>

<a href='#Canvas'>Canvas</a> maintains a stack of state that allows hierarchical drawing

### Example

<div><fiddle-embed name="bb1dbfdca3aedf716beb6f07e2aab065"><div><a href='#Draw'>Draw</a> to ever smaller clips</div></fiddle-embed></div>

Each <a href='#Clip'>Clip</a> uses the current <a href='#Matrix'>Matrix</a> for its coordinates

### Example

<div><fiddle-embed name="9f563a2d60aa31d4b26742e5aa17aa4e"><div>While <a href='#SkCanvas_clipRect'>clipRect</a> is given the same rectangle twice</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

<a name='SkCanvas_save'></a>
## save

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_save'>save</a>(
</pre>

Saves <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="e477dce358a9ba3b0aa1bf33b8a376de"><div>The black square is translated 50 pixels down and to the right</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_saveLayer'>saveLayer</a><sup><a href='#SkCanvas_saveLayer_2'>[2]</a></sup><sup><a href='#SkCanvas_saveLayer_3'>[3]</a></sup> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restore'>restore</a>(

---

<a name='SkCanvas_restore'></a>
## restore

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_restore'>restore</a>(
</pre>

Removes changes to <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a> since <a href='#Canvas'>Canvas</a> state was
last saved

### Example

<div><fiddle-embed name="e78471212a67f2f4fd39496e17a30d17"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_getSaveCount'></a>
## getSaveCount

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_getSaveCount'>getSaveCount</a>(
</pre>

Returns the number of saved states

### Return Value

depth of save state stack

### Example

<div><fiddle-embed name="005f2b207e078baac596681924fe591e">

#### Example Output

~~~~
depth = 1
depth = 2
depth = 1
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_restoreToCount'></a>
## restoreToCount

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_restoreToCount'>restoreToCount</a>(int saveCount
</pre>

Restores state to <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a> values when <a href='#SkCanvas_save'>save</a>(

### Parameters

<table>  <tr>    <td><a name='SkCanvas_restoreToCount_saveCount'><code><strong>saveCount</strong></code></a></td>
    <td>depth of state stack to restore</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9ed0d56436e114c7097fd49eed1aea47">

#### Example Output

~~~~
depth = 1
depth = 3
depth = 1
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_restore'>restore</a>(

---

## <a name='Layer'>Layer</a>

<a href='#Layer'>Layer</a> allocates a temporary <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to draw into

<a name='SkCanvas_saveLayer'></a>
## saveLayer

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Saves <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the size of the <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayer_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="42318b18d403e17e07a541652da91ee2"><div>Rectangles are blurred by <a href='undocumented#Image_Filter'>Image Filter</a> when <a href='#SkCanvas_restore'>restore</a>(</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_saveLayer_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Saves <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the size of <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayer_2_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="a17aec3aa4909527be039e26a7eda694"><div>Rectangles are blurred by <a href='undocumented#Image_Filter'>Image Filter</a> when <a href='#SkCanvas_restore'>restore</a>(</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_saveLayerPreserveLCDTextRequests'></a>
## saveLayerPreserveLCDTextRequests

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Saves <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayerPreserveLCDTextRequests_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the size of <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayerPreserveLCDTextRequests_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="8460bf8b013f46c67e0bd96e13451aff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_saveLayerAlpha'></a>
## saveLayerAlpha

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Saves <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayerAlpha_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the size of <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayerAlpha_alpha'><code><strong>alpha</strong></code></a></td>
    <td>opacity of <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="8ab88d86fb438856cc48d6e2f08a6e24"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

## <a name='SkCanvas_SaveLayerFlagsSet'>Enum SkCanvas::SaveLayerFlagsSet</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_SaveLayerFlagsSet'>SaveLayerFlagsSet</a> {
        <a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a> = 1 << 1,
        <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> = 1 << 2,
        <a href='#SkCanvas_kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag'>kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag</a> =
                                          1 << 3,
        <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>kDontClipToLayer_Legacy_SaveLayerFlag</a> =
           kDontClipToLayer_PrivateSaveLayerFlag,
    };
</pre>

## <a name='SkCanvas_SaveLayerFlags'>Typedef SkCanvas::SaveLayerFlags</a>
<a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> provides options that may be used in any combination in <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kPreserveLCDText_SaveLayerFlag'><code>SkCanvas::kPreserveLCDText_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Creates <a href='#Layer'>Layer</a> for LCD text</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kInitWithPrevious_SaveLayerFlag'><code>SkCanvas::kInitWithPrevious_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Initializes <a href='#Layer'>Layer</a> with the contents of the previous <a href='#Layer'>Layer</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag'><code>SkCanvas::kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Experimental. Do not use.

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'><code>SkCanvas::kDontClipToLayer_Legacy_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x80000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
To be deprecated soon.

</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="05db6a937225e8e31ae3481173d25dae"><div><a href='#Canvas'>Canvas</a> <a href='#Layer'>Layer</a> captures red and blue circles scaled up by four</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

## <a name='Layer_SaveLayerRec'>Layer SaveLayerRec</a>

<a name='SkCanvas_SaveLayerRec'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
</pre>

<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> contains the state used to create the <a href='#Layer'>Layer</a><table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkRect*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fBounds'><code>fBounds</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> is used as a hint to limit the size of <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkPaint*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fPaint'><code>fPaint</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a> modifies how <a href='#Layer'>Layer</a> overlays the prior <a href='#Layer'>Layer</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkImageFilter*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fBackdrop'><code>fBackdrop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> applies <a href='undocumented#Image_Filter'>Image Filter</a> to the prior <a href='#Layer'>Layer</a> when copying to the <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkImage*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fClipMask'><code>fClipMask</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_restore'>restore</a>(</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkMatrix*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fClipMatrix'><code>fClipMatrix</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> transforms <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> before it clips <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SaveLayerFlags</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fSaveLayerFlags'><code>fSaveLayerFlags</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a> are used to create <a href='#Layer'>Layer</a> without transparency</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee8c0b120234e27364f8c9a786cf8f89"><div><a href='#Canvas'>Canvas</a> <a href='#Layer'>Layer</a> captures a red <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a> circle and a blue <a href='undocumented#Alias'>Aliased</a> circle scaled
up by four</div></fiddle-embed></div>

## <a name='Layer_SaveLayerRec_Constructors'>Layer SaveLayerRec Constructors</a>

<a name='SkCanvas_SaveLayerRec_SaveLayerRec'></a>
## SaveLayerRec

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>

### Return Value

empty <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>

### Example

<div><fiddle-embed name="b5cea1eed80a0eb04ddbab3f36dff73f">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star'></a>
## SaveLayerRec

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='#Layer'>Layer</a> dimensions</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_paint'><code><strong>paint</strong></code></a></td>
    <td>applied to <a href='#Layer'>Layer</a> when overlaying prior <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> options to modify <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> with empty backdrop

### Example

<div><fiddle-embed name="027f920259888fc19591ea9a90d92873">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star'></a>
## SaveLayerRec

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='#Layer'>Layer</a> dimensions</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_paint'><code><strong>paint</strong></code></a></td>
    <td>applied to <a href='#Layer'>Layer</a> when overlaying prior <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_backdrop'><code><strong>backdrop</strong></code></a></td>
    <td>prior <a href='#Layer'>Layer</a> copied with <a href='undocumented#Image_Filter'>Image Filter</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> options to modify <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> fully specified

### Example

<div><fiddle-embed name="9b7fa2fe855642ffff6538829db15328">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star'></a>
## SaveLayerRec

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Experimental. Not ready for general use.

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='#Layer'>Layer</a> dimensions</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state applied to <a href='#Layer'>Layer</a> when overlaying prior
<a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_backdrop'><code><strong>backdrop</strong></code></a></td>
    <td>prior <a href='#Layer'>Layer</a> copied with <a href='undocumented#Image_Filter'>Image Filter</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'><code><strong>clipMask</strong></code></a></td>
    <td>clip applied to <a href='#Layer'>Layer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'><code><strong>clipMatrix</strong></code></a></td>
    <td>matrix applied to <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>clipMask</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> options to modify <a href='#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a> fully specified

### See Also

<a href='#SkCanvas_save'>save</a>(

---

<a name='SkCanvas_saveLayer_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>
</pre>

Saves <a href='#Matrix'>Matrix</a> and <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_3_layerRec'><code><strong>layerRec</strong></code></a></td>
    <td><a href='#Layer'>Layer</a> state</td>
  </tr>
</table>

### Return Value

depth of save state stack

### Example

<div><fiddle-embed name="7d3751e82d1b6ec328ffa3d6f48ca831"><div>The example draws an image</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a>(

---

## <a name='Matrix'>Matrix</a>

<a name='SkCanvas_translate'></a>
## translate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_translate'>translate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Translates <a href='#Matrix'>Matrix</a> by <a href='#SkCanvas_translate_dx'>dx</a> along the x

### Parameters

<table>  <tr>    <td><a name='SkCanvas_translate_dx'><code><strong>dx</strong></code></a></td>
    <td>distance to translate in x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_translate_dy'><code><strong>dy</strong></code></a></td>
    <td>distance to translate in y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="eb93d5fa66a5f7a10f4f9210494d7222"><div><a href='#SkCanvas_scale'>scale</a>(</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_scale'></a>
## scale

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_scale'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx
</pre>

Scales <a href='#Matrix'>Matrix</a> by <a href='#SkCanvas_scale_sx'>sx</a> on the x

### Parameters

<table>  <tr>    <td><a name='SkCanvas_scale_sx'><code><strong>sx</strong></code></a></td>
    <td>amount to scale in x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_scale_sy'><code><strong>sy</strong></code></a></td>
    <td>amount to scale in y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7d0d801ef13c6c6da51e840c22ac15b0"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_rotate'></a>
## rotate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_rotate'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees
</pre>

Rotates <a href='#Matrix'>Matrix</a> by <a href='#SkCanvas_rotate_degrees'>degrees</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_rotate_degrees'><code><strong>degrees</strong></code></a></td>
    <td>amount to rotate</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="963789ac8498d4e505748ab3b15cdaa5"><div><a href='#Draw'>Draw</a> clock hands at time 5</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_rotate_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_rotate'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees
</pre>

Rotates <a href='#Matrix'>Matrix</a> by <a href='#SkCanvas_rotate_2_degrees'>degrees</a> about a point at

### Parameters

<table>  <tr>    <td><a name='SkCanvas_rotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>amount to rotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_rotate_2_px'><code><strong>px</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_rotate_2_py'><code><strong>py</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bcf5baea1c66a957d5ffd7b54bbbfeff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_skew'></a>
## skew

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_skew'>skew</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx
</pre>

Skews <a href='#Matrix'>Matrix</a> by <a href='#SkCanvas_skew_sx'>sx</a> on the x

### Parameters

<table>  <tr>    <td><a name='SkCanvas_skew_sx'><code><strong>sx</strong></code></a></td>
    <td>amount to skew on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_skew_sy'><code><strong>sy</strong></code></a></td>
    <td>amount to skew on y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2e2acc21d7774df7e0940a30ad2ca99e"><div>Black text mimics an oblique text style by using a negative skew on x</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_concat'></a>
## concat

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_concat'>concat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
</pre>

Replaces <a href='#Matrix'>Matrix</a> with <a href='#SkCanvas_concat_matrix'>matrix</a> <a href='undocumented#Premultiply'>Premultiplied</a> with existing <a href='#Matrix'>Matrix</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_concat_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#SkCanvas_concat_matrix'>matrix</a> to <a href='undocumented#Premultiply'>Premultiply</a> with existing <a href='#Matrix'>Matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8f6818b25a92a88638ad99b2dd293f61"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_translate'>translate</a>(

---

<a name='SkCanvas_setMatrix'></a>
## setMatrix

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_setMatrix'>setMatrix</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
</pre>

Replaces <a href='#Matrix'>Matrix</a> with <a href='#SkCanvas_setMatrix_matrix'>matrix</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_setMatrix_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#SkCanvas_setMatrix_matrix'>matrix</a> to copy</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="24b9cf7e6f9a08394e1e07413bd8733a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_resetMatrix'>resetMatrix</a> <a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_resetMatrix'></a>
## resetMatrix

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(
</pre>

Sets <a href='#Matrix'>Matrix</a> to the identity matrix

### Example

<div><fiddle-embed name="412afffdf4682baa503a4e2e99201967"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_setMatrix'>setMatrix</a> <a href='#SkCanvas_concat'>concat</a>(

---

<a name='SkCanvas_getTotalMatrix'></a>
## getTotalMatrix

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
</pre>

Returns <a href='#Matrix'>Matrix</a>

### Return Value

<a href='#Matrix'>Matrix</a> in <a href='#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="c0d5fa544759704768f47cac91ae3832">

#### Example Output

~~~~
isIdentity true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_setMatrix'>setMatrix</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a> <a href='#SkCanvas_concat'>concat</a>(

---

## <a name='Clip'>Clip</a>

<a href='#Clip'>Clip</a> is built from a stack of clipping paths

### Example

<div><fiddle-embed name="862cc026601a41a58df49c0b9f0d7777"><div><a href='#Draw'>Draw</a> a red circle with an <a href='undocumented#Alias'>Aliased</a> clip and an <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a> clip</div></fiddle-embed></div>

<a name='SkCanvas_clipRect'></a>
## clipRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipRect_rect'>rect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if <a href='#Clip'>Clip</a> is to be <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6a614faa0fbcf19958b5559c19b02d0f"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipRect_2_rect'>rect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="13bbc5fa5597a6cd4d704b419dbc66d9"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipRect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipRect_3_rect'>rect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_3_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if <a href='#Clip'>Clip</a> is to be <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d4e0632c97e42692775d834fe10aa99"><div>A circle drawn in pieces looks uniform when drawn <a href='undocumented#Alias'>Aliased</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_androidFramework_setDeviceClipRestriction'></a>
## androidFramework_setDeviceClipRestriction

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_androidFramework_setDeviceClipRestriction'>androidFramework setDeviceClipRestriction</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>
</pre>

Sets the maximum clip rectanglePrivate: This private API is for use by Android framework only.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_androidFramework_setDeviceClipRestriction_rect'><code><strong>rect</strong></code></a></td>
    <td>maximum allowed clip in device coordinates</td>
  </tr>


---

<a name='SkCanvas_clipRRect'></a>
## clipRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipRRect_rrect'>rrect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#RRect'>Round Rect</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if <a href='#Clip'>Clip</a> is to be <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="182ef48ab5e04ba3578496fda8d9fa36"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipRRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipRRect_2_rrect'>rrect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_2_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#RRect'>Round Rect</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ef6ae2eaae6761130ce38065d0364abd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipRRect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipRRect_3_rrect'>rrect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_3_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#RRect'>Round Rect</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if <a href='#Clip'>Clip</a> is to be <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f583114580b2176fe3e75b0994476a84"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipPath'></a>
## clipPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipPath_path'>path</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if <a href='#Clip'>Clip</a> is to be <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee47ae6b813bfaa55e1a7b7c053ed60d"><div>Top figure uses <a href='SkPath_Reference#SkPath_kInverseWinding_FillType'>SkPath::kInverseWinding FillType</a> and <a href='undocumented#SkClipOp_kDifference'>SkClipOp::kDifference</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipPath_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipPath_2_path'>path</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_2_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7856755c1bf8431c286c734b353345ad"><div>Overlapping <a href='SkRect_Reference#Rect'>Rects</a> form a clip</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_clipPath_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection of <a href='#Clip'>Clip</a> and <a href='#SkCanvas_clipPath_3_path'>path</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_3_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if <a href='#Clip'>Clip</a> is to be <a href='SkPaint_Reference#Anti_Alias'>Anti Aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="187a7ae77a8176e417181411988534b6"><div><a href='#Clip'>Clip</a> loops over itself covering its center twice</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRegion'>clipRegion</a>

---

<a name='SkCanvas_setAllowSimplifyClip'></a>
## setAllowSimplifyClip

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_setAllowSimplifyClip'>setAllowSimplifyClip</a>(bool allow
</pre>

Experimental. For testing only.

Set to simplify clip stack using <a href='undocumented#PathOps'>PathOps</a>

---

<a name='SkCanvas_clipRegion'></a>
## clipRegion

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRegion'>clipRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>
</pre>

Replaces <a href='#Clip'>Clip</a> with the intersection or difference of <a href='#Clip'>Clip</a> and <a href='SkRegion_Reference#Region'>Region</a> <a href='#SkCanvas_clipRegion_deviceRgn'>deviceRgn</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRegion_deviceRgn'><code><strong>deviceRgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#Region'>Region</a> to combine with <a href='#Clip'>Clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRegion_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#Clip_Op'>Clip Op</a> to apply to <a href='#Clip'>Clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7bb57c0e456c5fda2c2cca4abb68b19e"><div>region is unaffected by canvas rotation</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a><sup><a href='#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRect_3'>[3]</a></sup> <a href='#SkCanvas_clipRRect'>clipRRect</a><sup><a href='#SkCanvas_clipRRect_2'>[2]</a></sup><sup><a href='#SkCanvas_clipRRect_3'>[3]</a></sup> <a href='#SkCanvas_clipPath'>clipPath</a><sup><a href='#SkCanvas_clipPath_2'>[2]</a></sup><sup><a href='#SkCanvas_clipPath_3'>[3]</a></sup>

---

<a name='SkCanvas_quickReject'></a>
## quickReject

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns true if <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_quickReject_rect'>rect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_quickReject_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to compare with <a href='#Clip'>Clip</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_quickReject_rect'>rect</a>

### Example

<div><fiddle-embed name="cfe4016241074477809dd45435be9cf4">

#### Example Output

~~~~
quickReject true
quickReject false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a><sup><a href='#SkCanvas_getLocalClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a> <a href='SkBitmap_Reference#SkBitmap_drawsNothing'>SkBitmap::drawsNothing</a>

---

<a name='SkCanvas_quickReject_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Returns true if <a href='#SkCanvas_quickReject_2_path'>path</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_quickReject_2_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> to compare with <a href='#Clip'>Clip</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_quickReject_2_path'>path</a>

### Example

<div><fiddle-embed name="56dcd14f943aea6f7d7aafe0de7e6c25">

#### Example Output

~~~~
quickReject true
quickReject false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a><sup><a href='#SkCanvas_getLocalClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a> <a href='SkBitmap_Reference#SkBitmap_drawsNothing'>SkBitmap::drawsNothing</a>

---

<a name='SkCanvas_getLocalClipBounds'></a>
## getLocalClipBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(
</pre>

Returns bounds of <a href='#Clip'>Clip</a>

### Return Value

bounds of <a href='#Clip'>Clip</a> in local coordinates

### Example

<div><fiddle-embed name="7f60cb030d3f9b2473adbe3e34b19d91"><div>Initial bounds is device bounds outset by 1 on all sides</div>

#### Example Output

~~~~
left:-1  top:-1  right:257  bottom:257
left:29  top:129  right:121  bottom:231
left:14.5  top:64.5  right:60.5  bottom:115.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a><sup><a href='#SkCanvas_getDeviceClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a><sup><a href='#SkCanvas_quickReject_2'>[2]</a></sup>

---

<a name='SkCanvas_getLocalClipBounds_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> of <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getLocalClipBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> of <a href='#Clip'>Clip</a> in local coordinates</td>
  </tr>
</table>

### Return Value

true if <a href='#Clip'>Clip</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> is not empty

### Example

<div><fiddle-embed name="85496614e90c66b020f8a70db8d06f4a">

#### Example Output

~~~~
local bounds empty = false
local bounds empty = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a><sup><a href='#SkCanvas_getDeviceClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a><sup><a href='#SkCanvas_quickReject_2'>[2]</a></sup>

---

<a name='SkCanvas_getDeviceClipBounds'></a>
## getDeviceClipBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>(
</pre>

Returns <a href='SkIRect_Reference#IRect'>IRect</a> bounds of <a href='#Clip'>Clip</a>

### Return Value

bounds of <a href='#Clip'>Clip</a> in <a href='undocumented#Device'>Device</a> coordinates

### Example

<div><fiddle-embed name="556832ac5711af662a98c21c547185e9"><div>Initial bounds is device bounds</div>

#### Example Output

~~~~
left:0  top:0  right:256  bottom:256
left:30  top:130  right:120  bottom:230
left:15  top:65  right:60  bottom:115
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a><sup><a href='#SkCanvas_getLocalClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a><sup><a href='#SkCanvas_quickReject_2'>[2]</a></sup>

---

<a name='SkCanvas_getDeviceClipBounds_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>
</pre>

Returns <a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> of <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getDeviceClipBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> of <a href='#Clip'>Clip</a> in device coordinates</td>
  </tr>
</table>

### Return Value

true if <a href='#Clip'>Clip</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> is not empty

### Example

<div><fiddle-embed name="6abb99f849a1f0e33e1dedc00d1c4f7a">

#### Example Output

~~~~
device bounds empty = false
device bounds empty = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a><sup><a href='#SkCanvas_getLocalClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a><sup><a href='#SkCanvas_quickReject_2'>[2]</a></sup>

---

## <a name='Draw'>Draw</a>

<a name='SkCanvas_drawColor'></a>
## drawColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawColor'>drawColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color
</pre>

Fills <a href='#Clip'>Clip</a> with <a href='SkColor_Reference#Color'>Color</a> <a href='#SkCanvas_drawColor_color'>color</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawColor_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> ARGB</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawColor_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> used to combine source <a href='#SkCanvas_drawColor_color'>color</a> and destination</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9cf94fead1e6b17d836c704b4eac269a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clear'>clear</a> <a href='SkBitmap_Reference#SkBitmap_erase'>SkBitmap::erase</a> <a href='#SkCanvas_drawPaint'>drawPaint</a>

---

<a name='SkCanvas_clear'></a>
## clear

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clear'>clear</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color
</pre>

Fills <a href='#Clip'>Clip</a> with <a href='SkColor_Reference#Color'>Color</a> <a href='#SkCanvas_clear_color'>color</a> using <a href='SkBlendMode_Reference#SkBlendMode_kSrc'>SkBlendMode::kSrc</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clear_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> ARGB</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8c4499e322f10153dcd9b0b9806233b9"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawColor'>drawColor</a> <a href='SkBitmap_Reference#SkBitmap_erase'>SkBitmap::erase</a> <a href='#SkCanvas_drawPaint'>drawPaint</a>

---

<a name='SkCanvas_discard'></a>
## discard

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_discard'>discard</a>(
</pre>

Makes <a href='#Canvas'>Canvas</a> contents undefined

### See Also

<a href='#SkCanvas_flush'>flush</a>(

---

<a name='SkCanvas_drawPaint'></a>
## drawPaint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPaint'>drawPaint</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>
</pre>

Fills <a href='#Clip'>Clip</a> with <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawPaint_paint'>paint</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPaint_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state used to fill <a href='#Canvas'>Canvas</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1cd076b9b1a7c976cdca72b93c4f42dd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clear'>clear</a> <a href='#SkCanvas_drawColor'>drawColor</a> <a href='SkBitmap_Reference#SkBitmap_erase'>SkBitmap::erase</a>

---

## <a name='SkCanvas_PointMode'>Enum SkCanvas::PointMode</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_PointMode'>PointMode</a> {
        <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>,
        <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>,
        <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>,
    };
</pre>

Selects if an array of points are drawn as discrete points

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kPoints_PointMode'><code>SkCanvas::kPoints_PointMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
draw each point separately</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kLines_PointMode'><code>SkCanvas::kLines_PointMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
draw each pair of points as a line segment</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kPolygon_PointMode'><code>SkCanvas::kPolygon_PointMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
draw the array of points as a open polygon</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="292b4b2008961b6f612434d3121fc4ce"><div>The upper left corner shows three squares when drawn as points</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup> <a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoints'></a>
## drawPoints

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoints'>drawPoints</a>(<a href='#SkCanvas_PointMode'>PointMode</a> mode
</pre>

Draws <a href='#SkCanvas_drawPoints_pts'>pts</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoints_mode'><code><strong>mode</strong></code></a></td>
    <td>whether <a href='#SkCanvas_drawPoints_pts'>pts</a> draws points or lines</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_count'><code><strong>count</strong></code></a></td>
    <td>number of points in the array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_pts'><code><strong>pts</strong></code></a></td>
    <td>array of points to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="635d54b4716e226e93dfbc21ad40e77d"><div>

<table>  <tr>
    <td>The first column draws points</td>
  </tr>  <tr>
    <td>The second column draws points as lines</td>
  </tr>  <tr>
    <td>The third column draws points as a polygon</td>
  </tr>  <tr>
    <td>The fourth column draws points as a polygonal path</td>
  </tr>  <tr>
    <td>The first row uses a round cap and round join</td>
  </tr>  <tr>
    <td>The second row uses a square cap and a miter join</td>
  </tr>  <tr>
    <td>The third row uses a butt cap and a bevel join</td>
  </tr>
</table>

The transparent color makes multiple line draws visible</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup> <a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawPoint'></a>
## drawPoint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='undocumented#SkScalar'>SkScalar</a> x
</pre>

Draws point at

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoint_x'><code><strong>x</strong></code></a></td>
    <td>left edge of circle or square</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_y'><code><strong>y</strong></code></a></td>
    <td>top edge of circle or square</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3476b553e7b547b604a3f6969f02d933"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoints'>drawPoints</a> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawPoint_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p
</pre>

Draws point <a href='#SkCanvas_drawPoint_2_p'>p</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoint_2_p'><code><strong>p</strong></code></a></td>
    <td>top</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_2_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1a0a839061c69d870acca2bcfbdf1a41"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoints'>drawPoints</a> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawLine'></a>
## drawLine

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='undocumented#SkScalar'>SkScalar</a> x0
</pre>

Draws line segment from

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawLine_x0'><code><strong>x0</strong></code></a></td>
    <td>start of line segment on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_y0'><code><strong>y0</strong></code></a></td>
    <td>start of line segment on y</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_x1'><code><strong>x1</strong></code></a></td>
    <td>end of line segment on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_y1'><code><strong>y1</strong></code></a></td>
    <td>end of line segment on y</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d10ee4a265f278d02afe11ad889b293b"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawLine_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p0
</pre>

Draws line segment from <a href='#SkCanvas_drawLine_2_p0'>p0</a> to <a href='#SkCanvas_drawLine_2_p1'>p1</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawLine_2_p0'><code><strong>p0</strong></code></a></td>
    <td>start of line segment</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_2_p1'><code><strong>p1</strong></code></a></td>
    <td>end of line segment</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_2_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f8525816cb596dde1a3855446792c8e0"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawRect'></a>
## drawRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRect'>drawRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Draws <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawRect_rect'>rect</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRect_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="871b0da9b4a23de11ae7a772ce14aed3"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawIRect'>drawIRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRegion'>drawRegion</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawIRect'></a>
## drawIRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawIRect'>drawIRect</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>
</pre>

Draws <a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawIRect_rect'>rect</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawIRect_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawIRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d3d8ca584134560750b1efa4a4c6e138"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRegion'>drawRegion</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawRegion'></a>
## drawRegion

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRegion'>drawRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>
</pre>

Draws <a href='SkRegion_Reference#Region'>Region</a> <a href='#SkCanvas_drawRegion_region'>region</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRegion_region'><code><strong>region</strong></code></a></td>
    <td><a href='#SkCanvas_drawRegion_region'>region</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRegion_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="80309e0deca0f8add616cec7bec634ca"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawIRect'>drawIRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawOval'></a>
## drawOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawOval'>drawOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Draws <a href='undocumented#Oval'>Oval</a> <a href='#SkCanvas_drawOval_oval'>oval</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawOval_oval'><code><strong>oval</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds of <a href='undocumented#Oval'>Oval</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawOval_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8b6b86f8a022811cd29a9c6ab771df12"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>

---

<a name='SkCanvas_drawRRect'></a>
## drawRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRRect'>drawRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
</pre>

Draws <a href='SkRRect_Reference#RRect'>Round Rect</a> <a href='#SkCanvas_drawRRect_rrect'>rrect</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#RRect'>Round Rect</a> with up to eight corner radii to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="90fed1bb11efb43aada94113338c63d8"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawDRRect'>drawDRRect</a> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawDRRect'></a>
## drawDRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDRRect'>drawDRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
</pre>

Draws <a href='SkRRect_Reference#RRect'>Round Rect</a> <a href='#SkCanvas_drawDRRect_outer'>outer</a> and <a href='#SkCanvas_drawDRRect_inner'>inner</a>
using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDRRect_outer'><code><strong>outer</strong></code></a></td>
    <td><a href='SkRRect_Reference#RRect'>Round Rect</a> <a href='#SkCanvas_drawDRRect_outer'>outer</a> bounds to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDRRect_inner'><code><strong>inner</strong></code></a></td>
    <td><a href='SkRRect_Reference#RRect'>Round Rect</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a> bounds to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDRRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="02e33141f13da2f19aef7feb7117b541"></fiddle-embed></div>

### Example

<div><fiddle-embed name="30823cb4edf884d330285ea161664931"><div>Outer <a href='SkRect_Reference#Rect'>Rect</a> has no corner radii</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawCircle'></a>
## drawCircle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> cx
</pre>

Draws <a href='undocumented#Circle'>Circle</a> at

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawCircle_cx'><code><strong>cx</strong></code></a></td>
    <td><a href='undocumented#Circle'>Circle</a> center on the x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_cy'><code><strong>cy</strong></code></a></td>
    <td><a href='undocumented#Circle'>Circle</a> center on the y</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_radius'><code><strong>radius</strong></code></a></td>
    <td>half the diameter of <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="841229e25ca9dfb68bd0dc4dfff356eb"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawCircle_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> center
</pre>

Draws <a href='undocumented#Circle'>Circle</a> at <a href='#SkCanvas_drawCircle_2_center'>center</a> with <a href='#SkCanvas_drawCircle_2_radius'>radius</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawCircle_2_center'><code><strong>center</strong></code></a></td>
    <td><a href='undocumented#Circle'>Circle</a> <a href='#SkCanvas_drawCircle_2_center'>center</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_2_radius'><code><strong>radius</strong></code></a></td>
    <td>half the diameter of <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9303ffae45ddd0b0a1f93d816a1762f4"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawArc'></a>
## drawArc

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawArc'>drawArc</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Draws <a href='undocumented#Arc'>Arc</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawArc_oval'><code><strong>oval</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds of <a href='undocumented#Oval'>Oval</a> containing <a href='undocumented#Arc'>Arc</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>angle in degrees where <a href='undocumented#Arc'>Arc</a> begins</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep angle in degrees</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_useCenter'><code><strong>useCenter</strong></code></a></td>
    <td>if true</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> stroke or fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="11f0fbe7b30d776913c2e7c92c02ff57"></fiddle-embed></div>

### Example

<div><fiddle-embed name="e91dbe45974489b8962c815017b7914f"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath_arcTo'>SkPath::arcTo</a><sup><a href='SkPath_Reference#SkPath_arcTo_2'>[2]</a></sup><sup><a href='SkPath_Reference#SkPath_arcTo_3'>[3]</a></sup><sup><a href='SkPath_Reference#SkPath_arcTo_4'>[4]</a></sup><sup><a href='SkPath_Reference#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

---

<a name='SkCanvas_drawRoundRect'></a>
## drawRoundRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Draws <a href='SkRRect_Reference#RRect'>Round Rect</a> bounded by <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawRoundRect_rect'>rect</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRoundRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds of <a href='SkRRect_Reference#RRect'>Round Rect</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_rx'><code><strong>rx</strong></code></a></td>
    <td>axis length on x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_ry'><code><strong>ry</strong></code></a></td>
    <td>axis length on y</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="199fe818c09026c114e165bff166a39f"><div>Top row has a zero radius a generates a rectangle</div></fiddle-embed></div>

### See Also

DrawRRect <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawDRRect'>drawDRRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawCircle'>drawCircle</a><sup><a href='#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPoint'>drawPoint</a><sup><a href='#SkCanvas_drawPoint_2'>[2]</a></sup>

---

<a name='SkCanvas_drawPath'></a>
## drawPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPath'>drawPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>
</pre>

Draws <a href='SkPath_Reference#Path'>Path</a> <a href='#SkCanvas_drawPath_path'>path</a> using <a href='#Clip'>Clip</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPath_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="fe2294131f422b8d6752f6a880f98ad9"><div>Top rows draw stroked <a href='#SkCanvas_drawPath_path'>path</a> with combinations of joins and caps</div></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkCanvas_drawLine'>drawLine</a><sup><a href='#SkCanvas_drawLine_2'>[2]</a></sup> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPoints'>drawPoints</a>

---

## <a name='Draw_Image'>Draw Image</a>

<a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawImage'></a>
## drawImage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImage_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImage_image'><code><strong>image</strong></code></a></td>
    <td>uncompressed rectangular map of pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_left'>left</a> side of <a href='#SkCanvas_drawImage_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_top'><code><strong>top</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_top'>top</a> side of <a href='#SkCanvas_drawImage_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="185746dc0faa6f1df30c4afe098646ff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup> <a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup> <a href='SkPaint_Reference#SkPaint_setImageFilter'>SkPaint::setImageFilter</a>

---

<a name='SkCanvas_drawImage_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImage_2_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImage_2_image'><code><strong>image</strong></code></a></td>
    <td>uncompressed rectangular map of pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_2_left'>left</a> side of <a href='#SkCanvas_drawImage_2_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_top'><code><strong>top</strong></code></a></td>
    <td>pop side of <a href='#SkCanvas_drawImage_2_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a4e877e891b1be5faa2b7fd07f673a10"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup> <a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup> <a href='SkPaint_Reference#SkPaint_setImageFilter'>SkPaint::setImageFilter</a>

---

## <a name='SkCanvas_SrcRectConstraint'>Enum SkCanvas::SrcRectConstraint</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> {
        <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>,
        <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>,
    };
</pre>

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> controls the behavior at the edge of source <a href='SkRect_Reference#Rect'>Rect</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kStrict_SrcRectConstraint'><code>SkCanvas::kStrict_SrcRectConstraint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Requires <a href='undocumented#Image_Filter'>Image Filter</a> to respect source <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kFast_SrcRectConstraint'><code>SkCanvas::kFast_SrcRectConstraint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Permits <a href='undocumented#Image_Filter'>Image Filter</a> to sample outside of source <a href='SkRect_Reference#Rect'>Rect</a>
by half the width of <a href='undocumented#Image_Filter'>Image Filter</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5df49d1f4da37275a1f10ef7f1a749f0"><div>redBorder contains a black and white checkerboard bordered by red</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='SkPaint_Reference#SkPaint_setImageFilter'>SkPaint::setImageFilter</a>

<a name='SkCanvas_drawImageRect'></a>
## drawImageRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageRect_src'>src</a> of <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageRect_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_src'>src</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bfd18e9cac896cdf94c9f154ccf94be8"><div>The left bitmap draws with <a href='SkPaint_Reference#Paint'>Paint</a> default <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawImageRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws <a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> of <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_2_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#IRect'>IRect</a> of <a href='#SkCanvas_drawImageRect_2_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_2_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f92cd5c9b9f4b1ac3cd933b08037bfe"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawImageRect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_3_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_3_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_3_image'>image</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3cf8fb639fef99993cafc064d550c739"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawImageRect_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageRect_4_src'>src</a> of <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_4_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_4_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_4_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_4_src'>src</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d4b35a9d24c32c042bd1f529b8de3c0d"><div><a href='#Canvas'>Canvas</a> scales and translates</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawImageRect_5'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawImageRect_5_isrc'>isrc</a> of <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_5_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#IRect'>IRect</a> of <a href='#SkCanvas_drawImageRect_5_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_5_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_5_image'>image</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d307e7e1237f39fb54d80723e5449857"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawImageRect_6'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_6_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageRect_6_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_6_image'>image</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3a47ef94cb70144455f80333d8653e6c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup>

---

<a name='SkCanvas_drawImageNine'></a>
## drawImageNine

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageNine_image'>image</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageNine_dst'>dst</a> If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> is supplied image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> image bounds If generated mask extends beyond <a href='#SkCanvas_drawImageNine_image'>image</a> bounds

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageNine_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> edge of <a href='#SkCanvas_drawImageNine_image'>image</a> corners and sides</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageNine_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4f153cf1d0dbe1a95acf5badeec14dae"><div>The leftmost <a href='#SkCanvas_drawImageNine_image'>image</a> is smaller than <a href='#SkCanvas_drawImageNine_center'>center</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup>

---

<a name='SkCanvas_drawImageNine_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageNine_2_dst'>dst</a> If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> is supplied image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> image bounds If generated mask extends beyond <a href='#SkCanvas_drawImageNine_2_image'>image</a> bounds

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageNine_2_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> edge of <a href='#SkCanvas_drawImageNine_2_image'>image</a> corners and sides</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageNine_2_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d597d9af8d17fd93e634dd12017058e2"><div>The two leftmost images has four corners and sides to the left and right of <a href='#SkCanvas_drawImageNine_2_center'>center</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup>

---

<a name='SkCanvas_drawBitmap'></a>
## drawBitmap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmap'>drawBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawBitmap_left'>left</a> side of <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_top'><code><strong>top</strong></code></a></td>
    <td><a href='#SkCanvas_drawBitmap_top'>top</a> side of <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4a521be1f850058541e136a808c65e78"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a><sup><a href='#SkCanvas_drawBitmapRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawBitmapRect_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_writePixels'>SkBitmap::writePixels</a><sup><a href='SkBitmap_Reference#SkBitmap_writePixels_2'>[2]</a></sup>

---

<a name='SkCanvas_drawBitmapRect'></a>
## drawBitmapRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Draws <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapRect_src'>src</a> of <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#Rect'>Rect</a> of image to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of image to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawBitmapRect_src'>src</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7d04932f2a259cc70d6e45cd25a6feb6"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

---

<a name='SkCanvas_drawBitmapRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Draws <a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a> of <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#IRect'>IRect</a> of image to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of image to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_constraint'><code><strong>constraint</strong></code></a></td>
    <td>sample strictly within <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0a3c6d2459566e58cee7d4910655ee21"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

---

<a name='SkCanvas_drawBitmapRect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of image to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bdbeac3c97f60a63987b1cc8e1f1e91e"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a><sup><a href='#SkCanvas_drawImageRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawImageRect_3'>[3]</a></sup><sup><a href='#SkCanvas_drawImageRect_4'>[4]</a></sup><sup><a href='#SkCanvas_drawImageRect_5'>[5]</a></sup><sup><a href='#SkCanvas_drawImageRect_6'>[6]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

---

<a name='SkCanvas_drawBitmapNine'></a>
## drawBitmapNine

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapNine_dst'>dst</a> If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> is supplied bitmap is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> bitmap bounds If generated mask extends beyond <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> bounds

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapNine_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> edge of image corners and sides</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of image to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e99e7be0d8f67dfacbecf85df585433d"><div>The two leftmost <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> draws has four corners and sides to the left and right of <a href='#SkCanvas_drawBitmapNine_center'>center</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a><sup><a href='#SkCanvas_drawBitmapRect_2'>[2]</a></sup><sup><a href='#SkCanvas_drawBitmapRect_3'>[3]</a></sup>

---

## <a name='Draw_Image_Lattice'>Draw Image Lattice</a>

<a name='SkCanvas_Lattice'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
</pre>

<a href='#SkCanvas_Lattice'>Lattice</a> divides <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> or <a href='SkImage_Reference#Image'>Image</a> into a rectangular grid

## <a name='SkCanvas_Lattice_RectType'>Enum SkCanvas::Lattice::RectType</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        enum <a href='#SkCanvas_Lattice_RectType'>RectType</a> : uint8_t {
            <a href='#SkCanvas_Lattice_kDefault'>kDefault</a> = 0,
            <a href='#SkCanvas_Lattice_kTransparent'>kTransparent</a>,
            <a href='#SkCanvas_Lattice_kFixedColor'>kFixedColor</a>,
        };
</pre>

Optional setting per rectangular grid entry to make it transparent

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_kDefault'><code>SkCanvas::Lattice::kDefault</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
draws Bitmap into lattice rectangle</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_kTransparent'><code>SkCanvas::Lattice::kTransparent</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
skips lattice rectangle by making it transparent</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_kFixedColor'><code>SkCanvas::Lattice::kFixedColor</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
draws one of fColors into lattice rectangle</td>
  </tr>
</table>

## <a name='Draw_Image_Lattice_Members'>Draw Image Lattice Members</a><table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;int*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fXDivs'><code>fXDivs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Array of x</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;int*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fYDivs'><code>fYDivs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Array of y</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;RectType*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fRectTypes'><code>fRectTypes</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional array of fill types <code></code></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fXCount'><code>fXCount</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Number of entries in <a href='#SkCanvas_Lattice_fXDivs'>fXDivs</a> array</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fYCount'><code>fYCount</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Number of entries in <a href='#SkCanvas_Lattice_fYDivs'>fYDivs</a> array</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkIRect*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fBounds'><code>fBounds</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional subset <a href='SkIRect_Reference#IRect'>IRect</a> source to draw from</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkColor*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fColors'><code>fColors</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional array of colors <code></code></td>
  </tr>
</table>

<a name='SkCanvas_drawBitmapLattice'></a>
## drawBitmapLattice

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapLattice_dst'>dst</a> If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> is supplied bitmap is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> bitmap bounds If generated mask extends beyond <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> bounds

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapLattice_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_lattice'><code><strong>lattice</strong></code></a></td>
    <td>division of <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> into fixed and variable rectangles</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of image to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c5bfa944e17ba4a4400dc799f032069c"><div>The two leftmost <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> draws has four corners and sides to the left and right of center</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_Lattice'>Lattice</a>

---

<a name='SkCanvas_drawImageLattice'></a>
## drawImageLattice

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageLattice_image'>image</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageLattice_dst'>dst</a> If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> is supplied image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> image bounds If generated mask extends beyond bitmap bounds

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageLattice_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_lattice'><code><strong>lattice</strong></code></a></td>
    <td>division of bitmap into fixed and variable rectangles</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageLattice_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4f153cf1d0dbe1a95acf5badeec14dae"><div>The leftmost <a href='#SkCanvas_drawImageLattice_image'>image</a> is smaller than center</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkCanvas_drawImageNine'>drawImageNine</a><sup><a href='#SkCanvas_drawImageNine_2'>[2]</a></sup> <a href='#SkCanvas_Lattice'>Lattice</a>

---

## <a name='Draw_Text'>Draw Text</a>

<a name='SkCanvas_drawText'></a>
## drawText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawText'>drawText</a>(const void
</pre>

Draws <a href='#SkCanvas_drawText_text'>text</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawText_text'><code><strong>text</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawText_text'>text</a> on <a href='#SkCanvas_drawText_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawText_text'>text</a> on <a href='#SkCanvas_drawText_y'>y</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawText_text'>text</a> size</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="55f5e59350622c5e2834d1c85789f732"><div>The same <a href='#SkCanvas_drawText_text'>text</a> is drawn varying <a href='SkPaint_Reference#Text_Size'>Paint Text Size</a> and varying
<a href='#Matrix'>Matrix</a></div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawString'>drawString</a><sup><a href='#SkCanvas_drawString_2'>[2]</a></sup> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a><sup><a href='#SkCanvas_drawTextBlob_2'>[2]</a></sup> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

---

<a name='SkCanvas_drawString'></a>
## drawString

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawString'>drawString</a>(const char
</pre>

Draws null terminated <a href='#SkCanvas_drawString_string'>string</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_string'><code><strong>string</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_string'>string</a> on <a href='#SkCanvas_drawString_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_string'>string</a> on <a href='#SkCanvas_drawString_y'>y</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_paint'><code><strong>paint</strong></code></a></td>
    <td>text size</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="85442cf8d0bce6b5a777853bc36a4dc4"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a><sup><a href='#SkCanvas_drawTextBlob_2'>[2]</a></sup> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

---

<a name='SkCanvas_drawString_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawString'>drawString</a>(const <a href='undocumented#SkString'>SkString</a>
</pre>

Draws null terminated <a href='#SkCanvas_drawString_2_string'>string</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_2_string'><code><strong>string</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_2_string'>string</a> on <a href='#SkCanvas_drawString_2_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_2_string'>string</a> on <a href='#SkCanvas_drawString_2_y'>y</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_paint'><code><strong>paint</strong></code></a></td>
    <td>text size</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="435178c09feb3bfec5e35d983609a013"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a><sup><a href='#SkCanvas_drawTextBlob_2'>[2]</a></sup> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

---

<a name='SkCanvas_drawPosText'></a>
## drawPosText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPosText'>drawPosText</a>(const void
</pre>

Draws each glyph in <a href='#SkCanvas_drawPosText_text'>text</a> with the origin in <a href='#SkCanvas_drawPosText_pos'>pos</a> array

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPosText_text'><code><strong>text</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawPosText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_pos'><code><strong>pos</strong></code></a></td>
    <td>array of glyph origins</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawPosText_text'>text</a> size</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf0b2402533a23b6392e0676b7a8414c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a><sup><a href='#SkCanvas_drawTextBlob_2'>[2]</a></sup> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

---

<a name='SkCanvas_drawPosTextH'></a>
## drawPosTextH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>(const void
</pre>

Draws each glyph in <a href='#SkCanvas_drawPosTextH_text'>text</a> with its

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPosTextH_text'><code><strong>text</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawPosTextH_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_xpos'><code><strong>xpos</strong></code></a></td>
    <td>array of x</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_constY'><code><strong>constY</strong></code></a></td>
    <td>shared y</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawPosTextH_text'>text</a> size</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="95c6a7ef82993a8d2add676080e9438a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a><sup><a href='#SkCanvas_drawTextBlob_2'>[2]</a></sup> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

---

<a name='SkCanvas_drawTextRSXform'></a>
## drawTextRSXform

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>(const void
</pre>

Draws <a href='#SkCanvas_drawTextRSXform_text'>text</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextRSXform_text'><code><strong>text</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawTextRSXform_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#RSXform'>RSXform</a> rotates</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds of <a href='#SkCanvas_drawTextRSXform_text'>text</a> for efficient clipping</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawTextRSXform_text'>text</a> size</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3ce367af833428b08e75d8a22fe67808"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a><sup><a href='#SkCanvas_drawTextBlob_2'>[2]</a></sup>

---

<a name='SkCanvas_drawTextBlob'></a>
## drawTextBlob

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
</pre>

Draws <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> <a href='#SkCanvas_drawTextBlob_blob'>blob</a> at <a href='undocumented#Typeface'>Typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextBlob_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset applied to <a href='#SkCanvas_drawTextBlob_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset applied to <a href='#SkCanvas_drawTextBlob_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_paint'><code><strong>paint</strong></code></a></td>
    <td>blend</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5f823814ec9df1f912a2ea943bedfca1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>

---

<a name='SkCanvas_drawTextBlob_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a> at <a href='undocumented#Typeface'>Typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextBlob_2_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset applied to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset applied to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_paint'><code><strong>paint</strong></code></a></td>
    <td>blend</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="af4c69fbbd165c8b0eb0c9bd49ccbd8d"><div><a href='SkPaint_Reference#Paint'>Paint</a> attributes unrelated to text</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>

---

<a name='SkCanvas_drawPicture'></a>
## drawPicture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>
</pre>

Draws <a href='SkPicture_Reference#Picture'>Picture</a> <a href='#SkCanvas_drawPicture_picture'>picture</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="83918a23fcffd47f59a1ef662c85a24c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a><sup><a href='#SkCanvas_drawDrawable_2'>[2]</a></sup> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture_playback'>SkPicture::playback</a>

---

<a name='SkCanvas_drawPicture_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkPicture_Reference#Picture'>Picture</a> <a href='#SkCanvas_drawPicture_2_picture'>picture</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_2_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="83918a23fcffd47f59a1ef662c85a24c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a><sup><a href='#SkCanvas_drawDrawable_2'>[2]</a></sup> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture_playback'>SkPicture::playback</a>

---

<a name='SkCanvas_drawPicture_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>
</pre>

Draws <a href='SkPicture_Reference#Picture'>Picture</a> <a href='#SkCanvas_drawPicture_3_picture'>picture</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_3_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_3_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#Matrix'>Matrix</a> to rotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> to apply transparency</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="759e4e5bac680838added8f70884dcdc"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a><sup><a href='#SkCanvas_drawDrawable_2'>[2]</a></sup> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture_playback'>SkPicture::playback</a>

---

<a name='SkCanvas_drawPicture_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='SkPicture_Reference#Picture'>Picture</a> <a href='#SkCanvas_drawPicture_4_picture'>picture</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_4_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_4_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#Matrix'>Matrix</a> to rotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> to apply transparency</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c4ff59439dd2fc871925d4eeb0c84ca1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a><sup><a href='#SkCanvas_drawDrawable_2'>[2]</a></sup> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture_playback'>SkPicture::playback</a>

---

<a name='SkCanvas_drawVertices'></a>
## drawVertices

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>
</pre>

Draws <a href='undocumented#Vertices'>Vertices</a> <a href='#SkCanvas_drawVertices_vertices'>vertices</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_mode'><code><strong>mode</strong></code></a></td>
    <td>combines <a href='undocumented#Colors'>Vertices Colors</a> with <a href='undocumented#Shader'>Shader</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#Shader'>Shader</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f48b22eaad1bb7adcc3faaa321754af6"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a><sup><a href='#SkCanvas_drawPatch_2'>[2]</a></sup> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawVertices_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='undocumented#Vertices'>Vertices</a> <a href='#SkCanvas_drawVertices_2_vertices'>vertices</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_2_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_2_mode'><code><strong>mode</strong></code></a></td>
    <td>combines <a href='undocumented#Colors'>Vertices Colors</a> with <a href='undocumented#Shader'>Shader</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_2_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#Shader'>Shader</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e8bdae9bea3227758989028424fcac3d"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a><sup><a href='#SkCanvas_drawPatch_2'>[2]</a></sup> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawVertices_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>
</pre>

Draws <a href='undocumented#Vertices'>Vertices</a> <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_3_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_bones'><code><strong>bones</strong></code></a></td>
    <td>bone matrix data</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_boneCount'><code><strong>boneCount</strong></code></a></td>
    <td>number of bone matrices</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_mode'><code><strong>mode</strong></code></a></td>
    <td>combines <a href='undocumented#Colors'>Vertices Colors</a> with <a href='undocumented#Shader'>Shader</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#Shader'>Shader</a></td>
  </tr>
</table>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a><sup><a href='#SkCanvas_drawPatch_2'>[2]</a></sup> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawVertices_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws <a href='undocumented#Vertices'>Vertices</a> <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_4_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_bones'><code><strong>bones</strong></code></a></td>
    <td>bone matrix data</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_boneCount'><code><strong>boneCount</strong></code></a></td>
    <td>number of bone matrices</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_mode'><code><strong>mode</strong></code></a></td>
    <td>combines <a href='undocumented#Colors'>Vertices Colors</a> with <a href='undocumented#Shader'>Shader</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#Shader'>Shader</a></td>
  </tr>
</table>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a><sup><a href='#SkCanvas_drawPatch_2'>[2]</a></sup> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawPatch'></a>
## drawPatch

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> cubics
</pre>

Draws a <a href='undocumented#Coons_Patch'>Coons Patch</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_cubics'><code><strong>cubics</strong></code></a></td>
    <td><a href='SkPath_Reference#Cubic'>Path Cubic</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_colors'><code><strong>colors</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>Color</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_texCoords'><code><strong>texCoords</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> array of texture coordinates</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> for <a href='#SkCanvas_drawPatch_colors'>colors</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Shader'>Shader</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="accb545d67984ced168f5be6ab824795"></fiddle-embed></div>

### See Also

SeeAlso <a href='#SkCanvas_drawVertices'>drawVertices</a><sup><a href='#SkCanvas_drawVertices_2'>[2]</a></sup><sup><a href='#SkCanvas_drawVertices_3'>[3]</a></sup><sup><a href='#SkCanvas_drawVertices_4'>[4]</a></sup> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawPatch_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> cubics
</pre>

Draws <a href='SkPath_Reference#Cubic'>Cubic</a> <a href='undocumented#Coons_Patch'>Coons Patch</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_2_cubics'><code><strong>cubics</strong></code></a></td>
    <td><a href='SkPath_Reference#Cubic'>Path Cubic</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_colors'><code><strong>colors</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>Color</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_texCoords'><code><strong>texCoords</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> array of texture coordinates</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Shader'>Shader</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4cf70f8d194867d053d7e177e5088445"></fiddle-embed></div>

### Example

<div><fiddle-embed name="3412c2a16cb529af0e04878d264451f2"></fiddle-embed></div>

### See Also

SeeAlso <a href='#SkCanvas_drawVertices'>drawVertices</a><sup><a href='#SkCanvas_drawVertices_2'>[2]</a></sup><sup><a href='#SkCanvas_drawVertices_3'>[3]</a></sup><sup><a href='#SkCanvas_drawVertices_4'>[4]</a></sup> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawAtlas'></a>
## drawAtlas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws a set of sprites from <a href='#SkCanvas_drawAtlas_atlas'>atlas</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing sprites</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#RSXform'>RSXform</a> mappings for sprites in <a href='#SkCanvas_drawAtlas_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> locations of sprites in <a href='#SkCanvas_drawAtlas_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_colors'><code><strong>colors</strong></code></a></td>
    <td>one per sprite</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_count'><code><strong>count</strong></code></a></td>
    <td>number of sprites to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> combining <a href='#SkCanvas_drawAtlas_colors'>colors</a> and sprites</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed sprites for efficient clipping</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Color_Filter'>Color Filter</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1df575f9b8132306ce0552a2554ed132"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup>

---

<a name='SkCanvas_drawAtlas_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws a set of sprites from <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_2_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing sprites</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#RSXform'>RSXform</a> mappings for sprites in <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> locations of sprites in <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_colors'><code><strong>colors</strong></code></a></td>
    <td>one per sprite</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_count'><code><strong>count</strong></code></a></td>
    <td>number of sprites to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> combining <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> and sprites</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed sprites for efficient clipping</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Color_Filter'>Color Filter</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e66a8f230a8d531bcef9f5ebdc5aac1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup>

---

<a name='SkCanvas_drawAtlas_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>
</pre>

Draws a set of sprites from <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_3_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing sprites</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#RSXform'>RSXform</a> mappings for sprites in <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> locations of sprites in <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_count'><code><strong>count</strong></code></a></td>
    <td>number of sprites to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed sprites for efficient clipping</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Color_Filter'>Color Filter</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8dc0d0fdeab20bbc21cac6874ddbefcd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup>

---

<a name='SkCanvas_drawAtlas_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Draws a set of sprites from <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_4_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing sprites</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#RSXform'>RSXform</a> mappings for sprites in <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> locations of sprites in <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_count'><code><strong>count</strong></code></a></td>
    <td>number of sprites to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed sprites for efficient clipping</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Color_Filter'>Color Filter</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c093c2b14bd3e6171ede7cd4049d9b57"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a><sup><a href='#SkCanvas_drawImage_2'>[2]</a></sup>

---

<a name='SkCanvas_drawDrawable'></a>
## drawDrawable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>
</pre>

Draws <a href='undocumented#Drawable'>Drawable</a> <a href='#SkCanvas_drawDrawable_drawable'>drawable</a> using <a href='#Clip'>Clip</a> and <a href='#Matrix'>Matrix</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDrawable_drawable'><code><strong>drawable</strong></code></a></td>
    <td>custom struct encapsulating drawing commands</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_matrix'><code><strong>matrix</strong></code></a></td>
    <td>transformation applied to drawing</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3a4dfcd08838866b5cfc0d82489195ba"></fiddle-embed></div>

### See Also

<a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawDrawable_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>
</pre>

Draws <a href='undocumented#Drawable'>Drawable</a> <a href='#SkCanvas_drawDrawable_2_drawable'>drawable</a> using <a href='#Clip'>Clip</a> and <a href='#Matrix'>Matrix</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDrawable_2_drawable'><code><strong>drawable</strong></code></a></td>
    <td>custom struct encapsulating drawing commands</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_2_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='#Canvas'>Canvas</a> writable pixels on <a href='#SkCanvas_drawDrawable_2_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_2_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='#Canvas'>Canvas</a> writable pixels on <a href='#SkCanvas_drawDrawable_2_y'>y</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1bdc07ad3b154c89b771722c2fcaee3f"></fiddle-embed></div>

### See Also

<a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawPicture'>drawPicture</a><sup><a href='#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkCanvas_drawAnnotation'></a>
## drawAnnotation

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Associates <a href='SkRect_Reference#Rect'>Rect</a> on <a href='#Canvas'>Canvas</a> with an annotation

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAnnotation_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> extent of canvas to annotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_key'><code><strong>key</strong></code></a></td>
    <td>string used for lookup</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_value'><code><strong>value</strong></code></a></td>
    <td>data holding <a href='#SkCanvas_drawAnnotation_value'>value</a> stored in annotation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="00b430bd80d740e19c6d020a940f56d5"></fiddle-embed></div>

### See Also

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#SkDocument'>SkDocument</a>

---

<a name='SkCanvas_drawAnnotation_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Associates <a href='SkRect_Reference#Rect'>Rect</a> on <a href='#Canvas'>Canvas</a> when an annotation

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAnnotation_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> extent of canvas to annotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_2_key'><code><strong>key</strong></code></a></td>
    <td>string used for lookup</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_2_value'><code><strong>value</strong></code></a></td>
    <td>data holding <a href='#SkCanvas_drawAnnotation_2_value'>value</a> stored in annotation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="00b430bd80d740e19c6d020a940f56d5"></fiddle-embed></div>

### See Also

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#SkDocument'>SkDocument</a>

---

<a name='SkCanvas_isClipEmpty'></a>
## isClipEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkCanvas_isClipEmpty'>isClipEmpty</a>(
</pre>

Returns true if <a href='#Clip'>Clip</a> is empty

### Return Value

true if <a href='#Clip'>Clip</a> is empty

### Example

<div><fiddle-embed name="f106f146a58c8604308d4d8d7086d2f5">

#### Example Output

~~~~
clip is not empty
clip is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_isClipRect'>isClipRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a><sup><a href='#SkCanvas_getLocalClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a><sup><a href='#SkCanvas_getDeviceClipBounds_2'>[2]</a></sup>

---

<a name='SkCanvas_isClipRect'></a>
## isClipRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkCanvas_isClipRect'>isClipRect</a>(
</pre>

Returns true if <a href='#Clip'>Clip</a> is <a href='SkRect_Reference#Rect'>Rect</a> and not empty

### Return Value

true if <a href='#Clip'>Clip</a> is <a href='SkRect_Reference#Rect'>Rect</a> and not empty

### Example

<div><fiddle-embed name="9894bfb476c78a8f6c8f49fbbca3d50d">

#### Example Output

~~~~
clip is rect
clip is not rect
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_isClipEmpty'>isClipEmpty</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a><sup><a href='#SkCanvas_getLocalClipBounds_2'>[2]</a></sup> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a><sup><a href='#SkCanvas_getDeviceClipBounds_2'>[2]</a></sup>

---

