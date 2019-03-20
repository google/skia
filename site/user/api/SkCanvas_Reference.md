SkCanvas Reference
===


<a name='SkCanvas'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> {

    static std::unique_ptr<<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>> <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels,
                                                      size_t rowBytes,
                                                      const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    static std::unique_ptr<<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>> <a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a>(int width, int height, <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>* pixels,
                                                         size_t rowBytes);
    <a href='#SkCanvas_empty_constructor'>SkCanvas()</a>;
    <a href='#SkCanvas_int_int_const_SkSurfaceProps_star'>SkCanvas</a>(int width, int height, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    explicit <a href='#SkCanvas_copy_const_SkBitmap'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>);
    <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& props);
    virtual <a href='#SkCanvas_destructor'>~SkCanvas()</a>;
    <a href='undocumented#SkMetaData'>SkMetaData</a>& <a href='#SkCanvas_getMetaData'>getMetaData</a>();
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>() const;
    bool <a href='#SkCanvas_getProps'>getProps</a>(<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props) const;
    void <a href='#SkCanvas_flush'>flush()</a>;
    virtual <a href='undocumented#SkISize'>SkISize</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkCanvas_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    virtual <a href='undocumented#GrContext'>GrContext</a>* <a href='#SkCanvas_getGrContext'>getGrContext</a>();
    void* <a href='#SkCanvas_accessTopLayerPixels'>accessTopLayerPixels</a>(<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>* info, size_t* rowBytes, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* origin = nullptr);
    <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_Handle'>Handle</a> <a href='#SkCanvas_accessTopRasterHandle'>accessTopRasterHandle</a>() const;
    bool <a href='#SkCanvas_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);
    bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, int srcX, int srcY);
    bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, int srcX, int srcY);
    bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* pixels, size_t rowBytes, int x, int y);
    bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, int x, int y);
    int <a href='#SkCanvas_save'>save()</a>;
    int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    int <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    int <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, <a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>);

    enum <a href='#SkCanvas_SaveLayerFlagsSet'>SaveLayerFlagsSet</a> {
        <a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a> = 1 << 1,
        <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> = 1 << 2,
    };

    typedef uint32_t <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a>;

    int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>& layerRec);
    void <a href='#SkCanvas_restore'>restore()</a>;
    int <a href='#SkCanvas_getSaveCount'>getSaveCount</a>() const;
    void <a href='#SkCanvas_restoreToCount'>restoreToCount</a>(int saveCount);
    void <a href='#SkCanvas_translate'>translate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkCanvas_scale'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    void <a href='#SkCanvas_rotate'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees);
    void <a href='#SkCanvas_rotate'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkCanvas_skew'>skew</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    void <a href='#SkCanvas_concat'>concat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>);
    void <a href='#SkCanvas_setMatrix'>setMatrix</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>);
    void <a href='#SkCanvas_resetMatrix'>resetMatrix</a>();
    void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias);
    void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op);
    void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, bool doAntiAlias = false);
    void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias);
    void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='undocumented#SkClipOp'>SkClipOp</a> op);
    void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, bool doAntiAlias = false);
    void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias);
    void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op);
    void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, bool doAntiAlias = false);
    void <a href='#SkCanvas_clipRegion'>clipRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& deviceRgn, <a href='undocumented#SkClipOp'>SkClipOp</a> op = <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a>);
    bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) const;
    bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>) const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>() const;
    bool <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>() const;
    bool <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* bounds) const;
    void <a href='#SkCanvas_drawColor'>drawColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode = <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>);
    void <a href='#SkCanvas_clear'>clear</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>);
    void <a href='#SkCanvas_discard'>discard()</a>;
    void <a href='#SkCanvas_drawPaint'>drawPaint</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);

    enum <a href='#SkCanvas_PointMode'>PointMode</a> {
        <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>,
        <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>,
        <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>,
    };

    void <a href='#SkCanvas_drawPoints'>drawPoints</a>(<a href='#SkCanvas_PointMode'>PointMode</a> mode, size_t count, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='undocumented#SkScalar'>SkScalar</a> x0, <a href='undocumented#SkScalar'>SkScalar</a> y0, <a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p0, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p1, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawRect'>drawRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawIRect'>drawIRect</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawRegion'>drawRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawOval'>drawOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawRRect'>drawRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawDRRect'>drawDRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& outer, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& inner, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> cx, <a href='undocumented#SkScalar'>SkScalar</a> cy, <a href='undocumented#SkScalar'>SkScalar</a> radius, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> center, <a href='undocumented#SkScalar'>SkScalar</a> radius, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawArc'>drawArc</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle,
                 bool useCenter, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPath'>drawPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);

    enum <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> {
        <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>,
        <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>,
    };

    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawBitmap'>drawBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
                    const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    void <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='#SkCanvas_Lattice'>Lattice</a>& lattice, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                           const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='#SkCanvas_Lattice'>Lattice</a>& lattice, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                          const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr);
    void <a href='#SkCanvas_drawText'>drawText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                  const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawString'>drawString</a>(const char* <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawString'>drawString</a>(const <a href='undocumented#SkString'>SkString</a>& <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPosText'>drawPosText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[],
                     const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='undocumented#SkScalar'>SkScalar</a> xpos[], <a href='undocumented#SkScalar'>SkScalar</a> constY,
                      const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[],
                         const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* blob, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>>& blob, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>);
    void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>>& <a href='SkPicture_Reference#Picture'>picture</a>);
    void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>>& <a href='SkPicture_Reference#Picture'>picture</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>,
                     const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkVertices'>SkVertices</a>>& <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, const <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> bones[], int boneCount,
                      <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkVertices'>SkVertices</a>>& <a href='undocumented#Vertices'>vertices</a>, const <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> bones[],
                      int boneCount, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[4],
                   const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> texCoords[4], <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[4],
                   const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> texCoords[4], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
                   const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[], int count, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
                   const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[], int count, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[], int count,
                   const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
                   int count, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a> = nullptr);
    void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y);
    void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const char key[], <a href='undocumented#SkData'>SkData</a>* value);
    void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const char key[], const <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>>& value);
    virtual bool <a href='#SkCanvas_isClipEmpty'>isClipEmpty</a>() const;
    virtual bool <a href='#SkCanvas_isClipRect'>isClipRect</a>() const;
    const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a>() const;
};

</pre>

<a href='SkCanvas_Reference#Canvas'>Canvas</a> provides an interface for drawing, and how the drawing is clipped and transformed.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> contains a stack of <a href='SkMatrix_Reference#Matrix'>Matrix</a> and Clip values.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> and <a href='SkPaint_Reference#Paint'>Paint</a> together provide the state to draw into <a href='SkSurface_Reference#Surface'>Surface</a> or <a href='undocumented#Device'>Device</a>.
Each <a href='SkCanvas_Reference#Canvas'>Canvas</a> draw call transforms the geometry of the object by the concatenation of all
<a href='SkMatrix_Reference#Matrix'>Matrix</a> values in the stack. The transformed geometry is clipped by the intersection
of all of Clip values in the stack. The <a href='SkCanvas_Reference#Canvas'>Canvas</a> draw calls use <a href='SkPaint_Reference#Paint'>Paint</a> to supply drawing
state such as <a href='SkColor_Reference#Color'>Color</a>, <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, stroke width, <a href='undocumented#Shader'>Shader</a> and so on.

To draw to a pixel-based destination, create <a href='#Raster_Surface'>Raster_Surface</a> or <a href='#GPU_Surface'>GPU_Surface</a>.
Request <a href='SkCanvas_Reference#Canvas'>Canvas</a> from <a href='SkSurface_Reference#Surface'>Surface</a> to obtain the interface to draw.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> generated by <a href='#Raster_Surface'>Raster_Surface</a> draws to memory visible to the CPU.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> generated by <a href='#GPU_Surface'>GPU_Surface</a> uses Vulkan or OpenGL to draw to the GPU.

To draw to a <a href='undocumented#Document'>document</a>, obtain <a href='SkCanvas_Reference#Canvas'>Canvas</a> from <a href='#SVG_Canvas'>SVG_Canvas</a>, <a href='#Document_PDF'>Document_PDF</a>, or <a href='#Picture_Recorder'>Picture_Recorder</a>.
<a href='undocumented#Document'>Document</a> based <a href='SkCanvas_Reference#Canvas'>Canvas</a> and other <a href='SkCanvas_Reference#Canvas'>Canvas</a> subclasses reference <a href='undocumented#Device'>Device</a> describing the
destination.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> can be constructed to draw to <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> without first creating <a href='#Raster_Surface'>Raster_Surface</a>.
This approach may be deprecated in the future.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> may be created directly when no <a href='SkSurface_Reference#Surface'>Surface</a> is required; some <a href='SkCanvas_Reference#Canvas'>Canvas</a> methods
implicitly create <a href='#Raster_Surface'>Raster_Surface</a>.

<a name='SkCanvas_MakeRasterDirect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&gt; <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels,
                                                  size_t rowBytes,
                                                  const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr)
</pre>

Allocates raster <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> that will draw directly into <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a>.

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned if all parameters are valid.
Valid parameters include:
<a href='#SkCanvas_MakeRasterDirect_info'>info</a> dimensions are zero or positive;
<a href='#SkCanvas_MakeRasterDirect_info'>info</a> contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> supported by  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> is not nullptr;
<a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> is zero or large enough to contain <a href='#SkCanvas_MakeRasterDirect_info'>info</a> width <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> of <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

Pass zero for <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> to compute <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> from <a href='#SkCanvas_MakeRasterDirect_info'>info</a> width and <a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a>.
If <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> is greater than zero, it must be equal to or greater than
<a href='#SkCanvas_MakeRasterDirect_info'>info</a> width times bytes required for <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> buffer <a href='undocumented#Size'>size</a> should be <a href='#SkCanvas_MakeRasterDirect_info'>info</a> height times computed <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a>.
Pixels are not initialized.
To access <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> after drawing, call <a href='#SkCanvas_flush'>flush()</a> or <a href='#SkCanvas_peekPixels'>peekPixels</a>().

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirect_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, of  <a href='undocumented#Raster_Surface'>raster surface</a>;</td>
  </tr>
</table>

width, or height, or both, may be zero

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> row to the next, or zero</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="525285073aae7e53eb8f454a398f880c"><div>Allocates a three by three <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, clears it to white, and draws a black <a href='undocumented#Pixel'>pixel</a>
in the center.
</div>

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>

<a name='SkCanvas_MakeRasterDirectN32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&gt; <a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a>(int width, int height, <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>* pixels,
                                                     size_t rowBytes)
</pre>

Allocates raster <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> specified by inline <a href='SkImage_Reference#Image'>image</a> specification. Subsequent <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
calls draw into <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a>.
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is set to <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is set to <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.
To access <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> after drawing, call <a href='#SkCanvas_flush'>flush()</a> or <a href='#SkCanvas_peekPixels'>peekPixels</a>().

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned if all parameters are valid.
Valid parameters include:
<a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> and <a href='#SkCanvas_MakeRasterDirectN32_height'>height</a> are zero or positive;
<a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> is not nullptr;
<a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> is zero or large enough to contain <a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> of <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.

Pass zero for <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> to compute <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> from <a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> and <a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a>.
If <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> is greater than zero, it must be equal to or greater than
<a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> times bytes required for <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> buffer <a href='undocumented#Size'>size</a> should be <a href='#SkCanvas_MakeRasterDirectN32_height'>height</a> times <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count on  <a href='undocumented#Raster_Surface'>raster surface</a> created; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count on  <a href='undocumented#Raster_Surface'>raster surface</a> created; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> buffer; buffer <a href='undocumented#Size'>size</a> should be <a href='#SkCanvas_MakeRasterDirectN32_height'>height</a></td>
  </tr>
</table>

times <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> row to the next, or zero</td>
  </tr>
</table>

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="87f55e62ec4c3535e1a5d0f1415b20c6"><div>Allocates a three by three <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, clears it to white, and draws a black <a href='undocumented#Pixel'>pixel</a>
in the center.
</div>

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_MakeN32Premul'>MakeN32Premul</a>

<a name='SkCanvas_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_empty_constructor'>SkCanvas()</a>
</pre>

Creates an empty <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> with no backing <a href='undocumented#Device'>device</a> or pixels, with
a width and height of zero.

### Return Value

empty <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>

### Example

<div><fiddle-embed name="4a00e6589e862fde5be532f4b6e316ce"><div>Passes a placeholder to a function that requires one.
</div>

#### Example Output

~~~~
rect stays rect is true
rect stays rect is false
rect stays rect is true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_getCanvas'>getCanvas</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

<a name='SkCanvas_int_int_const_SkSurfaceProps_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_int_int_const_SkSurfaceProps_star'>SkCanvas</a>(int width, int height, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr)
</pre>

Creates <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> of the specified dimensions without a <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.
Used by subclasses with custom implementations for draw member functions.

If <a href='#SkCanvas_int_int_const_SkSurfaceProps_star_props'>props</a> equals nullptr, <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> are created with
<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>::<a href='#SkSurfaceProps_InitType'>InitType</a> settings, which choose the <a href='undocumented#Pixel'>pixel</a> striping
direction and order. Since a platform may dynamically change its direction when
the <a href='undocumented#Device'>device</a> is rotated, and since a platform may have multiple monitors with
different characteristics, it is best not to rely on this legacy behavior.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_width'><code><strong>width</strong></code></a></td>
    <td>zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_height'><code><strong>height</strong></code></a></td>
    <td>zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> placeholder with dimensions

### Example

<div><fiddle-embed name="ce6a5ef2df447970b4453489d9d67930">

#### Example Output

~~~~
canvas is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkPixelGeometry'>SkPixelGeometry</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

<a name='SkCanvas_copy_const_SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
explicit <a href='#SkCanvas_copy_const_SkBitmap'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>)
</pre>

Constructs a <a href='SkCanvas_Reference#Canvas'>canvas</a> that draws into <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a>.
Sets <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>::<a href='#SkSurfaceProps_kLegacyFontHost_InitType'>kLegacyFontHost_InitType</a> in constructed <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> is copied so that subsequently editing <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a> will not affect
constructed <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

May be deprecated in the future.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_copy_const_SkBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='undocumented#Pixel'>pixel</a></td>
  </tr>
</table>

storage of  <a href='undocumented#Raster_Surface'>raster surface</a>

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> that can be used to draw into <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a>

### Example

<div><fiddle-embed name="dd92db963af190e849894038f39b598a"><div>The actual output depends on the installed fonts.
</div>

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

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_getCanvas'>getCanvas</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

<a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps'>SkCanvas</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& props)
</pre>

Constructs a <a href='SkCanvas_Reference#Canvas'>canvas</a> that draws into <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a>.
Use <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'>props</a> to match the <a href='undocumented#Device'>device</a> characteristics, like LCD striping.

<a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a> is copied so that subsequently editing <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a> will not affect
constructed <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>,</td>
  </tr>
</table>

and  <a href='undocumented#Pixel_Storage'>pixel storage</a> of  <a href='undocumented#Raster_Surface'>raster surface</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'><code><strong>props</strong></code></a></td>
    <td>order and orientation of RGB striping; and whether to use</td>
  </tr>
</table>

<a href='undocumented#Device'>device</a> independent fonts

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> that can be used to draw into <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a>

### Example

<div><fiddle-embed name="c26cfae4c42cb445240335cc12a50235"><div>The actual output depends on the installed fonts.
</div>

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

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_getCanvas'>getCanvas</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

<a name='SkCanvas_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='#SkCanvas_destructor'>~SkCanvas()</a>
</pre>

Draws saved <a href='SkCanvas_Reference#Layer'>layers</a>, if any.
Frees up resources used by <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

### Example

<div><fiddle-embed name="b7bc91ff16c9b9351b2a127f35394b82"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Layer'>Layer</a> draws into <a href='SkBitmap_Reference#Bitmap'>bitmap</a>. <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> sets up an additional
drawing <a href='SkSurface_Reference#Surface'>surface</a> that blends with the <a href='SkBitmap_Reference#Bitmap'>bitmap</a>. When <a href='SkCanvas_Reference#Layer'>Layer</a> goes out of
scope, <a href='SkCanvas_Reference#Layer'>Layer</a> destructor is called. The saved <a href='SkCanvas_Reference#Layer'>Layer</a> is restored, drawing
transparent letters.
</div></fiddle-embed></div>

### See Also

<a href='#Canvas_State_Stack'>State_Stack</a>

<a name='Property'></a>

<a name='SkCanvas_getMetaData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMetaData'>SkMetaData</a>& <a href='#SkCanvas_getMetaData'>getMetaData</a>()
</pre>

Returns storage to associate additional <a href='undocumented#Data'>data</a> with the <a href='SkCanvas_Reference#Canvas'>canvas</a>.
The storage is freed when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is deleted.

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

<a name='SkCanvas_imageInfo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>()const
</pre>

Returns <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> for <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>. If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is not associated with raster <a href='SkSurface_Reference#Surface'>surface</a> or
GPU <a href='SkSurface_Reference#Surface'>surface</a>, returned <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is set to <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

dimensions and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> of <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>

### Example

<div><fiddle-embed name="d93389d971f8084c4ccc7a66e4e157ee">

#### Example Output

~~~~
emptyInfo == canvasInfo
~~~~

</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkCanvas_makeSurface'>makeSurface</a>

<a name='SkCanvas_getProps'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_getProps'>getProps</a>(<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props)const
</pre>

Copies <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>, if <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is associated with  <a href='undocumented#Raster_Surface'>raster surface</a> or
<a href='undocumented#GPU_Surface'>GPU surface</a>, and returns true. Otherwise, returns false and leave <a href='#SkCanvas_getProps_props'>props</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getProps_props'><code><strong>props</strong></code></a></td>
    <td>storage for writable <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a></td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> was copied

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

<a name='Utility'></a>

<a name='SkCanvas_flush'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_flush'>flush()</a>
</pre>

Triggers the immediate execution of all pending draw operations.
If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is associated with GPU <a href='SkSurface_Reference#Surface'>surface</a>, resolves all pending GPU operations.
If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is associated with raster <a href='SkSurface_Reference#Surface'>surface</a>, has no effect; raster draw
operations are never deferred.

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_flush'>flush</a> <a href='undocumented#GrContext'>GrContext</a>::<a href='#GrContext_flush'>flush</a> <a href='undocumented#GrContext'>GrContext</a>::<a href='#GrContext_abandonContext'>abandonContext</a>

<a name='SkCanvas_getBaseLayerSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='undocumented#SkISize'>SkISize</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a>()const
</pre>

Gets the <a href='undocumented#Size'>size</a> of the base or root <a href='SkCanvas_Reference#Layer'>layer</a> in global <a href='SkCanvas_Reference#Canvas'>canvas</a> coordinates. The
origin of the base <a href='SkCanvas_Reference#Layer'>layer</a> is always (0,0). The area available for drawing may be
smaller (due to clipping or <a href='#SkCanvas_saveLayer'>saveLayer</a>).

### Return Value

integral width and height of base <a href='SkCanvas_Reference#Layer'>layer</a>

### Example

<div><fiddle-embed name="374e245d91cd729eca48fd20e631fdf3">

#### Example Output

~~~~
clip=10,30
size=20,30
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>

<a name='SkCanvas_makeSurface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkCanvas_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr)
</pre>

Creates <a href='SkSurface_Reference#SkSurface'>SkSurface</a> matching <a href='#SkCanvas_makeSurface_info'>info</a> and <a href='#SkCanvas_makeSurface_props'>props</a>, and associates it with <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.
Returns nullptr if no match found.

If <a href='#SkCanvas_makeSurface_props'>props</a> is nullptr, matches <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> in <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>. If <a href='#SkCanvas_makeSurface_props'>props</a> is nullptr and <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
does not have <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>, creates <a href='SkSurface_Reference#SkSurface'>SkSurface</a> with default <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_makeSurface_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_makeSurface_props'><code><strong>props</strong></code></a></td>
    <td><a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> to match; may be nullptr to match <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> matching <a href='#SkCanvas_makeSurface_info'>info</a> and <a href='#SkCanvas_makeSurface_props'>props</a>, or nullptr if no match is available

### Example

<div><fiddle-embed name="1ce28351444b41ab2b8e3128a4b9b9c2">

#### Example Output

~~~~
compatible != nullptr
size = 3, 4
~~~~

</fiddle-embed></div>

### See Also

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_makeSurface'>makeSurface</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>

<a name='SkCanvas_getGrContext'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='undocumented#GrContext'>GrContext</a>* <a href='#SkCanvas_getGrContext'>getGrContext</a>()
</pre>

Returns GPU context of the GPU <a href='SkSurface_Reference#Surface'>surface</a> associated with <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

### Return Value

GPU context, if available; nullptr otherwise

### Example

<div><fiddle-embed name="c4ea949e5fa5a0630dcb6b0204bd498f"></fiddle-embed></div>

### See Also

<a href='undocumented#GrContext'>GrContext</a>

<a name='SkCanvas_accessTopLayerPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkCanvas_accessTopLayerPixels'>accessTopLayerPixels</a>(<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>* info, size_t* rowBytes, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* origin = nullptr)
</pre>

Returns the <a href='undocumented#Pixel'>pixel</a> base address, <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='#SkCanvas_accessTopLayerPixels_rowBytes'>rowBytes</a>, and <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a> if the pixels
can be read directly. The returned address is only valid
while <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is in scope and unchanged. Any <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> call or <a href='SkSurface_Reference#SkSurface'>SkSurface</a> call
may invalidate the returned address and other returned values.

If pixels are inaccessible, <a href='#SkCanvas_accessTopLayerPixels_info'>info</a>, <a href='#SkCanvas_accessTopLayerPixels_rowBytes'>rowBytes</a>, and <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a> are unchanged.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_info'><code><strong>info</strong></code></a></td>
    <td>storage for writable pixels' <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>storage for writable pixels' row bytes; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_origin'><code><strong>origin</strong></code></a></td>
    <td>storage for <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> top <a href='SkCanvas_Reference#Layer'>layer</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a>, its top-left corner;</td>
  </tr>
</table>

may be nullptr

### Return Value

address of pixels, or nullptr if inaccessible

### Example

<div><fiddle-embed name="38d0d6ca9bea146d31bcbec197856359"></fiddle-embed></div>

### Example

<div><fiddle-embed name="a7ac9c21bbabcdeeca00f72a61cd0f3e"><div>Draws "ABC" on the <a href='undocumented#Device'>device</a>. Then draws "DEF" in <a href='SkCanvas_Reference#Layer'>Layer</a>, and reads
<a href='SkCanvas_Reference#Layer'>Layer</a> to add a large dotted "DEF". Finally blends <a href='SkCanvas_Reference#Layer'>Layer</a> with the
<a href='undocumented#Device'>device</a>.

The <a href='SkCanvas_Reference#Layer'>Layer</a> and blended result appear on the CPU and GPU but the large dotted
"DEF" appear only on the CPU.
</div></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>

<a name='SkCanvas_accessTopRasterHandle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_Handle'>Handle</a> <a href='#SkCanvas_accessTopRasterHandle'>accessTopRasterHandle</a>()const
</pre>

Returns custom context that tracks the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip.

Use <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a> to blend Skia drawing with custom drawing, typically performed
by the host platform user interface. The custom context returned is generated by
<a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a>, which creates a custom <a href='SkCanvas_Reference#Canvas'>canvas</a> with raster storage for
the drawing destination.

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

<a name='Pixels'></a>

<a name='SkCanvas_peekPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)
</pre>

Returns true if <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> has direct access to its pixels.

Pixels are readable when <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> is raster. Pixels are not readable when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
is returned from  <a href='undocumented#GPU_Surface'>GPU surface</a>, returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>, returned by
<a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, or <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is the base of a utility class
like <a href='undocumented#DebugCanvas'>DebugCanvas</a>.

<a href='#SkCanvas_peekPixels_pixmap'>pixmap</a> is valid only while <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is in scope and unchanged. Any
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> or <a href='SkSurface_Reference#SkSurface'>SkSurface</a> call may invalidate the <a href='#SkCanvas_peekPixels_pixmap'>pixmap</a> values.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> has direct access to pixels

### Example

<div><fiddle-embed name="e9411d676d1fa13b46331abe9e14ad3e">

#### Example Output

~~~~
width=256 height=256
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_peekPixels'>peekPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_peekPixels'>peekPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_peekPixels'>peekPixels</a>

<a name='SkCanvas_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkCanvas_Reference#Canvas'>Canvas</a> into <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> and Clip are
ignored.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkCanvas_readPixels_srcX'>srcX</a>, <a href='#SkCanvas_readPixels_srcY'>srcY</a>) and (<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a>).
Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() and <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() if required.

Pixels are readable when <a href='undocumented#Device'>Device</a> is raster, or backed by a GPU.
Pixels are not readable when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
returned by <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, or <a href='SkCanvas_Reference#Canvas'>Canvas</a> is the base of a utility
class like <a href='undocumented#DebugCanvas'>DebugCanvas</a>.

The destination  <a href='undocumented#Pixel_Storage'>pixel storage</a> must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkCanvas_readPixels_srcX'>srcX</a> or <a href='#SkCanvas_readPixels_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels could not be converted to <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() or <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels are not readable; for instance, <a href='SkCanvas_Reference#Canvas'>Canvas</a> is document-based.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_dstRowBytes'>dstRowBytes</a> is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> of <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels; <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a> times <a href='#SkCanvas_readPixels_dstRowBytes'>dstRowBytes</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of one destination row; <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a> times <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="102d014d7f753db2a9b9ee08893aaf11"><div>A black <a href='undocumented#Circle'>circle</a> drawn on a blue background provides an <a href='SkImage_Reference#Image'>image</a> to copy.
<a href='#SkCanvas_readPixels'>readPixels</a> copies one quarter of the <a href='SkCanvas_Reference#Canvas'>canvas</a> into each of the four corners.
The copied quarter <a href='undocumented#Circle'>circles</a> overdraw the original <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="481e990e923a0ed34654f4361b94f096"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> returned by <a href='#Raster_Surface'>Raster_Surface</a> has <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Pixel'>pixel</a> values.
<a href='#SkCanvas_clear'>clear()</a> takes <a href='undocumented#Unpremultiply'>Unpremultiplied</a> input with <a href='#Color_Alpha'>Color_Alpha</a> equal 0x80
and RGB equal 0x55, 0xAA, 0xFF. RGB is multiplied by <a href='#Color_Alpha'>Color_Alpha</a>
to generate <a href='undocumented#Premultiply'>Premultiplied</a> value 0x802B5580. <a href='#SkCanvas_readPixels'>readPixels</a> converts <a href='undocumented#Pixel'>pixel</a> back
to <a href='undocumented#Unpremultiply'>Unpremultiplied</a> value 0x8056A9FF, introducing error.
</div>

#### Example Output

~~~~
pixel = 802b5580
pixel = 8056a9ff
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='#SkCanvas_writePixels'>writePixels</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkCanvas_readPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkCanvas_Reference#Canvas'>Canvas</a> into <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> and Clip are
ignored.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkCanvas_readPixels_2_srcX'>srcX</a>, <a href='#SkCanvas_readPixels_2_srcY'>srcY</a>) and (<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_height'>height()</a>).
Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_colorType'>colorType</a>() and <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() if required.

Pixels are readable when <a href='undocumented#Device'>Device</a> is raster, or backed by a GPU.
Pixels are not readable when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
returned by <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, or <a href='SkCanvas_Reference#Canvas'>Canvas</a> is the base of a utility
class like <a href='undocumented#DebugCanvas'>DebugCanvas</a>.

Caller must allocate  <a href='undocumented#Pixel_Storage'>pixel storage</a> in <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a> if needed.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination <a href='SkRect_Reference#Rect'>Rects</a>
are copied.  <a href='SkPixmap_Reference#Pixmap_Pixels'>pixmap pixels</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkCanvas_readPixels_2_srcX'>srcX</a> or <a href='#SkCanvas_readPixels_2_srcY'>srcY</a> to offset pixels across or down <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels could not be converted to <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_colorType'>colorType</a>() or <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels are not readable; for instance, <a href='SkCanvas_Reference#Canvas'>Canvas</a> is document-based.</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> pixels could not be allocated.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_2_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for pixels copied from <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="85f199032943b6483722c34a91c4e20f"><div><a href='#SkCanvas_clear'>clear()</a> takes <a href='undocumented#Unpremultiply'>Unpremultiplied</a> input with <a href='#Color_Alpha'>Color_Alpha</a> equal 0x80
and RGB equal 0x55, 0xAA, 0xFF. RGB is multiplied by <a href='#Color_Alpha'>Color_Alpha</a>
to generate <a href='undocumented#Premultiply'>Premultiplied</a> value 0x802B5580.
</div>

#### Example Output

~~~~
pixel = 802b5580
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='#SkCanvas_writePixels'>writePixels</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkCanvas_readPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkCanvas_Reference#Canvas'>Canvas</a> into <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> and Clip are
ignored.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkCanvas_readPixels_3_srcX'>srcX</a>, <a href='#SkCanvas_readPixels_3_srcY'>srcY</a>) and (<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_width'>width()</a>, <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_height'>height()</a>).
Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_colorType'>colorType</a>() and <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_alphaType'>alphaType</a>() if required.

Pixels are readable when <a href='undocumented#Device'>Device</a> is raster, or backed by a GPU.
Pixels are not readable when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
returned by <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, or <a href='SkCanvas_Reference#Canvas'>Canvas</a> is the base of a utility
class like <a href='undocumented#DebugCanvas'>DebugCanvas</a>.

Caller must allocate  <a href='undocumented#Pixel_Storage'>pixel storage</a> in <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a> if needed.

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> pixels outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkCanvas_readPixels_3_srcX'>srcX</a> or <a href='#SkCanvas_readPixels_3_srcY'>srcY</a> to offset pixels across or down <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels could not be converted to <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_colorType'>colorType</a>() or <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels are not readable; for instance, <a href='SkCanvas_Reference#Canvas'>Canvas</a> is document-based.</td>
  </tr>  <tr>
    <td><a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a> could not be allocated.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_rowBytes'>rowBytes</a>() is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_3_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>storage for pixels copied from <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="af6dec8ef974aa67bf102f29915bcd6a"><div><a href='#SkCanvas_clear'>clear()</a> takes <a href='undocumented#Unpremultiply'>Unpremultiplied</a> input with <a href='#Color_Alpha'>Color_Alpha</a> equal 0x80
and RGB equal 0x55, 0xAA, 0xFF. RGB is multiplied by <a href='#Color_Alpha'>Color_Alpha</a>
to generate <a href='undocumented#Premultiply'>Premultiplied</a> value 0x802B5580.
</div>

#### Example Output

~~~~
pixel = 802b5580
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='#SkCanvas_writePixels'>writePixels</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkCanvas_writePixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* pixels, size_t rowBytes, int x, int y)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> from <a href='#SkCanvas_writePixels_pixels'>pixels</a> to <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> and Clip are ignored.
Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkCanvas_writePixels_x'>x</a>, <a href='#SkCanvas_writePixels_y'>y</a>) and
(<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).

Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() and <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>() if required.

Pixels are writable when <a href='undocumented#Device'>Device</a> is raster, or backed by a GPU.
Pixels are not writable when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
returned by <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, or <a href='SkCanvas_Reference#Canvas'>Canvas</a> is the base of a utility
class like <a href='undocumented#DebugCanvas'>DebugCanvas</a>.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only <a href='#SkCanvas_writePixels_pixels'>pixels</a> within both source and destination rectangles
are copied. <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkCanvas_writePixels_x'>x</a> or <a href='#SkCanvas_writePixels_y'>y</a> to offset <a href='#SkCanvas_writePixels_pixels'>pixels</a> to the left or
above <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a>.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_pixels'>pixels</a> could not be converted to <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() or
<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> are not writable; for instance, <a href='SkCanvas_Reference#Canvas'>Canvas</a> is document-based.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_rowBytes'>rowBytes</a> is too small to contain one row of <a href='#SkCanvas_writePixels_pixels'>pixels</a>.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_writePixels_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> of <a href='#SkCanvas_writePixels_pixels'>pixels</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td><a href='#SkCanvas_writePixels_pixels'>pixels</a> to copy, of <a href='undocumented#Size'>size</a> <a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_height'>height()</a> times <a href='#SkCanvas_writePixels_rowBytes'>rowBytes</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of one row of <a href='#SkCanvas_writePixels_pixels'>pixels</a>; <a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> times <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> writable <a href='#SkCanvas_writePixels_pixels'>pixels</a> on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> writable <a href='#SkCanvas_writePixels_pixels'>pixels</a> on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_writePixels_pixels'>pixels</a> were written to <a href='SkCanvas_Reference#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="29b98ebf58aa9fd1edfaabf9f4490b3a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_writePixels'>writePixels</a>

<a name='SkCanvas_writePixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, int x, int y)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> from pixels to <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> and Clip are ignored.
Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a>.<a href='#SkBitmap_width'>width()</a>, <a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a>.<a href='#SkBitmap_height'>height()</a>).

Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkCanvas_writePixels_2_x'>x</a>, <a href='#SkCanvas_writePixels_2_y'>y</a>) and
(<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).

Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() and <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>() if required.

Pixels are writable when <a href='undocumented#Device'>Device</a> is raster, or backed by a GPU.
Pixels are not writable when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> is returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
returned by <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, or <a href='SkCanvas_Reference#Canvas'>Canvas</a> is the base of a utility
class like <a href='undocumented#DebugCanvas'>DebugCanvas</a>.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkCanvas_writePixels_2_x'>x</a> or <a href='#SkCanvas_writePixels_2_y'>y</a> to offset pixels to the left or
above <a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> does not have allocated pixels.</td>
  </tr>  <tr>
    <td><a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a> could not be converted to <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() or
<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> pixels are not writable; for instance, <a href='SkCanvas_Reference#Canvas'>Canvas</a> is <a href='undocumented#Document'>document</a> based.</td>
  </tr>  <tr>
    <td><a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a> are inaccessible; for instance, <a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> wraps a <a href='undocumented#Texture'>texture</a>.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_writePixels_2_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>contains pixels copied to <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_2_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> writable pixels in <a href='#SkCanvas_writePixels_2_x'>x</a>; may be negative</td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_2_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> writable pixels in <a href='#SkCanvas_writePixels_2_y'>y</a>; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were written to <a href='SkCanvas_Reference#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="8b128e067881f9251357653692fa28da"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_writePixels'>writePixels</a>

<a name='State_Stack'></a>

---

<a href='SkCanvas_Reference#Canvas'>Canvas</a> maintains a stack of state that allows hierarchical drawing, commonly used
to implement windows and views. The initial state has an identity <a href='SkMatrix_Reference#Matrix'>matrix</a> and and
an infinite clip. Even with a wide-open clip, drawing is constrained by the
bounds of the <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkSurface_Reference#Surface'>Surface</a> or <a href='undocumented#Device'>Device</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> savable state consists of Clip and <a href='SkMatrix_Reference#Matrix'>Matrix</a>.
Clip describes the area that may be drawn to.
<a href='SkMatrix_Reference#Matrix'>Matrix</a> transforms the geometry.

<a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_saveLayer'>saveLayer</a>, <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>, and <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>
save state and return the depth of the stack.

<a href='#SkCanvas_restore'>restore()</a>, <a href='#SkCanvas_restoreToCount'>restoreToCount</a>, and <a href='#SkCanvas_destructor'>~SkCanvas()</a> revert state to its value when saved.

Each state on the stack intersects Clip with the previous Clip,
and concatenates <a href='SkMatrix_Reference#Matrix'>Matrix</a> with the previous <a href='SkMatrix_Reference#Matrix'>Matrix</a>.
The intersected Clip makes the drawing area the same or smaller;
the concatenated <a href='SkMatrix_Reference#Matrix'>Matrix</a> may move the origin and potentially scale or rotate
the coordinate space.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> does not require balancing the  <a href='#State_Stack'>state stack</a> but it is a good idea
to do so. Calling <a href='#SkCanvas_save'>save()</a> without <a href='#SkCanvas_restore'>restore()</a> will eventually cause Skia to fail;
mismatched <a href='#SkCanvas_save'>save()</a> and <a href='#SkCanvas_restore'>restore()</a> create hard to find bugs.

It is not possible to use state to draw outside of the clip defined by the
previous state.

### Example

<div><fiddle-embed name="bb1dbfdca3aedf716beb6f07e2aab065"><div>Draw to ever smaller clips; then restore drawing to full <a href='SkCanvas_Reference#Canvas'>canvas</a>.
Note that the second <a href='#SkCanvas_clipRect'>clipRect</a> is not permitted to enlarge Clip.
</div></fiddle-embed></div>

Each Clip uses the current <a href='SkMatrix_Reference#Matrix'>Matrix</a> for its coordinates.

### Example

<div><fiddle-embed name="9f563a2d60aa31d4b26742e5aa17aa4e"><div>While <a href='#SkCanvas_clipRect'>clipRect</a> is given the same rectangle twice, <a href='SkMatrix_Reference#Matrix'>Matrix</a> makes the second
<a href='#SkCanvas_clipRect'>clipRect</a> draw at half the <a href='undocumented#Size'>size</a> of the first.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_save'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_save'>save()</a>
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip.
Calling <a href='#SkCanvas_restore'>restore()</a> discards changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip,
restoring the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip to their state when <a href='#SkCanvas_save'>save()</a> was called.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be changed by <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_setMatrix'>setMatrix</a>(),
and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). Clip may be changed by <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(), <a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

Saved <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> state is put on a stack; multiple calls to <a href='#SkCanvas_save'>save()</a> should be balance
by an equal number of calls to <a href='#SkCanvas_restore'>restore()</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() with result to restore this and subsequent saves.

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="e477dce358a9ba3b0aa1bf33b8a376de"><div>The black square is translated 50 pixels down and to the right.
Restoring <a href='SkCanvas_Reference#Canvas'>Canvas</a> state removes <a href='#SkCanvas_translate'>translate()</a> from <a href='SkCanvas_Reference#Canvas'>Canvas</a> stack;
the red square is not translated, and is drawn at the origin.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_restore'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_restore'>restore()</a>
</pre>

Removes changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip since <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> state was
last saved. The state is removed from the stack.

Does nothing if the stack is empty.

### Example

<div><fiddle-embed name="e78471212a67f2f4fd39496e17a30d17"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_getSaveCount'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_getSaveCount'>getSaveCount</a>()const
</pre>

Returns the number of saved states, each containing: <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip.
Equals the number of <a href='#SkCanvas_save'>save()</a> calls less the number of <a href='#SkCanvas_restore'>restore()</a> calls plus one.
The save count of a new <a href='SkCanvas_Reference#Canvas'>canvas</a> is one.

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

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_restoreToCount'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_restoreToCount'>restoreToCount</a>(int saveCount)
</pre>

Restores state to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip values when <a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_saveLayer'>saveLayer</a>(),
<a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(), or <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>() returned <a href='#SkCanvas_restoreToCount_saveCount'>saveCount</a>.

Does nothing if <a href='#SkCanvas_restoreToCount_saveCount'>saveCount</a> is greater than  <a href='#State_Stack'>state stack</a> count.
Restores state to initial values if <a href='#SkCanvas_restoreToCount_saveCount'>saveCount</a> is less than or equal to one.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_restoreToCount_saveCount'><code><strong>saveCount</strong></code></a></td>
    <td>depth of  <a href='#State_Stack'>state stack</a> to restore</td>
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

<a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_getSaveCount'>getSaveCount</a> <a href='#SkCanvas_save'>save</a>

<a name='Layer'></a>

<a href='SkCanvas_Reference#Layer'>Layer</a> allocates a temporary <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to draw into. When the drawing is
complete, the <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is drawn into the <a href='SkCanvas_Reference#Canvas'>Canvas</a>.

<a href='SkCanvas_Reference#Layer'>Layer</a> is saved in a stack along with other saved state. When state with a <a href='SkCanvas_Reference#Layer'>Layer</a>
is restored, the <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is drawn into the previous <a href='SkCanvas_Reference#Layer'>Layer</a>.

<a href='SkCanvas_Reference#Layer'>Layer</a> may be initialized with the contents of the previous <a href='SkCanvas_Reference#Layer'>Layer</a>. When <a href='SkCanvas_Reference#Layer'>Layer</a> is
restored, its <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> can be modified by <a href='SkPaint_Reference#Paint'>Paint</a> passed to <a href='SkCanvas_Reference#Layer'>Layer</a> to apply
<a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>, and <a href='#Blend_Mode'>Blend_Mode</a>.

<a name='SkCanvas_saveLayer'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and allocates a <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> for subsequent drawing.
Calling <a href='#SkCanvas_restore'>restore()</a> discards changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and draws the <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be changed by <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). Clip may be changed by <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayer_bounds'>bounds</a> suggests but does not define the <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='undocumented#Size'>size</a>. To clip drawing to
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

Optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_saveLayer_paint'>paint</a> applies <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> when <a href='#SkCanvas_restore'>restore()</a> is called.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() with returned value to restore this and subsequent saves.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> of the <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayer_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="42318b18d403e17e07a541652da91ee2"><div>Rectangles are blurred by <a href='#Image_Filter'>Image_Filter</a> when <a href='#SkCanvas_restore'>restore()</a> draws <a href='SkCanvas_Reference#Layer'>Layer</a> to main
<a href='SkCanvas_Reference#Canvas'>Canvas</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='SkCanvas_saveLayer_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and allocates a <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> for subsequent drawing.
Calling <a href='#SkCanvas_restore'>restore()</a> discards changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and draws the <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be changed by <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). Clip may be changed by <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayer_2_bounds'>bounds</a> suggests but does not define the <a href='SkCanvas_Reference#Layer'>layer</a> <a href='undocumented#Size'>size</a>. To clip drawing to
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

Optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_saveLayer_2_paint'>paint</a> applies <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> when <a href='#SkCanvas_restore'>restore()</a> is called.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() with returned value to restore this and subsequent saves.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> of <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayer_2_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="a17aec3aa4909527be039e26a7eda694"><div>Rectangles are blurred by <a href='#Image_Filter'>Image_Filter</a> when <a href='#SkCanvas_restore'>restore()</a> draws <a href='SkCanvas_Reference#Layer'>Layer</a> to main <a href='SkCanvas_Reference#Canvas'>Canvas</a>.
The red rectangle is clipped; it does not fully fit on <a href='SkCanvas_Reference#Layer'>Layer</a>.
<a href='#Image_Filter'>Image_Filter</a> blurs past edge of <a href='SkCanvas_Reference#Layer'>Layer</a> so red rectangle is blurred on all sides.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='SkCanvas_saveLayerPreserveLCDTextRequests'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and allocates a <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> for subsequent drawing.
LCD <a href='undocumented#Text'>text</a> is preserved when the <a href='SkCanvas_Reference#Layer'>layer</a> is drawn to the prior <a href='SkCanvas_Reference#Layer'>layer</a>.

Calling <a href='#SkCanvas_restore'>restore()</a> discards changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and draws <a href='SkCanvas_Reference#Layer'>layer</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be changed by <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). Clip may be changed by <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>bounds</a> suggests but does not define the <a href='SkCanvas_Reference#Layer'>layer</a> <a href='undocumented#Size'>size</a>. To clip drawing to
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

Optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_paint'>paint</a> applies <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> when <a href='#SkCanvas_restore'>restore()</a> is called.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() with returned value to restore this and subsequent saves.

Draw <a href='undocumented#Text'>text</a> on an opaque background so that LCD <a href='undocumented#Text'>text</a> blends correctly with the
prior <a href='SkCanvas_Reference#Layer'>layer</a>. LCD <a href='undocumented#Text'>text</a> drawn on a background with transparency may result in
incorrect blending.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayerPreserveLCDTextRequests_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> of <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayerPreserveLCDTextRequests_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="8460bf8b013f46c67e0bd96e13451aff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='SkCanvas_saveLayerAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, <a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and allocates <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> for subsequent drawing.

Calling <a href='#SkCanvas_restore'>restore()</a> discards changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip,
and blends <a href='SkCanvas_Reference#Layer'>layer</a> with <a href='#SkCanvas_saveLayerAlpha_alpha'>alpha</a> opacity onto prior <a href='SkCanvas_Reference#Layer'>layer</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be changed by <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). Clip may be changed by <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>bounds</a> suggests but does not define <a href='SkCanvas_Reference#Layer'>layer</a> <a href='undocumented#Size'>size</a>. To clip drawing to
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

<a href='#SkCanvas_saveLayerAlpha_alpha'>alpha</a> of zero is fully transparent, 255 is fully opaque.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() with returned value to restore this and subsequent saves.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayerAlpha_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> of <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayerAlpha_alpha'><code><strong>alpha</strong></code></a></td>
    <td>opacity of <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="8ab88d86fb438856cc48d6e2f08a6e24"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='SkCanvas_SaveLayerFlagsSet'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_SaveLayerFlagsSet'>SaveLayerFlagsSet</a> {
        <a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a> = 1 << 1,
        <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> = 1 << 2,
    };

</pre>

<a name='SkCanvas_SaveLayerFlags'></a>

---

<a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> provides options that may be used in any combination in <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>,
defining how <a href='SkCanvas_Reference#Layer'>Layer</a> allocated by <a href='#SkCanvas_saveLayer'>saveLayer</a> operates. It may be set to zero,
<a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a>, <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a>, or both flags.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kPreserveLCDText_SaveLayerFlag'><code>SkCanvas::kPreserveLCDText_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Creates <a href='SkCanvas_Reference#Layer'>Layer</a> for LCD <a href='undocumented#Text'>text</a>. Flag is ignored if <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkPaint_Reference#Paint'>Paint</a> contains
<a href='#Image_Filter'>Image_Filter</a> or <a href='#Color_Filter'>Color_Filter</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kInitWithPrevious_SaveLayerFlag'><code>SkCanvas::kInitWithPrevious_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Initializes <a href='SkCanvas_Reference#Layer'>Layer</a> with the contents of the previous <a href='SkCanvas_Reference#Layer'>Layer</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="05db6a937225e8e31ae3481173d25dae"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Layer'>Layer</a> captures red and blue <a href='undocumented#Circle'>circles</a> scaled up by four.
scalePaint blends <a href='SkCanvas_Reference#Layer'>Layer</a> back with transparency.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='Layer_SaveLayerRec'></a>

<a name='SkCanvas_SaveLayerRec'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> {

        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec()</a>;
        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags = 0);
        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* backdrop,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags);

        const <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> = nullptr;
        const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a> = nullptr;
        const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> = nullptr;
        const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> = nullptr;
        const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> = nullptr;
        <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a> = 0;
    };

</pre>

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> contains the state used to create the <a href='SkCanvas_Reference#Layer'>Layer</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkRect*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fBounds'><code>fBounds</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> is used as a hint to limit the <a href='undocumented#Size'>size</a> of <a href='SkCanvas_Reference#Layer'>Layer</a>; may be nullptr.
<a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> suggests but does not define <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='undocumented#Size'>size</a>. To clip drawing to
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkPaint*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fPaint'><code>fPaint</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a> modifies how <a href='SkCanvas_Reference#Layer'>Layer</a> overlays the prior <a href='SkCanvas_Reference#Layer'>Layer</a>; may be nullptr.
<a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Draw_Looper'>Draw_Looper</a>, <a href='#Image_Filter'>Image_Filter</a>, and
<a href='#Mask_Filter'>Mask_Filter</a> affect <a href='SkCanvas_Reference#Layer'>Layer</a> draw.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkImageFilter*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fBackdrop'><code>fBackdrop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> applies <a href='#Image_Filter'>Image_Filter</a> to the prior <a href='SkCanvas_Reference#Layer'>Layer</a> when copying to the <a href='SkCanvas_Reference#Layer'>Layer</a>;
may be nullptr. Use <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> to copy the
prior <a href='SkCanvas_Reference#Layer'>Layer</a> without an <a href='#Image_Filter'>Image_Filter</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkImage*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fClipMask'><code>fClipMask</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_restore'>restore()</a> clips <a href='SkCanvas_Reference#Layer'>Layer</a> by the <a href='#Color_Alpha'>Color_Alpha</a> channel of <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> when
<a href='SkCanvas_Reference#Layer'>Layer</a> is copied to <a href='undocumented#Device'>Device</a>. <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> may be nullptr.    .
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkMatrix*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fClipMatrix'><code>fClipMatrix</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> transforms <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> before it clips <a href='SkCanvas_Reference#Layer'>Layer</a>. If
<a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> describes a translucent gradient, it may be scaled and rotated
without introducing artifacts. <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> may be nullptr.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SaveLayerFlags</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fSaveLayerFlags'><code>fSaveLayerFlags</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a> are used to create <a href='SkCanvas_Reference#Layer'>Layer</a> without transparency,
create <a href='SkCanvas_Reference#Layer'>Layer</a> for LCD <a href='undocumented#Text'>text</a>, and to create <a href='SkCanvas_Reference#Layer'>Layer</a> with the
contents of the previous <a href='SkCanvas_Reference#Layer'>Layer</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee8c0b120234e27364f8c9a786cf8f89"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Layer'>Layer</a> captures a red <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='undocumented#Circle'>circle</a> and a blue <a href='undocumented#Alias'>Aliased</a> <a href='undocumented#Circle'>circle</a> scaled
up by four. After drawing another red <a href='undocumented#Circle'>circle</a> without scaling on top, the <a href='SkCanvas_Reference#Layer'>Layer</a> is
transferred to the main <a href='SkCanvas_Reference#Canvas'>canvas</a>.
</div></fiddle-embed></div>

<a name='Layer_SaveLayerRec_Constructors'></a>

<a name='SkCanvas_SaveLayerRec_SaveLayerRec'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec()</a>
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, and <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> to nullptr. Clears <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>.

### Return Value

empty <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

### Example

<div><fiddle-embed name="b5cea1eed80a0eb04ddbab3f36dff73f">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags = 0)
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, and <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>; sets <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> to nullptr.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>layer</a> dimensions; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_paint'><code><strong>paint</strong></code></a></td>
    <td>applied to <a href='SkCanvas_Reference#Layer'>layer</a> when overlaying prior <a href='SkCanvas_Reference#Layer'>layer</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> options to modify <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> with empty <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a>

### Example

<div><fiddle-embed name="027f920259888fc19591ea9a90d92873">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* backdrop,
             <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> saveLayerFlags)
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a>, and <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>layer</a> dimensions; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_paint'><code><strong>paint</strong></code></a></td>
    <td>applied to <a href='SkCanvas_Reference#Layer'>layer</a> when overlaying prior <a href='SkCanvas_Reference#Layer'>layer</a>;</td>
  </tr>
</table>

may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_backdrop'><code><strong>backdrop</strong></code></a></td>
    <td>prior <a href='SkCanvas_Reference#Layer'>layer</a> copied with <a href='undocumented#SkImageFilter'>SkImageFilter</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> options to modify <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> fully specified

### Example

<div><fiddle-embed name="9b7fa2fe855642ffff6538829db15328">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='SkCanvas_saveLayer_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(const <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>& layerRec)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip, and allocates <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> for subsequent drawing.

Calling <a href='#SkCanvas_restore'>restore()</a> discards changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and clip,
and blends <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> with <a href='SkColor_Reference#Alpha'>alpha</a> opacity onto the prior <a href='SkCanvas_Reference#Layer'>layer</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be changed by <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). Clip may be changed by <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> contains the state used to create the <a href='SkCanvas_Reference#Layer'>layer</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() with returned value to restore this and subsequent saves.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_3_layerRec'><code><strong>layerRec</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>layer</a> state</td>
  </tr>
</table>

### Return Value

depth of save  <a href='#State_Stack'>state stack</a> before this call was made.

### Example

<div><fiddle-embed name="7d3751e82d1b6ec328ffa3d6f48ca831"><div>The example draws an <a href='SkImage_Reference#Image'>image</a>, and saves it into a <a href='SkCanvas_Reference#Layer'>Layer</a> with <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a>.
Next it punches a hole in <a href='SkCanvas_Reference#Layer'>Layer</a> and restore with <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a>.
Where <a href='SkCanvas_Reference#Layer'>Layer</a> was cleared, the original <a href='SkImage_Reference#Image'>image</a> will draw unchanged.
Outside of the <a href='undocumented#Circle'>circle</a> the mandrill is brightened.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='Matrix'></a>

<a name='SkCanvas_translate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void translate(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Translates <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by <a href='#SkCanvas_translate_dx'>dx</a> along the x-axis and <a href='#SkCanvas_translate_dy'>dy</a> along the y-axis.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with a translation <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of moving the drawing by (<a href='#SkCanvas_translate_dx'>dx</a>, <a href='#SkCanvas_translate_dy'>dy</a>) before transforming
the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_translate_dx'><code><strong>dx</strong></code></a></td>
    <td>distance to translate on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_translate_dy'><code><strong>dy</strong></code></a></td>
    <td>distance to translate on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="eb93d5fa66a5f7a10f4f9210494d7222"><div><a href='#SkCanvas_scale'>scale()</a> followed by <a href='#SkCanvas_translate'>translate()</a> produces different results from <a href='#SkCanvas_translate'>translate()</a> followed
by <a href='#SkCanvas_scale'>scale()</a>.

The blue stroke follows translate of (50, 50); a black
fill follows scale of (2, 1/2.f). After restoring the clip, which resets
<a href='SkMatrix_Reference#Matrix'>Matrix</a>, a red frame follows the same scale of (2, 1/2.f); a gray fill
follows translate of (50, 50).
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_scale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void scale(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy)
</pre>

Scales <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by <a href='#SkCanvas_scale_sx'>sx</a> on the x-axis and <a href='#SkCanvas_scale_sy'>sy</a> on the y-axis.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with a scale <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of scaling the drawing by (<a href='#SkCanvas_scale_sx'>sx</a>, <a href='#SkCanvas_scale_sy'>sy</a>) before transforming
the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_scale_sx'><code><strong>sx</strong></code></a></td>
    <td>amount to scale on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_scale_sy'><code><strong>sy</strong></code></a></td>
    <td>amount to scale on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7d0d801ef13c6c6da51e840c22ac15b0"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_rotate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void rotate(<a href='undocumented#SkScalar'>SkScalar</a> degrees)
</pre>

Rotates <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by <a href='#SkCanvas_rotate_degrees'>degrees</a>. Positive <a href='#SkCanvas_rotate_degrees'>degrees</a> rotates clockwise.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with a rotation <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of rotating the drawing by <a href='#SkCanvas_rotate_degrees'>degrees</a> before transforming
the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_rotate_degrees'><code><strong>degrees</strong></code></a></td>
    <td>amount to rotate, in <a href='#SkCanvas_rotate_degrees'>degrees</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="963789ac8498d4e505748ab3b15cdaa5"><div>Draw clock hands at time 5:10. The hour hand and minute hand <a href='SkPoint_Reference#Point'>point</a> up and
are rotated clockwise.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_rotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void rotate(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Rotates <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by <a href='#SkCanvas_rotate_2_degrees'>degrees</a> about a <a href='SkPoint_Reference#Point'>point</a> at (<a href='#SkCanvas_rotate_2_px'>px</a>, <a href='#SkCanvas_rotate_2_py'>py</a>). Positive <a href='#SkCanvas_rotate_2_degrees'>degrees</a> rotates
clockwise.

Mathematically, constructs a rotation <a href='SkMatrix_Reference#Matrix'>matrix</a>; <a href='undocumented#Premultiply'>premultiplies</a> the rotation <a href='SkMatrix_Reference#Matrix'>matrix</a> by
a translation <a href='SkMatrix_Reference#Matrix'>matrix</a>; then replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with the resulting <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of rotating the drawing about a given <a href='SkPoint_Reference#Point'>point</a> before
transforming the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_rotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>amount to rotate, in <a href='#SkCanvas_rotate_2_degrees'>degrees</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_rotate_2_px'><code><strong>px</strong></code></a></td>
    <td>x-axis value of the <a href='SkPoint_Reference#Point'>point</a> to rotate about</td>
  </tr>
  <tr>    <td><a name='SkCanvas_rotate_2_py'><code><strong>py</strong></code></a></td>
    <td>y-axis value of the <a href='SkPoint_Reference#Point'>point</a> to rotate about</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bcf5baea1c66a957d5ffd7b54bbbfeff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_skew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void skew(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy)
</pre>

Skews <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by <a href='#SkCanvas_skew_sx'>sx</a> on the x-axis and <a href='#SkCanvas_skew_sy'>sy</a> on the y-axis. A positive value of <a href='#SkCanvas_skew_sx'>sx</a>
skews the drawing right as y-axis values increase; a positive value of <a href='#SkCanvas_skew_sy'>sy</a> skews
the drawing down as x-axis values increase.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with a skew <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='undocumented#Premultiply'>premultiplied</a> with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of skewing the drawing by (<a href='#SkCanvas_skew_sx'>sx</a>, <a href='#SkCanvas_skew_sy'>sy</a>) before transforming
the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_skew_sx'><code><strong>sx</strong></code></a></td>
    <td>amount to skew on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_skew_sy'><code><strong>sy</strong></code></a></td>
    <td>amount to skew on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2e2acc21d7774df7e0940a30ad2ca99e"><div>Black <a href='undocumented#Text'>text</a> mimics an oblique <a href='undocumented#Text'>text</a> style by using a negative skew on x-axis
that shifts the geometry to the right as the y-axis values decrease.
Red <a href='undocumented#Text'>text</a> uses a positive skew on y-axis to shift the geometry down
as the x-axis values increase.
Blue <a href='undocumented#Text'>text</a> combines <a href='#SkCanvas_skew_sx'>sx</a> and <a href='#SkCanvas_skew_sy'>sy</a> skew to rotate and scale.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_concat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_concat'>concat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>)
</pre>

Replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with <a href='#SkCanvas_concat_matrix'>matrix</a> <a href='undocumented#Premultiply'>premultiplied</a> with existing <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of transforming the drawn geometry by <a href='#SkCanvas_concat_matrix'>matrix</a>, before
transforming the result with existing <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_concat_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#SkCanvas_concat_matrix'>matrix</a> to <a href='undocumented#Premultiply'>premultiply</a> with existing <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8f6818b25a92a88638ad99b2dd293f61"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_setMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_setMatrix'>setMatrix</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>)
</pre>

Replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with <a href='#SkCanvas_setMatrix_matrix'>matrix</a>.
Unlike <a href='#SkCanvas_concat'>concat()</a>, any prior <a href='#SkCanvas_setMatrix_matrix'>matrix</a> state is overwritten.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_setMatrix_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#SkCanvas_setMatrix_matrix'>matrix</a> to copy, replacing existing <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="24b9cf7e6f9a08394e1e07413bd8733a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_resetMatrix'>resetMatrix</a> <a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a>

<a name='SkCanvas_resetMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_resetMatrix'>resetMatrix</a>()
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to the identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
Any prior <a href='SkMatrix_Reference#Matrix'>matrix</a> state is overwritten.

### Example

<div><fiddle-embed name="412afffdf4682baa503a4e2e99201967"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_setMatrix'>setMatrix</a> <a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a>

<a name='SkCanvas_getTotalMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a>()const
</pre>

Returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This does not account for translation by <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> or <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> in <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>

### Example

<div><fiddle-embed name="c0d5fa544759704768f47cac91ae3832">

#### Example Output

~~~~
isIdentity true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_setMatrix'>setMatrix</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a> <a href='#SkCanvas_concat'>concat()</a>

<a name='Clip'></a>

---

Clip is built from a stack of clipping <a href='SkPath_Reference#Path'>paths</a>. Each <a href='SkPath_Reference#Path'>Path</a> in the
stack can be constructed from one or more <a href='#Path_Overview_Contour'>Path_Contour</a> elements. The
<a href='#Path_Overview_Contour'>Path_Contour</a> may be composed of any number of <a href='#Path_Verb'>Path_Verb</a> segments. Each
<a href='#Path_Overview_Contour'>Path_Contour</a> forms a closed area; <a href='#Path_Fill_Type'>Path_Fill_Type</a> defines the area enclosed
by <a href='#Path_Overview_Contour'>Path_Contour</a>.

Clip stack of <a href='SkPath_Reference#Path'>Path</a> elements successfully restrict the <a href='SkPath_Reference#Path'>Path</a> area. Each
<a href='SkPath_Reference#Path'>Path</a> is transformed by <a href='SkMatrix_Reference#Matrix'>Matrix</a>, then intersected with or subtracted from the
prior Clip to form the replacement Clip. Use <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kDifference'>kDifference</a>
to subtract <a href='SkPath_Reference#Path'>Path</a> from Clip; use <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a> to intersect <a href='SkPath_Reference#Path'>Path</a>
with Clip.

A clipping <a href='SkPath_Reference#Path'>Path</a> may be <a href='#Paint_Anti_Alias'>Anti_Aliased</a>; if <a href='SkPath_Reference#Path'>Path</a>, after transformation, is
composed of horizontal and vertical <a href='undocumented#Line'>lines</a>, clearing <a href='#Paint_Anti_Alias'>Anti_Alias</a> allows whole pixels
to either be inside or outside the clip. The fastest drawing has a <a href='undocumented#Alias'>Aliased</a>,
rectangular clip.

If clipping <a href='SkPath_Reference#Path'>Path</a> has <a href='#Paint_Anti_Alias'>Anti_Alias</a> set, clip may partially clip a <a href='undocumented#Pixel'>pixel</a>, requiring
that drawing blend partially with the destination along the edge. A rotated
rectangular <a href='#Paint_Anti_Alias'>Anti_Aliased</a> clip looks smoother but draws slower.

Clip can combine with <a href='SkRect_Reference#Rect'>Rect</a> and <a href='#RRect'>Round_Rect</a> primitives; like
<a href='SkPath_Reference#Path'>Path</a>, these are transformed by <a href='SkMatrix_Reference#Matrix'>Matrix</a> before they are combined with Clip.

Clip can combine with <a href='SkRegion_Reference#Region'>Region</a>. <a href='SkRegion_Reference#Region'>Region</a> is assumed to be in <a href='undocumented#Device'>Device</a> coordinates
and is unaffected by <a href='SkMatrix_Reference#Matrix'>Matrix</a>.

### Example

<div><fiddle-embed name="862cc026601a41a58df49c0b9f0d7777"><div>Draw a red <a href='undocumented#Circle'>circle</a> with an <a href='undocumented#Alias'>Aliased</a> clip and an <a href='#Paint_Anti_Alias'>Anti_Aliased</a> clip.
Use an  <a href='SkImage_Reference#Image'>image filter</a> to zoom into the pixels drawn.
The edge of the <a href='undocumented#Alias'>Aliased</a> clip fully draws pixels in the red <a href='undocumented#Circle'>circle</a>.
The edge of the <a href='#Paint_Anti_Alias'>Anti_Aliased</a> clip partially draws pixels in the red <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

<a name='SkCanvas_clipRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRect_rect'>rect</a>,
with an <a href='undocumented#Alias'>aliased</a> or <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> clip edge. <a href='#SkCanvas_clipRect_rect'>rect</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6a614faa0fbcf19958b5559c19b02d0f"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRect_2_rect'>rect</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; pixels are fully contained by the clip.
<a href='#SkCanvas_clipRect_2_rect'>rect</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="13bbc5fa5597a6cd4d704b419dbc66d9"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, bool doAntiAlias = false)
</pre>

Replaces clip with the intersection of clip and <a href='#SkCanvas_clipRect_3_rect'>rect</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; pixels are fully contained by the clip.
<a href='#SkCanvas_clipRect_3_rect'>rect</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_3_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d4e0632c97e42692775d834fe10aa99"><div>A <a href='undocumented#Circle'>circle</a> drawn in pieces looks uniform when drawn <a href='undocumented#Alias'>Aliased</a>.
The same <a href='undocumented#Circle'>circle</a> pieces blend with pixels more than once when <a href='#Paint_Anti_Alias'>Anti_Aliased</a>,
visible as a thin pair of <a href='undocumented#Line'>lines</a> through the right <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRRect_rrect'>rrect</a>,
with an <a href='undocumented#Alias'>aliased</a> or <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> clip edge.
<a href='#SkCanvas_clipRRect_rrect'>rrect</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="182ef48ab5e04ba3578496fda8d9fa36"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='undocumented#SkClipOp'>SkClipOp</a> op)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRRect_2_rrect'>rrect</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; pixels are fully contained by the clip.
<a href='#SkCanvas_clipRRect_2_rrect'>rrect</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_2_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ef6ae2eaae6761130ce38065d0364abd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, bool doAntiAlias = false)
</pre>

Replaces clip with the intersection of clip and <a href='#SkCanvas_clipRRect_3_rrect'>rrect</a>,
with an <a href='undocumented#Alias'>aliased</a> or <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> clip edge.
<a href='#SkCanvas_clipRRect_3_rrect'>rrect</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_3_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f583114580b2176fe3e75b0994476a84"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op, bool doAntiAlias)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipPath_path'>path</a>,
with an <a href='undocumented#Alias'>aliased</a> or <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> clip edge. <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> determines if <a href='#SkCanvas_clipPath_path'>path</a>
describes the area inside or outside its <a href='SkPath_Overview#Contour'>contours</a>; and if  <a href='SkPath_Overview#Contour'>path contour</a> overlaps
itself or another  <a href='SkPath_Overview#Contour'>path contour</a>, whether the overlaps form part of the area.
<a href='#SkCanvas_clipPath_path'>path</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee47ae6b813bfaa55e1a7b7c053ed60d"><div>Top figure uses <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> and <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kDifference'>kDifference</a>;
area outside clip is subtracted from <a href='undocumented#Circle'>circle</a>.

Bottom figure uses <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> and <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a>;
area inside clip is intersected with <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipPath_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> op)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipPath_2_path'>path</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; pixels are fully contained by the clip.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> determines if <a href='#SkCanvas_clipPath_2_path'>path</a>
describes the area inside or outside its <a href='SkPath_Overview#Contour'>contours</a>; and if  <a href='SkPath_Overview#Contour'>path contour</a> overlaps
itself or another  <a href='SkPath_Overview#Contour'>path contour</a>, whether the overlaps form part of the area.
<a href='#SkCanvas_clipPath_2_path'>path</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_2_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7856755c1bf8431c286c734b353345ad"><div>Overlapping <a href='SkRect_Reference#Rect'>Rects</a> form a clip. When clip <a href='#Path_Fill_Type'>Path_Fill_Type</a> is set to
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, the overlap is included. Set to
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>, the overlap is excluded and forms a hole.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipPath_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, bool doAntiAlias = false)
</pre>

Replaces clip with the intersection of clip and <a href='#SkCanvas_clipPath_3_path'>path</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; pixels are fully contained by the clip.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> determines if <a href='#SkCanvas_clipPath_3_path'>path</a>
describes the area inside or outside its <a href='SkPath_Overview#Contour'>contours</a>; and if  <a href='SkPath_Overview#Contour'>path contour</a> overlaps
itself or another  <a href='SkPath_Overview#Contour'>path contour</a>, whether the overlaps form part of the area.
<a href='#SkCanvas_clipPath_3_path'>path</a> is transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_3_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="187a7ae77a8176e417181411988534b6"><div>Clip loops over itself covering its center twice. When clip <a href='#Path_Fill_Type'>Path_Fill_Type</a>
is set to <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, the overlap is included. Set to
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>, the overlap is excluded and forms a hole.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRegion'>clipRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& deviceRgn, <a href='undocumented#SkClipOp'>SkClipOp</a> op = <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkCanvas_clipRegion_deviceRgn'>deviceRgn</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; pixels are fully contained by the clip.
<a href='#SkCanvas_clipRegion_deviceRgn'>deviceRgn</a> is unaffected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRegion_deviceRgn'><code><strong>deviceRgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> to combine with clip</td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRegion_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> to apply to clip</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7bb57c0e456c5fda2c2cca4abb68b19e"><div><a href='SkRegion_Reference#Region'>region</a> is unaffected by <a href='SkCanvas_Reference#Canvas'>canvas</a> rotation; iRect is affected by <a href='SkCanvas_Reference#Canvas'>canvas</a> rotation.
Both clips are <a href='undocumented#Alias'>Aliased</a>; this is not noticeable on <a href='SkRegion_Reference#Region'>Region</a> clip because it
aligns to <a href='undocumented#Pixel'>pixel</a> boundaries.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a>

<a name='SkCanvas_quickReject'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)const
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_quickReject_rect'>rect</a>, transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, can be quickly determined to be
outside of clip. May return false even though <a href='#SkCanvas_quickReject_rect'>rect</a> is outside of clip.

Use to check if an area to be drawn is clipped out, to skip subsequent draw calls.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_quickReject_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to compare with clip</td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_quickReject_rect'>rect</a>, transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, does not intersect clip

### Example

<div><fiddle-embed name="cfe4016241074477809dd45435be9cf4">

#### Example Output

~~~~
quickReject true
quickReject false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a> <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_drawsNothing'>drawsNothing</a>

<a name='SkCanvas_quickReject_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_quickReject'>quickReject</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>)const
</pre>

Returns true if <a href='#SkCanvas_quickReject_2_path'>path</a>, transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, can be quickly determined to be
outside of clip. May return false even though <a href='#SkCanvas_quickReject_2_path'>path</a> is outside of clip.

Use to check if an area to be drawn is clipped out, to skip subsequent draw calls.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_quickReject_2_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> to compare with clip</td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_quickReject_2_path'>path</a>, transformed by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, does not intersect clip

### Example

<div><fiddle-embed name="56dcd14f943aea6f7d7aafe0de7e6c25">

#### Example Output

~~~~
quickReject true
quickReject false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a> <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_drawsNothing'>drawsNothing</a>

<a name='SkCanvas_getLocalClipBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>()const
</pre>

Returns bounds of clip, transformed by inverse of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. If clip is empty,
return <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, where all <a href='SkRect_Reference#SkRect'>SkRect</a> sides equal zero.

<a href='SkRect_Reference#SkRect'>SkRect</a> returned is outset by one to account for partial <a href='undocumented#Pixel'>pixel</a> coverage if clip
is <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a>.

### Return Value

bounds of clip in local coordinates

### Example

<div><fiddle-embed name="7f60cb030d3f9b2473adbe3e34b19d91"><div>Initial bounds is <a href='undocumented#Device'>device</a> bounds outset by 1 on all sides.
Clipped bounds is <a href='#SkCanvas_clipPath'>clipPath</a> bounds outset by 1 on all sides.
Scaling the <a href='SkCanvas_Reference#Canvas'>canvas</a> by two on both axes scales the local bounds by 1/2
on both axes.
</div>

#### Example Output

~~~~
left:-1  top:-1  right:257  bottom:257
left:29  top:129  right:121  bottom:231
left:14.5  top:64.5  right:60.5  bottom:115.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a>

<a name='SkCanvas_getLocalClipBounds_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds)const
</pre>

Returns <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> of clip, transformed by inverse of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. If clip is empty,
return false, and set <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> to <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, where all <a href='SkRect_Reference#SkRect'>SkRect</a> sides equal zero.

<a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> is outset by one to account for partial <a href='undocumented#Pixel'>pixel</a> coverage if clip
is <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getLocalClipBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> of clip in local coordinates</td>
  </tr>
</table>

### Return Value

true if clip <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> is not empty

### Example

<div><fiddle-embed name="85496614e90c66b020f8a70db8d06f4a">

#### Example Output

~~~~
local bounds empty = false
local bounds empty = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a>

<a name='SkCanvas_getDeviceClipBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>()const
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> bounds of clip, unaffected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. If clip is empty,
return <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, where all <a href='SkRect_Reference#SkRect'>SkRect</a> sides equal zero.

Unlike <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(), returned <a href='SkIRect_Reference#SkIRect'>SkIRect</a> is not outset.

### Return Value

bounds of clip in <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> coordinates

### Example

<div><fiddle-embed name="556832ac5711af662a98c21c547185e9"><div>Initial bounds is <a href='undocumented#Device'>device</a> bounds, not outset.
Clipped bounds is <a href='#SkCanvas_clipPath'>clipPath</a> bounds, not outset.
Scaling the <a href='SkCanvas_Reference#Canvas'>canvas</a> by 1/2 on both axes scales the <a href='undocumented#Device'>device</a> bounds by 1/2
on both axes.
</div>

#### Example Output

~~~~
left:0  top:0  right:256  bottom:256
left:30  top:130  right:120  bottom:230
left:15  top:65  right:60  bottom:115
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a>

<a name='SkCanvas_getDeviceClipBounds_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* bounds)const
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> of clip, unaffected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. If clip is empty,
return false, and set <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> to <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, where all <a href='SkRect_Reference#SkRect'>SkRect</a> sides equal zero.

Unlike <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(), <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> is not outset.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getDeviceClipBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> of clip in <a href='undocumented#Device'>device</a> coordinates</td>
  </tr>
</table>

### Return Value

true if clip <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> is not empty

### Example

<div><fiddle-embed name="6abb99f849a1f0e33e1dedc00d1c4f7a">

#### Example Output

~~~~
device bounds empty = false
device bounds empty = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a> <a href='#SkCanvas_quickReject'>quickReject</a>

<a name='Draw'></a>

<a name='SkCanvas_drawColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawColor'>drawColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode = <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>)
</pre>

Fills clip with  <a href='#SkCanvas_drawColor_color'>color color</a>.
<a href='#SkCanvas_drawColor_mode'>mode</a> determines how ARGB is combined with destination.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawColor_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> ARGB</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawColor_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> used to combine source <a href='#SkCanvas_drawColor_color'>color</a> and destination</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9cf94fead1e6b17d836c704b4eac269a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clear'>clear</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a> <a href='#SkCanvas_drawPaint'>drawPaint</a>

<a name='SkCanvas_clear'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void clear(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>)
</pre>

Fills clip with  <a href='#SkCanvas_clear_color'>color color</a> using <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrc'>kSrc</a>.
This has the effect of replacing all pixels contained by clip with <a href='#SkCanvas_clear_color'>color</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clear_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> ARGB</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8c4499e322f10153dcd9b0b9806233b9"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawColor'>drawColor</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a> <a href='#SkCanvas_drawPaint'>drawPaint</a>

<a name='SkCanvas_discard'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_discard'>discard()</a>
</pre>

Makes <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> contents undefined. Subsequent calls that read <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> pixels,
such as drawing with <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, return undefined results. <a href='#SkCanvas_discard'>discard()</a> does
not change clip or <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

<a href='#SkCanvas_discard'>discard()</a> may do nothing, depending on the implementation of <a href='SkSurface_Reference#SkSurface'>SkSurface</a> or <a href='undocumented#SkBaseDevice'>SkBaseDevice</a>
that created <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

<a href='#SkCanvas_discard'>discard()</a> allows optimized performance on subsequent draws by removing
cached <a href='undocumented#Data'>data</a> associated with <a href='SkSurface_Reference#SkSurface'>SkSurface</a> or <a href='undocumented#SkBaseDevice'>SkBaseDevice</a>.
It is not necessary to call <a href='#SkCanvas_discard'>discard()</a> once done with <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>;
any cached <a href='undocumented#Data'>data</a> is deleted when owning <a href='SkSurface_Reference#SkSurface'>SkSurface</a> or <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> is deleted.

### See Also

<a href='#SkCanvas_flush'>flush()</a> <a href='undocumented#GrContext'>GrContext</a>::<a href='#GrContext_abandonContext'>abandonContext</a>

<a name='SkCanvas_drawPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPaint'>drawPaint</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Fills clip with <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPaint_paint'>paint</a>. <a href='SkPaint_Reference#SkPaint'>SkPaint</a> components <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> affect drawing;
<a href='undocumented#SkPathEffect'>SkPathEffect</a> in <a href='#SkCanvas_drawPaint_paint'>paint</a> is ignored.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPaint_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state used to fill <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1cd076b9b1a7c976cdca72b93c4f42dd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clear'>clear</a> <a href='#SkCanvas_drawColor'>drawColor</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a>

<a name='SkCanvas_PointMode'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_PointMode'>PointMode</a> {
        <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>,
        <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>,
        <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>,
    };

</pre>

Selects if an array of <a href='SkPoint_Reference#Point'>points</a> are drawn as discrete <a href='SkPoint_Reference#Point'>points</a>, as <a href='undocumented#Line'>lines</a>, or as
an open polygon.

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

<div><fiddle-embed name="292b4b2008961b6f612434d3121fc4ce"><div>The upper left corner shows three squares when drawn as <a href='SkPoint_Reference#Point'>points</a>.
The upper right corner shows one <a href='undocumented#Line'>line</a>; when drawn as <a href='undocumented#Line'>lines</a>, two <a href='SkPoint_Reference#Point'>points</a> are required per <a href='undocumented#Line'>line</a>.
The lower right corner shows two <a href='undocumented#Line'>lines</a>; when draw as polygon, no miter is drawn at the corner.
The lower left corner shows two <a href='undocumented#Line'>lines</a> with a miter when <a href='SkPath_Reference#Path'>path</a> contains polygon.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoints'>drawPoints</a>(<a href='#SkCanvas_PointMode'>PointMode</a> mode, size_t count, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawPoints_pts'>pts</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPoints_paint'>paint</a>.
<a href='#SkCanvas_drawPoints_count'>count</a> is the number of <a href='SkPoint_Reference#Point'>points</a>; if <a href='#SkCanvas_drawPoints_count'>count</a> is less than one, has no effect.
<a href='#SkCanvas_drawPoints_mode'>mode</a> may be one of: <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>, <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>, or <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>.

If <a href='#SkCanvas_drawPoints_mode'>mode</a> is <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>, the shape of <a href='SkPoint_Reference#Point'>point</a> drawn depends on <a href='#SkCanvas_drawPoints_paint'>paint</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a>. If <a href='#SkCanvas_drawPoints_paint'>paint</a> is set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, each <a href='SkPoint_Reference#Point'>point</a> draws a
<a href='undocumented#Circle'>circle</a> of diameter <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width. If <a href='#SkCanvas_drawPoints_paint'>paint</a> is set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>
or <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kButt_Cap'>kButt_Cap</a>, each <a href='SkPoint_Reference#Point'>point</a> draws a square of width and height
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width.

If <a href='#SkCanvas_drawPoints_mode'>mode</a> is <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>, each pair of <a href='SkPoint_Reference#Point'>points</a> draws a <a href='undocumented#Line'>line</a> segment.
One <a href='undocumented#Line'>line</a> is drawn for every two <a href='SkPoint_Reference#Point'>points</a>; each <a href='SkPoint_Reference#Point'>point</a> is used once. If <a href='#SkCanvas_drawPoints_count'>count</a> is odd,
the final <a href='SkPoint_Reference#Point'>point</a> is ignored.

If <a href='#SkCanvas_drawPoints_mode'>mode</a> is <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>, each adjacent pair of <a href='SkPoint_Reference#Point'>points</a> draws a <a href='undocumented#Line'>line</a> segment.
<a href='#SkCanvas_drawPoints_count'>count</a> minus one <a href='undocumented#Line'>lines</a> are drawn; the first and last <a href='SkPoint_Reference#Point'>point</a> are used once.

Each <a href='undocumented#Line'>line</a> segment respects <a href='#SkCanvas_drawPoints_paint'>paint</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> is ignored, as if were set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

Always draws each element one at a time; is not affected by
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a>, and unlike <a href='#SkCanvas_drawPath'>drawPath</a>(), does not create a mask from all <a href='SkPoint_Reference#Point'>points</a>
and <a href='undocumented#Line'>lines</a> before drawing.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoints_mode'><code><strong>mode</strong></code></a></td>
    <td>whether <a href='#SkCanvas_drawPoints_pts'>pts</a> draws <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Line'>lines</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#Point'>points</a> in the array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_pts'><code><strong>pts</strong></code></a></td>
    <td>array of <a href='SkPoint_Reference#Point'>points</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="635d54b4716e226e93dfbc21ad40e77d"><div>

<table>  <tr>
    <td>The first column draws <a href='SkPoint_Reference#Point'>points</a>.</td>
  </tr>  <tr>
    <td>The second column draws <a href='SkPoint_Reference#Point'>points</a> as <a href='undocumented#Line'>lines</a>.</td>
  </tr>  <tr>
    <td>The third column draws <a href='SkPoint_Reference#Point'>points</a> as a polygon.</td>
  </tr>  <tr>
    <td>The fourth column draws <a href='SkPoint_Reference#Point'>points</a> as a polygonal <a href='SkPath_Reference#Path'>path</a>.</td>
  </tr>  <tr>
    <td>The first row uses a round cap and round join.</td>
  </tr>  <tr>
    <td>The second row uses a square cap and a miter join.</td>
  </tr>  <tr>
    <td>The third row uses a butt cap and a bevel join.</td>
  </tr>
</table>

The transparent <a href='SkColor_Reference#Color'>color</a> makes multiple <a href='undocumented#Line'>line</a> draws visible;
the <a href='SkPath_Reference#Path'>path</a> is drawn all at once.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPoint_Reference#Point'>point</a> at (<a href='#SkCanvas_drawPoint_x'>x</a>, <a href='#SkCanvas_drawPoint_y'>y</a>) using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPoint_paint'>paint</a>.

The shape of <a href='SkPoint_Reference#Point'>point</a> drawn depends on <a href='#SkCanvas_drawPoint_paint'>paint</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a>.
If <a href='#SkCanvas_drawPoint_paint'>paint</a> is set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, draw a <a href='undocumented#Circle'>circle</a> of diameter
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width. If <a href='#SkCanvas_drawPoint_paint'>paint</a> is set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> or <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
draw a square of width and height <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> is ignored, as if were set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoint_x'><code><strong>x</strong></code></a></td>
    <td>left edge of <a href='undocumented#Circle'>circle</a> or square</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_y'><code><strong>y</strong></code></a></td>
    <td>top edge of <a href='undocumented#Circle'>circle</a> or square</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3476b553e7b547b604a3f6969f02d933"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoints'>drawPoints</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoint_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPoint_Reference#Point'>point</a> <a href='#SkCanvas_drawPoint_2_p'>p</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPoint_2_paint'>paint</a>.

The shape of <a href='SkPoint_Reference#Point'>point</a> drawn depends on <a href='#SkCanvas_drawPoint_2_paint'>paint</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a>.
If <a href='#SkCanvas_drawPoint_2_paint'>paint</a> is set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, draw a <a href='undocumented#Circle'>circle</a> of diameter
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width. If <a href='#SkCanvas_drawPoint_2_paint'>paint</a> is set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> or <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
draw a square of width and height <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> is ignored, as if were set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoint_2_p'><code><strong>p</strong></code></a></td>
    <td>top-left edge of <a href='undocumented#Circle'>circle</a> or square</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_2_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1a0a839061c69d870acca2bcfbdf1a41"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoints'>drawPoints</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawLine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='undocumented#SkScalar'>SkScalar</a> x0, <a href='undocumented#SkScalar'>SkScalar</a> y0, <a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Line'>line</a> segment from (<a href='#SkCanvas_drawLine_x0'>x0</a>, <a href='#SkCanvas_drawLine_y0'>y0</a>) to (<a href='#SkCanvas_drawLine_x1'>x1</a>, <a href='#SkCanvas_drawLine_y1'>y1</a>) using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawLine_paint'>paint</a>.
In <a href='#SkCanvas_drawLine_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> draws the end rounded or square;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> is ignored, as if were set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawLine_x0'><code><strong>x0</strong></code></a></td>
    <td>start of <a href='undocumented#Line'>line</a> segment on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_y0'><code><strong>y0</strong></code></a></td>
    <td>start of <a href='undocumented#Line'>line</a> segment on y-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_x1'><code><strong>x1</strong></code></a></td>
    <td>end of <a href='undocumented#Line'>line</a> segment on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_y1'><code><strong>y1</strong></code></a></td>
    <td>end of <a href='undocumented#Line'>line</a> segment on y-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d10ee4a265f278d02afe11ad889b293b"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawLine_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> p0, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p1, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Line'>line</a> segment from <a href='#SkCanvas_drawLine_2_p0'>p0</a> to <a href='#SkCanvas_drawLine_2_p1'>p1</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawLine_2_paint'>paint</a>.
In <a href='#SkCanvas_drawLine_2_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> draws the end rounded or square;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> is ignored, as if were set to <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawLine_2_p0'><code><strong>p0</strong></code></a></td>
    <td>start of <a href='undocumented#Line'>line</a> segment</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_2_p1'><code><strong>p1</strong></code></a></td>
    <td>end of <a href='undocumented#Line'>line</a> segment</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_2_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f8525816cb596dde1a3855446792c8e0"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRect'>drawRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawRect_rect'>rect</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRect_paint'>paint</a>.
In <a href='#SkCanvas_drawRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if rectangle is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness, and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> draws the corners rounded or square.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRect_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="871b0da9b4a23de11ae7a772ce14aed3"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawIRect'>drawIRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRegion'>drawRegion</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawIRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawIRect'>drawIRect</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawIRect_rect'>rect</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawIRect_paint'>paint</a>.
In <a href='#SkCanvas_drawIRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if rectangle is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness, and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> draws the corners rounded or square.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawIRect_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawIRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d3d8ca584134560750b1efa4a4c6e138"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRegion'>drawRegion</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRegion'>drawRegion</a>(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkCanvas_drawRegion_region'>region</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRegion_paint'>paint</a>.
In <a href='#SkCanvas_drawRegion_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if rectangle is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness, and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> draws the corners rounded or square.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRegion_region'><code><strong>region</strong></code></a></td>
    <td><a href='#SkCanvas_drawRegion_region'>region</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRegion_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="80309e0deca0f8add616cec7bec634ca"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawIRect'>drawIRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawOval'>drawOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawOval_oval'>oval</a> <a href='#SkCanvas_drawOval_oval'>oval</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a>.
In <a href='#SkCanvas_drawOval_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='#SkCanvas_drawOval_oval'>oval</a> is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawOval_oval'><code><strong>oval</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds of <a href='#SkCanvas_drawOval_oval'>oval</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawOval_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8b6b86f8a022811cd29a9c6ab771df12"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>

<a name='SkCanvas_drawRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRRect'>drawRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawRRect_rrect'>rrect</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRRect_paint'>paint</a>.
In <a href='#SkCanvas_drawRRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='#SkCanvas_drawRRect_rrect'>rrect</a> is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness.

<a href='#SkCanvas_drawRRect_rrect'>rrect</a> may represent a rectangle, <a href='undocumented#Circle'>circle</a>, <a href='undocumented#Oval'>oval</a>, uniformly rounded rectangle, or
may have any combination of positive non-square radii for the four corners.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> with up to eight corner radii to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="90fed1bb11efb43aada94113338c63d8"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawDRRect'>drawDRRect</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawDRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDRRect'>drawDRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& outer, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& inner, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawDRRect_outer'>outer</a> and <a href='#SkCanvas_drawDRRect_inner'>inner</a>
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawDRRect_paint'>paint</a>.
<a href='#SkCanvas_drawDRRect_outer'>outer</a> must contain <a href='#SkCanvas_drawDRRect_inner'>inner</a> or the drawing is undefined.
In <a href='#SkCanvas_drawDRRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness.
If stroked and <a href='SkRRect_Reference#SkRRect'>SkRRect</a> corner has  zero length radii, <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> can
draw corners rounded or square.

GPU-backed platforms optimize drawing when both <a href='#SkCanvas_drawDRRect_outer'>outer</a> and <a href='#SkCanvas_drawDRRect_inner'>inner</a> are
concave and <a href='#SkCanvas_drawDRRect_outer'>outer</a> contains <a href='#SkCanvas_drawDRRect_inner'>inner</a>. These platforms may not be able to draw
<a href='SkPath_Reference#SkPath'>SkPath</a> built with identical <a href='undocumented#Data'>data</a> as fast.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDRRect_outer'><code><strong>outer</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawDRRect_outer'>outer</a> bounds to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDRRect_inner'><code><strong>inner</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a> bounds to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDRRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="02e33141f13da2f19aef7feb7117b541"></fiddle-embed></div>

### Example

<div><fiddle-embed name="30823cb4edf884d330285ea161664931"><div>Outer <a href='SkRect_Reference#Rect'>Rect</a> has no corner radii, but stroke join is rounded.
Inner <a href='#RRect'>Round_Rect</a> has corner radii; outset stroke increases radii of corners.
Stroke join does not affect <a href='#SkCanvas_drawDRRect_inner'>inner</a> <a href='#RRect'>Round_Rect</a> since it has no sharp corners.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawCircle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> cx, <a href='undocumented#SkScalar'>SkScalar</a> cy, <a href='undocumented#SkScalar'>SkScalar</a> radius, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Circle'>circle</a> at (<a href='#SkCanvas_drawCircle_cx'>cx</a>, <a href='#SkCanvas_drawCircle_cy'>cy</a>) with <a href='#SkCanvas_drawCircle_radius'>radius</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawCircle_paint'>paint</a>.
If <a href='#SkCanvas_drawCircle_radius'>radius</a> is zero or less, nothing is drawn.
In <a href='#SkCanvas_drawCircle_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='undocumented#Circle'>circle</a> is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawCircle_cx'><code><strong>cx</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> center on the x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_cy'><code><strong>cy</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> center on the y-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_radius'><code><strong>radius</strong></code></a></td>
    <td>half the diameter of <a href='undocumented#Circle'>circle</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="841229e25ca9dfb68bd0dc4dfff356eb"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawCircle_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> center, <a href='undocumented#SkScalar'>SkScalar</a> radius, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Circle'>circle</a> at <a href='#SkCanvas_drawCircle_2_center'>center</a> with <a href='#SkCanvas_drawCircle_2_radius'>radius</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawCircle_2_paint'>paint</a>.
If <a href='#SkCanvas_drawCircle_2_radius'>radius</a> is zero or less, nothing is drawn.
In <a href='#SkCanvas_drawCircle_2_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='undocumented#Circle'>circle</a> is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawCircle_2_center'><code><strong>center</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> <a href='#SkCanvas_drawCircle_2_center'>center</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_2_radius'><code><strong>radius</strong></code></a></td>
    <td>half the diameter of <a href='undocumented#Circle'>circle</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9303ffae45ddd0b0a1f93d816a1762f4"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawArc'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawArc'>drawArc</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle, bool useCenter,
             const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Arc'>arc</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawArc_paint'>paint</a>.

<a href='undocumented#Arc'>Arc</a> is part of <a href='#SkCanvas_drawArc_oval'>oval</a> bounded by <a href='#SkCanvas_drawArc_oval'>oval</a>, sweeping from <a href='#SkCanvas_drawArc_startAngle'>startAngle</a> to <a href='#SkCanvas_drawArc_startAngle'>startAngle</a> plus
<a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a>. <a href='#SkCanvas_drawArc_startAngle'>startAngle</a> and <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> are in degrees.

<a href='#SkCanvas_drawArc_startAngle'>startAngle</a> of zero places start <a href='SkPoint_Reference#Point'>point</a> at the right middle edge of <a href='#SkCanvas_drawArc_oval'>oval</a>.
A positive <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> places <a href='undocumented#Arc'>arc</a> end <a href='SkPoint_Reference#Point'>point</a> clockwise from start <a href='SkPoint_Reference#Point'>point</a>;
a negative <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> places <a href='undocumented#Arc'>arc</a> end <a href='SkPoint_Reference#Point'>point</a> counterclockwise from start <a href='SkPoint_Reference#Point'>point</a>.
<a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> may exceed 360 degrees, a full <a href='undocumented#Circle'>circle</a>.
If <a href='#SkCanvas_drawArc_useCenter'>useCenter</a> is true, draw a wedge that includes <a href='undocumented#Line'>lines</a> from <a href='#SkCanvas_drawArc_oval'>oval</a>
center to <a href='undocumented#Arc'>arc</a> end <a href='SkPoint_Reference#Point'>points</a>. If <a href='#SkCanvas_drawArc_useCenter'>useCenter</a> is false, draw <a href='undocumented#Arc'>arc</a> between end <a href='SkPoint_Reference#Point'>points</a>.

If <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawArc_oval'>oval</a> is empty or <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> is zero, nothing is drawn.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawArc_oval'><code><strong>oval</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds of <a href='#SkCanvas_drawArc_oval'>oval</a> containing <a href='undocumented#Arc'>arc</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>angle in degrees where <a href='undocumented#Arc'>arc</a> begins</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep angle in degrees; positive is clockwise</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_useCenter'><code><strong>useCenter</strong></code></a></td>
    <td>if true, include the center of the <a href='#SkCanvas_drawArc_oval'>oval</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="11f0fbe7b30d776913c2e7c92c02ff57"></fiddle-embed></div>

### Example

<div><fiddle-embed name="e91dbe45974489b8962c815017b7914f"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawRoundRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRRect_Reference#SkRRect'>SkRRect</a> bounded by <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawRoundRect_rect'>rect</a>, with corner radii (<a href='#SkCanvas_drawRoundRect_rx'>rx</a>, <a href='#SkCanvas_drawRoundRect_ry'>ry</a>) using clip,
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRoundRect_paint'>paint</a>.

In <a href='#SkCanvas_drawRoundRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> is stroked or filled;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness.
If <a href='#SkCanvas_drawRoundRect_rx'>rx</a> or <a href='#SkCanvas_drawRoundRect_ry'>ry</a> are less than zero, they are treated as if they are zero.
If <a href='#SkCanvas_drawRoundRect_rx'>rx</a> plus <a href='#SkCanvas_drawRoundRect_ry'>ry</a> exceeds <a href='#SkCanvas_drawRoundRect_rect'>rect</a> width or <a href='#SkCanvas_drawRoundRect_rect'>rect</a> height, radii are scaled down to fit.
If <a href='#SkCanvas_drawRoundRect_rx'>rx</a> and <a href='#SkCanvas_drawRoundRect_ry'>ry</a> are zero, <a href='SkRRect_Reference#SkRRect'>SkRRect</a> is drawn as <a href='SkRect_Reference#SkRect'>SkRect</a> and if stroked is affected by
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRoundRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds of <a href='SkRRect_Reference#SkRRect'>SkRRect</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_rx'><code><strong>rx</strong></code></a></td>
    <td>axis length on x-axis of <a href='undocumented#Oval'>oval</a> describing rounded corners</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_ry'><code><strong>ry</strong></code></a></td>
    <td>axis length on y-axis of <a href='undocumented#Oval'>oval</a> describing rounded corners</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="199fe818c09026c114e165bff166a39f"><div>Top row has a zero radius a generates a rectangle.
Second row radii sum to less than sides.
Third row radii sum equals sides.
Fourth row radii sum exceeds sides; radii are scaled to fit.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawDRRect'>drawDRRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPoint'>drawPoint</a>

<a name='SkCanvas_drawPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPath'>drawPath</a>(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkCanvas_drawPath_path'>path</a> using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPath_paint'>paint</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> contains an array of  <a href='SkPath_Overview#Contour'>path contour</a>, each of which may be open or closed.

In <a href='#SkCanvas_drawPath_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> determines if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> is stroked or filled:
if filled, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> determines whether  <a href='SkPath_Overview#Contour'>path contour</a> describes inside or
outside of fill; if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> stroke width describes the <a href='undocumented#Line'>line</a> thickness,
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> describes <a href='undocumented#Line'>line</a> ends, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> describes how
corners are drawn.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPath_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="fe2294131f422b8d6752f6a880f98ad9"><div>Top rows draw stroked <a href='#SkCanvas_drawPath_path'>path</a> with combinations of joins and caps. The open <a href='SkPath_Overview#Contour'>contour</a>
is affected by caps; the closed <a href='SkPath_Overview#Contour'>contour</a> is affected by joins.
Bottom row draws fill the same for open and closed <a href='SkPath_Overview#Contour'>contour</a>.
First bottom column shows winding fills overlap.
Second bottom column shows even odd fills exclude overlap.
Third bottom column shows inverse winding fills area outside both <a href='SkPath_Overview#Contour'>contours</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPoints'>drawPoints</a>

<a name='Draw_Image'></a>

<a href='#SkCanvas_drawImage'>drawImage</a>, <a href='#SkCanvas_drawImageRect'>drawImageRect</a>, and <a href='#SkCanvas_drawImageNine'>drawImageNine</a> can be called with a bare pointer or
a  <a href='undocumented#Smart_Pointer'>smart pointer</a> as a convenience. The pairs of calls are otherwise identical.

<a name='SkCanvas_drawImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImage_image'>image</a>, with its top-left corner at (<a href='#SkCanvas_drawImage_left'>left</a>, <a href='#SkCanvas_drawImage_top'>top</a>),
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImage_paint'>paint</a>.

If <a href='#SkCanvas_drawImage_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>,
and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImage_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImage_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImage_image'>image</a> bounds. If generated
mask extends beyond <a href='#SkCanvas_drawImage_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImage_image'>image</a> edge colors, just as <a href='undocumented#SkShader'>SkShader</a>
made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the
<a href='#SkCanvas_drawImage_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImage_image'><code><strong>image</strong></code></a></td>
    <td>uncompressed rectangular map of pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_left'>left side</a> of <a href='#SkCanvas_drawImage_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_top'><code><strong>top</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_top'>top</a> side of <a href='#SkCanvas_drawImage_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Example

<div><fiddle-embed name="185746dc0faa6f1df30c4afe098646ff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setImageFilter'>setImageFilter</a>

<a name='SkCanvas_drawImage_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImage'>drawImage</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top,
               const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImage_2_image'>image</a>, with its top-left corner at (<a href='#SkCanvas_drawImage_2_left'>left</a>, <a href='#SkCanvas_drawImage_2_top'>top</a>),
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImage_2_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImage_2_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImage_2_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImage_2_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImage_2_image'>image</a> bounds. If generated
mask extends beyond <a href='#SkCanvas_drawImage_2_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImage_2_image'>image</a> edge colors, just as <a href='undocumented#SkShader'>SkShader</a>
made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the
<a href='#SkCanvas_drawImage_2_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImage_2_image'><code><strong>image</strong></code></a></td>
    <td>uncompressed rectangular map of pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_2_left'>left side</a> of <a href='#SkCanvas_drawImage_2_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_top'><code><strong>top</strong></code></a></td>
    <td>pop side of <a href='#SkCanvas_drawImage_2_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Example

<div><fiddle-embed name="a4e877e891b1be5faa2b7fd07f673a10"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setImageFilter'>setImageFilter</a>

<a name='SkCanvas_SrcRectConstraint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> {
        <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>,
        <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>,
    };

</pre>

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> controls the behavior at the edge of source <a href='SkRect_Reference#Rect'>Rect</a>,
provided to <a href='#SkCanvas_drawImageRect'>drawImageRect</a>, trading off speed for precision.

<a href='#Image_Filter'>Image_Filter</a> in <a href='SkPaint_Reference#Paint'>Paint</a> may sample multiple pixels in the <a href='SkImage_Reference#Image'>image</a>. Source <a href='SkRect_Reference#Rect'>Rect</a>
restricts the bounds of pixels that may be read. <a href='#Image_Filter'>Image_Filter</a> may slow down if
it cannot read outside the bounds, when sampling near the edge of source <a href='SkRect_Reference#Rect'>Rect</a>.
<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> specifies whether an <a href='#Image_Filter'>Image_Filter</a> is allowed to read pixels
outside source <a href='SkRect_Reference#Rect'>Rect</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kStrict_SrcRectConstraint'><code>SkCanvas::kStrict_SrcRectConstraint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Requires <a href='#Image_Filter'>Image_Filter</a> to respect source <a href='SkRect_Reference#Rect'>Rect</a>,
sampling only inside of its bounds, possibly with a performance penalty.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kFast_SrcRectConstraint'><code>SkCanvas::kFast_SrcRectConstraint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Permits <a href='#Image_Filter'>Image_Filter</a> to sample outside of source <a href='SkRect_Reference#Rect'>Rect</a>
by half the width of <a href='#Image_Filter'>Image_Filter</a>, permitting it to run faster but with
error at the <a href='SkImage_Reference#Image'>image</a> edges.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5df49d1f4da37275a1f10ef7f1a749f0"><div>redBorder contains a black and white checkerboard bordered by red.
redBorder is drawn scaled by 16 on the left.
The middle and right <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> are filtered checkerboards.
Drawing the checkerboard with <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> shows only a blur of black and white.
Drawing the checkerboard with <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows red to bleed in the corners.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setImageFilter'>setImageFilter</a>

<a name='SkCanvas_drawImageRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                   <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_src'>src</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_image'>image</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_dst'>dst</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImageRect_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImageRect_image'>image</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageRect_image'>image</a> edge colors, just
as <a href='undocumented#SkShader'>SkShader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageRect_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

<a href='#SkCanvas_drawImageRect_constraint'>constraint</a> set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawImageRect_src'>src</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_src'>src</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bfd18e9cac896cdf94c9f154ccf94be8"><div>The left <a href='SkBitmap_Reference#Bitmap'>bitmap</a> draws with <a href='SkPaint_Reference#Paint'>Paint</a> default <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, and stays within
its bounds; there is no bleeding with <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>.
the middle and right <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> draw with <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>; with
<a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>, the filter remains within the checkerboard, and
with <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> red bleeds on the edges.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                   <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_2_dst'>dst</a>.
Note that <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> is on integer <a href='undocumented#Pixel'>pixel</a> boundaries; <a href='#SkCanvas_drawImageRect_2_dst'>dst</a> may include fractional
boundaries. Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a>
<a href='#SkCanvas_drawImageRect_2_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_2_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImageRect_2_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_2_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImageRect_2_image'>image</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_2_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageRect_2_image'>image</a> edge colors, just
as <a href='undocumented#SkShader'>SkShader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageRect_2_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

<a href='#SkCanvas_drawImageRect_2_constraint'>constraint</a> set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_2_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#SkIRect'>SkIRect</a> of <a href='#SkCanvas_drawImageRect_2_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_2_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_2_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f92cd5c9b9f4b1ac3cd933b08037bfe"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_3_dst'>dst</a>, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>,
and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_3_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_3_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImageRect_3_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_3_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImageRect_3_image'>image</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_3_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageRect_3_image'>image</a> edge colors, just
as <a href='undocumented#SkShader'>SkShader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageRect_3_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_3_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_3_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Example

<div><fiddle-embed name="3cf8fb639fef99993cafc064d550c739"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_4_src'>src</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_4_dst'>dst</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_4_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_4_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImageRect_4_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_4_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImageRect_4_image'>image</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_4_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageRect_4_image'>image</a> edge colors, just
as <a href='undocumented#SkShader'>SkShader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageRect_4_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_4_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_4_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_4_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_4_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_4_src'>src</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d4b35a9d24c32c042bd1f529b8de3c0d"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> scales and translates; transformation from <a href='#SkCanvas_drawImageRect_4_src'>src</a> to <a href='#SkCanvas_drawImageRect_4_dst'>dst</a> also scales.
The two <a href='SkMatrix_Reference#Matrix'>matrices</a> are concatenated to create the final transformation.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_5'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawImageRect_5_isrc'>isrc</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_5_dst'>dst</a>.
<a href='#SkCanvas_drawImageRect_5_isrc'>isrc</a> is on integer <a href='undocumented#Pixel'>pixel</a> boundaries; <a href='#SkCanvas_drawImageRect_5_dst'>dst</a> may include fractional boundaries.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_5_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_5_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImageRect_5_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_5_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImageRect_5_image'>image</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_5_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageRect_5_image'>image</a> edge colors, just
as <a href='undocumented#SkShader'>SkShader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageRect_5_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

<a href='#SkCanvas_drawImageRect_5_constraint'>constraint</a> set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawImageRect_5_image'>image</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_5_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#SkIRect'>SkIRect</a> of <a href='#SkCanvas_drawImageRect_5_image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_5_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_5_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_5_image'>image</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d307e7e1237f39fb54d80723e5449857"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_6'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_6_dst'>dst</a>,
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_6_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_6_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawImageRect_6_image'>image</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_6_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawImageRect_6_image'>image</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_6_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageRect_6_image'>image</a> edge colors, just
as <a href='undocumented#SkShader'>SkShader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageRect_6_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

constraint set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawImageRect_6_image'>image</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_6_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='#SkCanvas_drawImageRect_6_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Example

<div><fiddle-embed name="3a47ef94cb70144455f80333d8653e6c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageNine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageNine_image'>image</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageNine_dst'>dst</a>.
<a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawImageNine_center'>center</a> divides the <a href='#SkCanvas_drawImageNine_image'>image</a> into nine sections: four sides, four corners, and
the <a href='#SkCanvas_drawImageNine_center'>center</a>. Corners are unmodified or scaled down proportionately if their sides
are larger than <a href='#SkCanvas_drawImageNine_dst'>dst</a>; <a href='#SkCanvas_drawImageNine_center'>center</a> and four sides are scaled to fit remaining space, if any.

Additionally transform draw using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and optional <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> is supplied, apply <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, and <a href='#Draw_Looper'>Draw_Looper</a>. If image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#Shader'>Shader</a>.
If <a href='#SkCanvas_drawImageNine_paint'>paint</a> contains <a href='#Mask_Filter'>Mask_Filter</a>, generate mask from image bounds. If <a href='#SkCanvas_drawImageNine_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> set to <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, disable <a href='undocumented#Pixel'>pixel</a> filtering. For all
other values of <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, use <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> to filter pixels.
Any <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> on <a href='#SkCanvas_drawImageNine_paint'>paint</a> is ignored as is <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> state.
If generated mask extends beyond <a href='#SkCanvas_drawImageNine_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageNine_image'>image</a> edge colors, just
as <a href='undocumented#Shader'>Shader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageNine_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageNine_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> edge of <a href='#SkCanvas_drawImageNine_image'>image</a> corners and sides</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageNine_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
and so on; or nullptr
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4f153cf1d0dbe1a95acf5badeec14dae"><div>The leftmost <a href='#SkCanvas_drawImageNine_image'>image</a> is smaller than <a href='#SkCanvas_drawImageNine_center'>center</a>; only corners are drawn, all scaled to fit.
The second <a href='#SkCanvas_drawImageNine_image'>image</a> equals the <a href='undocumented#Size'>size</a> of <a href='#SkCanvas_drawImageNine_center'>center</a>; only corners are drawn without scaling.
The remaining images are larger than <a href='#SkCanvas_drawImageNine_center'>center</a>. All corners draw without scaling.
The sides and <a href='#SkCanvas_drawImageNine_center'>center</a> are scaled if needed to take up the remaining space.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>

<a name='SkCanvas_drawImageNine_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& <a href='SkImage_Reference#Image'>image</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                   const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageNine_2_dst'>dst</a>.
<a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawImageNine_2_center'>center</a> divides the <a href='#SkCanvas_drawImageNine_2_image'>image</a> into nine sections: four sides, four corners, and
the <a href='#SkCanvas_drawImageNine_2_center'>center</a>. Corners are not scaled, or scaled down proportionately if their sides
are larger than <a href='#SkCanvas_drawImageNine_2_dst'>dst</a>; <a href='#SkCanvas_drawImageNine_2_center'>center</a> and four sides are scaled to fit remaining space, if any.

Additionally transform draw using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and optional <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> is supplied, apply <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, and <a href='#Draw_Looper'>Draw_Looper</a>. If image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#Shader'>Shader</a>.
If <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> contains <a href='#Mask_Filter'>Mask_Filter</a>, generate mask from image bounds. If <a href='#SkCanvas_drawImageNine_2_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> set to <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, disable <a href='undocumented#Pixel'>pixel</a> filtering. For all
other values of <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, use <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> to filter pixels.
Any <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> on <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> is ignored as is <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> state.
If generated mask extends beyond <a href='#SkCanvas_drawImageNine_2_image'>image</a> bounds, replicate <a href='#SkCanvas_drawImageNine_2_image'>image</a> edge colors, just
as <a href='undocumented#Shader'>Shader</a> made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> with <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set
replicates the <a href='#SkCanvas_drawImageNine_2_image'>image</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageNine_2_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> edge of <a href='#SkCanvas_drawImageNine_2_image'>image</a> corners and sides</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageNine_2_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
and so on; or nullptr
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d597d9af8d17fd93e634dd12017058e2"><div>The two leftmost images has four corners and sides to the left and right of <a href='#SkCanvas_drawImageNine_2_center'>center</a>.
The leftmost <a href='#SkCanvas_drawImageNine_2_image'>image</a> scales the width of corners proportionately to fit.
The third and fourth <a href='#SkCanvas_drawImageNine_2_image'>image</a> corners are not scaled; the sides and <a href='#SkCanvas_drawImageNine_2_center'>center</a> are scaled to
fill the remaining space.
The rightmost <a href='#SkCanvas_drawImageNine_2_image'>image</a> has four corners scaled vertically to fit, and uses sides above
and below <a href='#SkCanvas_drawImageNine_2_center'>center</a> to fill the remaining space.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>

<a name='SkCanvas_drawBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmap'>drawBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a>, with its top-left corner at (<a href='#SkCanvas_drawBitmap_left'>left</a>, <a href='#SkCanvas_drawBitmap_top'>top</a>),
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmap_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmap_paint'>paint</a> is not nullptr, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmap_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> bounds, replicate <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#SkShader'>SkShader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawBitmap_left'>left side</a> of <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_top'><code><strong>top</strong></code></a></td>
    <td><a href='#SkCanvas_drawBitmap_top'>top</a> side of <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Example

<div><fiddle-embed name="4a521be1f850058541e136a808c65e78"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_writePixels'>writePixels</a>

<a name='SkCanvas_drawBitmapRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                    const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_src'>src</a> of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_dst'>dst</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmapRect_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> bounds, replicate <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#SkShader'>SkShader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

<a href='#SkCanvas_drawBitmapRect_constraint'>constraint</a> set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawBitmapRect_src'>src</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='SkImage_Reference#Image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='SkImage_Reference#Image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawBitmapRect_src'>src</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7d04932f2a259cc70d6e45cd25a6feb6"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

<a name='SkCanvas_drawBitmapRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& isrc, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                    const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a> of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_2_dst'>dst</a>.
<a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a> is on integer <a href='undocumented#Pixel'>pixel</a> boundaries; <a href='#SkCanvas_drawBitmapRect_2_dst'>dst</a> may include fractional boundaries.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmapRect_2_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> bounds, replicate <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#SkShader'>SkShader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

<a href='#SkCanvas_drawBitmapRect_2_constraint'>constraint</a> set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#SkIRect'>SkIRect</a> of <a href='SkImage_Reference#Image'>image</a> to draw from</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='SkImage_Reference#Image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_constraint'><code><strong>constraint</strong></code></a></td>
    <td>sample strictly within <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a>, or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0a3c6d2459566e58cee7d4910655ee21"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

<a name='SkCanvas_drawBitmapRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                    <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> constraint = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a>, scaled and translated to fill <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_3_dst'>dst</a>.
<a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> bounds is on integer <a href='undocumented#Pixel'>pixel</a> boundaries; <a href='#SkCanvas_drawBitmapRect_3_dst'>dst</a> may include fractional boundaries.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>paint</a> is supplied, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. If <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmapRect_3_paint'>paint</a> contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, generate mask from <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> bounds.

If generated mask extends beyond <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> bounds, replicate <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#SkShader'>SkShader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

<a href='#SkCanvas_drawBitmapRect_3_constraint'>constraint</a> set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> limits <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> to
sample within <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a>; set to <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> allows sampling outside to
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> of <a href='SkImage_Reference#Image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> or draw faster</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bdbeac3c97f60a63987b1cc8e1f1e91e"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

<a name='SkCanvas_drawBitmapNine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& center, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                    const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapNine_dst'>dst</a>.
<a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawBitmapNine_center'>center</a> divides the <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> into nine sections: four sides, four corners,
and the <a href='#SkCanvas_drawBitmapNine_center'>center</a>. Corners are not scaled, or scaled down proportionately if their
sides are larger than <a href='#SkCanvas_drawBitmapNine_dst'>dst</a>; <a href='#SkCanvas_drawBitmapNine_center'>center</a> and four sides are scaled to fit remaining
space, if any.

Additionally transform draw using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and optional <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> is supplied, apply <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, and <a href='#Draw_Looper'>Draw_Looper</a>. If bitmap is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#Shader'>Shader</a>.
If <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> contains <a href='#Mask_Filter'>Mask_Filter</a>, generate mask from bitmap bounds. If <a href='#SkCanvas_drawBitmapNine_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> set to <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, disable <a href='undocumented#Pixel'>pixel</a> filtering. For all
other values of <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, use <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> to filter pixels.
Any <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> on <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> is ignored as is <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> state.
If generated mask extends beyond <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> bounds, replicate <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#Shader'>Shader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapNine_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> edge of <a href='SkImage_Reference#Image'>image</a> corners and sides</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='SkImage_Reference#Image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
and so on; or nullptr
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e99e7be0d8f67dfacbecf85df585433d"><div>The two leftmost <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> draws has four corners and sides to the left and right of <a href='#SkCanvas_drawBitmapNine_center'>center</a>.
The leftmost  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> scales the width of corners proportionately to fit.
The third and fourth draw corners are not scaled; the sides and <a href='#SkCanvas_drawBitmapNine_center'>center</a> are scaled to
fill the remaining space.
The rightmost  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> has four corners scaled vertically to fit, and uses sides above
and below <a href='#SkCanvas_drawBitmapNine_center'>center</a> to fill the remaining space.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>

<a name='Draw_Image_Lattice'></a>

<a name='SkCanvas_Lattice'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
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

</pre>

<a href='#SkCanvas_Lattice'>Lattice</a> divides <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> or <a href='SkImage_Reference#Image'>Image</a> into a rectangular grid.
Grid entries on even columns and even rows are fixed; these entries are
always drawn at their original <a href='undocumented#Size'>size</a> if the destination is large enough.
If the destination side is too small to hold the fixed entries, all fixed
entries are proportionately scaled down to fit.
The grid entries not on even columns and rows are scaled to fit the
remaining space, if any.

<a name='SkCanvas_Lattice_RectType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        enum <a href='#SkCanvas_Lattice_RectType'>RectType</a> : uint8_t {
            <a href='#SkCanvas_Lattice_kDefault'>kDefault</a> = 0,
            <a href='#SkCanvas_Lattice_kTransparent'>kTransparent</a>,
            <a href='#SkCanvas_Lattice_kFixedColor'>kFixedColor</a>,
        };

</pre>

Optional setting per rectangular grid entry to make it transparent,
or to fill the grid entry with a <a href='SkColor_Reference#Color'>color</a>.

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

<a name='Draw_Image_Lattice_Members'></a><table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;int*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fXDivs'><code>fXDivs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Array of x-axis values that divide the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> vertically.
Array entries must be unique, increasing, greater than or equal to
<a href='#SkCanvas_Lattice_fBounds'>fBounds</a> left edge, and less than <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> right edge.
Set the first element to <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> left to collapse the left column of
fixed grid entries.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;int*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fYDivs'><code>fYDivs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Array of y-axis values that divide the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> horizontally.
Array entries must be unique, increasing, greater than or equal to
<a href='#SkCanvas_Lattice_fBounds'>fBounds</a> top edge, and less than <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> bottom edge.
Set the first element to <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> top to collapse the top row of fixed
grid entries.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;RectType*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fRectTypes'><code>fRectTypes</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional array of fill types, one per rectangular grid entry:
array length must be <code>(<a href='#SkCanvas_Lattice_fXCount'>fXCount</a> + 1) * (<a href='#SkCanvas_Lattice_fYCount'>fYCount</a> + 1)</code>.

Each <a href='#SkCanvas_Lattice_RectType'>RectType</a> is one of: <a href='#SkCanvas_Lattice_kDefault'>kDefault</a>, <a href='#SkCanvas_Lattice_kTransparent'>kTransparent</a>, <a href='#SkCanvas_Lattice_kFixedColor'>kFixedColor</a>.

Array entries correspond to the rectangular grid entries, ascending
left to right and then top to bottom.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fXCount'><code>fXCount</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Number of entries in <a href='#SkCanvas_Lattice_fXDivs'>fXDivs</a> array; one less than the number of
horizontal divisions.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fYCount'><code>fYCount</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Number of entries in <a href='#SkCanvas_Lattice_fYDivs'>fYDivs</a> array; one less than the number of vertical
divisions.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkIRect*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fBounds'><code>fBounds</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional subset <a href='SkIRect_Reference#IRect'>IRect</a> source to draw from.
If nullptr, source bounds is dimensions of <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> or <a href='SkImage_Reference#Image'>Image</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkColor*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fColors'><code>fColors</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional array of colors, one per rectangular grid entry.
Array length must be <code>(<a href='#SkCanvas_Lattice_fXCount'>fXCount</a> + 1) * (<a href='#SkCanvas_Lattice_fYCount'>fYCount</a> + 1)</code>.

Array entries correspond to the rectangular grid entries, ascending
left to right, then top to bottom.
</td>
  </tr>
</table>

<a name='SkCanvas_drawBitmapLattice'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, const <a href='#SkCanvas_Lattice'>Lattice</a>& lattice, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                       const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapLattice_dst'>dst</a>.

<a href='#SkCanvas_Lattice'>Lattice</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>lattice</a> divides <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> into a rectangular grid.
Each intersection of an even-numbered row and column is fixed; like the corners
of <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>, fixed <a href='#SkCanvas_drawBitmapLattice_lattice'>lattice</a> elements never scale larger than their initial
<a href='undocumented#Size'>size</a> and shrink proportionately when all fixed elements exceed the <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a>
dimension. All other grid elements scale to fill the available space, if any.

Additionally transform draw using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and optional <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> is supplied, apply <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, and <a href='#Draw_Looper'>Draw_Looper</a>. If bitmap is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#Shader'>Shader</a>.
If <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> contains <a href='#Mask_Filter'>Mask_Filter</a>, generate mask from bitmap bounds. If <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> set to <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, disable <a href='undocumented#Pixel'>pixel</a> filtering. For all
other values of <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, use <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> to filter pixels.
Any <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> on <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> is ignored as is <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> state.
If generated mask extends beyond <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> bounds, replicate <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#Shader'>Shader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapLattice_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_lattice'><code><strong>lattice</strong></code></a></td>
    <td>division of <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> into fixed and variable rectangles</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='SkImage_Reference#Image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
and so on; or nullptr
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c5bfa944e17ba4a4400dc799f032069c"><div>The two leftmost <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> draws has four corners and sides to the left and right of center.
The leftmost  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> scales the width of corners proportionately to fit.
The third and fourth draw corners are not scaled; the sides are scaled to
fill the remaining space; the center is transparent.
The rightmost  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> has four corners scaled vertically to fit, and uses sides above
and below center to fill the remaining space.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_Lattice'>Lattice</a>

<a name='SkCanvas_drawImageLattice'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, const <a href='#SkCanvas_Lattice'>Lattice</a>& lattice, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst,
                      const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr)
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageLattice_image'>image</a> stretched proportionally to fit into <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageLattice_dst'>dst</a>.

<a href='#SkCanvas_Lattice'>Lattice</a> <a href='#SkCanvas_drawImageLattice_lattice'>lattice</a> divides <a href='#SkCanvas_drawImageLattice_image'>image</a> into a rectangular grid.
Each intersection of an even-numbered row and column is fixed; like the corners
of <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>, fixed <a href='#SkCanvas_drawImageLattice_lattice'>lattice</a> elements never scale larger than their initial
<a href='undocumented#Size'>size</a> and shrink proportionately when all fixed elements exceed the <a href='SkBitmap_Reference#Bitmap'>bitmap</a>
dimension. All other grid elements scale to fill the available space, if any.

Additionally transform draw using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and optional <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> is supplied, apply <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, and <a href='#Draw_Looper'>Draw_Looper</a>. If image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, apply <a href='undocumented#Shader'>Shader</a>.
If <a href='#SkCanvas_drawImageLattice_paint'>paint</a> contains <a href='#Mask_Filter'>Mask_Filter</a>, generate mask from image bounds. If <a href='#SkCanvas_drawImageLattice_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> set to <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, disable <a href='undocumented#Pixel'>pixel</a> filtering. For all
other values of <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, use <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> to filter pixels.
Any <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> on <a href='#SkCanvas_drawImageLattice_paint'>paint</a> is ignored as is <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> state.
If generated mask extends beyond <a href='SkBitmap_Reference#Bitmap'>bitmap</a> bounds, replicate <a href='SkBitmap_Reference#Bitmap'>bitmap</a> edge colors,
just as <a href='undocumented#Shader'>Shader</a> made from <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> with
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> set replicates the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> edge <a href='SkColor_Reference#Color'>color</a> when it samples
outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageLattice_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> containing pixels, dimensions, and format</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_lattice'><code><strong>lattice</strong></code></a></td>
    <td>division of <a href='SkBitmap_Reference#Bitmap'>bitmap</a> into fixed and variable rectangles</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> of <a href='#SkCanvas_drawImageLattice_image'>image</a> to draw to</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
and so on; or nullptr
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4f153cf1d0dbe1a95acf5badeec14dae"><div>The leftmost <a href='#SkCanvas_drawImageLattice_image'>image</a> is smaller than center; only corners are drawn, all scaled to fit.
The second <a href='#SkCanvas_drawImageLattice_image'>image</a> equals the <a href='undocumented#Size'>size</a> of center; only corners are drawn without scaling.
The remaining images are larger than center. All corners draw without scaling. The sides
are scaled if needed to take up the remaining space; the center is transparent.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_Lattice'>Lattice</a>

<a name='Draw_Text'></a>

<a name='SkCanvas_drawText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawText'>drawText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawText_text'>text</a>, with origin at (<a href='#SkCanvas_drawText_x'>x</a>, <a href='#SkCanvas_drawText_y'>y</a>), using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawText_paint'>paint</a>.

<a href='#SkCanvas_drawText_text'>text</a> meaning depends on <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>; by default, <a href='#SkCanvas_drawText_text'>text</a> is encoded as
UTF-8.

<a href='#SkCanvas_drawText_x'>x</a> and <a href='#SkCanvas_drawText_y'>y</a> meaning depends on <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::Align and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> vertical <a href='#SkCanvas_drawText_text'>text</a>; by default
<a href='#SkCanvas_drawText_text'>text</a> draws left to right, positioning the first <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> at <a href='#SkCanvas_drawText_x'>x</a>
and its baseline at <a href='#SkCanvas_drawText_y'>y</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> is affected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawText_text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawText_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; apply to <a href='#SkCanvas_drawText_text'>text</a>. By default, draws
filled 12 <a href='SkPoint_Reference#Point'>point</a> black <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawText_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawText_text'>text</a> on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawText_text'>text</a> on y-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawText_text'>text</a> <a href='undocumented#Size'>size</a>, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="55f5e59350622c5e2834d1c85789f732"><div>The same <a href='#SkCanvas_drawText_text'>text</a> is drawn varying <a href='#Paint_Text_Size'>Paint_Text_Size</a> and varying
<a href='SkMatrix_Reference#Matrix'>Matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawString'>drawString</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawString'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawString'>drawString</a>(const char* <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws null terminated <a href='#SkCanvas_drawString_string'>string</a>, with origin at (<a href='#SkCanvas_drawString_x'>x</a>, <a href='#SkCanvas_drawString_y'>y</a>), using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawString_paint'>paint</a>.

<a href='#SkCanvas_drawString_string'>string</a> meaning depends on <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>; by default, strings are encoded
as UTF-8. Other values of <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> are unlikely to produce the desired
results, since zero bytes may be embedded in the <a href='#SkCanvas_drawString_string'>string</a>.

<a href='#SkCanvas_drawString_x'>x</a> and <a href='#SkCanvas_drawString_y'>y</a> meaning depends on <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::Align and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> vertical <a href='undocumented#Text'>text</a>; by default
<a href='#SkCanvas_drawString_string'>string</a> draws left to right, positioning the first <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> at <a href='#SkCanvas_drawString_x'>x</a>
and its baseline at <a href='#SkCanvas_drawString_y'>y</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> is affected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawString_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; apply to <a href='undocumented#Text'>text</a>. By default, draws
filled 12 <a href='SkPoint_Reference#Point'>point</a> black <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>glyphs</a> drawn,</td>
  </tr>
</table>

ending with a char value of zero

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_string'>string</a> on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_string'>string</a> on y-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="85442cf8d0bce6b5a777853bc36a4dc4"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawString_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawString'>drawString</a>(const <a href='undocumented#SkString'>SkString</a>& <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws null terminated <a href='#SkCanvas_drawString_2_string'>string</a>, with origin at (<a href='#SkCanvas_drawString_2_x'>x</a>, <a href='#SkCanvas_drawString_2_y'>y</a>), using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawString_2_paint'>paint</a>.

<a href='#SkCanvas_drawString_2_string'>string</a> meaning depends on <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>; by default, strings are encoded
as UTF-8. Other values of <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> are unlikely to produce the desired
results, since zero bytes may be embedded in the <a href='#SkCanvas_drawString_2_string'>string</a>.

<a href='#SkCanvas_drawString_2_x'>x</a> and <a href='#SkCanvas_drawString_2_y'>y</a> meaning depends on <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::Align and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> vertical <a href='undocumented#Text'>text</a>; by default
<a href='#SkCanvas_drawString_2_string'>string</a> draws left to right, positioning the first <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> at <a href='#SkCanvas_drawString_2_x'>x</a>
and its baseline at <a href='#SkCanvas_drawString_2_y'>y</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> is affected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawString_2_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; apply to <a href='undocumented#Text'>text</a>. By default, draws
filled 12 <a href='SkPoint_Reference#Point'>point</a> black <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_2_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>glyphs</a> drawn,</td>
  </tr>
</table>

ending with a char value of zero

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_2_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_2_string'>string</a> on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_2_string'>string</a> on y-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="435178c09feb3bfec5e35d983609a013"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawPosText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPosText'>drawPosText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pos[], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws each <a href='undocumented#Glyph'>glyph</a> in <a href='#SkCanvas_drawPosText_text'>text</a> with the origin in <a href='#SkCanvas_drawPosText_pos'>pos</a> array, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosText_paint'>paint</a>. The number of entries in <a href='#SkCanvas_drawPosText_pos'>pos</a> array must match the number of <a href='undocumented#Glyph'>glyphs</a>
described by <a href='#SkCanvas_drawPosText_byteLength'>byteLength</a> of <a href='#SkCanvas_drawPosText_text'>text</a>.

<a href='#SkCanvas_drawPosText_text'>text</a> meaning depends on <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>; by default, <a href='#SkCanvas_drawPosText_text'>text</a> is encoded as
UTF-8. <a href='#SkCanvas_drawPosText_pos'>pos</a> elements meaning depends on <a href='SkPaint_Reference#SkPaint'>SkPaint</a> vertical <a href='#SkCanvas_drawPosText_text'>text</a>; by default
<a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> and baseline are relative to <a href='SkPoint_Reference#SkPoint'>SkPoint</a> in <a href='#SkCanvas_drawPosText_pos'>pos</a> array.
<a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> is affected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosText_text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawPosText_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; apply to <a href='#SkCanvas_drawPosText_text'>text</a>. By default, draws
filled 12 <a href='SkPoint_Reference#Point'>point</a> black <a href='undocumented#Glyph'>glyphs</a>.

Layout engines such as Harfbuzz typically position each <a href='undocumented#Glyph'>glyph</a>
rather than using the  <a href='SkFont_Reference#Font_Advance'>font advance</a> widths.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPosText_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawPosText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_pos'><code><strong>pos</strong></code></a></td>
    <td>array of <a href='undocumented#Glyph'>glyph</a> origins</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawPosText_text'>text</a> <a href='undocumented#Size'>size</a>, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf0b2402533a23b6392e0676b7a8414c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawPosTextH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='undocumented#SkScalar'>SkScalar</a> xpos[], <a href='undocumented#SkScalar'>SkScalar</a> constY,
                  const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws each <a href='undocumented#Glyph'>glyph</a> in <a href='#SkCanvas_drawPosTextH_text'>text</a> with its origin composed from <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> array and
<a href='#SkCanvas_drawPosTextH_constY'>constY</a>, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosTextH_paint'>paint</a>. The number of entries in <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> array
must match the number of <a href='undocumented#Glyph'>glyphs</a> described by <a href='#SkCanvas_drawPosTextH_byteLength'>byteLength</a> of <a href='#SkCanvas_drawPosTextH_text'>text</a>.

<a href='#SkCanvas_drawPosTextH_text'>text</a> meaning depends on <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>; by default, <a href='#SkCanvas_drawPosTextH_text'>text</a> is encoded as
UTF-8. <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> elements meaning depends on <a href='SkPaint_Reference#SkPaint'>SkPaint</a> vertical <a href='#SkCanvas_drawPosTextH_text'>text</a>;
by default each <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> is positioned at an <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> element and
its baseline is positioned at <a href='#SkCanvas_drawPosTextH_constY'>constY</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> is affected by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> and
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawPosTextH_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; apply to <a href='#SkCanvas_drawPosTextH_text'>text</a>. By default, draws
filled 12 <a href='SkPoint_Reference#Point'>point</a> black <a href='undocumented#Glyph'>glyphs</a>.

Layout engines such as Harfbuzz typically position each <a href='undocumented#Glyph'>glyph</a>
rather than using the  <a href='SkFont_Reference#Font_Advance'>font advance</a> widths if all <a href='undocumented#Glyph'>glyphs</a> share the same
baseline.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPosTextH_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawPosTextH_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_xpos'><code><strong>xpos</strong></code></a></td>
    <td>array of x-axis positions, used to position each <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_constY'><code><strong>constY</strong></code></a></td>
    <td>shared y-axis value for all of x-axis positions</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='undocumented#Size'>size</a>, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="95c6a7ef82993a8d2add676080e9438a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawTextRSXform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[],
                     const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawTextRSXform_text'>text</a>, transforming each <a href='undocumented#Glyph'>glyph</a> by the corresponding <a href='undocumented#SkRSXform'>SkRSXform</a>,
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawTextRSXform_paint'>paint</a>.

<a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawTextRSXform_xform'>xform</a> array specifies a separate square scale, rotation, and translation
for each <a href='undocumented#Glyph'>glyph</a>. <a href='#SkCanvas_drawTextRSXform_xform'>xform</a> does not affect <a href='#SkCanvas_drawTextRSXform_paint'>paint</a> <a href='undocumented#SkShader'>SkShader</a>.

Optional <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>cullRect</a> is a conservative bounds of <a href='#SkCanvas_drawTextRSXform_text'>text</a>, taking into account
<a href='undocumented#SkRSXform'>SkRSXform</a> and <a href='#SkCanvas_drawTextRSXform_paint'>paint</a>. If <a href='#SkCanvas_drawTextRSXform_cullRect'>cullRect</a> is outside of clip, <a href='SkCanvas_Reference#Canvas'>canvas</a> can skip drawing.

All elements of <a href='#SkCanvas_drawTextRSXform_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; apply to <a href='#SkCanvas_drawTextRSXform_text'>text</a>. By default, draws
filled 12 <a href='SkPoint_Reference#Point'>point</a> black <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextRSXform_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawTextRSXform_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> rotates, scales, and translates each <a href='undocumented#Glyph'>glyph</a> individually</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds of <a href='#SkCanvas_drawTextRSXform_text'>text</a> for efficient clipping; or nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawTextRSXform_text'>text</a> <a href='undocumented#Size'>size</a>, blend, <a href='SkColor_Reference#Color'>color</a>, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="935c8f8b9782d297a73d7186f6ef7945"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>

<a name='SkCanvas_drawTextBlob'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* blob, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#Text_Blob'>Text_Blob</a> <a href='#SkCanvas_drawTextBlob_blob'>blob</a> at (<a href='#SkCanvas_drawTextBlob_x'>x</a>, <a href='#SkCanvas_drawTextBlob_y'>y</a>), using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawTextBlob_paint'>paint</a>.

<a href='#SkCanvas_drawTextBlob_blob'>blob</a> contains <a href='undocumented#Glyph'>Glyphs</a>, their positions, and <a href='#SkCanvas_drawTextBlob_paint'>paint</a> attributes specific to <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Font_Size'>Font_Size</a>, <a href='#Font_Scale_X'>Font_Scale_X</a>,
<a href='#Font_Skew_X'>Font_Skew_X</a>, <a href='#Font_Hinting'>Font_Hinting</a>, <a href='#Paint_Anti_Alias'>Paint_Anti_Alias</a>, <a href='#Font_Embolden'>Font_Embolden</a>, <a href='#Font_Force_Hinting'>Font_Force_Hinting</a>,
<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Font_Hinting_Spacing'>Font_Hinting_Spacing</a>, <a href='#Font_Anti_Alias'>Font_Anti_Alias</a>, <a href='#Font_Linear'>Font_Linear</a>,
and <a href='#Font_Subpixel'>Font_Subpixel</a>
.

<a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a> must be set to <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>.

Elements of <a href='#SkCanvas_drawTextBlob_paint'>paint</a>: <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Blend_Mode'>Blend_Mode</a>, <a href='SkColor_Reference#Color'>Color</a> including <a href='#Color_Alpha'>Color_Alpha</a>,
<a href='#Color_Filter'>Color_Filter</a>, <a href='#Paint_Dither'>Paint_Dither</a>, <a href='#Draw_Looper'>Draw_Looper</a>, <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Path_Effect'>Path_Effect</a>, <a href='undocumented#Shader'>Shader</a>, and
<a href='#Paint_Style'>Paint_Style</a>; apply to <a href='#SkCanvas_drawTextBlob_blob'>blob</a>. If <a href='SkPaint_Reference#Paint'>Paint</a> contains <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>:
<a href='#Paint_Miter_Limit'>Paint_Miter_Limit</a>, <a href='#Paint_Stroke_Cap'>Paint_Stroke_Cap</a>, <a href='#Paint_Stroke_Join'>Paint_Stroke_Join</a>, and <a href='#Paint_Stroke_Width'>Paint_Stroke_Width</a>;
apply to <a href='SkPath_Reference#Path'>Path</a> created from <a href='#SkCanvas_drawTextBlob_blob'>blob</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextBlob_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a>, positions, and their paints' <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>, and so on</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset applied to <a href='#SkCanvas_drawTextBlob_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset applied to <a href='#SkCanvas_drawTextBlob_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_paint'><code><strong>paint</strong></code></a></td>
    <td>blend, <a href='SkColor_Reference#Color'>color</a>, stroking, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="005502b502c1282cb8d306d6c8d998fb"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>

<a name='SkCanvas_drawTextBlob_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt;& blob, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#Text_Blob'>Text_Blob</a> <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a> at (<a href='#SkCanvas_drawTextBlob_2_x'>x</a>, <a href='#SkCanvas_drawTextBlob_2_y'>y</a>), using Clip, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, and <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a>.

<a href='#SkCanvas_drawTextBlob_2_blob'>blob</a> contains <a href='undocumented#Glyph'>Glyphs</a>, their positions, and <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a> attributes specific to <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Font_Size'>Font_Size</a>, <a href='#Font_Scale_X'>Font_Scale_X</a>,
<a href='#Font_Skew_X'>Font_Skew_X</a>, <a href='#Font_Hinting'>Font_Hinting</a>, <a href='#Paint_Anti_Alias'>Paint_Anti_Alias</a>, <a href='#Font_Embolden'>Font_Embolden</a>, <a href='#Font_Force_Hinting'>Font_Force_Hinting</a>,
<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Font_Hinting_Spacing'>Font_Hinting_Spacing</a>, <a href='#Font_Anti_Alias'>Font_Anti_Alias</a>, <a href='#Font_Linear'>Font_Linear</a>,
and <a href='#Font_Subpixel'>Font_Subpixel</a>
.

<a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a> must be set to <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>.

Elements of <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a>: <a href='#Path_Effect'>Path_Effect</a>, <a href='#Mask_Filter'>Mask_Filter</a>, <a href='undocumented#Shader'>Shader</a>, <a href='#Color_Filter'>Color_Filter</a>,
<a href='#Image_Filter'>Image_Filter</a>, and <a href='#Draw_Looper'>Draw_Looper</a>; apply to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextBlob_2_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a>, positions, and their paints' <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>, and so on</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset applied to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset applied to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_paint'><code><strong>paint</strong></code></a></td>
    <td>blend, <a href='SkColor_Reference#Color'>color</a>, stroking, and so on, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1cae21e7b63b24de3eca0bbd9be1936b"><div><a href='SkPaint_Reference#Paint'>Paint</a> attributes related to <a href='undocumented#Text'>text</a>, like <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, have no effect on <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a> passed to <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>

<a name='SkCanvas_drawPicture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_picture'>picture</a>, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> are unchanged by <a href='#SkCanvas_drawPicture_picture'>picture</a> contents, as if
<a href='#SkCanvas_save'>save()</a> was called before and <a href='#SkCanvas_restore'>restore()</a> was called after <a href='#SkCanvas_drawPicture'>drawPicture</a>().

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> records a series of draw commands for later playback.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="83918a23fcffd47f59a1ef662c85a24c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>

<a name='SkCanvas_drawPicture_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&gt;& <a href='SkPicture_Reference#Picture'>picture</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_2_picture'>picture</a>, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> are unchanged by <a href='#SkCanvas_drawPicture_2_picture'>picture</a> contents, as if
<a href='#SkCanvas_save'>save()</a> was called before and <a href='#SkCanvas_restore'>restore()</a> was called after <a href='#SkCanvas_drawPicture'>drawPicture</a>().

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> records a series of draw commands for later playback.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_2_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="83918a23fcffd47f59a1ef662c85a24c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>

<a name='SkCanvas_drawPicture_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_3_picture'>picture</a>, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>; transforming <a href='#SkCanvas_drawPicture_3_picture'>picture</a> with
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkCanvas_drawPicture_3_matrix'>matrix</a>, if provided; and use <a href='SkPaint_Reference#SkPaint'>SkPaint</a>  <a href='#SkCanvas_drawPicture_3_paint'>paint alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, if provided.

<a href='#SkCanvas_drawPicture_3_matrix'>matrix</a> transformation is equivalent to: <a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.
<a href='#SkCanvas_drawPicture_3_paint'>paint</a> use is equivalent to: <a href='#SkCanvas_saveLayer'>saveLayer</a>(), <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_3_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_3_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate, scale, translate, and so on; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to apply transparency, filtering, and so on; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="759e4e5bac680838added8f70884dcdc"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>

<a name='SkCanvas_drawPicture_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&gt;& <a href='SkPicture_Reference#Picture'>picture</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_4_picture'>picture</a>, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>; transforming <a href='#SkCanvas_drawPicture_4_picture'>picture</a> with
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkCanvas_drawPicture_4_matrix'>matrix</a>, if provided; and use <a href='SkPaint_Reference#SkPaint'>SkPaint</a>  <a href='#SkCanvas_drawPicture_4_paint'>paint alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, if provided.

<a href='#SkCanvas_drawPicture_4_matrix'>matrix</a> transformation is equivalent to: <a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.
<a href='#SkCanvas_drawPicture_4_paint'>paint</a> use is equivalent to: <a href='#SkCanvas_saveLayer'>saveLayer</a>(), <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_4_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_4_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate, scale, translate, and so on; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to apply transparency, filtering, and so on; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c4ff59439dd2fc871925d4eeb0c84ca1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>

<a name='SkCanvas_drawVertices'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_vertices'>vertices</a>, a triangle mesh, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> and  <a href='undocumented#Vertices_Colors'>vertices colors</a> are defined in <a href='#SkCanvas_drawVertices_vertices'>vertices</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_mode'>mode</a> combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_mode'><code><strong>mode</strong></code></a></td>
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>, if both are present</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, used as <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f48b22eaad1bb7adcc3faaa321754af6"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawVertices_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkVertices'>SkVertices</a>&gt;& <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_2_vertices'>vertices</a>, a triangle mesh, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> and  <a href='undocumented#Vertices_Colors'>vertices colors</a> are defined in <a href='#SkCanvas_drawVertices_2_vertices'>vertices</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_2_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_2_mode'>mode</a> combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_2_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_2_mode'><code><strong>mode</strong></code></a></td>
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>, if both are present</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_2_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, used as <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>, may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e8bdae9bea3227758989028424fcac3d"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawVertices_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, const <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> bones[], int boneCount,
                  <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a>, a triangle mesh, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Bone <a href='undocumented#Data'>data</a> is used to
deform <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a> with bone <a href='SkPath_Reference#Conic_Weight'>weights</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> and  <a href='undocumented#Vertices_Colors'>vertices colors</a> are defined in <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_3_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_3_mode'>mode</a> combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>.
The first element of <a href='#SkCanvas_drawVertices_3_bones'>bones</a> should be an object to world space transformation <a href='SkMatrix_Reference#Matrix'>matrix</a> that
will be applied before performing mesh deformations. If no such transformation is needed,
it should be the identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
<a href='#SkCanvas_drawVertices_3_boneCount'>boneCount</a> must be at most 80, and thus the <a href='undocumented#Size'>size</a> of <a href='#SkCanvas_drawVertices_3_bones'>bones</a> should be at most 80.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_3_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_bones'><code><strong>bones</strong></code></a></td>
    <td>bone <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='undocumented#Data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_boneCount'><code><strong>boneCount</strong></code></a></td>
    <td>number of bone <a href='SkMatrix_Reference#Matrix'>matrices</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_mode'><code><strong>mode</strong></code></a></td>
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>, if both are present</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, used as <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>, may be nullptr</td>
  </tr>
</table>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawVertices_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkVertices'>SkVertices</a>&gt;& <a href='undocumented#Vertices'>vertices</a>, const <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> bones[], int boneCount,
                  <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a>, a triangle mesh, using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Bone <a href='undocumented#Data'>data</a> is used to
deform <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a> with bone <a href='SkPath_Reference#Conic_Weight'>weights</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> and  <a href='undocumented#Vertices_Colors'>vertices colors</a> are defined in <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a>, and <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_4_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_4_mode'>mode</a> combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>.
The first element of <a href='#SkCanvas_drawVertices_4_bones'>bones</a> should be an object to world space transformation <a href='SkMatrix_Reference#Matrix'>matrix</a> that
will be applied before performing mesh deformations. If no such transformation is needed,
it should be the identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
<a href='#SkCanvas_drawVertices_4_boneCount'>boneCount</a> must be at most 80, and thus the <a href='undocumented#Size'>size</a> of <a href='#SkCanvas_drawVertices_4_bones'>bones</a> should be at most 80.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_4_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_bones'><code><strong>bones</strong></code></a></td>
    <td>bone <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='undocumented#Data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_boneCount'><code><strong>boneCount</strong></code></a></td>
    <td>number of bone <a href='SkMatrix_Reference#Matrix'>matrices</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_mode'><code><strong>mode</strong></code></a></td>
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> with <a href='undocumented#SkShader'>SkShader</a>, if both are present</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, used as <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>, may be nullptr</td>
  </tr>
</table>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawPatch'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[4], const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> texCoords[4],
               <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a  <a href='undocumented#Coons_Patch'>Coons patch</a>: the interpolation of four <a href='#SkCanvas_drawPatch_cubics'>cubics</a> with shared corners,
associating a <a href='SkColor_Reference#Color'>color</a>, and optionally a <a href='undocumented#Texture'>texture</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, with each corner.

<a href='undocumented#Coons_Patch'>Coons patch</a> uses clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='#SkCanvas_drawPatch_paint'>paint</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>. If <a href='undocumented#SkShader'>SkShader</a> is provided it is treated
as  <a href='undocumented#Coons_Patch'>Coons patch</a> <a href='undocumented#Texture'>texture</a>; <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawPatch_mode'>mode</a> combines <a href='SkColor_Reference#Color'>color</a> <a href='#SkCanvas_drawPatch_colors'>colors</a> and <a href='undocumented#SkShader'>SkShader</a> if
both are provided.

<a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_cubics'>cubics</a> specifies four <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> starting at the top-left corner,
in clockwise order, sharing every fourth <a href='SkPoint_Reference#Point'>point</a>. The last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> ends at the
first <a href='SkPoint_Reference#Point'>point</a>.

<a href='SkColor_Reference#Color'>Color</a> array <a href='SkColor_Reference#Color'>color</a> associates <a href='#SkCanvas_drawPatch_colors'>colors</a> with corners in top-left, top-right,
bottom-right, bottom-left order.

If <a href='#SkCanvas_drawPatch_paint'>paint</a> contains <a href='undocumented#SkShader'>SkShader</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_texCoords'>texCoords</a> maps <a href='undocumented#SkShader'>SkShader</a> as <a href='undocumented#Texture'>texture</a> to
corners in top-left, top-right, bottom-right, bottom-left order.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_cubics'><code><strong>cubics</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> array, sharing common <a href='SkPoint_Reference#Point'>points</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_colors'><code><strong>colors</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>color</a> array, one for each corner</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_texCoords'><code><strong>texCoords</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a> of <a href='undocumented#Texture'>texture</a> coordinates, mapping <a href='undocumented#SkShader'>SkShader</a> to corners;</td>
  </tr>
</table>

may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> for <a href='#SkCanvas_drawPatch_colors'>colors</a>, and for <a href='undocumented#SkShader'>SkShader</a> if <a href='#SkCanvas_drawPatch_paint'>paint</a> has one</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="accb545d67984ced168f5be6ab824795"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawVertices'>drawVertices</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawPatch_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPatch'>drawPatch</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[4], const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> texCoords[4],
               const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a>  <a href='undocumented#Coons_Patch'>Coons patch</a>: the interpolation of four <a href='#SkCanvas_drawPatch_2_cubics'>cubics</a> with shared corners,
associating a <a href='SkColor_Reference#Color'>color</a>, and optionally a <a href='undocumented#Texture'>texture</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, with each corner.

<a href='undocumented#Coons_Patch'>Coons patch</a> uses clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='#SkCanvas_drawPatch_2_paint'>paint</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>. If <a href='undocumented#SkShader'>SkShader</a> is provided it is treated
as  <a href='undocumented#Coons_Patch'>Coons patch</a> <a href='undocumented#Texture'>texture</a>; <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode combines <a href='SkColor_Reference#Color'>color</a> <a href='#SkCanvas_drawPatch_2_colors'>colors</a> and <a href='undocumented#SkShader'>SkShader</a> if
both are provided.

<a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_2_cubics'>cubics</a> specifies four <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> starting at the top-left corner,
in clockwise order, sharing every fourth <a href='SkPoint_Reference#Point'>point</a>. The last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> ends at the
first <a href='SkPoint_Reference#Point'>point</a>.

<a href='SkColor_Reference#Color'>Color</a> array <a href='SkColor_Reference#Color'>color</a> associates <a href='#SkCanvas_drawPatch_2_colors'>colors</a> with corners in top-left, top-right,
bottom-right, bottom-left order.

If <a href='#SkCanvas_drawPatch_2_paint'>paint</a> contains <a href='undocumented#SkShader'>SkShader</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_2_texCoords'>texCoords</a> maps <a href='undocumented#SkShader'>SkShader</a> as <a href='undocumented#Texture'>texture</a> to
corners in top-left, top-right, bottom-right, bottom-left order.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_2_cubics'><code><strong>cubics</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> array, sharing common <a href='SkPoint_Reference#Point'>points</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_colors'><code><strong>colors</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>color</a> array, one for each corner</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_texCoords'><code><strong>texCoords</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a> of <a href='undocumented#Texture'>texture</a> coordinates, mapping <a href='undocumented#SkShader'>SkShader</a> to corners;</td>
  </tr>
</table>

may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, used to draw</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4e8b7409531c9211a2afcf632005a38c"></fiddle-embed></div>

### Example

<div><fiddle-embed name="3412c2a16cb529af0e04878d264451f2"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawVertices'>drawVertices</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawAtlas'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
               const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[], int count, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
               const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> from <a href='#SkCanvas_drawAtlas_atlas'>atlas</a>, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_paint'>paint</a> uses <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_tex'>tex</a> locates <a href='undocumented#Sprite'>sprite</a> in
<a href='#SkCanvas_drawAtlas_atlas'>atlas</a>, and <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_xform'>xform</a> transforms it into destination space.

<a href='#SkCanvas_drawAtlas_xform'>xform</a>, <a href='undocumented#Text'>text</a>, and <a href='#SkCanvas_drawAtlas_colors'>colors</a> if present, must contain <a href='#SkCanvas_drawAtlas_count'>count</a> entries.
Optional <a href='#SkCanvas_drawAtlas_colors'>colors</a> are applied for each <a href='undocumented#Sprite'>sprite</a> using <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawAtlas_mode'>mode</a>, treating
<a href='undocumented#Sprite'>sprite</a> as source and <a href='#SkCanvas_drawAtlas_colors'>colors</a> as destination.
Optional <a href='#SkCanvas_drawAtlas_cullRect'>cullRect</a> is a conservative bounds of all transformed <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_cullRect'>cullRect</a> is outside of clip, <a href='SkCanvas_Reference#Canvas'>canvas</a> can skip drawing.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> mappings for <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> locations of <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_colors'><code><strong>colors</strong></code></a></td>
    <td>one per <a href='undocumented#Sprite'>sprite</a>, blended with <a href='undocumented#Sprite'>sprite</a> using <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> combining <a href='#SkCanvas_drawAtlas_colors'>colors</a> and <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> for efficient clipping; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and so on; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1df575f9b8132306ce0552a2554ed132"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawAtlas_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[],
               const <a href='SkColor_Reference#SkColor'>SkColor</a> colors[], int count, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> mode, const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect,
               const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> from <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a>, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_2_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_2_paint'>paint</a> uses <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_2_tex'>tex</a> locates <a href='undocumented#Sprite'>sprite</a> in
<a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a>, and <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_2_xform'>xform</a> transforms it into destination space.

<a href='#SkCanvas_drawAtlas_2_xform'>xform</a>, <a href='undocumented#Text'>text</a>, and <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> if present, must contain <a href='#SkCanvas_drawAtlas_2_count'>count</a> entries.
Optional <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> is applied for each <a href='undocumented#Sprite'>sprite</a> using <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>.
Optional <a href='#SkCanvas_drawAtlas_2_cullRect'>cullRect</a> is a conservative bounds of all transformed <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_2_cullRect'>cullRect</a> is outside of clip, <a href='SkCanvas_Reference#Canvas'>canvas</a> can skip drawing.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_2_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> mappings for <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> locations of <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_colors'><code><strong>colors</strong></code></a></td>
    <td>one per <a href='undocumented#Sprite'>sprite</a>, blended with <a href='undocumented#Sprite'>sprite</a> using <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> combining <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> and <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> for efficient clipping; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and so on; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e66a8f230a8d531bcef9f5ebdc5aac1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawAtlas_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='SkImage_Reference#SkImage'>SkImage</a>* atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[], int count,
               const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> from <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a>, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_3_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_3_paint'>paint</a> uses <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_3_tex'>tex</a> locates <a href='undocumented#Sprite'>sprite</a> in
<a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a>, and <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_3_xform'>xform</a> transforms it into destination space.

<a href='#SkCanvas_drawAtlas_3_xform'>xform</a> and <a href='undocumented#Text'>text</a> must contain <a href='#SkCanvas_drawAtlas_3_count'>count</a> entries.
Optional <a href='#SkCanvas_drawAtlas_3_cullRect'>cullRect</a> is a conservative bounds of all transformed <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_3_cullRect'>cullRect</a> is outside of clip, <a href='SkCanvas_Reference#Canvas'>canvas</a> can skip drawing.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_3_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> mappings for <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> locations of <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> for efficient clipping; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and so on; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8dc0d0fdeab20bbc21cac6874ddbefcd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawAtlas_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt;& atlas, const <a href='undocumented#SkRSXform'>SkRSXform</a> xform[], const <a href='SkRect_Reference#SkRect'>SkRect</a> tex[], int count,
               const <a href='SkRect_Reference#SkRect'>SkRect</a>* cullRect, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> from <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a>, using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_4_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_4_paint'>paint</a> uses <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, and <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_4_tex'>tex</a> locates <a href='undocumented#Sprite'>sprite</a> in
<a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a>, and <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_4_xform'>xform</a> transforms it into destination space.

<a href='#SkCanvas_drawAtlas_4_xform'>xform</a> and <a href='undocumented#Text'>text</a> must contain <a href='#SkCanvas_drawAtlas_4_count'>count</a> entries.
Optional <a href='#SkCanvas_drawAtlas_4_cullRect'>cullRect</a> is a conservative bounds of all transformed <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_4_cullRect'>cullRect</a> is outside of clip, <a href='SkCanvas_Reference#Canvas'>canvas</a> can skip drawing.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_4_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> containing <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> mappings for <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> locations of <a href='undocumented#Sprite'>sprites</a> in <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> for efficient clipping; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and so on; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c093c2b14bd3e6171ede7cd4049d9b57"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawDrawable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a> = nullptr)
</pre>

Draws <a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawDrawable_drawable'>drawable</a> using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, concatenated with
optional <a href='#SkCanvas_drawDrawable_matrix'>matrix</a>.

If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> has an asynchronous implementation, as is the case
when it is recording into <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, then <a href='#SkCanvas_drawDrawable_drawable'>drawable</a> will be referenced,
so that <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> can be called when the operation is finalized. To force
immediate drawing, call <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> instead.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDrawable_drawable'><code><strong>drawable</strong></code></a></td>
    <td>custom struct encapsulating drawing commands</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_matrix'><code><strong>matrix</strong></code></a></td>
    <td>transformation applied to drawing; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3a4dfcd08838866b5cfc0d82489195ba"></fiddle-embed></div>

### See Also

<a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawDrawable_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Draws <a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawDrawable_2_drawable'>drawable</a> using clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, offset by (<a href='#SkCanvas_drawDrawable_2_x'>x</a>, <a href='#SkCanvas_drawDrawable_2_y'>y</a>).

If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> has an asynchronous implementation, as is the case
when it is recording into <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, then <a href='#SkCanvas_drawDrawable_2_drawable'>drawable</a> will be referenced,
so that <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> can be called when the operation is finalized. To force
immediate drawing, call <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> instead.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDrawable_2_drawable'><code><strong>drawable</strong></code></a></td>
    <td>custom struct encapsulating drawing commands</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_2_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> writable pixels on x-axis</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_2_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> writable pixels on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1bdc07ad3b154c89b771722c2fcaee3f"></fiddle-embed></div>

### See Also

<a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawAnnotation'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const char key[], <a href='undocumented#SkData'>SkData</a>* value)
</pre>

Associates <a href='SkRect_Reference#SkRect'>SkRect</a> on <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> with an annotation; a key-value pair, where the <a href='#SkCanvas_drawAnnotation_key'>key</a> is
a null-terminated UTF-8 <a href='undocumented#String'>string</a>, and optional <a href='#SkCanvas_drawAnnotation_value'>value</a> is stored as <a href='undocumented#SkData'>SkData</a>.

Only some <a href='SkCanvas_Reference#Canvas'>canvas</a> implementations, such as recording to <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, or drawing to
<a href='undocumented#Document_PDF'>document PDF</a>, use annotations.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAnnotation_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> extent of <a href='SkCanvas_Reference#Canvas'>canvas</a> to annotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_key'><code><strong>key</strong></code></a></td>
    <td><a href='undocumented#String'>string</a> used for lookup</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_value'><code><strong>value</strong></code></a></td>
    <td><a href='undocumented#Data'>data</a> holding <a href='#SkCanvas_drawAnnotation_value'>value</a> stored in annotation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="00b430bd80d740e19c6d020a940f56d5"></fiddle-embed></div>

### See Also

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#SkDocument'>SkDocument</a>

<a name='SkCanvas_drawAnnotation_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const char key[], const <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt;& value)
</pre>

Associates <a href='SkRect_Reference#SkRect'>SkRect</a> on <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> when an annotation; a key-value pair, where the <a href='#SkCanvas_drawAnnotation_2_key'>key</a> is
a null-terminated UTF-8 <a href='undocumented#String'>string</a>, and optional <a href='#SkCanvas_drawAnnotation_2_value'>value</a> is stored as <a href='undocumented#SkData'>SkData</a>.

Only some <a href='SkCanvas_Reference#Canvas'>canvas</a> implementations, such as recording to <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, or drawing to
<a href='undocumented#Document_PDF'>document PDF</a>, use annotations.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAnnotation_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> extent of <a href='SkCanvas_Reference#Canvas'>canvas</a> to annotate</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_2_key'><code><strong>key</strong></code></a></td>
    <td><a href='undocumented#String'>string</a> used for lookup</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_2_value'><code><strong>value</strong></code></a></td>
    <td><a href='undocumented#Data'>data</a> holding <a href='#SkCanvas_drawAnnotation_2_value'>value</a> stored in annotation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="00b430bd80d740e19c6d020a940f56d5"></fiddle-embed></div>

### See Also

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#SkDocument'>SkDocument</a>

<a name='SkCanvas_isClipEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkCanvas_isClipEmpty'>isClipEmpty</a>()const
</pre>

Returns true if clip is empty; that is, nothing will draw.

May do work when called; it should not be called
more often than needed. However, once called, subsequent calls perform no
work until clip changes.

### Return Value

true if clip is empty

### Example

<div><fiddle-embed name="f106f146a58c8604308d4d8d7086d2f5">

#### Example Output

~~~~
clip is not empty
clip is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_isClipRect'>isClipRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>

<a name='SkCanvas_isClipRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkCanvas_isClipRect'>isClipRect</a>()const
</pre>

Returns true if clip is <a href='SkRect_Reference#SkRect'>SkRect</a> and not empty.
Returns false if the clip is empty, or if it is not <a href='SkRect_Reference#SkRect'>SkRect</a>.

### Return Value

true if clip is <a href='SkRect_Reference#SkRect'>SkRect</a> and not empty

### Example

<div><fiddle-embed name="9894bfb476c78a8f6c8f49fbbca3d50d">

#### Example Output

~~~~
clip is rect
clip is not rect
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_isClipEmpty'>isClipEmpty</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>

