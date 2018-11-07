SkCanvas Reference
===


<a name='SkCanvas'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> {
<a href='SkCanvas_Reference#SkCanvas'>public</a>:
    <a href='SkCanvas_Reference#SkCanvas'>static</a> <a href='SkCanvas_Reference#SkCanvas'>std</a>::<a href='SkCanvas_Reference#SkCanvas'>unique_ptr</a><<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>> <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a>(<a href='#SkCanvas_MakeRasterDirect'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>,
                                               <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                               <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#SkSurfaceProps'>std</a>::<a href='undocumented#SkSurfaceProps'>unique_ptr</a><<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>> <a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a>(<a href='#SkCanvas_MakeRasterDirectN32'>int</a> <a href='#SkCanvas_MakeRasterDirectN32'>width</a>, <a href='#SkCanvas_MakeRasterDirectN32'>int</a> <a href='#SkCanvas_MakeRasterDirectN32'>height</a>, <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>* <a href='SkColor_Reference#SkPMColor'>pixels</a>,
                                                         <a href='SkColor_Reference#SkPMColor'>size_t</a> <a href='SkColor_Reference#SkPMColor'>rowBytes</a>);
    <a href='#SkCanvas_empty_constructor'>SkCanvas()</a>;
    <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>int</a> <a href='SkCanvas_Reference#SkCanvas'>width</a>, <a href='SkCanvas_Reference#SkCanvas'>int</a> <a href='SkCanvas_Reference#SkCanvas'>height</a>, <a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>explicit</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkBaseDevice'>SkBaseDevice</a>> <a href='undocumented#Device'>device</a>);
    <a href='undocumented#Device'>explicit</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>);

    <a href='SkBitmap_Reference#Bitmap'>enum</a> <a href='SkBitmap_Reference#Bitmap'>class</a> <a href='#SkCanvas_ColorBehavior'>ColorBehavior</a> {
        <a href='#SkCanvas_ColorBehavior'>kLegacy</a>,
    };

    <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='#SkCanvas_ColorBehavior'>ColorBehavior</a> <a href='#SkCanvas_ColorBehavior'>behavior</a>);
    <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='undocumented#SkSurfaceProps'>props</a>);
    <a href='undocumented#SkSurfaceProps'>virtual</a> ~<a href='#SkCanvas_empty_constructor'>SkCanvas()</a>;
    <a href='undocumented#SkMetaData'>SkMetaData</a>& <a href='#SkCanvas_getMetaData'>getMetaData</a>();
    <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>() <a href='#SkCanvas_imageInfo'>const</a>;
    <a href='#SkCanvas_imageInfo'>bool</a> <a href='#SkCanvas_getProps'>getProps</a>(<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a>) <a href='undocumented#SkSurfaceProps'>const</a>;
    <a href='undocumented#SkSurfaceProps'>void</a> <a href='#SkCanvas_flush'>flush()</a>;
    <a href='#SkCanvas_flush'>virtual</a> <a href='undocumented#SkISize'>SkISize</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a>() <a href='#SkCanvas_getBaseLayerSize'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkCanvas_makeSurface'>makeSurface</a>(<a href='#SkCanvas_makeSurface'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>virtual</a> <a href='undocumented#GrContext'>GrContext</a>* <a href='#SkCanvas_getGrContext'>getGrContext</a>();
    <a href='#SkCanvas_getGrContext'>void</a>* <a href='#SkCanvas_accessTopLayerPixels'>accessTopLayerPixels</a>(<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>* <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a>* <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>origin</a> = <a href='SkIPoint_Reference#SkIPoint'>nullptr</a>);
    <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_Handle'>Handle</a> <a href='#SkCanvas_accessTopRasterHandle'>accessTopRasterHandle</a>() <a href='#SkCanvas_accessTopRasterHandle'>const</a>;
    <a href='#SkCanvas_accessTopRasterHandle'>bool</a> <a href='#SkCanvas_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    <a href='SkPixmap_Reference#Pixmap'>bool</a> <a href='#SkCanvas_readPixels'>readPixels</a>(<a href='#SkCanvas_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>,
                    <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkCanvas_readPixels'>readPixels</a>(<a href='#SkCanvas_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, <a href='SkPixmap_Reference#Pixmap'>int</a> <a href='SkPixmap_Reference#Pixmap'>srcX</a>, <a href='SkPixmap_Reference#Pixmap'>int</a> <a href='SkPixmap_Reference#Pixmap'>srcY</a>);
    <a href='SkPixmap_Reference#Pixmap'>bool</a> <a href='#SkCanvas_readPixels'>readPixels</a>(<a href='#SkCanvas_readPixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>srcX</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>srcY</a>);
    <a href='SkBitmap_Reference#Bitmap'>bool</a> <a href='#SkCanvas_writePixels'>writePixels</a>(<a href='#SkCanvas_writePixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>x</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>y</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkCanvas_writePixels'>writePixels</a>(<a href='#SkCanvas_writePixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>x</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>y</a>);
    <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='#SkCanvas_save'>save()</a>;
    <a href='#SkCanvas_save'>int</a> <a href='#SkCanvas_saveLayer'>saveLayer</a>(<a href='#SkCanvas_saveLayer'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>int</a> <a href='#SkCanvas_saveLayer'>saveLayer</a>(<a href='#SkCanvas_saveLayer'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>int</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(<a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>int</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>(<a href='#SkCanvas_saveLayerAlpha'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>);

    <a href='SkColor_Reference#Alpha'>enum</a> <a href='#SkCanvas_SaveLayerFlagsSet'>SaveLayerFlagsSet</a> {
        <a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a> = 1 << 1,
        <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> = 1 << 2,
        <a href='#SkCanvas_kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag'>kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag</a> =
                                          1 << 3,
        <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>kDontClipToLayer_Legacy_SaveLayerFlag</a> =
           <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>kDontClipToLayer_PrivateSaveLayerFlag</a>,
    };

    <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>typedef</a> <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>uint32_t</a> <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a>;

    <a href='#SkCanvas_SaveLayerFlags'>struct</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> {
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>();
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a> = 0);
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>backdrop</a>,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a>);
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>backdrop</a>,
                     <a href='undocumented#SkImageFilter'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>clipMask</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>clipMatrix</a>,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a>);
        <a href='#SkCanvas_SaveLayerFlags'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>fBounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>;
        <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#SkPaint'>fPaint</a> = <a href='SkPaint_Reference#SkPaint'>nullptr</a>;
        <a href='SkPaint_Reference#SkPaint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>fBackdrop</a> = <a href='undocumented#SkImageFilter'>nullptr</a>;
        <a href='undocumented#SkImageFilter'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>fClipMask</a> = <a href='SkImage_Reference#SkImage'>nullptr</a>;
        <a href='SkImage_Reference#SkImage'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>fClipMatrix</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>;
        <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>fSaveLayerFlags</a> = 0;
    };

    <a href='#SkCanvas_SaveLayerFlags'>int</a> <a href='#SkCanvas_saveLayer'>saveLayer</a>(<a href='#SkCanvas_saveLayer'>const</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>& <a href='#SkCanvas_SaveLayerRec'>layerRec</a>);
    <a href='#SkCanvas_SaveLayerRec'>void</a> <a href='#SkCanvas_restore'>restore()</a>;
    <a href='#SkCanvas_restore'>int</a> <a href='#SkCanvas_getSaveCount'>getSaveCount</a>() <a href='#SkCanvas_getSaveCount'>const</a>;
    <a href='#SkCanvas_getSaveCount'>void</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>(<a href='#SkCanvas_restoreToCount'>int</a> <a href='#SkCanvas_restoreToCount'>saveCount</a>);
    <a href='#SkCanvas_restoreToCount'>void</a> <a href='#SkCanvas_restoreToCount'>translate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>rotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>skew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkCanvas_concat'>concat</a>(<a href='#SkCanvas_concat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>);
    <a href='SkMatrix_Reference#Matrix'>void</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>(<a href='#SkCanvas_setMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>);
    <a href='SkMatrix_Reference#Matrix'>void</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a>();
    <a href='#SkCanvas_resetMatrix'>void</a> <a href='#SkCanvas_clipRect'>clipRect</a>(<a href='#SkCanvas_clipRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>, <a href='undocumented#SkClipOp'>bool</a> <a href='undocumented#SkClipOp'>doAntiAlias</a>);
    <a href='undocumented#SkClipOp'>void</a> <a href='#SkCanvas_clipRect'>clipRect</a>(<a href='#SkCanvas_clipRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>);
    <a href='undocumented#SkClipOp'>void</a> <a href='#SkCanvas_clipRect'>clipRect</a>(<a href='#SkCanvas_clipRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>bool</a> <a href='SkRect_Reference#Rect'>doAntiAlias</a> = <a href='SkRect_Reference#Rect'>false</a>);
    <a href='SkRect_Reference#Rect'>void</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction'>androidFramework_setDeviceClipRestriction</a>(<a href='#SkCanvas_androidFramework_setDeviceClipRestriction'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>);
    <a href='SkRect_Reference#Rect'>void</a> <a href='#SkCanvas_clipRRect'>clipRRect</a>(<a href='#SkCanvas_clipRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>, <a href='undocumented#SkClipOp'>bool</a> <a href='undocumented#SkClipOp'>doAntiAlias</a>);
    <a href='undocumented#SkClipOp'>void</a> <a href='#SkCanvas_clipRRect'>clipRRect</a>(<a href='#SkCanvas_clipRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>);
    <a href='undocumented#SkClipOp'>void</a> <a href='#SkCanvas_clipRRect'>clipRRect</a>(<a href='#SkCanvas_clipRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='SkRRect_Reference#SkRRect'>bool</a> <a href='SkRRect_Reference#SkRRect'>doAntiAlias</a> = <a href='SkRRect_Reference#SkRRect'>false</a>);
    <a href='SkRRect_Reference#SkRRect'>void</a> <a href='#SkCanvas_clipPath'>clipPath</a>(<a href='#SkCanvas_clipPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>, <a href='undocumented#SkClipOp'>bool</a> <a href='undocumented#SkClipOp'>doAntiAlias</a>);
    <a href='undocumented#SkClipOp'>void</a> <a href='#SkCanvas_clipPath'>clipPath</a>(<a href='#SkCanvas_clipPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>);
    <a href='undocumented#SkClipOp'>void</a> <a href='#SkCanvas_clipPath'>clipPath</a>(<a href='#SkCanvas_clipPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>doAntiAlias</a> = <a href='SkPath_Reference#Path'>false</a>);
    <a href='SkPath_Reference#Path'>void</a> <a href='#SkCanvas_setAllowSimplifyClip'>setAllowSimplifyClip</a>(<a href='#SkCanvas_setAllowSimplifyClip'>bool</a> <a href='#SkCanvas_setAllowSimplifyClip'>allow</a>);
    <a href='#SkCanvas_setAllowSimplifyClip'>void</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>(<a href='#SkCanvas_clipRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>deviceRgn</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a> = <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a>);
    <a href='#SkClipOp_kIntersect'>bool</a> <a href='#SkCanvas_quickReject'>quickReject</a>(<a href='#SkCanvas_quickReject'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#Rect'>bool</a> <a href='#SkCanvas_quickReject'>quickReject</a>(<a href='#SkCanvas_quickReject'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>() <a href='#SkCanvas_getLocalClipBounds'>const</a>;
    <a href='#SkCanvas_getLocalClipBounds'>bool</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>() <a href='#SkCanvas_getDeviceClipBounds'>const</a>;
    <a href='#SkCanvas_getDeviceClipBounds'>bool</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>bounds</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='#SkCanvas_drawColor'>drawColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a> = <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>);
    <a href='#SkBlendMode_kSrcOver'>void</a> <a href='#SkBlendMode_kSrcOver'>clear</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>);
    <a href='SkColor_Reference#Color'>void</a> <a href='#SkCanvas_discard'>discard()</a>;
    <a href='#SkCanvas_discard'>void</a> <a href='#SkCanvas_drawPaint'>drawPaint</a>(<a href='#SkCanvas_drawPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);

    <a href='SkPaint_Reference#Paint'>enum</a> <a href='#SkCanvas_PointMode'>PointMode</a> {
        <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>,
        <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>,
        <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>,
    };

    <a href='#SkCanvas_kPolygon_PointMode'>void</a> <a href='#SkCanvas_drawPoints'>drawPoints</a>(<a href='#SkCanvas_PointMode'>PointMode</a> <a href='#SkCanvas_PointMode'>mode</a>, <a href='#SkCanvas_PointMode'>size_t</a> <a href='#SkCanvas_PointMode'>count</a>, <a href='#SkCanvas_PointMode'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p0</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawRect'>drawRect</a>(<a href='#SkCanvas_drawRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawIRect'>drawIRect</a>(<a href='#SkCanvas_drawIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawRegion'>drawRegion</a>(<a href='#SkCanvas_drawRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawOval'>drawOval</a>(<a href='#SkCanvas_drawOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Oval'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawRRect'>drawRRect</a>(<a href='#SkCanvas_drawRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawDRRect'>drawDRRect</a>(<a href='#SkCanvas_drawDRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>outer</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>inner</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>center</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawArc'>drawArc</a>(<a href='#SkCanvas_drawArc'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>startAngle</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sweepAngle</a>,
                 <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>useCenter</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>(<a href='#SkCanvas_drawRoundRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPath'>drawPath</a>(<a href='#SkCanvas_drawPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawImage'>drawImage</a>(<a href='#SkCanvas_drawImage'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>,
                   <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawImage'>drawImage</a>(<a href='#SkCanvas_drawImage'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>,
                   <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);

    <a href='SkPaint_Reference#Paint'>enum</a> <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> {
        <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>,
        <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>,
    };

    <a href='#SkCanvas_kFast_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>isrc</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>isrc</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                       <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(<a href='#SkCanvas_drawImageNine'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>center</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(<a href='#SkCanvas_drawImageNine'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>center</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a>(<a href='#SkCanvas_drawBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>,
                    <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(<a href='#SkCanvas_drawBitmapRect'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                        <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(<a href='#SkCanvas_drawBitmapRect'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>isrc</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                        <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(<a href='#SkCanvas_drawBitmapRect'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                        <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>);
    <a href='#SkCanvas_kStrict_SrcRectConstraint'>void</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>(<a href='#SkCanvas_drawBitmapNine'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>center</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                        <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);

    <a href='SkPaint_Reference#Paint'>struct</a> <a href='#SkCanvas_Lattice'>Lattice</a> {

        <a href='#SkCanvas_Lattice'>enum</a> <a href='#SkCanvas_Lattice'>RectType</a> : <a href='#SkCanvas_Lattice'>uint8_t</a> {
            <a href='#SkCanvas_Lattice'>kDefault</a> = 0,
            <a href='#SkCanvas_Lattice'>kTransparent</a>,
            <a href='#SkCanvas_Lattice'>kFixedColor</a>,
        };

        <a href='#SkCanvas_Lattice'>const</a> <a href='#SkCanvas_Lattice'>int</a>* <a href='#SkCanvas_Lattice'>fXDivs</a>;
        <a href='#SkCanvas_Lattice'>const</a> <a href='#SkCanvas_Lattice'>int</a>* <a href='#SkCanvas_Lattice'>fYDivs</a>;
        <a href='#SkCanvas_Lattice'>const</a> <a href='#SkCanvas_Lattice'>RectType</a>* <a href='#SkCanvas_Lattice'>fRectTypes</a>;
        <a href='#SkCanvas_Lattice'>int</a> <a href='#SkCanvas_Lattice'>fXCount</a>;
        <a href='#SkCanvas_Lattice'>int</a> <a href='#SkCanvas_Lattice'>fYCount</a>;
        <a href='#SkCanvas_Lattice'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>fBounds</a>;
        <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a>* <a href='SkColor_Reference#SkColor'>fColors</a>;
    };

    <a href='SkColor_Reference#SkColor'>void</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a>(<a href='#SkCanvas_drawBitmapLattice'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='#SkCanvas_Lattice'>Lattice</a>& <a href='#SkCanvas_Lattice'>lattice</a>, <a href='#SkCanvas_Lattice'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                           <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a>(<a href='#SkCanvas_drawImageLattice'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='#SkCanvas_Lattice'>Lattice</a>& <a href='#SkCanvas_Lattice'>lattice</a>, <a href='#SkCanvas_Lattice'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                          <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>);

    <a href='SkPaint_Reference#Paint'>enum</a> <a href='#SkCanvas_QuadAAFlags'>QuadAAFlags</a> : <a href='#SkCanvas_QuadAAFlags'>unsigned</a> {
        <a href='#SkCanvas_kLeft_QuadAAFlag'>kLeft_QuadAAFlag</a> = 0<a href='#SkCanvas_kLeft_QuadAAFlag'>b0001</a>,
        <a href='#SkCanvas_kTop_QuadAAFlag'>kTop_QuadAAFlag</a> = 0<a href='#SkCanvas_kTop_QuadAAFlag'>b0010</a>,
        <a href='#SkCanvas_kRight_QuadAAFlag'>kRight_QuadAAFlag</a> = 0<a href='#SkCanvas_kRight_QuadAAFlag'>b0100</a>,
        <a href='#SkCanvas_kBottom_QuadAAFlag'>kBottom_QuadAAFlag</a> = 0<a href='#SkCanvas_kBottom_QuadAAFlag'>b1000</a>,
        <a href='#SkCanvas_kNone_QuadAAFlags'>kNone_QuadAAFlags</a> = 0<a href='#SkCanvas_kNone_QuadAAFlags'>b0000</a>,
        <a href='#SkCanvas_kAll_QuadAAFlags'>kAll_QuadAAFlags</a> = 0<a href='#SkCanvas_kAll_QuadAAFlags'>b1111</a>,
    };

    <a href='#SkCanvas_kAll_QuadAAFlags'>struct</a> <a href='#SkCanvas_ImageSetEntry'>ImageSetEntry</a> {
        <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#sk_sp'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='SkImage_Reference#SkImage'>fImage</a>;
        <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>fSrcRect</a>;
        <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>fDstRect</a>;
        <a href='SkRect_Reference#SkRect'>unsigned</a> <a href='SkRect_Reference#SkRect'>fAAFlags</a>;
    };

    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkCanvas_experimental_DrawImageSetV0'>experimental_DrawImageSetV0</a>(<a href='#SkCanvas_experimental_DrawImageSetV0'>const</a> <a href='#SkCanvas_ImageSetEntry'>ImageSetEntry</a> <a href='#SkCanvas_ImageSetEntry'>imageSet</a>[], <a href='#SkCanvas_ImageSetEntry'>int</a> <a href='#SkCanvas_ImageSetEntry'>cnt</a>, <a href='#SkCanvas_ImageSetEntry'>float</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
                                     <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>quality</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>);
    <a href='SkBlendMode_Reference#SkBlendMode'>void</a> <a href='#SkCanvas_drawText'>drawText</a>(<a href='#SkCanvas_drawText'>const</a> <a href='#SkCanvas_drawText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                  <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawString'>drawString</a>(<a href='#SkCanvas_drawString'>const</a> <a href='#SkCanvas_drawString'>char</a>* <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawString'>drawString</a>(<a href='#SkCanvas_drawString'>const</a> <a href='undocumented#SkString'>SkString</a>& <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPosText'>drawPosText</a>(<a href='#SkCanvas_drawPosText'>const</a> <a href='#SkCanvas_drawPosText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pos</a>[],
                     <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>(<a href='#SkCanvas_drawPosTextH'>const</a> <a href='#SkCanvas_drawPosTextH'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xpos</a>[], <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>constY</a>,
                      <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>(<a href='#SkCanvas_drawTextRSXform'>const</a> <a href='#SkCanvas_drawTextRSXform'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[],
                         <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(<a href='#SkCanvas_drawTextBlob'>const</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* <a href='SkTextBlob_Reference#SkTextBlob'>blob</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(<a href='#SkCanvas_drawTextBlob'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>>& <a href='SkTextBlob_Reference#SkTextBlob'>blob</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>);
    <a href='SkPicture_Reference#Picture'>void</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>>& <a href='SkPicture_Reference#Picture'>picture</a>);
    <a href='SkPicture_Reference#Picture'>void</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>>& <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>,
                     <a href='SkMatrix_Reference#Matrix'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkVertices'>SkVertices</a>>& <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, <a href='undocumented#Vertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> <a href='#SkVertices_Bone'>bones</a>[], <a href='#SkVertices_Bone'>int</a> <a href='#SkVertices_Bone'>boneCount</a>,
                      <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkVertices'>SkVertices</a>>& <a href='undocumented#Vertices'>vertices</a>, <a href='undocumented#Vertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> <a href='#SkVertices_Bone'>bones</a>[],
                      <a href='#SkVertices_Bone'>int</a> <a href='#SkVertices_Bone'>boneCount</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPatch'>drawPatch</a>(<a href='#SkCanvas_drawPatch'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], <a href='SkPath_Reference#Cubic'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[4],
                   <a href='SkColor_Reference#SkColor'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>texCoords</a>[4], <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawPatch'>drawPatch</a>(<a href='#SkCanvas_drawPatch'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], <a href='SkPath_Reference#Cubic'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[4],
                   <a href='SkColor_Reference#SkColor'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>texCoords</a>[4], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[],
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[], <a href='SkColor_Reference#SkColor'>int</a> <a href='SkColor_Reference#SkColor'>count</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[],
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[], <a href='SkColor_Reference#SkColor'>int</a> <a href='SkColor_Reference#SkColor'>count</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[], <a href='SkRect_Reference#SkRect'>int</a> <a href='SkRect_Reference#SkRect'>count</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>>& <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[],
                   <a href='SkRect_Reference#SkRect'>int</a> <a href='SkRect_Reference#SkRect'>count</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>void</a> <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, <a href='undocumented#Drawable'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a> = <a href='SkMatrix_Reference#Matrix'>nullptr</a>);
    <a href='SkMatrix_Reference#Matrix'>void</a> <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(<a href='#SkCanvas_drawAnnotation'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkRect_Reference#Rect'>char</a> <a href='SkRect_Reference#Rect'>key</a>[], <a href='undocumented#SkData'>SkData</a>* <a href='undocumented#SkData'>value</a>);
    <a href='undocumented#SkData'>void</a> <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(<a href='#SkCanvas_drawAnnotation'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkRect_Reference#Rect'>char</a> <a href='SkRect_Reference#Rect'>key</a>[], <a href='SkRect_Reference#Rect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>>& <a href='undocumented#SkData'>value</a>);
    <a href='undocumented#SkData'>virtual</a> <a href='undocumented#SkData'>bool</a> <a href='#SkCanvas_isClipEmpty'>isClipEmpty</a>() <a href='#SkCanvas_isClipEmpty'>const</a>;
    <a href='#SkCanvas_isClipEmpty'>virtual</a> <a href='#SkCanvas_isClipEmpty'>bool</a> <a href='#SkCanvas_isClipRect'>isClipRect</a>() <a href='#SkCanvas_isClipRect'>const</a>;
    <a href='#SkCanvas_isClipRect'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a>() <a href='#SkCanvas_getTotalMatrix'>const</a>;
};
</pre>

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>provides</a> <a href='SkCanvas_Reference#Canvas'>an</a> <a href='SkCanvas_Reference#Canvas'>interface</a> <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>, <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>how</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>drawing</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>clipped</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>transformed</a>.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>contains</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>stack</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>values</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>together</a> <a href='SkPaint_Reference#Paint'>provide</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkPaint_Reference#Paint'>state</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>draw</a> <a href='SkPaint_Reference#Paint'>into</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>or</a> <a href='undocumented#Device'>Device</a>.
<a href='undocumented#Device'>Each</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>draw</a> <a href='SkCanvas_Reference#Canvas'>call</a> <a href='SkCanvas_Reference#Canvas'>transforms</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>geometry</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>object</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>concatenation</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>all</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>values</a> <a href='SkMatrix_Reference#Matrix'>in</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>stack</a>. <a href='SkMatrix_Reference#Matrix'>The</a> <a href='SkMatrix_Reference#Matrix'>transformed</a> <a href='SkMatrix_Reference#Matrix'>geometry</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>clipped</a> <a href='SkMatrix_Reference#Matrix'>by</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>intersection</a>
<a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>all</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>values</a> <a href='SkMatrix_Reference#Matrix'>in</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>stack</a>. <a href='SkMatrix_Reference#Matrix'>The</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>draw</a> <a href='SkCanvas_Reference#Canvas'>calls</a> <a href='SkCanvas_Reference#Canvas'>use</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkPaint_Reference#Paint'>supply</a> <a href='SkPaint_Reference#Paint'>drawing</a>
<a href='SkPaint_Reference#Paint'>state</a> <a href='SkPaint_Reference#Paint'>such</a> <a href='SkPaint_Reference#Paint'>as</a> <a href='SkColor_Reference#Color'>Color</a>, <a href='undocumented#Typeface'>Typeface</a>, <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>stroke</a> <a href='undocumented#Size'>width</a>, <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>and</a> <a href='undocumented#Shader'>so</a> <a href='undocumented#Shader'>on</a>.

<a href='undocumented#Shader'>To</a> <a href='undocumented#Shader'>draw</a> <a href='undocumented#Shader'>to</a> <a href='undocumented#Shader'>a</a> <a href='undocumented#Shader'>pixel-based</a> <a href='undocumented#Shader'>destination</a>, <a href='undocumented#Shader'>create</a> <a href='#Raster_Surface'>Raster_Surface</a> <a href='#Raster_Surface'>or</a> <a href='#GPU_Surface'>GPU_Surface</a>.
<a href='#GPU_Surface'>Request</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>from</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>obtain</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>interface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>draw</a>.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>generated</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='#Raster_Surface'>Raster_Surface</a> <a href='#Raster_Surface'>draws</a> <a href='#Raster_Surface'>to</a> <a href='#Raster_Surface'>memory</a> <a href='#Raster_Surface'>visible</a> <a href='#Raster_Surface'>to</a> <a href='#Raster_Surface'>the</a> <a href='#Raster_Surface'>CPU</a>.
<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>generated</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='#GPU_Surface'>GPU_Surface</a> <a href='#GPU_Surface'>uses</a> <a href='#GPU_Surface'>Vulkan</a> <a href='#GPU_Surface'>or</a> <a href='#GPU_Surface'>OpenGL</a> <a href='#GPU_Surface'>to</a> <a href='#GPU_Surface'>draw</a> <a href='#GPU_Surface'>to</a> <a href='#GPU_Surface'>the</a> <a href='#GPU_Surface'>GPU</a>.

<a href='#GPU_Surface'>To</a> <a href='#GPU_Surface'>draw</a> <a href='#GPU_Surface'>to</a> <a href='#GPU_Surface'>a</a> <a href='undocumented#Document'>document</a>, <a href='undocumented#Document'>obtain</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>from</a> <a href='#SVG_Canvas'>SVG_Canvas</a>, <a href='#Document_PDF'>Document_PDF</a>, <a href='#Document_PDF'>or</a> <a href='#Picture_Recorder'>Picture_Recorder</a>.
<a href='undocumented#Document'>Document</a> <a href='undocumented#Document'>based</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>other</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>subclasses</a> <a href='SkCanvas_Reference#Canvas'>reference</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>describing</a> <a href='undocumented#Device'>the</a>
<a href='undocumented#Device'>destination</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>be</a> <a href='SkCanvas_Reference#Canvas'>constructed</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>draw</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>without</a> <a href='SkBitmap_Reference#Bitmap'>first</a> <a href='SkBitmap_Reference#Bitmap'>creating</a> <a href='#Raster_Surface'>Raster_Surface</a>.
<a href='#Raster_Surface'>This</a> <a href='#Raster_Surface'>approach</a> <a href='#Raster_Surface'>may</a> <a href='#Raster_Surface'>be</a> <a href='#Raster_Surface'>deprecated</a> <a href='#Raster_Surface'>in</a> <a href='#Raster_Surface'>the</a> <a href='#Raster_Surface'>future</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>may</a> <a href='SkCanvas_Reference#Canvas'>be</a> <a href='SkCanvas_Reference#Canvas'>created</a> <a href='SkCanvas_Reference#Canvas'>directly</a> <a href='SkCanvas_Reference#Canvas'>when</a> <a href='SkCanvas_Reference#Canvas'>no</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>required</a>; <a href='SkSurface_Reference#Surface'>some</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>methods</a>
<a href='SkCanvas_Reference#Canvas'>implicitly</a> <a href='SkCanvas_Reference#Canvas'>create</a> <a href='#Raster_Surface'>Raster_Surface</a>.

<a name='SkCanvas_MakeRasterDirect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&<a href='SkCanvas_Reference#SkCanvas'>gt</a>; <a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a>(<a href='#SkCanvas_MakeRasterDirect'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>,
                                                  <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                                  <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Allocates raster <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>will</a> <a href='SkCanvas_Reference#SkCanvas'>draw</a> <a href='SkCanvas_Reference#SkCanvas'>directly</a> <a href='SkCanvas_Reference#SkCanvas'>into</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a>.

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>if</a> <a href='SkCanvas_Reference#SkCanvas'>all</a> <a href='SkCanvas_Reference#SkCanvas'>parameters</a> <a href='SkCanvas_Reference#SkCanvas'>are</a> <a href='SkCanvas_Reference#SkCanvas'>valid</a>.
Valid parameters include:
<a href='#SkCanvas_MakeRasterDirect_info'>info</a> <a href='#SkCanvas_MakeRasterDirect_info'>dimensions</a> <a href='#SkCanvas_MakeRasterDirect_info'>are</a> <a href='#SkCanvas_MakeRasterDirect_info'>zero</a> <a href='#SkCanvas_MakeRasterDirect_info'>or</a> <a href='#SkCanvas_MakeRasterDirect_info'>positive</a>;
<a href='#SkCanvas_MakeRasterDirect_info'>info</a> <a href='#SkCanvas_MakeRasterDirect_info'>contains</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>supported</a> <a href='SkImageInfo_Reference#SkAlphaType'>by</a>  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>is</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>not</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>nullptr</a>;
<a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>is</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>zero</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>or</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>large</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>enough</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>to</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>contain</a> <a href='#SkCanvas_MakeRasterDirect_info'>info</a> <a href='#SkCanvas_MakeRasterDirect_info'>width</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>of</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

Pass zero for <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>to</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>compute</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>from</a> <a href='#SkCanvas_MakeRasterDirect_info'>info</a> <a href='#SkCanvas_MakeRasterDirect_info'>width</a> <a href='#SkCanvas_MakeRasterDirect_info'>and</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a>.
If <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>is</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>greater</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>than</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>zero</a>, <a href='#SkCanvas_MakeRasterDirect_rowBytes'>it</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>must</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>be</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>equal</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>to</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>or</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>greater</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>than</a>
<a href='#SkCanvas_MakeRasterDirect_info'>info</a> <a href='#SkCanvas_MakeRasterDirect_info'>width</a> <a href='#SkCanvas_MakeRasterDirect_info'>times</a> <a href='#SkCanvas_MakeRasterDirect_info'>bytes</a> <a href='#SkCanvas_MakeRasterDirect_info'>required</a> <a href='#SkCanvas_MakeRasterDirect_info'>for</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>buffer</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>should</a> <a href='undocumented#Size'>be</a> <a href='#SkCanvas_MakeRasterDirect_info'>info</a> <a href='#SkCanvas_MakeRasterDirect_info'>height</a> <a href='#SkCanvas_MakeRasterDirect_info'>times</a> <a href='#SkCanvas_MakeRasterDirect_info'>computed</a> <a href='#SkCanvas_MakeRasterDirect_rowBytes'>rowBytes</a>.
Pixels are not initialized.
To access <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>after</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>drawing</a>, <a href='#SkCanvas_MakeRasterDirect_pixels'>call</a> <a href='#SkCanvas_flush'>flush()</a> <a href='#SkCanvas_flush'>or</a> <a href='#SkCanvas_peekPixels'>peekPixels</a>().

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirect_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>of</a>  <a href='undocumented#Raster_Surface'>raster surface</a>;</td>
  </tr>
</table>

width, or height, or both, may be zero

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkCanvas_MakeRasterDirect_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirect_pixels'>buffer</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>row</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>next</a>, <a href='SkSurface_Reference#SkSurface'>or</a> <a href='SkSurface_Reference#SkSurface'>zero</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirect_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>if</a> <a href='SkCanvas_Reference#SkCanvas'>all</a> <a href='SkCanvas_Reference#SkCanvas'>parameters</a> <a href='SkCanvas_Reference#SkCanvas'>are</a> <a href='SkCanvas_Reference#SkCanvas'>valid</a>; <a href='SkCanvas_Reference#SkCanvas'>otherwise</a>, <a href='SkCanvas_Reference#SkCanvas'>nullptr</a>

### Example

<div><fiddle-embed name="525285073aae7e53eb8f454a398f880c"><div>Allocates a three by three <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>clears</a> <a href='SkBitmap_Reference#Bitmap'>it</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>white</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draws</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>black</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>center</a>.
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
static std::unique_ptr&lt;<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>&<a href='SkCanvas_Reference#SkCanvas'>gt</a>; <a href='#SkCanvas_MakeRasterDirectN32'>MakeRasterDirectN32</a>(<a href='#SkCanvas_MakeRasterDirectN32'>int</a> <a href='#SkCanvas_MakeRasterDirectN32'>width</a>, <a href='#SkCanvas_MakeRasterDirectN32'>int</a> <a href='#SkCanvas_MakeRasterDirectN32'>height</a>, <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>* <a href='SkColor_Reference#SkPMColor'>pixels</a>,
                                                     <a href='SkColor_Reference#SkPMColor'>size_t</a> <a href='SkColor_Reference#SkPMColor'>rowBytes</a>)
</pre>

Allocates raster <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>specified</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkCanvas_Reference#SkCanvas'>inline</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>specification</a>. <a href='SkImage_Reference#Image'>Subsequent</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
calls draw into <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a>.
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>set</a> <a href='SkImageInfo_Reference#SkColorType'>to</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#SkAlphaType'>set</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.
To access <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>after</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>drawing</a>, <a href='#SkCanvas_MakeRasterDirectN32_pixels'>call</a> <a href='#SkCanvas_flush'>flush()</a> <a href='#SkCanvas_flush'>or</a> <a href='#SkCanvas_peekPixels'>peekPixels</a>().

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>if</a> <a href='SkCanvas_Reference#SkCanvas'>all</a> <a href='SkCanvas_Reference#SkCanvas'>parameters</a> <a href='SkCanvas_Reference#SkCanvas'>are</a> <a href='SkCanvas_Reference#SkCanvas'>valid</a>.
Valid parameters include:
<a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>and</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>height</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>are</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>zero</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>or</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>positive</a>;
<a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>is</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>not</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>nullptr</a>;
<a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>is</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>zero</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>or</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>large</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>enough</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>to</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>contain</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>of</a> <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.

Pass zero for <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>to</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>compute</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>from</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>and</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a>.
If <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>is</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>greater</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>than</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>zero</a>, <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>it</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>must</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>be</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>equal</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>to</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>or</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>greater</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>than</a>
<a href='#SkCanvas_MakeRasterDirectN32_width'>width</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>times</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>bytes</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>required</a> <a href='#SkCanvas_MakeRasterDirectN32_width'>for</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>buffer</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>should</a> <a href='undocumented#Size'>be</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>height</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>times</a> <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>on</a>  <a href='undocumented#Raster_Surface'>raster surface</a> <a href='undocumented#Pixel'>created</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>on</a>  <a href='undocumented#Raster_Surface'>raster surface</a> <a href='undocumented#Pixel'>created</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkCanvas_MakeRasterDirectN32_pixels'>pixels</a> <a href='#SkCanvas_MakeRasterDirectN32_pixels'>buffer</a>; <a href='#SkCanvas_MakeRasterDirectN32_pixels'>buffer</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>should</a> <a href='undocumented#Size'>be</a> <a href='#SkCanvas_MakeRasterDirectN32_height'>height</a></td>
  </tr>
</table>

times <a href='#SkCanvas_MakeRasterDirectN32_rowBytes'>rowBytes</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_MakeRasterDirectN32_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>row</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>next</a>, <a href='SkSurface_Reference#SkSurface'>or</a> <a href='SkSurface_Reference#SkSurface'>zero</a></td>
  </tr>
</table>

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>if</a> <a href='SkCanvas_Reference#SkCanvas'>all</a> <a href='SkCanvas_Reference#SkCanvas'>parameters</a> <a href='SkCanvas_Reference#SkCanvas'>are</a> <a href='SkCanvas_Reference#SkCanvas'>valid</a>; <a href='SkCanvas_Reference#SkCanvas'>otherwise</a>, <a href='SkCanvas_Reference#SkCanvas'>nullptr</a>

### Example

<div><fiddle-embed name="87f55e62ec4c3535e1a5d0f1415b20c6"><div>Allocates a three by three <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>clears</a> <a href='SkBitmap_Reference#Bitmap'>it</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>white</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draws</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>black</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>center</a>.
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

Creates an empty <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>no</a> <a href='SkCanvas_Reference#SkCanvas'>backing</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>pixels</a>, <a href='undocumented#Device'>with</a>
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
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>int</a> <a href='SkCanvas_Reference#SkCanvas'>width</a>, <a href='SkCanvas_Reference#SkCanvas'>int</a> <a href='SkCanvas_Reference#SkCanvas'>height</a>, <a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Creates <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>of</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkCanvas_Reference#SkCanvas'>specified</a> <a href='SkCanvas_Reference#SkCanvas'>dimensions</a> <a href='SkCanvas_Reference#SkCanvas'>without</a> <a href='SkCanvas_Reference#SkCanvas'>a</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.
Used by subclasses with custom implementations for draw member functions.

If <a href='#SkCanvas_int_int_const_SkSurfaceProps_star_props'>props</a> <a href='#SkCanvas_int_int_const_SkSurfaceProps_star_props'>equals</a> <a href='#SkCanvas_int_int_const_SkSurfaceProps_star_props'>nullptr</a>, <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkSurfaceProps'>are</a> <a href='undocumented#SkSurfaceProps'>created</a> <a href='undocumented#SkSurfaceProps'>with</a>
<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>::<a href='#SkSurfaceProps_InitType'>InitType</a> <a href='#SkSurfaceProps_InitType'>settings</a>, <a href='#SkSurfaceProps_InitType'>which</a> <a href='#SkSurfaceProps_InitType'>choose</a> <a href='#SkSurfaceProps_InitType'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>striping</a>
direction and order. Since a platform may dynamically change its direction when
the <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>rotated</a>, <a href='undocumented#Device'>and</a> <a href='undocumented#Device'>since</a> <a href='undocumented#Device'>a</a> <a href='undocumented#Device'>platform</a> <a href='undocumented#Device'>may</a> <a href='undocumented#Device'>have</a> <a href='undocumented#Device'>multiple</a> <a href='undocumented#Device'>monitors</a> <a href='undocumented#Device'>with</a>
different characteristics, it is best not to rely on this legacy behavior.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_width'><code><strong>width</strong></code></a></td>
    <td>zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_height'><code><strong>height</strong></code></a></td>
    <td>zero or greater</td>
  </tr>
  <tr>    <td><a name='SkCanvas_int_int_const_SkSurfaceProps_star_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>placeholder</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>dimensions</a>

### Example

<div><fiddle-embed name="ce6a5ef2df447970b4453489d9d67930">

#### Example Output

~~~~
canvas is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkPixelGeometry'>SkPixelGeometry</a> <a href='undocumented#SkCreateColorSpaceXformCanvas'>SkCreateColorSpaceXformCanvas</a>

<a name='SkCanvas_copy_SkBaseDevice'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
explicit <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkBaseDevice'>SkBaseDevice</a>&<a href='undocumented#SkBaseDevice'>gt</a>; <a href='undocumented#Device'>device</a>)
</pre>

To be deprecated soon.

<a name='SkCanvas_copy_const_SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
explicit <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>)
</pre>

Constructs a <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>that</a> <a href='SkCanvas_Reference#Canvas'>draws</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a>.
Sets <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>::<a href='#SkSurfaceProps_kLegacyFontHost_InitType'>kLegacyFontHost_InitType</a> <a href='#SkSurfaceProps_kLegacyFontHost_InitType'>in</a> <a href='#SkSurfaceProps_kLegacyFontHost_InitType'>constructed</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>copied</a> <a href='SkBitmap_Reference#SkBitmap'>so</a> <a href='SkBitmap_Reference#SkBitmap'>that</a> <a href='SkBitmap_Reference#SkBitmap'>subsequently</a> <a href='SkBitmap_Reference#SkBitmap'>editing</a> <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>will</a> <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>not</a> <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>affect</a>
constructed <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

May be deprecated in the future.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_copy_const_SkBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='undocumented#Pixel'>pixel</a></td>
  </tr>
</table>

storage of  <a href='undocumented#Raster_Surface'>raster surface</a>

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>can</a> <a href='SkCanvas_Reference#SkCanvas'>be</a> <a href='SkCanvas_Reference#SkCanvas'>used</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>draw</a> <a href='SkCanvas_Reference#SkCanvas'>into</a> <a href='#SkCanvas_copy_const_SkBitmap_bitmap'>bitmap</a>

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

<a name='SkCanvas_ColorBehavior'></a>

---

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
Is a placeholder to allow specialized constructor; has no meaning.
</td>
  </tr>
</table>

<a name='SkCanvas_const_SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='#SkCanvas_ColorBehavior'>ColorBehavior</a> <a href='#SkCanvas_ColorBehavior'>behavior</a>)
</pre>

For use by Android framework only.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>specifies a <a href='#SkCanvas_const_SkBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_const_SkBitmap_bitmap'>for</a> <a href='#SkCanvas_const_SkBitmap_bitmap'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>draw</a> <a href='SkCanvas_Reference#Canvas'>into</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_const_SkBitmap_behavior'><code><strong>behavior</strong></code></a></td>
    <td>specializes this constructor; value is unused</td>
  </tr>
</table>

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>can</a> <a href='SkCanvas_Reference#SkCanvas'>be</a> <a href='SkCanvas_Reference#SkCanvas'>used</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>draw</a> <a href='SkCanvas_Reference#SkCanvas'>into</a> <a href='#SkCanvas_const_SkBitmap_bitmap'>bitmap</a>

<a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>(<a href='SkCanvas_Reference#SkCanvas'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='undocumented#SkSurfaceProps'>props</a>)
</pre>

Constructs a <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>that</a> <a href='SkCanvas_Reference#Canvas'>draws</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a>.
Use <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'>props</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'>to</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'>match</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'>the</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>characteristics</a>, <a href='undocumented#Device'>like</a> <a href='undocumented#Device'>LCD</a> <a href='undocumented#Device'>striping</a>.

<a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>is</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>copied</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>so</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>that</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>subsequently</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>editing</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>will</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>not</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>affect</a>
constructed <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>,</td>
  </tr>
</table>

and  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>of</a>  <a href='undocumented#Raster_Surface'>raster surface</a>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_const_SkBitmap_const_SkSurfaceProps_props'><code><strong>props</strong></code></a></td>
    <td>order and orientation of RGB striping; and whether to use</td>
  </tr>
</table>

<a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>

### Return Value

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>can</a> <a href='SkCanvas_Reference#SkCanvas'>be</a> <a href='SkCanvas_Reference#SkCanvas'>used</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>draw</a> <a href='SkCanvas_Reference#SkCanvas'>into</a> <a href='#SkCanvas_const_SkBitmap_const_SkSurfaceProps_bitmap'>bitmap</a>

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
virtual ~<a href='#SkCanvas_empty_constructor'>SkCanvas()</a>
</pre>

Draws saved <a href='SkCanvas_Reference#Layer'>layers</a>, <a href='SkCanvas_Reference#Layer'>if</a> <a href='SkCanvas_Reference#Layer'>any</a>.
Frees up resources used by <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

### Example

<div><fiddle-embed name="b7bc91ff16c9b9351b2a127f35394b82"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>draws</a> <a href='SkCanvas_Reference#Layer'>into</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>. <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_saveLayerAlpha'>sets</a> <a href='#SkCanvas_saveLayerAlpha'>up</a> <a href='#SkCanvas_saveLayerAlpha'>an</a> <a href='#SkCanvas_saveLayerAlpha'>additional</a>
<a href='#SkCanvas_saveLayerAlpha'>drawing</a> <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>that</a> <a href='SkSurface_Reference#Surface'>blends</a> <a href='SkSurface_Reference#Surface'>with</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>. <a href='SkBitmap_Reference#Bitmap'>When</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>goes</a> <a href='SkCanvas_Reference#Layer'>out</a> <a href='SkCanvas_Reference#Layer'>of</a>
<a href='SkCanvas_Reference#Layer'>scope</a>, <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>destructor</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>called</a>. <a href='SkCanvas_Reference#Layer'>The</a> <a href='SkCanvas_Reference#Layer'>saved</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>restored</a>, <a href='SkCanvas_Reference#Layer'>drawing</a>
<a href='SkCanvas_Reference#Layer'>transparent</a> <a href='SkCanvas_Reference#Layer'>letters</a>.
</div></fiddle-embed></div>

### See Also

<a href='#Canvas_State_Stack'>State_Stack</a>

<a name='Property'></a>

<a name='SkCanvas_getMetaData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkMetaData'>SkMetaData</a>& <a href='#SkCanvas_getMetaData'>getMetaData</a>()
</pre>

Returns storage to associate additional <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>with</a> <a href='undocumented#Data'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a>.
The storage is freed when <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>deleted</a>.

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
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>() <a href='#SkCanvas_imageInfo'>const</a>
</pre>

Returns <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>for</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>. <a href='SkCanvas_Reference#SkCanvas'>If</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>not</a> <a href='SkCanvas_Reference#SkCanvas'>associated</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>raster</a> <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>or</a>
GPU <a href='SkSurface_Reference#Surface'>surface</a>, <a href='SkSurface_Reference#Surface'>returned</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>set</a> <a href='SkImageInfo_Reference#SkColorType'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

dimensions and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>of</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>

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
bool <a href='#SkCanvas_getProps'>getProps</a>(<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a>) <a href='undocumented#SkSurfaceProps'>const</a>
</pre>

Copies <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>, <a href='undocumented#SkSurfaceProps'>if</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>associated</a> <a href='SkCanvas_Reference#SkCanvas'>with</a>  <a href='undocumented#Raster_Surface'>raster surface</a> <a href='SkCanvas_Reference#SkCanvas'>or</a>
<a href='undocumented#GPU_Surface'>GPU surface</a>, and returns true. Otherwise, returns false and leave <a href='#SkCanvas_getProps_props'>props</a> <a href='#SkCanvas_getProps_props'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getProps_props'><code><strong>props</strong></code></a></td>
    <td>storage for writable <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a></td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkSurfaceProps'>was</a> <a href='undocumented#SkSurfaceProps'>copied</a>

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
If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>associated</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>GPU</a> <a href='SkSurface_Reference#Surface'>surface</a>, <a href='SkSurface_Reference#Surface'>resolves</a> <a href='SkSurface_Reference#Surface'>all</a> <a href='SkSurface_Reference#Surface'>pending</a> <a href='SkSurface_Reference#Surface'>GPU</a> <a href='SkSurface_Reference#Surface'>operations</a>.
If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>associated</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>raster</a> <a href='SkSurface_Reference#Surface'>surface</a>, <a href='SkSurface_Reference#Surface'>has</a> <a href='SkSurface_Reference#Surface'>no</a> <a href='SkSurface_Reference#Surface'>effect</a>; <a href='SkSurface_Reference#Surface'>raster</a> <a href='SkSurface_Reference#Surface'>draw</a>
operations are never deferred.

### See Also

<a href='#SkCanvas_peekPixels'>peekPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_flush'>flush</a> <a href='undocumented#GrContext'>GrContext</a>::<a href='#GrContext_flush'>flush</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a> <a href='undocumented#GrContext'>GrContext</a>::<a href='#GrContext_abandonContext'>abandonContext</a>

<a name='SkCanvas_getBaseLayerSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='undocumented#SkISize'>SkISize</a> <a href='#SkCanvas_getBaseLayerSize'>getBaseLayerSize</a>() <a href='#SkCanvas_getBaseLayerSize'>const</a>
</pre>

Gets the <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>base</a> <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>root</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>in</a> <a href='SkCanvas_Reference#Layer'>global</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>coordinates</a>. <a href='SkCanvas_Reference#Canvas'>The</a>
origin of the base <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>always</a> (0,0). <a href='SkCanvas_Reference#Layer'>The</a> <a href='SkCanvas_Reference#Layer'>area</a> <a href='SkCanvas_Reference#Layer'>available</a> <a href='SkCanvas_Reference#Layer'>for</a> <a href='SkCanvas_Reference#Layer'>drawing</a> <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a>
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
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkCanvas_makeSurface'>makeSurface</a>(<a href='#SkCanvas_makeSurface'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Creates <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>matching</a> <a href='#SkCanvas_makeSurface_info'>info</a> <a href='#SkCanvas_makeSurface_info'>and</a> <a href='#SkCanvas_makeSurface_props'>props</a>, <a href='#SkCanvas_makeSurface_props'>and</a> <a href='#SkCanvas_makeSurface_props'>associates</a> <a href='#SkCanvas_makeSurface_props'>it</a> <a href='#SkCanvas_makeSurface_props'>with</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.
Returns nullptr if no match found.

If <a href='#SkCanvas_makeSurface_props'>props</a> <a href='#SkCanvas_makeSurface_props'>is</a> <a href='#SkCanvas_makeSurface_props'>nullptr</a>, <a href='#SkCanvas_makeSurface_props'>matches</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkSurfaceProps'>in</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>. <a href='SkCanvas_Reference#SkCanvas'>If</a> <a href='#SkCanvas_makeSurface_props'>props</a> <a href='#SkCanvas_makeSurface_props'>is</a> <a href='#SkCanvas_makeSurface_props'>nullptr</a> <a href='#SkCanvas_makeSurface_props'>and</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
does not have <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>, <a href='undocumented#SkSurfaceProps'>creates</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>with</a> <a href='SkSurface_Reference#SkSurface'>default</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_makeSurface_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_makeSurface_props'><code><strong>props</strong></code></a></td>
    <td><a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkSurfaceProps'>to</a> <a href='undocumented#SkSurfaceProps'>match</a>; <a href='undocumented#SkSurfaceProps'>may</a> <a href='undocumented#SkSurfaceProps'>be</a> <a href='undocumented#SkSurfaceProps'>nullptr</a> <a href='undocumented#SkSurfaceProps'>to</a> <a href='undocumented#SkSurfaceProps'>match</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>matching</a> <a href='#SkCanvas_makeSurface_info'>info</a> <a href='#SkCanvas_makeSurface_info'>and</a> <a href='#SkCanvas_makeSurface_props'>props</a>, <a href='#SkCanvas_makeSurface_props'>or</a> <a href='#SkCanvas_makeSurface_props'>nullptr</a> <a href='#SkCanvas_makeSurface_props'>if</a> <a href='#SkCanvas_makeSurface_props'>no</a> <a href='#SkCanvas_makeSurface_props'>match</a> <a href='#SkCanvas_makeSurface_props'>is</a> <a href='#SkCanvas_makeSurface_props'>available</a>

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

Returns GPU context of the GPU <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>associated</a> <a href='SkSurface_Reference#Surface'>with</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

### Return Value

GPU context, if available; nullptr otherwise

### Example

<div><fiddle-embed name="c4ea949e5fa5a0630dcb6b0204bd498f"></fiddle-embed></div>

### See Also

<a href='undocumented#GrContext'>GrContext</a>

<a name='SkCanvas_accessTopLayerPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkCanvas_accessTopLayerPixels'>accessTopLayerPixels</a>(<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>* <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a>* <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>origin</a> = <a href='SkIPoint_Reference#SkIPoint'>nullptr</a>)
</pre>

Returns the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>base</a> <a href='undocumented#Pixel'>address</a>, <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='#SkCanvas_accessTopLayerPixels_rowBytes'>rowBytes</a>, <a href='#SkCanvas_accessTopLayerPixels_rowBytes'>and</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>if</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>the</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>pixels</a>
can be read directly. The returned address is only valid
while <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>in</a> <a href='SkCanvas_Reference#SkCanvas'>scope</a> <a href='SkCanvas_Reference#SkCanvas'>and</a> <a href='SkCanvas_Reference#SkCanvas'>unchanged</a>. <a href='SkCanvas_Reference#SkCanvas'>Any</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>call</a> <a href='SkCanvas_Reference#SkCanvas'>or</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>call</a>
may invalidate the returned address and other returned values.

If pixels are inaccessible, <a href='#SkCanvas_accessTopLayerPixels_info'>info</a>, <a href='#SkCanvas_accessTopLayerPixels_rowBytes'>rowBytes</a>, <a href='#SkCanvas_accessTopLayerPixels_rowBytes'>and</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>are</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_info'><code><strong>info</strong></code></a></td>
    <td>storage for writable pixels' <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>; <a href='SkImageInfo_Reference#SkImageInfo'>may</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>storage for writable pixels' row bytes; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkCanvas_accessTopLayerPixels_origin'><code><strong>origin</strong></code></a></td>
    <td>storage for <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>top</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>origin</a>, <a href='#SkCanvas_accessTopLayerPixels_origin'>its</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>top-left</a> <a href='#SkCanvas_accessTopLayerPixels_origin'>corner</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

address of pixels, or nullptr if inaccessible

### Example

<div><fiddle-embed name="38d0d6ca9bea146d31bcbec197856359"></fiddle-embed></div>

### Example

<div><fiddle-embed name="a7ac9c21bbabcdeeca00f72a61cd0f3e"><div>Draws "ABC" on the <a href='undocumented#Device'>device</a>. <a href='undocumented#Device'>Then</a> <a href='undocumented#Device'>draws</a> "<a href='undocumented#Device'>DEF</a>" <a href='undocumented#Device'>in</a> <a href='SkCanvas_Reference#Layer'>Layer</a>, <a href='SkCanvas_Reference#Layer'>and</a> <a href='SkCanvas_Reference#Layer'>reads</a>
<a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>add</a> <a href='SkCanvas_Reference#Layer'>a</a> <a href='SkCanvas_Reference#Layer'>large</a> <a href='SkCanvas_Reference#Layer'>dotted</a> "<a href='SkCanvas_Reference#Layer'>DEF</a>". <a href='SkCanvas_Reference#Layer'>Finally</a> <a href='SkCanvas_Reference#Layer'>blends</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>the</a>
<a href='undocumented#Device'>device</a>.

<a href='undocumented#Device'>The</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>and</a> <a href='SkCanvas_Reference#Layer'>blended</a> <a href='SkCanvas_Reference#Layer'>result</a> <a href='SkCanvas_Reference#Layer'>appear</a> <a href='SkCanvas_Reference#Layer'>on</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>CPU</a> <a href='SkCanvas_Reference#Layer'>and</a> <a href='SkCanvas_Reference#Layer'>GPU</a> <a href='SkCanvas_Reference#Layer'>but</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>large</a> <a href='SkCanvas_Reference#Layer'>dotted</a>
"<a href='SkCanvas_Reference#Layer'>DEF</a>" <a href='SkCanvas_Reference#Layer'>appear</a> <a href='SkCanvas_Reference#Layer'>only</a> <a href='SkCanvas_Reference#Layer'>on</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>CPU</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>

<a name='SkCanvas_accessTopRasterHandle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_Handle'>Handle</a> <a href='#SkCanvas_accessTopRasterHandle'>accessTopRasterHandle</a>() <a href='#SkCanvas_accessTopRasterHandle'>const</a>
</pre>

Returns custom context that tracks the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.

Use <a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a> <a href='undocumented#SkRasterHandleAllocator'>to</a> <a href='undocumented#SkRasterHandleAllocator'>blend</a> <a href='undocumented#SkRasterHandleAllocator'>Skia</a> <a href='undocumented#SkRasterHandleAllocator'>drawing</a> <a href='undocumented#SkRasterHandleAllocator'>with</a> <a href='undocumented#SkRasterHandleAllocator'>custom</a> <a href='undocumented#SkRasterHandleAllocator'>drawing</a>, <a href='undocumented#SkRasterHandleAllocator'>typically</a> <a href='undocumented#SkRasterHandleAllocator'>performed</a>
by the host platform user interface. The custom context returned is generated by
<a href='undocumented#SkRasterHandleAllocator'>SkRasterHandleAllocator</a>::<a href='#SkRasterHandleAllocator_MakeCanvas'>MakeCanvas</a>, <a href='#SkRasterHandleAllocator_MakeCanvas'>which</a> <a href='#SkRasterHandleAllocator_MakeCanvas'>creates</a> <a href='#SkRasterHandleAllocator_MakeCanvas'>a</a> <a href='#SkRasterHandleAllocator_MakeCanvas'>custom</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>with</a> <a href='SkCanvas_Reference#Canvas'>raster</a> <a href='SkCanvas_Reference#Canvas'>storage</a> <a href='SkCanvas_Reference#Canvas'>for</a>
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

Returns true if <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>has</a> <a href='SkCanvas_Reference#SkCanvas'>direct</a> <a href='SkCanvas_Reference#SkCanvas'>access</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>its</a> <a href='SkCanvas_Reference#SkCanvas'>pixels</a>.

Pixels are readable when <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>is</a> <a href='undocumented#SkBaseDevice'>raster</a>. <a href='undocumented#SkBaseDevice'>Pixels</a> <a href='undocumented#SkBaseDevice'>are</a> <a href='undocumented#SkBaseDevice'>not</a> <a href='undocumented#SkBaseDevice'>readable</a> <a href='undocumented#SkBaseDevice'>when</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
is returned from  <a href='undocumented#GPU_Surface'>GPU surface</a>, returned by <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>, <a href='#SkDocument_beginPage'>returned</a> <a href='#SkDocument_beginPage'>by</a>
<a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, <a href='#SkPictureRecorder_beginRecording'>or</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkCanvas_Reference#SkCanvas'>base</a> <a href='SkCanvas_Reference#SkCanvas'>of</a> <a href='SkCanvas_Reference#SkCanvas'>a</a> <a href='SkCanvas_Reference#SkCanvas'>utility</a> <a href='SkCanvas_Reference#SkCanvas'>class</a>
like <a href='undocumented#SkDebugCanvas'>SkDebugCanvas</a>.

<a href='#SkCanvas_peekPixels_pixmap'>pixmap</a> <a href='#SkCanvas_peekPixels_pixmap'>is</a> <a href='#SkCanvas_peekPixels_pixmap'>valid</a> <a href='#SkCanvas_peekPixels_pixmap'>only</a> <a href='#SkCanvas_peekPixels_pixmap'>while</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>in</a> <a href='SkCanvas_Reference#SkCanvas'>scope</a> <a href='SkCanvas_Reference#SkCanvas'>and</a> <a href='SkCanvas_Reference#SkCanvas'>unchanged</a>. <a href='SkCanvas_Reference#SkCanvas'>Any</a>
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>or</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>call</a> <a href='SkSurface_Reference#SkSurface'>may</a> <a href='SkSurface_Reference#SkSurface'>invalidate</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='#SkCanvas_peekPixels_pixmap'>pixmap</a> <a href='#SkCanvas_peekPixels_pixmap'>values</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>state</a> <a href='undocumented#Pixel'>if</a> <a href='undocumented#Pixel'>pixels</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>readable</a>; <a href='undocumented#Pixel'>otherwise</a>, <a href='undocumented#Pixel'>ignored</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>has</a> <a href='SkCanvas_Reference#SkCanvas'>direct</a> <a href='SkCanvas_Reference#SkCanvas'>access</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>pixels</a>

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
bool <a href='#SkCanvas_readPixels'>readPixels</a>(<a href='#SkCanvas_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>are</a>
<a href='SkMatrix_Reference#Matrix'>ignored</a>.

<a href='SkMatrix_Reference#Matrix'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkCanvas_readPixels_srcX'>srcX</a>, <a href='#SkCanvas_readPixels_srcY'>srcY</a>) <a href='#SkCanvas_readPixels_srcY'>and</a> (<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).
<a href='#SkImageInfo_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a>).
<a href='#SkImageInfo_height'>Copies</a> <a href='#SkImageInfo_height'>each</a> <a href='#SkImageInfo_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>and</a> <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>if</a> <a href='#SkImageInfo_alphaType'>required</a>.

<a href='#SkImageInfo_alphaType'>Pixels</a> <a href='#SkImageInfo_alphaType'>are</a> <a href='#SkImageInfo_alphaType'>readable</a> <a href='#SkImageInfo_alphaType'>when</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>raster</a>, <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>backed</a> <a href='undocumented#Device'>by</a> <a href='undocumented#Device'>a</a> <a href='undocumented#Device'>GPU</a>.
<a href='undocumented#Device'>Pixels</a> <a href='undocumented#Device'>are</a> <a href='undocumented#Device'>not</a> <a href='undocumented#Device'>readable</a> <a href='undocumented#Device'>when</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
<a href='#SkDocument_beginPage'>returned</a> <a href='#SkDocument_beginPage'>by</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, <a href='#SkPictureRecorder_beginRecording'>or</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>base</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>utility</a>
<a href='SkCanvas_Reference#Canvas'>class</a> <a href='SkCanvas_Reference#Canvas'>like</a> <a href='undocumented#SkDebugCanvas'>SkDebugCanvas</a>.

<a href='undocumented#SkDebugCanvas'>The</a> <a href='undocumented#SkDebugCanvas'>destination</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>allocated</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>caller</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='#Image_Info_Alpha_Type'>rectangles</a>
<a href='#Image_Info_Alpha_Type'>are</a> <a href='#Image_Info_Alpha_Type'>copied</a>. <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a> <a href='#SkCanvas_readPixels_dstPixels'>contents</a> <a href='#SkCanvas_readPixels_dstPixels'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkCanvas_readPixels_srcX'>srcX</a> <a href='#SkCanvas_readPixels_srcX'>or</a> <a href='#SkCanvas_readPixels_srcY'>srcY</a> <a href='#SkCanvas_readPixels_srcY'>to</a> <a href='#SkCanvas_readPixels_srcY'>offset</a> <a href='#SkCanvas_readPixels_srcY'>pixels</a> <a href='#SkCanvas_readPixels_srcY'>across</a> <a href='#SkCanvas_readPixels_srcY'>or</a> <a href='#SkCanvas_readPixels_srcY'>down</a> <a href='#SkCanvas_readPixels_srcY'>destination</a>.

<a href='#SkCanvas_readPixels_srcY'>Does</a> <a href='#SkCanvas_readPixels_srcY'>not</a> <a href='#SkCanvas_readPixels_srcY'>copy</a>, <a href='#SkCanvas_readPixels_srcY'>and</a> <a href='#SkCanvas_readPixels_srcY'>returns</a> <a href='#SkCanvas_readPixels_srcY'>false</a> <a href='#SkCanvas_readPixels_srcY'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>could</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>be</a> <a href='SkCanvas_Reference#Canvas'>converted</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>or</a> <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>are</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>readable</a>; <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>instance</a>, <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>document-based</a>.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkCanvas_readPixels_dstRowBytes'>is</a> <a href='#SkCanvas_readPixels_dstRowBytes'>too</a> <a href='#SkCanvas_readPixels_dstRowBytes'>small</a> <a href='#SkCanvas_readPixels_dstRowBytes'>to</a> <a href='#SkCanvas_readPixels_dstRowBytes'>contain</a> <a href='#SkCanvas_readPixels_dstRowBytes'>one</a> <a href='#SkCanvas_readPixels_dstRowBytes'>row</a> <a href='#SkCanvas_readPixels_dstRowBytes'>of</a> <a href='#SkCanvas_readPixels_dstRowBytes'>pixels</a>.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>of</a> <a href='#SkCanvas_readPixels_dstPixels'>dstPixels</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels; <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>times</a> <a href='#SkCanvas_readPixels_dstRowBytes'>dstRowBytes</a>, <a href='#SkCanvas_readPixels_dstRowBytes'>or</a> <a href='#SkCanvas_readPixels_dstRowBytes'>larger</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>one</a> <a href='undocumented#Size'>destination</a> <a href='undocumented#Size'>row</a>; <a href='#SkCanvas_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>larger</a></td>
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

<div><fiddle-embed name="102d014d7f753db2a9b9ee08893aaf11"><div>A black <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>drawn</a> <a href='undocumented#Circle'>on</a> <a href='undocumented#Circle'>a</a> <a href='undocumented#Circle'>blue</a> <a href='undocumented#Circle'>background</a> <a href='undocumented#Circle'>provides</a> <a href='undocumented#Circle'>an</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>copy</a>.
<a href='#SkCanvas_readPixels'>readPixels</a> <a href='#SkCanvas_readPixels'>copies</a> <a href='#SkCanvas_readPixels'>one</a> <a href='#SkCanvas_readPixels'>quarter</a> <a href='#SkCanvas_readPixels'>of</a> <a href='#SkCanvas_readPixels'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='SkCanvas_Reference#Canvas'>each</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>four</a> <a href='SkCanvas_Reference#Canvas'>corners</a>.
<a href='SkCanvas_Reference#Canvas'>The</a> <a href='SkCanvas_Reference#Canvas'>copied</a> <a href='SkCanvas_Reference#Canvas'>quarter</a> <a href='undocumented#Circle'>circles</a> <a href='undocumented#Circle'>overdraw</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>original</a> <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="481e990e923a0ed34654f4361b94f096"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>returned</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='#Raster_Surface'>Raster_Surface</a> <a href='#Raster_Surface'>has</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>values</a>.
<a href='#SkCanvas_clear'>clear()</a> <a href='#SkCanvas_clear'>takes</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>input</a> <a href='undocumented#Unpremultiply'>with</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>equal</a> 0<a href='#Color_Alpha'>x80</a>
<a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>equal</a> 0<a href='#Color_Alpha'>x55</a>, 0<a href='#Color_Alpha'>xAA</a>, 0<a href='#Color_Alpha'>xFF</a>. <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>is</a> <a href='#Color_Alpha'>multiplied</a> <a href='#Color_Alpha'>by</a> <a href='#Color_Alpha'>Color_Alpha</a>
<a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>generate</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>value</a> 0<a href='undocumented#Premultiply'>x802B5580</a>. <a href='#SkCanvas_readPixels'>readPixels</a> <a href='#SkCanvas_readPixels'>converts</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>back</a>
<a href='undocumented#Pixel'>to</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>value</a> 0<a href='undocumented#Unpremultiply'>x8056A9FF</a>, <a href='undocumented#Unpremultiply'>introducing</a> <a href='undocumented#Unpremultiply'>error</a>.
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
bool <a href='#SkCanvas_readPixels'>readPixels</a>(<a href='#SkCanvas_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, <a href='SkPixmap_Reference#Pixmap'>int</a> <a href='SkPixmap_Reference#Pixmap'>srcX</a>, <a href='SkPixmap_Reference#Pixmap'>int</a> <a href='SkPixmap_Reference#Pixmap'>srcY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>are</a>
<a href='SkMatrix_Reference#Matrix'>ignored</a>.

<a href='SkMatrix_Reference#Matrix'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkCanvas_readPixels_2_srcX'>srcX</a>, <a href='#SkCanvas_readPixels_2_srcY'>srcY</a>) <a href='#SkCanvas_readPixels_2_srcY'>and</a> (<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).
<a href='#SkImageInfo_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_height'>height()</a>).
<a href='#SkPixmap_height'>Copies</a> <a href='#SkPixmap_height'>each</a> <a href='#SkPixmap_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>and</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>if</a> <a href='#SkPixmap_alphaType'>required</a>.

<a href='#SkPixmap_alphaType'>Pixels</a> <a href='#SkPixmap_alphaType'>are</a> <a href='#SkPixmap_alphaType'>readable</a> <a href='#SkPixmap_alphaType'>when</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>raster</a>, <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>backed</a> <a href='undocumented#Device'>by</a> <a href='undocumented#Device'>a</a> <a href='undocumented#Device'>GPU</a>.
<a href='undocumented#Device'>Pixels</a> <a href='undocumented#Device'>are</a> <a href='undocumented#Device'>not</a> <a href='undocumented#Device'>readable</a> <a href='undocumented#Device'>when</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
<a href='#SkDocument_beginPage'>returned</a> <a href='#SkDocument_beginPage'>by</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, <a href='#SkPictureRecorder_beginRecording'>or</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>base</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>utility</a>
<a href='SkCanvas_Reference#Canvas'>class</a> <a href='SkCanvas_Reference#Canvas'>like</a> <a href='undocumented#SkDebugCanvas'>SkDebugCanvas</a>.

<a href='undocumented#SkDebugCanvas'>Caller</a> <a href='undocumented#SkDebugCanvas'>must</a> <a href='undocumented#SkDebugCanvas'>allocate</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>in</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a> <a href='#SkCanvas_readPixels_2_pixmap'>if</a> <a href='#SkCanvas_readPixels_2_pixmap'>needed</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='SkRect_Reference#Rect'>Rects</a>
<a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>copied</a>.  <a href='SkPixmap_Reference#Pixmap_Pixels'>pixmap pixels</a> <a href='#SkCanvas_readPixels_2_pixmap'>contents</a> <a href='#SkCanvas_readPixels_2_pixmap'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkCanvas_readPixels_2_srcX'>srcX</a> <a href='#SkCanvas_readPixels_2_srcX'>or</a> <a href='#SkCanvas_readPixels_2_srcY'>srcY</a> <a href='#SkCanvas_readPixels_2_srcY'>to</a> <a href='#SkCanvas_readPixels_2_srcY'>offset</a> <a href='#SkCanvas_readPixels_2_srcY'>pixels</a> <a href='#SkCanvas_readPixels_2_srcY'>across</a> <a href='#SkCanvas_readPixels_2_srcY'>or</a> <a href='#SkCanvas_readPixels_2_srcY'>down</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.

<a href='#SkCanvas_readPixels_2_pixmap'>Does</a> <a href='#SkCanvas_readPixels_2_pixmap'>not</a> <a href='#SkCanvas_readPixels_2_pixmap'>copy</a>, <a href='#SkCanvas_readPixels_2_pixmap'>and</a> <a href='#SkCanvas_readPixels_2_pixmap'>returns</a> <a href='#SkCanvas_readPixels_2_pixmap'>false</a> <a href='#SkCanvas_readPixels_2_pixmap'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>could</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>be</a> <a href='SkCanvas_Reference#Canvas'>converted</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>or</a> <a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>are</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>readable</a>; <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>instance</a>, <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>document-based</a>.</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>pixels</a> <a href='SkPixmap_Reference#Pixmap'>could</a> <a href='SkPixmap_Reference#Pixmap'>not</a> <a href='SkPixmap_Reference#Pixmap'>be</a> <a href='SkPixmap_Reference#Pixmap'>allocated</a>.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_2_pixmap'>pixmap</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>too</a> <a href='#SkPixmap_rowBytes'>small</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>contain</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>of</a> <a href='#SkPixmap_rowBytes'>pixels</a>.</td>
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

<div><fiddle-embed name="85f199032943b6483722c34a91c4e20f"><div><a href='#SkCanvas_clear'>clear()</a> <a href='#SkCanvas_clear'>takes</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>input</a> <a href='undocumented#Unpremultiply'>with</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>equal</a> 0<a href='#Color_Alpha'>x80</a>
<a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>equal</a> 0<a href='#Color_Alpha'>x55</a>, 0<a href='#Color_Alpha'>xAA</a>, 0<a href='#Color_Alpha'>xFF</a>. <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>is</a> <a href='#Color_Alpha'>multiplied</a> <a href='#Color_Alpha'>by</a> <a href='#Color_Alpha'>Color_Alpha</a>
<a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>generate</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>value</a> 0<a href='undocumented#Premultiply'>x802B5580</a>.
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
bool <a href='#SkCanvas_readPixels'>readPixels</a>(<a href='#SkCanvas_readPixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>srcX</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>srcY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>are</a>
<a href='SkMatrix_Reference#Matrix'>ignored</a>.

<a href='SkMatrix_Reference#Matrix'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkCanvas_readPixels_3_srcX'>srcX</a>, <a href='#SkCanvas_readPixels_3_srcY'>srcY</a>) <a href='#SkCanvas_readPixels_3_srcY'>and</a> (<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).
<a href='#SkImageInfo_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_width'>width()</a>, <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_height'>height()</a>).
<a href='#SkBitmap_height'>Copies</a> <a href='#SkBitmap_height'>each</a> <a href='#SkBitmap_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>and</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_alphaType'>alphaType</a>() <a href='#SkBitmap_alphaType'>if</a> <a href='#SkBitmap_alphaType'>required</a>.

<a href='#SkBitmap_alphaType'>Pixels</a> <a href='#SkBitmap_alphaType'>are</a> <a href='#SkBitmap_alphaType'>readable</a> <a href='#SkBitmap_alphaType'>when</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>raster</a>, <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>backed</a> <a href='undocumented#Device'>by</a> <a href='undocumented#Device'>a</a> <a href='undocumented#Device'>GPU</a>.
<a href='undocumented#Device'>Pixels</a> <a href='undocumented#Device'>are</a> <a href='undocumented#Device'>not</a> <a href='undocumented#Device'>readable</a> <a href='undocumented#Device'>when</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
<a href='#SkDocument_beginPage'>returned</a> <a href='#SkDocument_beginPage'>by</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, <a href='#SkPictureRecorder_beginRecording'>or</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>base</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>utility</a>
<a href='SkCanvas_Reference#Canvas'>class</a> <a href='SkCanvas_Reference#Canvas'>like</a> <a href='undocumented#SkDebugCanvas'>SkDebugCanvas</a>.

<a href='undocumented#SkDebugCanvas'>Caller</a> <a href='undocumented#SkDebugCanvas'>must</a> <a href='undocumented#SkDebugCanvas'>allocate</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>in</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a> <a href='#SkCanvas_readPixels_3_bitmap'>if</a> <a href='#SkCanvas_readPixels_3_bitmap'>needed</a>.

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>values</a> <a href='SkBitmap_Reference#Bitmap'>are</a> <a href='SkBitmap_Reference#Bitmap'>converted</a> <a href='SkBitmap_Reference#Bitmap'>only</a> <a href='SkBitmap_Reference#Bitmap'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='#Image_Info_Alpha_Type'>rectangles</a>
<a href='#Image_Info_Alpha_Type'>are</a> <a href='#Image_Info_Alpha_Type'>copied</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a> <a href='SkBitmap_Reference#Bitmap'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkCanvas_readPixels_3_srcX'>srcX</a> <a href='#SkCanvas_readPixels_3_srcX'>or</a> <a href='#SkCanvas_readPixels_3_srcY'>srcY</a> <a href='#SkCanvas_readPixels_3_srcY'>to</a> <a href='#SkCanvas_readPixels_3_srcY'>offset</a> <a href='#SkCanvas_readPixels_3_srcY'>pixels</a> <a href='#SkCanvas_readPixels_3_srcY'>across</a> <a href='#SkCanvas_readPixels_3_srcY'>or</a> <a href='#SkCanvas_readPixels_3_srcY'>down</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.

<a href='#SkCanvas_readPixels_3_bitmap'>Does</a> <a href='#SkCanvas_readPixels_3_bitmap'>not</a> <a href='#SkCanvas_readPixels_3_bitmap'>copy</a>, <a href='#SkCanvas_readPixels_3_bitmap'>and</a> <a href='#SkCanvas_readPixels_3_bitmap'>returns</a> <a href='#SkCanvas_readPixels_3_bitmap'>false</a> <a href='#SkCanvas_readPixels_3_bitmap'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>could</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>be</a> <a href='SkCanvas_Reference#Canvas'>converted</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>or</a> <a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>are</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>readable</a>; <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>instance</a>, <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>document-based</a>.</td>
  </tr>  <tr>
    <td><a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a> <a href='#SkCanvas_readPixels_3_bitmap'>could</a> <a href='#SkCanvas_readPixels_3_bitmap'>not</a> <a href='#SkCanvas_readPixels_3_bitmap'>be</a> <a href='#SkCanvas_readPixels_3_bitmap'>allocated</a>.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_readPixels_3_bitmap'>bitmap</a>.<a href='#SkBitmap_rowBytes'>rowBytes</a>() <a href='#SkBitmap_rowBytes'>is</a> <a href='#SkBitmap_rowBytes'>too</a> <a href='#SkBitmap_rowBytes'>small</a> <a href='#SkBitmap_rowBytes'>to</a> <a href='#SkBitmap_rowBytes'>contain</a> <a href='#SkBitmap_rowBytes'>one</a> <a href='#SkBitmap_rowBytes'>row</a> <a href='#SkBitmap_rowBytes'>of</a> <a href='#SkBitmap_rowBytes'>pixels</a>.</td>
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

<div><fiddle-embed name="af6dec8ef974aa67bf102f29915bcd6a"><div><a href='#SkCanvas_clear'>clear()</a> <a href='#SkCanvas_clear'>takes</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>input</a> <a href='undocumented#Unpremultiply'>with</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>equal</a> 0<a href='#Color_Alpha'>x80</a>
<a href='#Color_Alpha'>and</a> <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>equal</a> 0<a href='#Color_Alpha'>x55</a>, 0<a href='#Color_Alpha'>xAA</a>, 0<a href='#Color_Alpha'>xFF</a>. <a href='#Color_Alpha'>RGB</a> <a href='#Color_Alpha'>is</a> <a href='#Color_Alpha'>multiplied</a> <a href='#Color_Alpha'>by</a> <a href='#Color_Alpha'>Color_Alpha</a>
<a href='#Color_Alpha'>to</a> <a href='#Color_Alpha'>generate</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>value</a> 0<a href='undocumented#Premultiply'>x802B5580</a>.
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
bool <a href='#SkCanvas_writePixels'>writePixels</a>(<a href='#SkCanvas_writePixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>x</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>y</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>from</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>ignored</a>.
<a href='SkMatrix_Reference#Matrix'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_height'>height()</a>).
<a href='#SkImageInfo_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkCanvas_writePixels_x'>x</a>, <a href='#SkCanvas_writePixels_y'>y</a>) <a href='#SkCanvas_writePixels_y'>and</a>
(<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).

<a href='#SkImageInfo_height'>Copies</a> <a href='#SkImageInfo_height'>each</a> <a href='#SkImageInfo_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>and</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>if</a> <a href='#SkImageInfo_alphaType'>required</a>.

<a href='#SkImageInfo_alphaType'>Pixels</a> <a href='#SkImageInfo_alphaType'>are</a> <a href='#SkImageInfo_alphaType'>writable</a> <a href='#SkImageInfo_alphaType'>when</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>raster</a>, <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>backed</a> <a href='undocumented#Device'>by</a> <a href='undocumented#Device'>a</a> <a href='undocumented#Device'>GPU</a>.
<a href='undocumented#Device'>Pixels</a> <a href='undocumented#Device'>are</a> <a href='undocumented#Device'>not</a> <a href='undocumented#Device'>writable</a> <a href='undocumented#Device'>when</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
<a href='#SkDocument_beginPage'>returned</a> <a href='#SkDocument_beginPage'>by</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, <a href='#SkPictureRecorder_beginRecording'>or</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>base</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>utility</a>
<a href='SkCanvas_Reference#Canvas'>class</a> <a href='SkCanvas_Reference#Canvas'>like</a> <a href='undocumented#SkDebugCanvas'>SkDebugCanvas</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>within</a> <a href='#SkCanvas_writePixels_pixels'>both</a> <a href='#SkCanvas_writePixels_pixels'>source</a> <a href='#SkCanvas_writePixels_pixels'>and</a> <a href='#SkCanvas_writePixels_pixels'>destination</a> <a href='#SkCanvas_writePixels_pixels'>rectangles</a>
<a href='#SkCanvas_writePixels_pixels'>are</a> <a href='#SkCanvas_writePixels_pixels'>copied</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkCanvas_writePixels_x'>x</a> <a href='#SkCanvas_writePixels_x'>or</a> <a href='#SkCanvas_writePixels_y'>y</a> <a href='#SkCanvas_writePixels_y'>to</a> <a href='#SkCanvas_writePixels_y'>offset</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>to</a> <a href='#SkCanvas_writePixels_pixels'>the</a> <a href='#SkCanvas_writePixels_pixels'>left</a> <a href='#SkCanvas_writePixels_pixels'>or</a>
<a href='#SkCanvas_writePixels_pixels'>above</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a>.

<a href='#SkCanvas_writePixels_pixels'>Does</a> <a href='#SkCanvas_writePixels_pixels'>not</a> <a href='#SkCanvas_writePixels_pixels'>copy</a>, <a href='#SkCanvas_writePixels_pixels'>and</a> <a href='#SkCanvas_writePixels_pixels'>returns</a> <a href='#SkCanvas_writePixels_pixels'>false</a> <a href='#SkCanvas_writePixels_pixels'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>could</a> <a href='#SkCanvas_writePixels_pixels'>not</a> <a href='#SkCanvas_writePixels_pixels'>be</a> <a href='#SkCanvas_writePixels_pixels'>converted</a> <a href='#SkCanvas_writePixels_pixels'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>or</a>
<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>are</a> <a href='#SkCanvas_writePixels_pixels'>not</a> <a href='#SkCanvas_writePixels_pixels'>writable</a>; <a href='#SkCanvas_writePixels_pixels'>for</a> <a href='#SkCanvas_writePixels_pixels'>instance</a>, <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>document-based</a>.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_rowBytes'>rowBytes</a> <a href='#SkCanvas_writePixels_rowBytes'>is</a> <a href='#SkCanvas_writePixels_rowBytes'>too</a> <a href='#SkCanvas_writePixels_rowBytes'>small</a> <a href='#SkCanvas_writePixels_rowBytes'>to</a> <a href='#SkCanvas_writePixels_rowBytes'>contain</a> <a href='#SkCanvas_writePixels_rowBytes'>one</a> <a href='#SkCanvas_writePixels_rowBytes'>row</a> <a href='#SkCanvas_writePixels_rowBytes'>of</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a>.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_writePixels_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>of</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td><a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>to</a> <a href='#SkCanvas_writePixels_pixels'>copy</a>, <a href='#SkCanvas_writePixels_pixels'>of</a> <a href='undocumented#Size'>size</a> <a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>times</a> <a href='#SkCanvas_writePixels_rowBytes'>rowBytes</a>, <a href='#SkCanvas_writePixels_rowBytes'>or</a> <a href='#SkCanvas_writePixels_rowBytes'>larger</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>one</a> <a href='undocumented#Size'>row</a> <a href='undocumented#Size'>of</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a>; <a href='#SkCanvas_writePixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>larger</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>writable</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>on</a> <a href='#SkCanvas_writePixels_pixels'>x-axis</a>; <a href='#SkCanvas_writePixels_pixels'>may</a> <a href='#SkCanvas_writePixels_pixels'>be</a> <a href='#SkCanvas_writePixels_pixels'>negative</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>writable</a> <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>on</a> <a href='#SkCanvas_writePixels_pixels'>y-axis</a>; <a href='#SkCanvas_writePixels_pixels'>may</a> <a href='#SkCanvas_writePixels_pixels'>be</a> <a href='#SkCanvas_writePixels_pixels'>negative</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_writePixels_pixels'>pixels</a> <a href='#SkCanvas_writePixels_pixels'>were</a> <a href='#SkCanvas_writePixels_pixels'>written</a> <a href='#SkCanvas_writePixels_pixels'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>

### Example

<div><fiddle-embed name="29b98ebf58aa9fd1edfaabf9f4490b3a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_readPixels'>readPixels</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_writePixels'>writePixels</a>

<a name='SkCanvas_writePixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_writePixels'>writePixels</a>(<a href='#SkCanvas_writePixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>x</a>, <a href='SkBitmap_Reference#Bitmap'>int</a> <a href='SkBitmap_Reference#Bitmap'>y</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>ignored</a>.
<a href='SkMatrix_Reference#Matrix'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a>.<a href='#SkBitmap_width'>width()</a>, <a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a>.<a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkCanvas_writePixels_2_x'>x</a>, <a href='#SkCanvas_writePixels_2_y'>y</a>) <a href='#SkCanvas_writePixels_2_y'>and</a>
(<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_width'>width()</a>, <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_height'>height()</a>).

<a href='#SkImageInfo_height'>Copies</a> <a href='#SkImageInfo_height'>each</a> <a href='#SkImageInfo_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>and</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>if</a> <a href='#SkImageInfo_alphaType'>required</a>.

<a href='#SkImageInfo_alphaType'>Pixels</a> <a href='#SkImageInfo_alphaType'>are</a> <a href='#SkImageInfo_alphaType'>writable</a> <a href='#SkImageInfo_alphaType'>when</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>raster</a>, <a href='undocumented#Device'>or</a> <a href='undocumented#Device'>backed</a> <a href='undocumented#Device'>by</a> <a href='undocumented#Device'>a</a> <a href='undocumented#Device'>GPU</a>.
<a href='undocumented#Device'>Pixels</a> <a href='undocumented#Device'>are</a> <a href='undocumented#Device'>not</a> <a href='undocumented#Device'>writable</a> <a href='undocumented#Device'>when</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='undocumented#SkDocument'>SkDocument</a>::<a href='#SkDocument_beginPage'>beginPage</a>,
<a href='#SkDocument_beginPage'>returned</a> <a href='#SkDocument_beginPage'>by</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>::<a href='#SkPictureRecorder_beginRecording'>beginRecording</a>, <a href='#SkPictureRecorder_beginRecording'>or</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>base</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>utility</a>
<a href='SkCanvas_Reference#Canvas'>class</a> <a href='SkCanvas_Reference#Canvas'>like</a> <a href='undocumented#SkDebugCanvas'>SkDebugCanvas</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='#Image_Info_Alpha_Type'>rectangles</a>
<a href='#Image_Info_Alpha_Type'>are</a> <a href='#Image_Info_Alpha_Type'>copied</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkCanvas_writePixels_2_x'>x</a> <a href='#SkCanvas_writePixels_2_x'>or</a> <a href='#SkCanvas_writePixels_2_y'>y</a> <a href='#SkCanvas_writePixels_2_y'>to</a> <a href='#SkCanvas_writePixels_2_y'>offset</a> <a href='#SkCanvas_writePixels_2_y'>pixels</a> <a href='#SkCanvas_writePixels_2_y'>to</a> <a href='#SkCanvas_writePixels_2_y'>the</a> <a href='#SkCanvas_writePixels_2_y'>left</a> <a href='#SkCanvas_writePixels_2_y'>or</a>
<a href='#SkCanvas_writePixels_2_y'>above</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a>.

<a href='SkCanvas_Reference#Canvas'>Does</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>copy</a>, <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>returns</a> <a href='SkCanvas_Reference#Canvas'>false</a> <a href='SkCanvas_Reference#Canvas'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> <a href='#SkCanvas_writePixels_2_bitmap'>does</a> <a href='#SkCanvas_writePixels_2_bitmap'>not</a> <a href='#SkCanvas_writePixels_2_bitmap'>have</a> <a href='#SkCanvas_writePixels_2_bitmap'>allocated</a> <a href='#SkCanvas_writePixels_2_bitmap'>pixels</a>.</td>
  </tr>  <tr>
    <td><a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a> <a href='#SkCanvas_writePixels_2_bitmap'>could</a> <a href='#SkCanvas_writePixels_2_bitmap'>not</a> <a href='#SkCanvas_writePixels_2_bitmap'>be</a> <a href='#SkCanvas_writePixels_2_bitmap'>converted</a> <a href='#SkCanvas_writePixels_2_bitmap'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>or</a>
<a href='#SkCanvas_imageInfo'>imageInfo</a>().<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>are</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>writable</a>; <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>instance</a>, <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='undocumented#Document'>document</a> <a href='undocumented#Document'>based</a>.</td>
  </tr>  <tr>
    <td><a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a> <a href='#SkCanvas_writePixels_2_bitmap'>are</a> <a href='#SkCanvas_writePixels_2_bitmap'>inaccessible</a>; <a href='#SkCanvas_writePixels_2_bitmap'>for</a> <a href='#SkCanvas_writePixels_2_bitmap'>instance</a>, <a href='#SkCanvas_writePixels_2_bitmap'>bitmap</a> <a href='#SkCanvas_writePixels_2_bitmap'>wraps</a> <a href='#SkCanvas_writePixels_2_bitmap'>a</a> <a href='undocumented#Texture'>texture</a>.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkCanvas_writePixels_2_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>contains pixels copied to <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_2_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>writable</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>in</a> <a href='#SkCanvas_writePixels_2_x'>x</a>; <a href='#SkCanvas_writePixels_2_x'>may</a> <a href='#SkCanvas_writePixels_2_x'>be</a> <a href='#SkCanvas_writePixels_2_x'>negative</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_writePixels_2_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>writable</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>in</a> <a href='#SkCanvas_writePixels_2_y'>y</a>; <a href='#SkCanvas_writePixels_2_y'>may</a> <a href='#SkCanvas_writePixels_2_y'>be</a> <a href='#SkCanvas_writePixels_2_y'>negative</a></td>
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

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>maintains</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>stack</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>state</a> <a href='SkCanvas_Reference#Canvas'>that</a> <a href='SkCanvas_Reference#Canvas'>allows</a> <a href='SkCanvas_Reference#Canvas'>hierarchical</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>, <a href='SkCanvas_Reference#Canvas'>commonly</a> <a href='SkCanvas_Reference#Canvas'>used</a>
<a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>implement</a> <a href='SkCanvas_Reference#Canvas'>windows</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>views</a>. <a href='SkCanvas_Reference#Canvas'>The</a> <a href='SkCanvas_Reference#Canvas'>initial</a> <a href='SkCanvas_Reference#Canvas'>state</a> <a href='SkCanvas_Reference#Canvas'>has</a> <a href='SkCanvas_Reference#Canvas'>an</a> <a href='SkCanvas_Reference#Canvas'>identity</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>and</a>
<a href='SkMatrix_Reference#Matrix'>an</a> <a href='SkMatrix_Reference#Matrix'>infinite</a> <a href='SkMatrix_Reference#Matrix'>clip</a>. <a href='SkMatrix_Reference#Matrix'>Even</a> <a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkMatrix_Reference#Matrix'>a</a> <a href='SkMatrix_Reference#Matrix'>wide-open</a> <a href='SkMatrix_Reference#Matrix'>clip</a>, <a href='SkMatrix_Reference#Matrix'>drawing</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>constrained</a> <a href='SkMatrix_Reference#Matrix'>by</a> <a href='SkMatrix_Reference#Matrix'>the</a>
<a href='SkMatrix_Reference#Matrix'>bounds</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>or</a> <a href='undocumented#Device'>Device</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>savable</a> <a href='SkCanvas_Reference#Canvas'>state</a> <a href='SkCanvas_Reference#Canvas'>consists</a> <a href='SkCanvas_Reference#Canvas'>of</a> <a href='SkCanvas_Reference#Canvas'>Clip</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>.
<a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>describes</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>area</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='SkMatrix_Reference#Matrix'>may</a> <a href='SkMatrix_Reference#Matrix'>be</a> <a href='SkMatrix_Reference#Matrix'>drawn</a> <a href='SkMatrix_Reference#Matrix'>to</a>.
<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>transforms</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>geometry</a>.

<a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_saveLayer'>saveLayer</a>, <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>, <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>and</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>
<a href='#SkCanvas_saveLayerAlpha'>save</a> <a href='#SkCanvas_saveLayerAlpha'>state</a> <a href='#SkCanvas_saveLayerAlpha'>and</a> <a href='#SkCanvas_saveLayerAlpha'>return</a> <a href='#SkCanvas_saveLayerAlpha'>the</a> <a href='#SkCanvas_saveLayerAlpha'>depth</a> <a href='#SkCanvas_saveLayerAlpha'>of</a> <a href='#SkCanvas_saveLayerAlpha'>the</a> <a href='#SkCanvas_saveLayerAlpha'>stack</a>.

<a href='#SkCanvas_restore'>restore()</a>, <a href='#SkCanvas_restoreToCount'>restoreToCount</a>, <a href='#SkCanvas_restoreToCount'>and</a> ~<a href='#SkCanvas_empty_constructor'>SkCanvas()</a> <a href='SkCanvas_Reference#SkCanvas'>revert</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>its</a> <a href='SkCanvas_Reference#SkCanvas'>value</a> <a href='SkCanvas_Reference#SkCanvas'>when</a> <a href='SkCanvas_Reference#SkCanvas'>saved</a>.

<a href='SkCanvas_Reference#SkCanvas'>Each</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>on</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkCanvas_Reference#SkCanvas'>stack</a> <a href='SkCanvas_Reference#SkCanvas'>intersects</a> <a href='SkCanvas_Reference#SkCanvas'>Clip</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkCanvas_Reference#SkCanvas'>previous</a> <a href='SkCanvas_Reference#SkCanvas'>Clip</a>,
<a href='SkCanvas_Reference#SkCanvas'>and</a> <a href='SkCanvas_Reference#SkCanvas'>concatenates</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>previous</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>.
<a href='SkMatrix_Reference#Matrix'>The</a> <a href='SkMatrix_Reference#Matrix'>intersected</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>makes</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>drawing</a> <a href='SkMatrix_Reference#Matrix'>area</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>same</a> <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>smaller</a>;
<a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>concatenated</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>may</a> <a href='SkMatrix_Reference#Matrix'>move</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>origin</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>potentially</a> <a href='SkMatrix_Reference#Matrix'>scale</a> <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>rotate</a>
<a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>coordinate</a> <a href='SkMatrix_Reference#Matrix'>space</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>does</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>require</a> <a href='SkCanvas_Reference#Canvas'>balancing</a> <a href='SkCanvas_Reference#Canvas'>the</a>  <a href='#State_Stack'>state stack</a> <a href='SkCanvas_Reference#Canvas'>but</a> <a href='SkCanvas_Reference#Canvas'>it</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>good</a> <a href='SkCanvas_Reference#Canvas'>idea</a>
<a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>do</a> <a href='SkCanvas_Reference#Canvas'>so</a>. <a href='SkCanvas_Reference#Canvas'>Calling</a> <a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>without</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>will</a> <a href='#SkCanvas_restore'>eventually</a> <a href='#SkCanvas_restore'>cause</a> <a href='#SkCanvas_restore'>Skia</a> <a href='#SkCanvas_restore'>to</a> <a href='#SkCanvas_restore'>fail</a>;
<a href='#SkCanvas_restore'>mismatched</a> <a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>and</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>create</a> <a href='#SkCanvas_restore'>hard</a> <a href='#SkCanvas_restore'>to</a> <a href='#SkCanvas_restore'>find</a> <a href='#SkCanvas_restore'>bugs</a>.

<a href='#SkCanvas_restore'>It</a> <a href='#SkCanvas_restore'>is</a> <a href='#SkCanvas_restore'>not</a> <a href='#SkCanvas_restore'>possible</a> <a href='#SkCanvas_restore'>to</a> <a href='#SkCanvas_restore'>use</a> <a href='#SkCanvas_restore'>state</a> <a href='#SkCanvas_restore'>to</a> <a href='#SkCanvas_restore'>draw</a> <a href='#SkCanvas_restore'>outside</a> <a href='#SkCanvas_restore'>of</a> <a href='#SkCanvas_restore'>the</a> <a href='#SkCanvas_restore'>clip</a> <a href='#SkCanvas_restore'>defined</a> <a href='#SkCanvas_restore'>by</a> <a href='#SkCanvas_restore'>the</a>
<a href='#SkCanvas_restore'>previous</a> <a href='#SkCanvas_restore'>state</a>.

### Example

<div><fiddle-embed name="bb1dbfdca3aedf716beb6f07e2aab065"><div>Draw to ever smaller clips; then restore drawing to full <a href='SkCanvas_Reference#Canvas'>canvas</a>.
<a href='SkCanvas_Reference#Canvas'>Note</a> <a href='SkCanvas_Reference#Canvas'>that</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>second</a> <a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRect'>is</a> <a href='#SkCanvas_clipRect'>not</a> <a href='#SkCanvas_clipRect'>permitted</a> <a href='#SkCanvas_clipRect'>to</a> <a href='#SkCanvas_clipRect'>enlarge</a> <a href='#SkCanvas_clipRect'>Clip</a>.
</div></fiddle-embed></div>

Each Clip uses the current <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>for</a> <a href='SkMatrix_Reference#Matrix'>its</a> <a href='SkMatrix_Reference#Matrix'>coordinates</a>.

### Example

<div><fiddle-embed name="9f563a2d60aa31d4b26742e5aa17aa4e"><div>While <a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRect'>is</a> <a href='#SkCanvas_clipRect'>given</a> <a href='#SkCanvas_clipRect'>the</a> <a href='#SkCanvas_clipRect'>same</a> <a href='#SkCanvas_clipRect'>rectangle</a> <a href='#SkCanvas_clipRect'>twice</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>makes</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>second</a>
<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRect'>draw</a> <a href='#SkCanvas_clipRect'>at</a> <a href='#SkCanvas_clipRect'>half</a> <a href='#SkCanvas_clipRect'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>first</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_save'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_save'>save()</a>
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.
Calling <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>discards</a> <a href='#SkCanvas_restore'>changes</a> <a href='#SkCanvas_restore'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>,
restoring the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>their</a> <a href='SkMatrix_Reference#SkMatrix'>state</a> <a href='SkMatrix_Reference#SkMatrix'>when</a> <a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>was</a> <a href='#SkCanvas_save'>called</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>changed</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_setMatrix'>setMatrix</a>(),
and <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). <a href='#SkCanvas_resetMatrix'>Clip</a> <a href='#SkCanvas_resetMatrix'>may</a> <a href='#SkCanvas_resetMatrix'>be</a> <a href='#SkCanvas_resetMatrix'>changed</a> <a href='#SkCanvas_resetMatrix'>by</a> <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(), <a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

Saved <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>put</a> <a href='SkCanvas_Reference#SkCanvas'>on</a> <a href='SkCanvas_Reference#SkCanvas'>a</a> <a href='SkCanvas_Reference#SkCanvas'>stack</a>; <a href='SkCanvas_Reference#SkCanvas'>multiple</a> <a href='SkCanvas_Reference#SkCanvas'>calls</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>should</a> <a href='#SkCanvas_save'>be</a> <a href='#SkCanvas_save'>balance</a>
by an equal number of calls to <a href='#SkCanvas_restore'>restore()</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() <a href='#SkCanvas_restoreToCount'>with</a> <a href='#SkCanvas_restoreToCount'>result</a> <a href='#SkCanvas_restoreToCount'>to</a> <a href='#SkCanvas_restoreToCount'>restore</a> <a href='#SkCanvas_restoreToCount'>this</a> <a href='#SkCanvas_restoreToCount'>and</a> <a href='#SkCanvas_restoreToCount'>subsequent</a> <a href='#SkCanvas_restoreToCount'>saves</a>.

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="e477dce358a9ba3b0aa1bf33b8a376de"><div>The black square is translated 50 pixels down and to the right.
Restoring <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>state</a> <a href='SkCanvas_Reference#Canvas'>removes</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_translate'>from</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>stack</a>;
<a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>red</a> <a href='SkCanvas_Reference#Canvas'>square</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>translated</a>, <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>drawn</a> <a href='SkCanvas_Reference#Canvas'>at</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>origin</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_restore'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_restore'>restore()</a>
</pre>

Removes changes to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>since</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>was</a>
last saved. The state is removed from the stack.

Does nothing if the stack is empty.

### Example

<div><fiddle-embed name="e78471212a67f2f4fd39496e17a30d17"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_restoreToCount'>restoreToCount</a>

<a name='SkCanvas_getSaveCount'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_getSaveCount'>getSaveCount</a>() <a href='#SkCanvas_getSaveCount'>const</a>
</pre>

Returns the number of saved states, each containing: <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.
Equals the number of <a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>calls</a> <a href='#SkCanvas_save'>less</a> <a href='#SkCanvas_save'>the</a> <a href='#SkCanvas_save'>number</a> <a href='#SkCanvas_save'>of</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>calls</a> <a href='#SkCanvas_restore'>plus</a> <a href='#SkCanvas_restore'>one</a>.
The save count of a new <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>one</a>.

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
void <a href='#SkCanvas_restoreToCount'>restoreToCount</a>(<a href='#SkCanvas_restoreToCount'>int</a> <a href='#SkCanvas_restoreToCount'>saveCount</a>)
</pre>

Restores state to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>values</a> <a href='SkMatrix_Reference#SkMatrix'>when</a> <a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_saveLayer'>saveLayer</a>(),
<a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(), <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>or</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>() <a href='#SkCanvas_saveLayerAlpha'>returned</a> <a href='#SkCanvas_restoreToCount_saveCount'>saveCount</a>.

Does nothing if <a href='#SkCanvas_restoreToCount_saveCount'>saveCount</a> <a href='#SkCanvas_restoreToCount_saveCount'>is</a> <a href='#SkCanvas_restoreToCount_saveCount'>greater</a> <a href='#SkCanvas_restoreToCount_saveCount'>than</a>  <a href='#State_Stack'>state stack</a> <a href='#SkCanvas_restoreToCount_saveCount'>count</a>.
Restores state to initial values if <a href='#SkCanvas_restoreToCount_saveCount'>saveCount</a> <a href='#SkCanvas_restoreToCount_saveCount'>is</a> <a href='#SkCanvas_restoreToCount_saveCount'>less</a> <a href='#SkCanvas_restoreToCount_saveCount'>than</a> <a href='#SkCanvas_restoreToCount_saveCount'>or</a> <a href='#SkCanvas_restoreToCount_saveCount'>equal</a> <a href='#SkCanvas_restoreToCount_saveCount'>to</a> <a href='#SkCanvas_restoreToCount_saveCount'>one</a>.

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

<a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>allocates</a> <a href='SkCanvas_Reference#Layer'>a</a> <a href='SkCanvas_Reference#Layer'>temporary</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>into</a>. <a href='SkBitmap_Reference#Bitmap'>When</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>drawing</a> <a href='SkBitmap_Reference#Bitmap'>is</a>
<a href='SkBitmap_Reference#Bitmap'>complete</a>, <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>into</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>.

<a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>saved</a> <a href='SkCanvas_Reference#Layer'>in</a> <a href='SkCanvas_Reference#Layer'>a</a> <a href='SkCanvas_Reference#Layer'>stack</a> <a href='SkCanvas_Reference#Layer'>along</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>other</a> <a href='SkCanvas_Reference#Layer'>saved</a> <a href='SkCanvas_Reference#Layer'>state</a>. <a href='SkCanvas_Reference#Layer'>When</a> <a href='SkCanvas_Reference#Layer'>state</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>a</a> <a href='SkCanvas_Reference#Layer'>Layer</a>
<a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>restored</a>, <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>into</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>previous</a> <a href='SkCanvas_Reference#Layer'>Layer</a>.

<a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>initialized</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>contents</a> <a href='SkCanvas_Reference#Layer'>of</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>previous</a> <a href='SkCanvas_Reference#Layer'>Layer</a>. <a href='SkCanvas_Reference#Layer'>When</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>is</a>
<a href='SkCanvas_Reference#Layer'>restored</a>, <a href='SkCanvas_Reference#Layer'>its</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>can</a> <a href='SkBitmap_Reference#Bitmap'>be</a> <a href='SkBitmap_Reference#Bitmap'>modified</a> <a href='SkBitmap_Reference#Bitmap'>by</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>passed</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>apply</a>
<a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>, <a href='#Image_Filter'>and</a> <a href='#Blend_Mode'>Blend_Mode</a>.

<a name='SkCanvas_saveLayer'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(<a href='#SkCanvas_saveLayer'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>allocates</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>subsequent</a> <a href='SkBitmap_Reference#SkBitmap'>drawing</a>.
Calling <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>discards</a> <a href='#SkCanvas_restore'>changes</a> <a href='#SkCanvas_restore'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>draws</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>changed</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), <a href='#SkCanvas_setMatrix'>and</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). <a href='#SkCanvas_resetMatrix'>Clip</a> <a href='#SkCanvas_resetMatrix'>may</a> <a href='#SkCanvas_resetMatrix'>be</a> <a href='#SkCanvas_resetMatrix'>changed</a> <a href='#SkCanvas_resetMatrix'>by</a> <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayer_bounds'>bounds</a> <a href='#SkCanvas_saveLayer_bounds'>suggests</a> <a href='#SkCanvas_saveLayer_bounds'>but</a> <a href='#SkCanvas_saveLayer_bounds'>does</a> <a href='#SkCanvas_saveLayer_bounds'>not</a> <a href='#SkCanvas_saveLayer_bounds'>define</a> <a href='#SkCanvas_saveLayer_bounds'>the</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='undocumented#Size'>size</a>. <a href='undocumented#Size'>To</a> <a href='undocumented#Size'>clip</a> <a href='undocumented#Size'>drawing</a> <a href='undocumented#Size'>to</a>
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

Optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_saveLayer_paint'>paint</a> <a href='#SkCanvas_saveLayer_paint'>applies</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a>
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>when</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>is</a> <a href='#SkCanvas_restore'>called</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() <a href='#SkCanvas_restoreToCount'>with</a> <a href='#SkCanvas_restoreToCount'>returned</a> <a href='#SkCanvas_restoreToCount'>value</a> <a href='#SkCanvas_restoreToCount'>to</a> <a href='#SkCanvas_restoreToCount'>restore</a> <a href='#SkCanvas_restoreToCount'>this</a> <a href='#SkCanvas_restoreToCount'>and</a> <a href='#SkCanvas_restoreToCount'>subsequent</a> <a href='#SkCanvas_restoreToCount'>saves</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayer_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="42318b18d403e17e07a541652da91ee2"><div>Rectangles are blurred by <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>when</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>draws</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>main</a>
<a href='SkCanvas_Reference#Canvas'>Canvas</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='SkCanvas_saveLayer_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(<a href='#SkCanvas_saveLayer'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>allocates</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>subsequent</a> <a href='SkBitmap_Reference#SkBitmap'>drawing</a>.
Calling <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>discards</a> <a href='#SkCanvas_restore'>changes</a> <a href='#SkCanvas_restore'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>draws</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>changed</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), <a href='#SkCanvas_setMatrix'>and</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). <a href='#SkCanvas_resetMatrix'>Clip</a> <a href='#SkCanvas_resetMatrix'>may</a> <a href='#SkCanvas_resetMatrix'>be</a> <a href='#SkCanvas_resetMatrix'>changed</a> <a href='#SkCanvas_resetMatrix'>by</a> <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayer_2_bounds'>bounds</a> <a href='#SkCanvas_saveLayer_2_bounds'>suggests</a> <a href='#SkCanvas_saveLayer_2_bounds'>but</a> <a href='#SkCanvas_saveLayer_2_bounds'>does</a> <a href='#SkCanvas_saveLayer_2_bounds'>not</a> <a href='#SkCanvas_saveLayer_2_bounds'>define</a> <a href='#SkCanvas_saveLayer_2_bounds'>the</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='undocumented#Size'>size</a>. <a href='undocumented#Size'>To</a> <a href='undocumented#Size'>clip</a> <a href='undocumented#Size'>drawing</a> <a href='undocumented#Size'>to</a>
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

Optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_saveLayer_2_paint'>paint</a> <a href='#SkCanvas_saveLayer_2_paint'>applies</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a>
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>when</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>is</a> <a href='#SkCanvas_restore'>called</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() <a href='#SkCanvas_restoreToCount'>with</a> <a href='#SkCanvas_restoreToCount'>returned</a> <a href='#SkCanvas_restoreToCount'>value</a> <a href='#SkCanvas_restoreToCount'>to</a> <a href='#SkCanvas_restoreToCount'>restore</a> <a href='#SkCanvas_restoreToCount'>this</a> <a href='#SkCanvas_restoreToCount'>and</a> <a href='#SkCanvas_restoreToCount'>subsequent</a> <a href='#SkCanvas_restoreToCount'>saves</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayer_2_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
</table>

### Return Value

depth of saved stack

### Example

<div><fiddle-embed name="a17aec3aa4909527be039e26a7eda694"><div>Rectangles are blurred by <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>when</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>draws</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>main</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>.
<a href='SkCanvas_Reference#Canvas'>The</a> <a href='SkCanvas_Reference#Canvas'>red</a> <a href='SkCanvas_Reference#Canvas'>rectangle</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>clipped</a>; <a href='SkCanvas_Reference#Canvas'>it</a> <a href='SkCanvas_Reference#Canvas'>does</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>fully</a> <a href='SkCanvas_Reference#Canvas'>fit</a> <a href='SkCanvas_Reference#Canvas'>on</a> <a href='SkCanvas_Reference#Layer'>Layer</a>.
<a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>blurs</a> <a href='#Image_Filter'>past</a> <a href='#Image_Filter'>edge</a> <a href='#Image_Filter'>of</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>so</a> <a href='SkCanvas_Reference#Layer'>red</a> <a href='SkCanvas_Reference#Layer'>rectangle</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>blurred</a> <a href='SkCanvas_Reference#Layer'>on</a> <a href='SkCanvas_Reference#Layer'>all</a> <a href='SkCanvas_Reference#Layer'>sides</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='SkCanvas_saveLayerPreserveLCDTextRequests'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a>(<a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>allocates</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>subsequent</a> <a href='SkBitmap_Reference#SkBitmap'>drawing</a>.
<a href='SkPaint_Reference#LCD_Text'>LCD text</a> is preserved when the <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>drawn</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>prior</a> <a href='SkCanvas_Reference#Layer'>layer</a>.

Calling <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>discards</a> <a href='#SkCanvas_restore'>changes</a> <a href='#SkCanvas_restore'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>draws</a> <a href='SkCanvas_Reference#Layer'>layer</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>changed</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), <a href='#SkCanvas_setMatrix'>and</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). <a href='#SkCanvas_resetMatrix'>Clip</a> <a href='#SkCanvas_resetMatrix'>may</a> <a href='#SkCanvas_resetMatrix'>be</a> <a href='#SkCanvas_resetMatrix'>changed</a> <a href='#SkCanvas_resetMatrix'>by</a> <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>bounds</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>suggests</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>but</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>does</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>not</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>define</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_bounds'>the</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='undocumented#Size'>size</a>. <a href='undocumented#Size'>To</a> <a href='undocumented#Size'>clip</a> <a href='undocumented#Size'>drawing</a> <a href='undocumented#Size'>to</a>
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

Optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_paint'>paint</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests_paint'>applies</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a>
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>when</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>is</a> <a href='#SkCanvas_restore'>called</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() <a href='#SkCanvas_restoreToCount'>with</a> <a href='#SkCanvas_restoreToCount'>returned</a> <a href='#SkCanvas_restoreToCount'>value</a> <a href='#SkCanvas_restoreToCount'>to</a> <a href='#SkCanvas_restoreToCount'>restore</a> <a href='#SkCanvas_restoreToCount'>this</a> <a href='#SkCanvas_restoreToCount'>and</a> <a href='#SkCanvas_restoreToCount'>subsequent</a> <a href='#SkCanvas_restoreToCount'>saves</a>.

Draw <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>an</a> <a href='undocumented#Text'>opaque</a> <a href='undocumented#Text'>background</a> <a href='undocumented#Text'>so</a> <a href='undocumented#Text'>that</a>  <a href='SkPaint_Reference#LCD_Text'>LCD text</a> <a href='undocumented#Text'>blends</a> <a href='undocumented#Text'>correctly</a> <a href='undocumented#Text'>with</a> <a href='undocumented#Text'>the</a>
prior <a href='SkCanvas_Reference#Layer'>layer</a>.  <a href='SkPaint_Reference#LCD_Text'>LCD text</a> <a href='SkCanvas_Reference#Layer'>drawn</a> <a href='SkCanvas_Reference#Layer'>on</a> <a href='SkCanvas_Reference#Layer'>a</a> <a href='SkCanvas_Reference#Layer'>background</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>transparency</a> <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>result</a> <a href='SkCanvas_Reference#Layer'>in</a>
incorrect blending.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayerPreserveLCDTextRequests_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_saveLayerPreserveLCDTextRequests_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state for <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
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
int <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>(<a href='#SkCanvas_saveLayerAlpha'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>allocates</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>subsequent</a> <a href='SkBitmap_Reference#SkBitmap'>drawing</a>.

Calling <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>discards</a> <a href='#SkCanvas_restore'>changes</a> <a href='#SkCanvas_restore'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>,
and blends <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>alpha</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>opacity</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>onto</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>prior</a> <a href='SkCanvas_Reference#Layer'>layer</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>changed</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), <a href='#SkCanvas_setMatrix'>and</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). <a href='#SkCanvas_resetMatrix'>Clip</a> <a href='#SkCanvas_resetMatrix'>may</a> <a href='#SkCanvas_resetMatrix'>be</a> <a href='#SkCanvas_resetMatrix'>changed</a> <a href='#SkCanvas_resetMatrix'>by</a> <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>bounds</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>suggests</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>but</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>does</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>not</a> <a href='#SkCanvas_saveLayerAlpha_bounds'>define</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='undocumented#Size'>size</a>. <a href='undocumented#Size'>To</a> <a href='undocumented#Size'>clip</a> <a href='undocumented#Size'>drawing</a> <a href='undocumented#Size'>to</a>
a specific rectangle, use <a href='#SkCanvas_clipRect'>clipRect</a>().

<a href='#SkCanvas_saveLayerAlpha_alpha'>alpha</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>of</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>zero</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>is</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>fully</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>transparent</a>, 255 <a href='#SkCanvas_saveLayerAlpha_alpha'>is</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>fully</a> <a href='#SkCanvas_saveLayerAlpha_alpha'>opaque</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() <a href='#SkCanvas_restoreToCount'>with</a> <a href='#SkCanvas_restoreToCount'>returned</a> <a href='#SkCanvas_restoreToCount'>value</a> <a href='#SkCanvas_restoreToCount'>to</a> <a href='#SkCanvas_restoreToCount'>restore</a> <a href='#SkCanvas_restoreToCount'>this</a> <a href='#SkCanvas_restoreToCount'>and</a> <a href='#SkCanvas_restoreToCount'>subsequent</a> <a href='#SkCanvas_restoreToCount'>saves</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayerAlpha_bounds'><code><strong>bounds</strong></code></a></td>
    <td>hint to limit the <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
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
        <a href='#SkCanvas_kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag'>kMaskAgainstCoverage_EXPERIMENTAL_DONT_USE_SaveLayerFlag</a> =
                                          1 << 3,
        <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>kDontClipToLayer_Legacy_SaveLayerFlag</a> =
           <a href='#SkCanvas_kDontClipToLayer_Legacy_SaveLayerFlag'>kDontClipToLayer_PrivateSaveLayerFlag</a>,
    };
</pre>

<a name='SkCanvas_SaveLayerFlags'></a>

---

<a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>provides</a> <a href='#SkCanvas_SaveLayerFlags'>options</a> <a href='#SkCanvas_SaveLayerFlags'>that</a> <a href='#SkCanvas_SaveLayerFlags'>may</a> <a href='#SkCanvas_SaveLayerFlags'>be</a> <a href='#SkCanvas_SaveLayerFlags'>used</a> <a href='#SkCanvas_SaveLayerFlags'>in</a> <a href='#SkCanvas_SaveLayerFlags'>any</a> <a href='#SkCanvas_SaveLayerFlags'>combination</a> <a href='#SkCanvas_SaveLayerFlags'>in</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>,
<a href='#SkCanvas_SaveLayerRec'>defining</a> <a href='#SkCanvas_SaveLayerRec'>how</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>allocated</a> <a href='SkCanvas_Reference#Layer'>by</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayer'>operates</a>. <a href='#SkCanvas_saveLayer'>It</a> <a href='#SkCanvas_saveLayer'>may</a> <a href='#SkCanvas_saveLayer'>be</a> <a href='#SkCanvas_saveLayer'>set</a> <a href='#SkCanvas_saveLayer'>to</a> <a href='#SkCanvas_saveLayer'>zero</a>,
<a href='#SkCanvas_kPreserveLCDText_SaveLayerFlag'>kPreserveLCDText_SaveLayerFlag</a>, <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a>, <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>or</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>both</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>flags</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kPreserveLCDText_SaveLayerFlag'><code>SkCanvas::kPreserveLCDText_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Creates <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>for</a>  <a href='SkPaint_Reference#LCD_Text'>LCD text</a>. <a href='SkCanvas_Reference#Layer'>Flag</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>ignored</a> <a href='SkCanvas_Reference#Layer'>if</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>contains</a>
<a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>or</a> <a href='#Color_Filter'>Color_Filter</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kInitWithPrevious_SaveLayerFlag'><code>SkCanvas::kInitWithPrevious_SaveLayerFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Initializes <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>contents</a> <a href='SkCanvas_Reference#Layer'>of</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>previous</a> <a href='SkCanvas_Reference#Layer'>Layer</a>.
</td>
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

<div><fiddle-embed name="05db6a937225e8e31ae3481173d25dae"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>captures</a> <a href='SkCanvas_Reference#Layer'>red</a> <a href='SkCanvas_Reference#Layer'>and</a> <a href='SkCanvas_Reference#Layer'>blue</a> <a href='undocumented#Circle'>circles</a> <a href='undocumented#Circle'>scaled</a> <a href='undocumented#Circle'>up</a> <a href='undocumented#Circle'>by</a> <a href='undocumented#Circle'>four</a>.
<a href='undocumented#Circle'>scalePaint</a> <a href='undocumented#Circle'>blends</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>back</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>transparency</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>

<a name='Layer_SaveLayerRec'></a>

<a name='SkCanvas_SaveLayerRec'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> {
        <a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec()</a>;
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a> = 0);
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>backdrop</a>,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a>);
        <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>backdrop</a>,
                     <a href='undocumented#SkImageFilter'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>clipMask</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>clipMatrix</a>,
                     <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a>);
        <a href='#SkCanvas_SaveLayerFlags'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> = <a href='#SkCanvas_SaveLayerRec_fBounds'>nullptr</a>;
        <a href='#SkCanvas_SaveLayerRec_fBounds'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a> = <a href='#SkCanvas_SaveLayerRec_fPaint'>nullptr</a>;
        <a href='#SkCanvas_SaveLayerRec_fPaint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> = <a href='#SkCanvas_SaveLayerRec_fBackdrop'>nullptr</a>;
        <a href='#SkCanvas_SaveLayerRec_fBackdrop'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> = <a href='#SkCanvas_SaveLayerRec_fClipMask'>nullptr</a>;
        <a href='#SkCanvas_SaveLayerRec_fClipMask'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> = <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>nullptr</a>;
        <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a> = 0;
    };
</pre>

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>contains</a> <a href='#SkCanvas_SaveLayerRec'>the</a> <a href='#SkCanvas_SaveLayerRec'>state</a> <a href='#SkCanvas_SaveLayerRec'>used</a> <a href='#SkCanvas_SaveLayerRec'>to</a> <a href='#SkCanvas_SaveLayerRec'>create</a> <a href='#SkCanvas_SaveLayerRec'>the</a> <a href='SkCanvas_Reference#Layer'>Layer</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkRect*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fBounds'><code>fBounds</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>is</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>used</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>as</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>a</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>hint</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>to</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>limit</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkCanvas_Reference#Layer'>Layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a>.
<a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>suggests</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>but</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>does</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>not</a> <a href='#SkCanvas_SaveLayerRec_fBounds'>define</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='undocumented#Size'>size</a>. <a href='undocumented#Size'>To</a> <a href='undocumented#Size'>clip</a> <a href='undocumented#Size'>drawing</a> <a href='undocumented#Size'>to</a>
<a href='undocumented#Size'>a</a> <a href='undocumented#Size'>specific</a> <a href='undocumented#Size'>rectangle</a>, <a href='undocumented#Size'>use</a> <a href='#SkCanvas_clipRect'>clipRect</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkPaint*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fPaint'><code>fPaint</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a> <a href='#SkCanvas_SaveLayerRec_fPaint'>modifies</a> <a href='#SkCanvas_SaveLayerRec_fPaint'>how</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>overlays</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>prior</a> <a href='SkCanvas_Reference#Layer'>Layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a>.
<a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Draw_Looper'>Draw_Looper</a>, <a href='#Image_Filter'>Image_Filter</a>, <a href='#Image_Filter'>and</a>
<a href='#Mask_Filter'>Mask_Filter</a> <a href='#Mask_Filter'>affect</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>draw</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkImageFilter*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fBackdrop'><code>fBackdrop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>applies</a> <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>to</a> <a href='#Image_Filter'>the</a> <a href='#Image_Filter'>prior</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>when</a> <a href='SkCanvas_Reference#Layer'>copying</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>Layer</a>;
<a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a>. <a href='SkCanvas_Reference#Layer'>Use</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>to</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>copy</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>the</a>
<a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>prior</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>without</a> <a href='SkCanvas_Reference#Layer'>an</a> <a href='#Image_Filter'>Image_Filter</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkImage*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fClipMask'><code>fClipMask</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>clips</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>by</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>channel</a> <a href='#Color_Alpha'>of</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>when</a>
<a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>is</a> <a href='SkCanvas_Reference#Layer'>copied</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='undocumented#Device'>Device</a>. <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>may</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>be</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>nullptr</a>.    .
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkMatrix*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fClipMatrix'><code>fClipMatrix</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>transforms</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>before</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>it</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>clips</a> <a href='SkCanvas_Reference#Layer'>Layer</a>. <a href='SkCanvas_Reference#Layer'>If</a>
<a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>describes</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>a</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>translucent</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>gradient</a>, <a href='#SkCanvas_SaveLayerRec_fClipMask'>it</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>may</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>be</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>scaled</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>and</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>rotated</a>
<a href='#SkCanvas_SaveLayerRec_fClipMask'>without</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>introducing</a> <a href='#SkCanvas_SaveLayerRec_fClipMask'>artifacts</a>. <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a> <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>may</a> <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>be</a> <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>nullptr</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SaveLayerFlags</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_SaveLayerRec_fSaveLayerFlags'><code>fSaveLayerFlags</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>are</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>used</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>to</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>create</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>without</a> <a href='SkCanvas_Reference#Layer'>transparency</a>,
<a href='SkCanvas_Reference#Layer'>create</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>for</a>  <a href='SkPaint_Reference#LCD_Text'>LCD text</a>, <a href='SkCanvas_Reference#Layer'>and</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>create</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkCanvas_Reference#Layer'>the</a>
<a href='SkCanvas_Reference#Layer'>contents</a> <a href='SkCanvas_Reference#Layer'>of</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>previous</a> <a href='SkCanvas_Reference#Layer'>Layer</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee8c0b120234e27364f8c9a786cf8f89"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>captures</a> <a href='SkCanvas_Reference#Layer'>a</a> <a href='SkCanvas_Reference#Layer'>red</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>and</a> <a href='undocumented#Circle'>a</a> <a href='undocumented#Circle'>blue</a> <a href='undocumented#Alias'>Aliased</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>scaled</a>
<a href='undocumented#Circle'>up</a> <a href='undocumented#Circle'>by</a> <a href='undocumented#Circle'>four</a>. <a href='undocumented#Circle'>After</a> <a href='undocumented#Circle'>drawing</a> <a href='undocumented#Circle'>another</a> <a href='undocumented#Circle'>red</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>without</a> <a href='undocumented#Circle'>scaling</a> <a href='undocumented#Circle'>on</a> <a href='undocumented#Circle'>top</a>, <a href='undocumented#Circle'>the</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>is</a>
<a href='SkCanvas_Reference#Layer'>transferred</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>main</a> <a href='SkCanvas_Reference#Canvas'>canvas</a>.
</div></fiddle-embed></div>

<a name='Layer_SaveLayerRec_Constructors'></a>

<a name='SkCanvas_SaveLayerRec_SaveLayerRec'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec_SaveLayerRec'>SaveLayerRec()</a>
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>and</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>to</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>nullptr</a>. <a href='#SkCanvas_SaveLayerRec_fBackdrop'>Clears</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>.

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
<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a> = 0)
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>and</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>; <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>sets</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>to</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>dimensions</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_paint'><code><strong>paint</strong></code></a></td>
    <td>applied to <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>when</a> <a href='SkCanvas_Reference#Layer'>overlaying</a> <a href='SkCanvas_Reference#Layer'>prior</a> <a href='SkCanvas_Reference#Layer'>layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>options</a> <a href='#SkCanvas_SaveLayerRec'>to</a> <a href='#SkCanvas_SaveLayerRec'>modify</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>with</a> <a href='#SkCanvas_SaveLayerRec'>empty</a> <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a>

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
<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>backdrop</a>,
             <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a>)
</pre>

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a>, <a href='#SkCanvas_SaveLayerRec_fBackdrop'>and</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>dimensions</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_paint'><code><strong>paint</strong></code></a></td>
    <td>applied to <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>when</a> <a href='SkCanvas_Reference#Layer'>overlaying</a> <a href='SkCanvas_Reference#Layer'>prior</a> <a href='SkCanvas_Reference#Layer'>layer</a>;</td>
  </tr>
</table>

may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_backdrop'><code><strong>backdrop</strong></code></a></td>
    <td>prior <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>copied</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>; <a href='undocumented#SkImageFilter'>may</a> <a href='undocumented#SkImageFilter'>be</a> <a href='undocumented#SkImageFilter'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>options</a> <a href='#SkCanvas_SaveLayerRec'>to</a> <a href='#SkCanvas_SaveLayerRec'>modify</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>fully</a> <a href='#SkCanvas_SaveLayerRec'>specified</a>

### Example

<div><fiddle-embed name="9b7fa2fe855642ffff6538829db15328">

#### Example Output

~~~~
rec1 == rec2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>(<a href='#SkCanvas_SaveLayerRec'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkPaint_Reference#Paint'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>backdrop</a>,
             <a href='undocumented#SkImageFilter'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>clipMask</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>clipMatrix</a>, <a href='#SkCanvas_SaveLayerFlags'>SaveLayerFlags</a> <a href='#SkCanvas_SaveLayerFlags'>saveLayerFlags</a>)
</pre>

Experimental. Not ready for general use.

Sets <a href='#SkCanvas_SaveLayerRec_fBounds'>fBounds</a>, <a href='#SkCanvas_SaveLayerRec_fPaint'>fPaint</a>, <a href='#SkCanvas_SaveLayerRec_fBackdrop'>fBackdrop</a>, <a href='#SkCanvas_SaveLayerRec_fClipMask'>fClipMask</a>, <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>fClipMatrix</a>, <a href='#SkCanvas_SaveLayerRec_fClipMatrix'>and</a> <a href='#SkCanvas_SaveLayerRec_fSaveLayerFlags'>fSaveLayerFlags</a>.
<a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'>clipMatrix</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'>uses</a> <a href='#Color_Alpha'>Color_Alpha</a> <a href='#Color_Alpha'>channel</a> <a href='#Color_Alpha'>of</a> <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>transformed</a> <a href='SkImage_Reference#Image'>by</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'>clipMatrix</a>, <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'>to</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'>clip</a>
<a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>when</a> <a href='SkCanvas_Reference#Layer'>drawn</a> <a href='SkCanvas_Reference#Layer'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>.

<a href='SkCanvas_Reference#Canvas'>Implementation</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>complete</a>; <a href='SkCanvas_Reference#Canvas'>has</a> <a href='SkCanvas_Reference#Canvas'>no</a> <a href='SkCanvas_Reference#Canvas'>effect</a> <a href='SkCanvas_Reference#Canvas'>if</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>GPU-backed</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>dimensions</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_paint'><code><strong>paint</strong></code></a></td>
    <td>graphics state applied to <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>when</a> <a href='SkCanvas_Reference#Layer'>overlaying</a> <a href='SkCanvas_Reference#Layer'>prior</a>
<a href='SkCanvas_Reference#Layer'>Layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a>
</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_backdrop'><code><strong>backdrop</strong></code></a></td>
    <td>prior <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>copied</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='#Image_Filter'>Image_Filter</a>;
<a href='#Image_Filter'>may</a> <a href='#Image_Filter'>be</a> <a href='#Image_Filter'>nullptr</a>
</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'><code><strong>clipMask</strong></code></a></td>
    <td>clip applied to <a href='SkCanvas_Reference#Layer'>Layer</a>; <a href='SkCanvas_Reference#Layer'>may</a> <a href='SkCanvas_Reference#Layer'>be</a> <a href='SkCanvas_Reference#Layer'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMatrix'><code><strong>clipMatrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>applied</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>clipMask</a>; <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>may</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>be</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>nullptr</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>to</a> <a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>use</a>
<a href='#SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_clipMask'>identity</a> <a href='SkMatrix_Reference#Matrix'>matrix </a>
</td>
  </tr>
  <tr>    <td><a name='SkCanvas_SaveLayerRec_const_SkRect_star_const_SkPaint_star_const_SkImageFilter_star_const_SkImage_star_const_SkMatrix_star_saveLayerFlags'><code><strong>saveLayerFlags</strong></code></a></td>
    <td><a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>options</a> <a href='#SkCanvas_SaveLayerRec'>to</a> <a href='#SkCanvas_SaveLayerRec'>modify</a> <a href='SkCanvas_Reference#Layer'>Layer</a></td>
  </tr>
</table>

### Return Value

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>fully</a> <a href='#SkCanvas_SaveLayerRec'>specified</a>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='SkCanvas_saveLayer_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkCanvas_saveLayer'>saveLayer</a>(<a href='#SkCanvas_saveLayer'>const</a> <a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a>& <a href='#SkCanvas_SaveLayerRec'>layerRec</a>)
</pre>

Saves <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>allocates</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>for</a> <a href='SkBitmap_Reference#SkBitmap'>subsequent</a> <a href='SkBitmap_Reference#SkBitmap'>drawing</a>.

Calling <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>discards</a> <a href='#SkCanvas_restore'>changes</a> <a href='#SkCanvas_restore'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>,
and blends <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>with</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>opacity</a> <a href='SkColor_Reference#Alpha'>onto</a> <a href='SkColor_Reference#Alpha'>the</a> <a href='SkColor_Reference#Alpha'>prior</a> <a href='SkCanvas_Reference#Layer'>layer</a>.

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>changed</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate'>translate()</a>, <a href='#SkCanvas_scale'>scale()</a>, <a href='#SkCanvas_rotate'>rotate()</a>, <a href='#SkCanvas_skew'>skew()</a>, <a href='#SkCanvas_concat'>concat()</a>,
<a href='#SkCanvas_setMatrix'>setMatrix</a>(), <a href='#SkCanvas_setMatrix'>and</a> <a href='#SkCanvas_resetMatrix'>resetMatrix</a>(). <a href='#SkCanvas_resetMatrix'>Clip</a> <a href='#SkCanvas_resetMatrix'>may</a> <a href='#SkCanvas_resetMatrix'>be</a> <a href='#SkCanvas_resetMatrix'>changed</a> <a href='#SkCanvas_resetMatrix'>by</a> <a href='#SkCanvas_clipRect'>clipRect</a>(), <a href='#SkCanvas_clipRRect'>clipRRect</a>(),
<a href='#SkCanvas_clipPath'>clipPath</a>(), <a href='#SkCanvas_clipRegion'>clipRegion</a>().

<a href='#SkCanvas_SaveLayerRec'>SaveLayerRec</a> <a href='#SkCanvas_SaveLayerRec'>contains</a> <a href='#SkCanvas_SaveLayerRec'>the</a> <a href='#SkCanvas_SaveLayerRec'>state</a> <a href='#SkCanvas_SaveLayerRec'>used</a> <a href='#SkCanvas_SaveLayerRec'>to</a> <a href='#SkCanvas_SaveLayerRec'>create</a> <a href='#SkCanvas_SaveLayerRec'>the</a> <a href='SkCanvas_Reference#Layer'>layer</a>.

Call <a href='#SkCanvas_restoreToCount'>restoreToCount</a>() <a href='#SkCanvas_restoreToCount'>with</a> <a href='#SkCanvas_restoreToCount'>returned</a> <a href='#SkCanvas_restoreToCount'>value</a> <a href='#SkCanvas_restoreToCount'>to</a> <a href='#SkCanvas_restoreToCount'>restore</a> <a href='#SkCanvas_restoreToCount'>this</a> <a href='#SkCanvas_restoreToCount'>and</a> <a href='#SkCanvas_restoreToCount'>subsequent</a> <a href='#SkCanvas_restoreToCount'>saves</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_saveLayer_3_layerRec'><code><strong>layerRec</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>state</a></td>
  </tr>
</table>

### Return Value

depth of save  <a href='#State_Stack'>state stack</a>

### Example

<div><fiddle-embed name="7d3751e82d1b6ec328ffa3d6f48ca831"><div>The example draws an <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>saves</a> <a href='SkImage_Reference#Image'>it</a> <a href='SkImage_Reference#Image'>into</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>kInitWithPrevious_SaveLayerFlag</a>.
<a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>Next</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>it</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>punches</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>a</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>hole</a> <a href='#SkCanvas_kInitWithPrevious_SaveLayerFlag'>in</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>and</a> <a href='SkCanvas_Reference#Layer'>restore</a> <a href='SkCanvas_Reference#Layer'>with</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kPlus'>kPlus</a>.
<a href='#SkBlendMode_kPlus'>Where</a> <a href='SkCanvas_Reference#Layer'>Layer</a> <a href='SkCanvas_Reference#Layer'>was</a> <a href='SkCanvas_Reference#Layer'>cleared</a>, <a href='SkCanvas_Reference#Layer'>the</a> <a href='SkCanvas_Reference#Layer'>original</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>will</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>unchanged</a>.
<a href='SkImage_Reference#Image'>Outside</a> <a href='SkImage_Reference#Image'>of</a> <a href='SkImage_Reference#Image'>the</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>mandrill</a> <a href='undocumented#Circle'>is</a> <a href='undocumented#Circle'>brightened</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_save'>save</a> <a href='#SkCanvas_restore'>restore</a> <a href='#SkCanvas_saveLayer'>saveLayer</a> <a href='#SkCanvas_saveLayerPreserveLCDTextRequests'>saveLayerPreserveLCDTextRequests</a> <a href='#SkCanvas_saveLayerAlpha'>saveLayerAlpha</a>

<a name='Matrix'></a>

<a name='SkCanvas_translate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void translate(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Translates <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_translate_dx'>dx</a> <a href='#SkCanvas_translate_dx'>along</a> <a href='#SkCanvas_translate_dx'>the</a> <a href='#SkCanvas_translate_dx'>x-axis</a> <a href='#SkCanvas_translate_dx'>and</a> <a href='#SkCanvas_translate_dy'>dy</a> <a href='#SkCanvas_translate_dy'>along</a> <a href='#SkCanvas_translate_dy'>the</a> <a href='#SkCanvas_translate_dy'>y-axis</a>.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of moving the drawing by (<a href='#SkCanvas_translate_dx'>dx</a>, <a href='#SkCanvas_translate_dy'>dy</a>) <a href='#SkCanvas_translate_dy'>before</a> <a href='#SkCanvas_translate_dy'>transforming</a>
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

<div><fiddle-embed name="eb93d5fa66a5f7a10f4f9210494d7222"><div><a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_scale'>followed</a> <a href='#SkCanvas_scale'>by</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_translate'>produces</a> <a href='#SkCanvas_translate'>different</a> <a href='#SkCanvas_translate'>results</a> <a href='#SkCanvas_translate'>from</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_translate'>followed</a>
<a href='#SkCanvas_translate'>by</a> <a href='#SkCanvas_scale'>scale()</a>.

<a href='#SkCanvas_scale'>The</a> <a href='#SkCanvas_scale'>blue</a> <a href='#SkCanvas_scale'>stroke</a> <a href='#SkCanvas_scale'>follows</a> <a href='#SkCanvas_scale'>translate</a> <a href='#SkCanvas_scale'>of</a> (50, 50); <a href='#SkCanvas_scale'>a</a> <a href='#SkCanvas_scale'>black</a>
<a href='#SkCanvas_scale'>fill</a> <a href='#SkCanvas_scale'>follows</a> <a href='#SkCanvas_scale'>scale</a> <a href='#SkCanvas_scale'>of</a> (2, 1/2.<a href='#SkCanvas_scale'>f</a>). <a href='#SkCanvas_scale'>After</a> <a href='#SkCanvas_scale'>restoring</a> <a href='#SkCanvas_scale'>the</a> <a href='#SkCanvas_scale'>clip</a>, <a href='#SkCanvas_scale'>which</a> <a href='#SkCanvas_scale'>resets</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>a</a> <a href='SkMatrix_Reference#Matrix'>red</a> <a href='SkMatrix_Reference#Matrix'>frame</a> <a href='SkMatrix_Reference#Matrix'>follows</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>same</a> <a href='SkMatrix_Reference#Matrix'>scale</a> <a href='SkMatrix_Reference#Matrix'>of</a> (2, 1/2.<a href='SkMatrix_Reference#Matrix'>f</a>); <a href='SkMatrix_Reference#Matrix'>a</a> <a href='SkMatrix_Reference#Matrix'>gray</a> <a href='SkMatrix_Reference#Matrix'>fill</a>
<a href='SkMatrix_Reference#Matrix'>follows</a> <a href='SkMatrix_Reference#Matrix'>translate</a> <a href='SkMatrix_Reference#Matrix'>of</a> (50, 50).
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_scale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void scale(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>)
</pre>

Scales <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_scale_sx'>sx</a> <a href='#SkCanvas_scale_sx'>on</a> <a href='#SkCanvas_scale_sx'>the</a> <a href='#SkCanvas_scale_sx'>x-axis</a> <a href='#SkCanvas_scale_sx'>and</a> <a href='#SkCanvas_scale_sy'>sy</a> <a href='#SkCanvas_scale_sy'>on</a> <a href='#SkCanvas_scale_sy'>the</a> <a href='#SkCanvas_scale_sy'>y-axis</a>.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of scaling the drawing by (<a href='#SkCanvas_scale_sx'>sx</a>, <a href='#SkCanvas_scale_sy'>sy</a>) <a href='#SkCanvas_scale_sy'>before</a> <a href='#SkCanvas_scale_sy'>transforming</a>
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
void rotate(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>)
</pre>

Rotates <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_rotate_degrees'>degrees</a>. <a href='#SkCanvas_rotate_degrees'>Positive</a> <a href='#SkCanvas_rotate_degrees'>degrees</a> <a href='#SkCanvas_rotate_degrees'>rotates</a> <a href='#SkCanvas_rotate_degrees'>clockwise</a>.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkMatrix_Reference#SkMatrix'>rotation</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of rotating the drawing by <a href='#SkCanvas_rotate_degrees'>degrees</a> <a href='#SkCanvas_rotate_degrees'>before</a> <a href='#SkCanvas_rotate_degrees'>transforming</a>
the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_rotate_degrees'><code><strong>degrees</strong></code></a></td>
    <td>amount to rotate, in <a href='#SkCanvas_rotate_degrees'>degrees</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="963789ac8498d4e505748ab3b15cdaa5"><div>Draw clock hands at time 5:10. The hour hand and minute hand <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>up</a> <a href='SkPoint_Reference#Point'>and</a>
<a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>rotated</a> <a href='SkPoint_Reference#Point'>clockwise</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_rotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void rotate(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Rotates <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_rotate_2_degrees'>degrees</a> <a href='#SkCanvas_rotate_2_degrees'>about</a> <a href='#SkCanvas_rotate_2_degrees'>a</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (<a href='#SkCanvas_rotate_2_px'>px</a>, <a href='#SkCanvas_rotate_2_py'>py</a>). <a href='#SkCanvas_rotate_2_py'>Positive</a> <a href='#SkCanvas_rotate_2_degrees'>degrees</a> <a href='#SkCanvas_rotate_2_degrees'>rotates</a>
clockwise.

Mathematically, constructs a rotation <a href='SkMatrix_Reference#Matrix'>matrix</a>; <a href='undocumented#Premultiply'>premultiplies</a> <a href='undocumented#Premultiply'>the</a> <a href='undocumented#Premultiply'>rotation</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>by</a>
a translation <a href='SkMatrix_Reference#Matrix'>matrix</a>; <a href='SkMatrix_Reference#Matrix'>then</a> <a href='SkMatrix_Reference#Matrix'>replaces</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkMatrix_Reference#SkMatrix'>resulting</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>
<a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of rotating the drawing about a given <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>before</a>
transforming the result with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_rotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>amount to rotate, in <a href='#SkCanvas_rotate_2_degrees'>degrees</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_rotate_2_px'><code><strong>px</strong></code></a></td>
    <td>x-axis value of the <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>rotate</a> <a href='SkPoint_Reference#Point'>about</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_rotate_2_py'><code><strong>py</strong></code></a></td>
    <td>y-axis value of the <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>rotate</a> <a href='SkPoint_Reference#Point'>about</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bcf5baea1c66a957d5ffd7b54bbbfeff"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_skew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void skew(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>)
</pre>

Skews <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_skew_sx'>sx</a> <a href='#SkCanvas_skew_sx'>on</a> <a href='#SkCanvas_skew_sx'>the</a> <a href='#SkCanvas_skew_sx'>x-axis</a> <a href='#SkCanvas_skew_sx'>and</a> <a href='#SkCanvas_skew_sy'>sy</a> <a href='#SkCanvas_skew_sy'>on</a> <a href='#SkCanvas_skew_sy'>the</a> <a href='#SkCanvas_skew_sy'>y-axis</a>. <a href='#SkCanvas_skew_sy'>A</a> <a href='#SkCanvas_skew_sy'>positive</a> <a href='#SkCanvas_skew_sy'>value</a> <a href='#SkCanvas_skew_sy'>of</a> <a href='#SkCanvas_skew_sx'>sx</a>
skews the drawing right as y-axis values increase; a positive value of <a href='#SkCanvas_skew_sy'>sy</a> <a href='#SkCanvas_skew_sy'>skews</a>
the drawing down as x-axis values increase.

Mathematically, replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkMatrix_Reference#SkMatrix'>skew</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of skewing the drawing by (<a href='#SkCanvas_skew_sx'>sx</a>, <a href='#SkCanvas_skew_sy'>sy</a>) <a href='#SkCanvas_skew_sy'>before</a> <a href='#SkCanvas_skew_sy'>transforming</a>
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

<div><fiddle-embed name="2e2acc21d7774df7e0940a30ad2ca99e"><div>Black <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>mimics</a> <a href='undocumented#Text'>an</a> <a href='undocumented#Text'>oblique</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>style</a> <a href='undocumented#Text'>by</a> <a href='undocumented#Text'>using</a> <a href='undocumented#Text'>a</a> <a href='undocumented#Text'>negative</a> <a href='undocumented#Text'>skew</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>x-axis</a>
<a href='undocumented#Text'>that</a> <a href='undocumented#Text'>shifts</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>geometry</a> <a href='undocumented#Text'>to</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>right</a> <a href='undocumented#Text'>as</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>y-axis</a> <a href='undocumented#Text'>values</a> <a href='undocumented#Text'>decrease</a>.
<a href='undocumented#Text'>Red</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>uses</a> <a href='undocumented#Text'>a</a> <a href='undocumented#Text'>positive</a> <a href='undocumented#Text'>skew</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>y-axis</a> <a href='undocumented#Text'>to</a> <a href='undocumented#Text'>shift</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>geometry</a> <a href='undocumented#Text'>down</a>
<a href='undocumented#Text'>as</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>x-axis</a> <a href='undocumented#Text'>values</a> <a href='undocumented#Text'>increase</a>.
<a href='undocumented#Text'>Blue</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>combines</a> <a href='#SkCanvas_skew_sx'>sx</a> <a href='#SkCanvas_skew_sx'>and</a> <a href='#SkCanvas_skew_sy'>sy</a> <a href='#SkCanvas_skew_sy'>skew</a> <a href='#SkCanvas_skew_sy'>to</a> <a href='#SkCanvas_skew_sy'>rotate</a> <a href='#SkCanvas_skew_sy'>and</a> <a href='#SkCanvas_skew_sy'>scale</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_concat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_concat'>concat</a>(<a href='#SkCanvas_concat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>)
</pre>

Replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='#SkCanvas_concat_matrix'>matrix</a> <a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>with</a> <a href='undocumented#Premultiply'>existing</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

This has the effect of transforming the drawn geometry by <a href='#SkCanvas_concat_matrix'>matrix</a>, <a href='#SkCanvas_concat_matrix'>before</a>
transforming the result with existing <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_concat_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#SkCanvas_concat_matrix'>matrix</a> <a href='#SkCanvas_concat_matrix'>to</a> <a href='undocumented#Premultiply'>premultiply</a> <a href='undocumented#Premultiply'>with</a> <a href='undocumented#Premultiply'>existing</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8f6818b25a92a88638ad99b2dd293f61"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a> <a href='#SkCanvas_setMatrix'>setMatrix</a>

<a name='SkCanvas_setMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_setMatrix'>setMatrix</a>(<a href='#SkCanvas_setMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>)
</pre>

Replaces <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='#SkCanvas_setMatrix_matrix'>matrix</a>.
Unlike <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_concat'>any</a> <a href='#SkCanvas_concat'>prior</a> <a href='#SkCanvas_setMatrix_matrix'>matrix</a> <a href='#SkCanvas_setMatrix_matrix'>state</a> <a href='#SkCanvas_setMatrix_matrix'>is</a> <a href='#SkCanvas_setMatrix_matrix'>overwritten</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_setMatrix_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='#SkCanvas_setMatrix_matrix'>matrix</a> <a href='#SkCanvas_setMatrix_matrix'>to</a> <a href='#SkCanvas_setMatrix_matrix'>copy</a>, <a href='#SkCanvas_setMatrix_matrix'>replacing</a> <a href='#SkCanvas_setMatrix_matrix'>existing</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
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

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.
Any prior <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>state</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>overwritten</a>.

### Example

<div><fiddle-embed name="412afffdf4682baa503a4e2e99201967"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_setMatrix'>setMatrix</a> <a href='#SkCanvas_concat'>concat()</a> <a href='#SkCanvas_translate'>translate()</a> <a href='#SkCanvas_rotate'>rotate()</a> <a href='#SkCanvas_scale'>scale()</a> <a href='#SkCanvas_skew'>skew()</a>

<a name='SkCanvas_getTotalMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkCanvas_getTotalMatrix'>getTotalMatrix</a>() <a href='#SkCanvas_getTotalMatrix'>const</a>
</pre>

Returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This does not account for translation by <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>or</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>in</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>

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

Clip is built from a stack of clipping <a href='SkPath_Reference#Path'>paths</a>. <a href='SkPath_Reference#Path'>Each</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>in</a> <a href='SkPath_Reference#Path'>the</a>
<a href='SkPath_Reference#Path'>stack</a> <a href='SkPath_Reference#Path'>can</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>constructed</a> <a href='SkPath_Reference#Path'>from</a> <a href='SkPath_Reference#Path'>one</a> <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>more</a> <a href='#Path_Overview_Contour'>Path_Contour</a> <a href='#Path_Overview_Contour'>elements</a>. <a href='#Path_Overview_Contour'>The</a>
<a href='#Path_Overview_Contour'>Path_Contour</a> <a href='#Path_Overview_Contour'>may</a> <a href='#Path_Overview_Contour'>be</a> <a href='#Path_Overview_Contour'>composed</a> <a href='#Path_Overview_Contour'>of</a> <a href='#Path_Overview_Contour'>any</a> <a href='#Path_Overview_Contour'>number</a> <a href='#Path_Overview_Contour'>of</a> <a href='#Path_Verb'>Path_Verb</a> <a href='#Path_Verb'>segments</a>. <a href='#Path_Verb'>Each</a>
<a href='#Path_Overview_Contour'>Path_Contour</a> <a href='#Path_Overview_Contour'>forms</a> <a href='#Path_Overview_Contour'>a</a> <a href='#Path_Overview_Contour'>closed</a> <a href='#Path_Overview_Contour'>area</a>; <a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Fill_Type'>defines</a> <a href='#Path_Fill_Type'>the</a> <a href='#Path_Fill_Type'>area</a> <a href='#Path_Fill_Type'>enclosed</a>
<a href='#Path_Fill_Type'>by</a> <a href='#Path_Overview_Contour'>Path_Contour</a>.

<a href='#Path_Overview_Contour'>Clip</a> <a href='#Path_Overview_Contour'>stack</a> <a href='#Path_Overview_Contour'>of</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>elements</a> <a href='SkPath_Reference#Path'>successfully</a> <a href='SkPath_Reference#Path'>restrict</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>area</a>. <a href='SkPath_Reference#Path'>Each</a>
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>transformed</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>then</a> <a href='SkMatrix_Reference#Matrix'>intersected</a> <a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>subtracted</a> <a href='SkMatrix_Reference#Matrix'>from</a> <a href='SkMatrix_Reference#Matrix'>the</a>
<a href='SkMatrix_Reference#Matrix'>prior</a> <a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>form</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>replacement</a> <a href='SkMatrix_Reference#Matrix'>Clip</a>. <a href='SkMatrix_Reference#Matrix'>Use</a> <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kDifference'>kDifference</a>
<a href='#SkClipOp_kDifference'>to</a> <a href='#SkClipOp_kDifference'>subtract</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>from</a> <a href='SkPath_Reference#Path'>Clip</a>; <a href='SkPath_Reference#Path'>use</a> <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a> <a href='#SkClipOp_kIntersect'>to</a> <a href='#SkClipOp_kIntersect'>intersect</a> <a href='SkPath_Reference#Path'>Path</a>
<a href='SkPath_Reference#Path'>with</a> <a href='SkPath_Reference#Path'>Clip</a>.

<a href='SkPath_Reference#Path'>A</a> <a href='SkPath_Reference#Path'>clipping</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>be</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a>; <a href='#Paint_Anti_Alias'>if</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>after</a> <a href='SkPath_Reference#Path'>transformation</a>, <a href='SkPath_Reference#Path'>is</a>
<a href='SkPath_Reference#Path'>composed</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>horizontal</a> <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>vertical</a> <a href='undocumented#Line'>lines</a>, <a href='undocumented#Line'>clearing</a> <a href='#Paint_Anti_Alias'>Anti_Alias</a> <a href='#Paint_Anti_Alias'>allows</a> <a href='#Paint_Anti_Alias'>whole</a> <a href='#Paint_Anti_Alias'>pixels</a>
<a href='#Paint_Anti_Alias'>to</a> <a href='#Paint_Anti_Alias'>either</a> <a href='#Paint_Anti_Alias'>be</a> <a href='#Paint_Anti_Alias'>inside</a> <a href='#Paint_Anti_Alias'>or</a> <a href='#Paint_Anti_Alias'>outside</a> <a href='#Paint_Anti_Alias'>the</a> <a href='#Paint_Anti_Alias'>clip</a>. <a href='#Paint_Anti_Alias'>The</a> <a href='#Paint_Anti_Alias'>fastest</a> <a href='#Paint_Anti_Alias'>drawing</a> <a href='#Paint_Anti_Alias'>has</a> <a href='#Paint_Anti_Alias'>a</a> <a href='undocumented#Alias'>Aliased</a>,
<a href='undocumented#Alias'>rectangular</a> <a href='undocumented#Alias'>clip</a>.

<a href='undocumented#Alias'>If</a> <a href='undocumented#Alias'>clipping</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>has</a> <a href='#Paint_Anti_Alias'>Anti_Alias</a> <a href='#Paint_Anti_Alias'>set</a>, <a href='#Paint_Anti_Alias'>clip</a> <a href='#Paint_Anti_Alias'>may</a> <a href='#Paint_Anti_Alias'>partially</a> <a href='#Paint_Anti_Alias'>clip</a> <a href='#Paint_Anti_Alias'>a</a> <a href='undocumented#Pixel'>pixel</a>, <a href='undocumented#Pixel'>requiring</a>
<a href='undocumented#Pixel'>that</a> <a href='undocumented#Pixel'>drawing</a> <a href='undocumented#Pixel'>blend</a> <a href='undocumented#Pixel'>partially</a> <a href='undocumented#Pixel'>with</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>destination</a> <a href='undocumented#Pixel'>along</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>edge</a>. <a href='undocumented#Pixel'>A</a> <a href='undocumented#Pixel'>rotated</a>
<a href='undocumented#Pixel'>rectangular</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='#Paint_Anti_Alias'>clip</a> <a href='#Paint_Anti_Alias'>looks</a> <a href='#Paint_Anti_Alias'>smoother</a> <a href='#Paint_Anti_Alias'>but</a> <a href='#Paint_Anti_Alias'>draws</a> <a href='#Paint_Anti_Alias'>slower</a>.

<a href='#Paint_Anti_Alias'>Clip</a> <a href='#Paint_Anti_Alias'>can</a> <a href='#Paint_Anti_Alias'>combine</a> <a href='#Paint_Anti_Alias'>with</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>and</a> <a href='#RRect'>Round_Rect</a> <a href='#RRect'>primitives</a>; <a href='#RRect'>like</a>
<a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>these</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>transformed</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>before</a> <a href='SkMatrix_Reference#Matrix'>they</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>combined</a> <a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkMatrix_Reference#Matrix'>Clip</a>.

<a href='SkMatrix_Reference#Matrix'>Clip</a> <a href='SkMatrix_Reference#Matrix'>can</a> <a href='SkMatrix_Reference#Matrix'>combine</a> <a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkRegion_Reference#Region'>Region</a>. <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>is</a> <a href='SkRegion_Reference#Region'>assumed</a> <a href='SkRegion_Reference#Region'>to</a> <a href='SkRegion_Reference#Region'>be</a> <a href='SkRegion_Reference#Region'>in</a> <a href='undocumented#Device'>Device</a> <a href='undocumented#Device'>coordinates</a>
<a href='undocumented#Device'>and</a> <a href='undocumented#Device'>is</a> <a href='undocumented#Device'>unaffected</a> <a href='undocumented#Device'>by</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>.

### Example

<div><fiddle-embed name="862cc026601a41a58df49c0b9f0d7777"><div>Draw a red <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>with</a> <a href='undocumented#Circle'>an</a> <a href='undocumented#Alias'>Aliased</a> <a href='undocumented#Alias'>clip</a> <a href='undocumented#Alias'>and</a> <a href='undocumented#Alias'>an</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='#Paint_Anti_Alias'>clip</a>.
<a href='#Paint_Anti_Alias'>Use</a> <a href='#Paint_Anti_Alias'>an</a>  <a href='SkImage_Reference#Image'>image filter</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>zoom</a> <a href='SkImage_Reference#Image'>into</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>drawn</a>.
<a href='SkImage_Reference#Image'>The</a> <a href='SkImage_Reference#Image'>edge</a> <a href='SkImage_Reference#Image'>of</a> <a href='SkImage_Reference#Image'>the</a> <a href='undocumented#Alias'>Aliased</a> <a href='undocumented#Alias'>clip</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>draws</a> <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>in</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>red</a> <a href='undocumented#Circle'>circle</a>.
<a href='undocumented#Circle'>The</a> <a href='undocumented#Circle'>edge</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>the</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a> <a href='#Paint_Anti_Alias'>clip</a> <a href='#Paint_Anti_Alias'>partially</a> <a href='#Paint_Anti_Alias'>draws</a> <a href='#Paint_Anti_Alias'>pixels</a> <a href='#Paint_Anti_Alias'>in</a> <a href='#Paint_Anti_Alias'>the</a> <a href='#Paint_Anti_Alias'>red</a> <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

<a name='SkCanvas_clipRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(<a href='#SkCanvas_clipRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>, <a href='undocumented#SkClipOp'>bool</a> <a href='undocumented#SkClipOp'>doAntiAlias</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRect_rect'>rect</a>,
with an <a href='undocumented#Alias'>aliased</a> <a href='undocumented#Alias'>or</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> <a href='SkPaint_Reference#Anti_Alias'>clip</a> <a href='SkPaint_Reference#Anti_Alias'>edge</a>. <a href='#SkCanvas_clipRect_rect'>rect</a> <a href='#SkCanvas_clipRect_rect'>is</a> <a href='#SkCanvas_clipRect_rect'>transformed</a> <a href='#SkCanvas_clipRect_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>combine</a> <a href='SkRect_Reference#SkRect'>with</a> <a href='SkRect_Reference#SkRect'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
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
void <a href='#SkCanvas_clipRect'>clipRect</a>(<a href='#SkCanvas_clipRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRect_2_rect'>rect</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>contained</a> <a href='undocumented#Alias'>by</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>clip</a>.
<a href='#SkCanvas_clipRect_2_rect'>rect</a> <a href='#SkCanvas_clipRect_2_rect'>is</a> <a href='#SkCanvas_clipRect_2_rect'>transformed</a> <a href='#SkCanvas_clipRect_2_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>before</a> <a href='SkMatrix_Reference#SkMatrix'>it</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>combined</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>combine</a> <a href='SkRect_Reference#SkRect'>with</a> <a href='SkRect_Reference#SkRect'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="13bbc5fa5597a6cd4d704b419dbc66d9"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRect'>clipRect</a>(<a href='#SkCanvas_clipRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>bool</a> <a href='SkRect_Reference#Rect'>doAntiAlias</a> = <a href='SkRect_Reference#Rect'>false</a>)
</pre>

Replaces clip with the intersection of clip and <a href='#SkCanvas_clipRect_3_rect'>rect</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>contained</a> <a href='undocumented#Alias'>by</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>clip</a>.
<a href='#SkCanvas_clipRect_3_rect'>rect</a> <a href='#SkCanvas_clipRect_3_rect'>is</a> <a href='#SkCanvas_clipRect_3_rect'>transformed</a> <a href='#SkCanvas_clipRect_3_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRect_3_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>combine</a> <a href='SkRect_Reference#SkRect'>with</a> <a href='SkRect_Reference#SkRect'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRect_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d4e0632c97e42692775d834fe10aa99"><div>A <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>drawn</a> <a href='undocumented#Circle'>in</a> <a href='undocumented#Circle'>pieces</a> <a href='undocumented#Circle'>looks</a> <a href='undocumented#Circle'>uniform</a> <a href='undocumented#Circle'>when</a> <a href='undocumented#Circle'>drawn</a> <a href='undocumented#Alias'>Aliased</a>.
<a href='undocumented#Alias'>The</a> <a href='undocumented#Alias'>same</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>pieces</a> <a href='undocumented#Circle'>blend</a> <a href='undocumented#Circle'>with</a> <a href='undocumented#Circle'>pixels</a> <a href='undocumented#Circle'>more</a> <a href='undocumented#Circle'>than</a> <a href='undocumented#Circle'>once</a> <a href='undocumented#Circle'>when</a> <a href='#Paint_Anti_Alias'>Anti_Aliased</a>,
<a href='#Paint_Anti_Alias'>visible</a> <a href='#Paint_Anti_Alias'>as</a> <a href='#Paint_Anti_Alias'>a</a> <a href='#Paint_Anti_Alias'>thin</a> <a href='#Paint_Anti_Alias'>pair</a> <a href='#Paint_Anti_Alias'>of</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>through</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>right</a> <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_androidFramework_setDeviceClipRestriction'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_androidFramework_setDeviceClipRestriction'>androidFramework_setDeviceClipRestriction</a>(<a href='#SkCanvas_androidFramework_setDeviceClipRestriction'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)
</pre>

Sets the maximum clip rectangle, which can be set by <a href='#SkCanvas_clipRect'>clipRect</a>, <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRRect'>and</a>
<a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipPath'>and</a> <a href='#SkCanvas_clipPath'>intersect</a> <a href='#SkCanvas_clipPath'>the</a> <a href='#SkCanvas_clipPath'>current</a> <a href='#SkCanvas_clipPath'>clip</a> <a href='#SkCanvas_clipPath'>with</a> <a href='#SkCanvas_clipPath'>the</a> <a href='#SkCanvas_clipPath'>specified</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>rect</a>.
<a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>The</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>maximum</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>clip</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>affects</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>only</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>future</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>clipping</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>operations</a>; <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>it</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>is</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>not</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>retroactive</a>.
<a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>The</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>clip</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>restriction</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>is</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>not</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>recorded</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>in</a> <a href='SkPicture_Reference#Picture'>pictures</a>.

<a href='SkPicture_Reference#Picture'>Pass</a> <a href='SkPicture_Reference#Picture'>an</a> <a href='SkPicture_Reference#Picture'>empty</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>rect</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>to</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>disable</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>maximum</a> <a href='#SkCanvas_androidFramework_setDeviceClipRestriction_rect'>clip</a>.

Private: This private API is for use by Android framework only.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_androidFramework_setDeviceClipRestriction_rect'><code><strong>rect</strong></code></a></td>
    <td>maximum allowed clip in <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>coordinates</a></td>
  </tr>
#

<a name='SkCanvas_clipRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(<a href='#SkCanvas_clipRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>, <a href='undocumented#SkClipOp'>bool</a> <a href='undocumented#SkClipOp'>doAntiAlias</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRRect_rrect'>rrect</a>,
with an <a href='undocumented#Alias'>aliased</a> <a href='undocumented#Alias'>or</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> <a href='SkPaint_Reference#Anti_Alias'>clip</a> <a href='SkPaint_Reference#Anti_Alias'>edge</a>.
<a href='#SkCanvas_clipRRect_rrect'>rrect</a> <a href='#SkCanvas_clipRRect_rrect'>is</a> <a href='#SkCanvas_clipRRect_rrect'>transformed</a> <a href='#SkCanvas_clipRRect_rrect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>combine</a> <a href='SkRRect_Reference#SkRRect'>with</a> <a href='SkRRect_Reference#SkRRect'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
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
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(<a href='#SkCanvas_clipRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipRRect_2_rrect'>rrect</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>contained</a> <a href='undocumented#Alias'>by</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>clip</a>.
<a href='#SkCanvas_clipRRect_2_rrect'>rrect</a> <a href='#SkCanvas_clipRRect_2_rrect'>is</a> <a href='#SkCanvas_clipRRect_2_rrect'>transformed</a> <a href='#SkCanvas_clipRRect_2_rrect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>before</a> <a href='SkMatrix_Reference#SkMatrix'>it</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>combined</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_2_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>combine</a> <a href='SkRRect_Reference#SkRRect'>with</a> <a href='SkRRect_Reference#SkRRect'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRRect_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ef6ae2eaae6761130ce38065d0364abd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipRRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRRect'>clipRRect</a>(<a href='#SkCanvas_clipRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='SkRRect_Reference#SkRRect'>bool</a> <a href='SkRRect_Reference#SkRRect'>doAntiAlias</a> = <a href='SkRRect_Reference#SkRRect'>false</a>)
</pre>

Replaces clip with the intersection of clip and <a href='#SkCanvas_clipRRect_3_rrect'>rrect</a>,
with an <a href='undocumented#Alias'>aliased</a> <a href='undocumented#Alias'>or</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> <a href='SkPaint_Reference#Anti_Alias'>clip</a> <a href='SkPaint_Reference#Anti_Alias'>edge</a>.
<a href='#SkCanvas_clipRRect_3_rrect'>rrect</a> <a href='#SkCanvas_clipRRect_3_rrect'>is</a> <a href='#SkCanvas_clipRRect_3_rrect'>transformed</a> <a href='#SkCanvas_clipRRect_3_rrect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>before</a> <a href='SkMatrix_Reference#SkMatrix'>it</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>combined</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRRect_3_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>combine</a> <a href='SkRRect_Reference#SkRRect'>with</a> <a href='SkRRect_Reference#SkRRect'>clip</a></td>
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
void <a href='#SkCanvas_clipPath'>clipPath</a>(<a href='#SkCanvas_clipPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>, <a href='undocumented#SkClipOp'>bool</a> <a href='undocumented#SkClipOp'>doAntiAlias</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipPath_path'>path</a>,
with an <a href='undocumented#Alias'>aliased</a> <a href='undocumented#Alias'>or</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a> <a href='SkPaint_Reference#Anti_Alias'>clip</a> <a href='SkPaint_Reference#Anti_Alias'>edge</a>. <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>determines</a> <a href='#SkPath_FillType'>if</a> <a href='#SkCanvas_clipPath_path'>path</a>
describes the area inside or outside its <a href='SkPath_Overview#Contour'>contours</a>; <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>if</a>  <a href='SkPath_Overview#Contour'>path contour</a> <a href='#SkCanvas_clipPath_path'>overlaps</a>
itself or another  <a href='SkPath_Overview#Contour'>path contour</a>, <a href='#SkCanvas_clipPath_path'>whether</a> <a href='#SkCanvas_clipPath_path'>the</a> <a href='#SkCanvas_clipPath_path'>overlaps</a> <a href='#SkCanvas_clipPath_path'>form</a> <a href='#SkCanvas_clipPath_path'>part</a> <a href='#SkCanvas_clipPath_path'>of</a> <a href='#SkCanvas_clipPath_path'>the</a> <a href='#SkCanvas_clipPath_path'>area</a>.
<a href='#SkCanvas_clipPath_path'>path</a> <a href='#SkCanvas_clipPath_path'>is</a> <a href='#SkCanvas_clipPath_path'>transformed</a> <a href='#SkCanvas_clipPath_path'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>before</a> <a href='SkMatrix_Reference#SkMatrix'>it</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>combined</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>combine</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee47ae6b813bfaa55e1a7b7c053ed60d"><div>Top figure uses <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> <a href='#SkPath_kInverseWinding_FillType'>and</a> <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kDifference'>kDifference</a>;
<a href='#SkClipOp_kDifference'>area</a> <a href='#SkClipOp_kDifference'>outside</a> <a href='#SkClipOp_kDifference'>clip</a> <a href='#SkClipOp_kDifference'>is</a> <a href='#SkClipOp_kDifference'>subtracted</a> <a href='#SkClipOp_kDifference'>from</a> <a href='undocumented#Circle'>circle</a>.

<a href='undocumented#Circle'>Bottom</a> <a href='undocumented#Circle'>figure</a> <a href='undocumented#Circle'>uses</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> <a href='#SkPath_kWinding_FillType'>and</a> <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a>;
<a href='#SkClipOp_kIntersect'>area</a> <a href='#SkClipOp_kIntersect'>inside</a> <a href='#SkClipOp_kIntersect'>clip</a> <a href='#SkClipOp_kIntersect'>is</a> <a href='#SkClipOp_kIntersect'>intersected</a> <a href='#SkClipOp_kIntersect'>with</a> <a href='undocumented#Circle'>circle</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipPath_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(<a href='#SkCanvas_clipPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='#SkCanvas_clipPath_2_path'>path</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>contained</a> <a href='undocumented#Alias'>by</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>clip</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>determines</a> <a href='#SkPath_FillType'>if</a> <a href='#SkCanvas_clipPath_2_path'>path</a>
describes the area inside or outside its <a href='SkPath_Overview#Contour'>contours</a>; <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>if</a>  <a href='SkPath_Overview#Contour'>path contour</a> <a href='#SkCanvas_clipPath_2_path'>overlaps</a>
itself or another  <a href='SkPath_Overview#Contour'>path contour</a>, <a href='#SkCanvas_clipPath_2_path'>whether</a> <a href='#SkCanvas_clipPath_2_path'>the</a> <a href='#SkCanvas_clipPath_2_path'>overlaps</a> <a href='#SkCanvas_clipPath_2_path'>form</a> <a href='#SkCanvas_clipPath_2_path'>part</a> <a href='#SkCanvas_clipPath_2_path'>of</a> <a href='#SkCanvas_clipPath_2_path'>the</a> <a href='#SkCanvas_clipPath_2_path'>area</a>.
<a href='#SkCanvas_clipPath_2_path'>path</a> <a href='#SkCanvas_clipPath_2_path'>is</a> <a href='#SkCanvas_clipPath_2_path'>transformed</a> <a href='#SkCanvas_clipPath_2_path'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
before it is combined with clip.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_2_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>combine</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_2_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7856755c1bf8431c286c734b353345ad"><div>Overlapping <a href='SkRect_Reference#Rect'>Rects</a> <a href='SkRect_Reference#Rect'>form</a> <a href='SkRect_Reference#Rect'>a</a> <a href='SkRect_Reference#Rect'>clip</a>. <a href='SkRect_Reference#Rect'>When</a> <a href='SkRect_Reference#Rect'>clip</a> <a href='#Path_Fill_Type'>Path_Fill_Type</a> <a href='#Path_Fill_Type'>is</a> <a href='#Path_Fill_Type'>set</a> <a href='#Path_Fill_Type'>to</a>
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, <a href='#SkPath_kWinding_FillType'>the</a> <a href='#SkPath_kWinding_FillType'>overlap</a> <a href='#SkPath_kWinding_FillType'>is</a> <a href='#SkPath_kWinding_FillType'>included</a>. <a href='#SkPath_kWinding_FillType'>Set</a> <a href='#SkPath_kWinding_FillType'>to</a>
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>the</a> <a href='#SkPath_kEvenOdd_FillType'>overlap</a> <a href='#SkPath_kEvenOdd_FillType'>is</a> <a href='#SkPath_kEvenOdd_FillType'>excluded</a> <a href='#SkPath_kEvenOdd_FillType'>and</a> <a href='#SkPath_kEvenOdd_FillType'>forms</a> <a href='#SkPath_kEvenOdd_FillType'>a</a> <a href='#SkPath_kEvenOdd_FillType'>hole</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_clipPath_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipPath'>clipPath</a>(<a href='#SkCanvas_clipPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>doAntiAlias</a> = <a href='SkPath_Reference#Path'>false</a>)
</pre>

Replaces clip with the intersection of clip and <a href='#SkCanvas_clipPath_3_path'>path</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>contained</a> <a href='undocumented#Alias'>by</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>clip</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>determines</a> <a href='#SkPath_FillType'>if</a> <a href='#SkCanvas_clipPath_3_path'>path</a>
describes the area inside or outside its <a href='SkPath_Overview#Contour'>contours</a>; <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>if</a>  <a href='SkPath_Overview#Contour'>path contour</a> <a href='#SkCanvas_clipPath_3_path'>overlaps</a>
itself or another  <a href='SkPath_Overview#Contour'>path contour</a>, <a href='#SkCanvas_clipPath_3_path'>whether</a> <a href='#SkCanvas_clipPath_3_path'>the</a> <a href='#SkCanvas_clipPath_3_path'>overlaps</a> <a href='#SkCanvas_clipPath_3_path'>form</a> <a href='#SkCanvas_clipPath_3_path'>part</a> <a href='#SkCanvas_clipPath_3_path'>of</a> <a href='#SkCanvas_clipPath_3_path'>the</a> <a href='#SkCanvas_clipPath_3_path'>area</a>.
<a href='#SkCanvas_clipPath_3_path'>path</a> <a href='#SkCanvas_clipPath_3_path'>is</a> <a href='#SkCanvas_clipPath_3_path'>transformed</a> <a href='#SkCanvas_clipPath_3_path'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>before</a> <a href='SkMatrix_Reference#SkMatrix'>it</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>combined</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipPath_3_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>combine</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipPath_3_doAntiAlias'><code><strong>doAntiAlias</strong></code></a></td>
    <td>true if clip is to be <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="187a7ae77a8176e417181411988534b6"><div>Clip loops over itself covering its center twice. When clip <a href='#Path_Fill_Type'>Path_Fill_Type</a>
<a href='#Path_Fill_Type'>is</a> <a href='#Path_Fill_Type'>set</a> <a href='#Path_Fill_Type'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, <a href='#SkPath_kWinding_FillType'>the</a> <a href='#SkPath_kWinding_FillType'>overlap</a> <a href='#SkPath_kWinding_FillType'>is</a> <a href='#SkPath_kWinding_FillType'>included</a>. <a href='#SkPath_kWinding_FillType'>Set</a> <a href='#SkPath_kWinding_FillType'>to</a>
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>the</a> <a href='#SkPath_kEvenOdd_FillType'>overlap</a> <a href='#SkPath_kEvenOdd_FillType'>is</a> <a href='#SkPath_kEvenOdd_FillType'>excluded</a> <a href='#SkPath_kEvenOdd_FillType'>and</a> <a href='#SkPath_kEvenOdd_FillType'>forms</a> <a href='#SkPath_kEvenOdd_FillType'>a</a> <a href='#SkPath_kEvenOdd_FillType'>hole</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipRegion'>clipRegion</a>

<a name='SkCanvas_setAllowSimplifyClip'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_setAllowSimplifyClip'>setAllowSimplifyClip</a>(<a href='#SkCanvas_setAllowSimplifyClip'>bool</a> <a href='#SkCanvas_setAllowSimplifyClip'>allow</a>)
</pre>

Experimental. For testing only.

Set to simplify clip stack using <a href='undocumented#PathOps'>PathOps</a>.

<a name='SkCanvas_clipRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_clipRegion'>clipRegion</a>(<a href='#SkCanvas_clipRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>deviceRgn</a>, <a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>op</a> = <a href='undocumented#SkClipOp'>SkClipOp</a>::<a href='#SkClipOp_kIntersect'>kIntersect</a>)
</pre>

Replaces clip with the intersection or difference of clip and <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkCanvas_clipRegion_deviceRgn'>deviceRgn</a>.
Resulting clip is <a href='undocumented#Alias'>aliased</a>; <a href='undocumented#Alias'>pixels</a> <a href='undocumented#Alias'>are</a> <a href='undocumented#Alias'>fully</a> <a href='undocumented#Alias'>contained</a> <a href='undocumented#Alias'>by</a> <a href='undocumented#Alias'>the</a> <a href='undocumented#Alias'>clip</a>.
<a href='#SkCanvas_clipRegion_deviceRgn'>deviceRgn</a> <a href='#SkCanvas_clipRegion_deviceRgn'>is</a> <a href='#SkCanvas_clipRegion_deviceRgn'>unaffected</a> <a href='#SkCanvas_clipRegion_deviceRgn'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clipRegion_deviceRgn'><code><strong>deviceRgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>combine</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>clip</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_clipRegion_op'><code><strong>op</strong></code></a></td>
    <td><a href='undocumented#SkClipOp'>SkClipOp</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>apply</a> <a href='undocumented#SkClipOp'>to</a> <a href='undocumented#SkClipOp'>clip</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7bb57c0e456c5fda2c2cca4abb68b19e"><div><a href='SkRegion_Reference#Region'>region</a> <a href='SkRegion_Reference#Region'>is</a> <a href='SkRegion_Reference#Region'>unaffected</a> <a href='SkRegion_Reference#Region'>by</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>rotation</a>; <a href='SkCanvas_Reference#Canvas'>iRect</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>affected</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>rotation</a>.
<a href='SkCanvas_Reference#Canvas'>Both</a> <a href='SkCanvas_Reference#Canvas'>clips</a> <a href='SkCanvas_Reference#Canvas'>are</a> <a href='undocumented#Alias'>Aliased</a>; <a href='undocumented#Alias'>this</a> <a href='undocumented#Alias'>is</a> <a href='undocumented#Alias'>not</a> <a href='undocumented#Alias'>noticeable</a> <a href='undocumented#Alias'>on</a> <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>clip</a> <a href='SkRegion_Reference#Region'>because</a> <a href='SkRegion_Reference#Region'>it</a>
<a href='SkRegion_Reference#Region'>aligns</a> <a href='SkRegion_Reference#Region'>to</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>boundaries</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_clipRect'>clipRect</a> <a href='#SkCanvas_clipRRect'>clipRRect</a> <a href='#SkCanvas_clipPath'>clipPath</a>

<a name='SkCanvas_quickReject'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkCanvas_quickReject'>quickReject</a>(<a href='#SkCanvas_quickReject'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_quickReject_rect'>rect</a>, <a href='#SkCanvas_quickReject_rect'>transformed</a> <a href='#SkCanvas_quickReject_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>can</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>quickly</a> <a href='SkMatrix_Reference#SkMatrix'>determined</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>be</a>
outside of clip. May return false even though <a href='#SkCanvas_quickReject_rect'>rect</a> <a href='#SkCanvas_quickReject_rect'>is</a> <a href='#SkCanvas_quickReject_rect'>outside</a> <a href='#SkCanvas_quickReject_rect'>of</a> <a href='#SkCanvas_quickReject_rect'>clip</a>.

Use to check if an area to be drawn is clipped out, to skip subsequent draw calls.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_quickReject_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a> <a href='SkRect_Reference#SkRect'>with</a> <a href='SkRect_Reference#SkRect'>clip</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_quickReject_rect'>rect</a>, <a href='#SkCanvas_quickReject_rect'>transformed</a> <a href='#SkCanvas_quickReject_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>does</a> <a href='SkMatrix_Reference#SkMatrix'>not</a> <a href='SkMatrix_Reference#SkMatrix'>intersect</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>

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
bool <a href='#SkCanvas_quickReject'>quickReject</a>(<a href='#SkCanvas_quickReject'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>
</pre>

Returns true if <a href='#SkCanvas_quickReject_2_path'>path</a>, <a href='#SkCanvas_quickReject_2_path'>transformed</a> <a href='#SkCanvas_quickReject_2_path'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>can</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>quickly</a> <a href='SkMatrix_Reference#SkMatrix'>determined</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>be</a>
outside of clip. May return false even though <a href='#SkCanvas_quickReject_2_path'>path</a> <a href='#SkCanvas_quickReject_2_path'>is</a> <a href='#SkCanvas_quickReject_2_path'>outside</a> <a href='#SkCanvas_quickReject_2_path'>of</a> <a href='#SkCanvas_quickReject_2_path'>clip</a>.

Use to check if an area to be drawn is clipped out, to skip subsequent draw calls.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_quickReject_2_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>compare</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>clip</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkCanvas_quickReject_2_path'>path</a>, <a href='#SkCanvas_quickReject_2_path'>transformed</a> <a href='#SkCanvas_quickReject_2_path'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>does</a> <a href='SkMatrix_Reference#SkMatrix'>not</a> <a href='SkMatrix_Reference#SkMatrix'>intersect</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a>

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
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>() <a href='#SkCanvas_getLocalClipBounds'>const</a>
</pre>

Returns bounds of clip, transformed by inverse of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>empty</a>,
return <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, <a href='#SkRect_MakeEmpty'>where</a> <a href='#SkRect_MakeEmpty'>all</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>sides</a> <a href='SkRect_Reference#SkRect'>equal</a> <a href='SkRect_Reference#SkRect'>zero</a>.

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>outset</a> <a href='SkRect_Reference#SkRect'>by</a> <a href='SkRect_Reference#SkRect'>one</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>account</a> <a href='SkRect_Reference#SkRect'>for</a> <a href='SkRect_Reference#SkRect'>partial</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>coverage</a> <a href='undocumented#Pixel'>if</a> <a href='undocumented#Pixel'>clip</a>
is <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a>.

### Return Value

bounds of clip in local coordinates

### Example

<div><fiddle-embed name="7f60cb030d3f9b2473adbe3e34b19d91"><div>Initial bounds is <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>bounds</a> <a href='undocumented#Device'>outset</a> <a href='undocumented#Device'>by</a> 1 <a href='undocumented#Device'>on</a> <a href='undocumented#Device'>all</a> <a href='undocumented#Device'>sides</a>.
<a href='undocumented#Device'>Clipped</a> <a href='undocumented#Device'>bounds</a> <a href='undocumented#Device'>is</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipPath'>bounds</a> <a href='#SkCanvas_clipPath'>outset</a> <a href='#SkCanvas_clipPath'>by</a> 1 <a href='#SkCanvas_clipPath'>on</a> <a href='#SkCanvas_clipPath'>all</a> <a href='#SkCanvas_clipPath'>sides</a>.
<a href='#SkCanvas_clipPath'>Scaling</a> <a href='#SkCanvas_clipPath'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='SkCanvas_Reference#Canvas'>two</a> <a href='SkCanvas_Reference#Canvas'>on</a> <a href='SkCanvas_Reference#Canvas'>both</a> <a href='SkCanvas_Reference#Canvas'>axes</a> <a href='SkCanvas_Reference#Canvas'>scales</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>local</a> <a href='SkCanvas_Reference#Canvas'>bounds</a> <a href='SkCanvas_Reference#Canvas'>by</a> 1/2
<a href='SkCanvas_Reference#Canvas'>on</a> <a href='SkCanvas_Reference#Canvas'>both</a> <a href='SkCanvas_Reference#Canvas'>axes</a>.
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
bool <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>of</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>clip</a>, <a href='#SkCanvas_getLocalClipBounds_2_bounds'>transformed</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>by</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>inverse</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>of</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>empty</a>,
return false, and set <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>to</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, <a href='#SkRect_MakeEmpty'>where</a> <a href='#SkRect_MakeEmpty'>all</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>sides</a> <a href='SkRect_Reference#SkRect'>equal</a> <a href='SkRect_Reference#SkRect'>zero</a>.

<a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>is</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>outset</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>by</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>one</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>to</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>account</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>for</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>partial</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>coverage</a> <a href='undocumented#Pixel'>if</a> <a href='undocumented#Pixel'>clip</a>
is <a href='SkPaint_Reference#Anti_Alias'>anti-aliased</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getLocalClipBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>clip</a> <a href='SkRect_Reference#SkRect'>in</a> <a href='SkRect_Reference#SkRect'>local</a> <a href='SkRect_Reference#SkRect'>coordinates</a></td>
  </tr>
</table>

### Return Value

true if clip <a href='#SkCanvas_getLocalClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>is</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>not</a> <a href='#SkCanvas_getLocalClipBounds_2_bounds'>empty</a>

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
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>() <a href='#SkCanvas_getDeviceClipBounds'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>bounds</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='SkIRect_Reference#SkIRect'>clip</a>, <a href='SkIRect_Reference#SkIRect'>unaffected</a> <a href='SkIRect_Reference#SkIRect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>empty</a>,
return <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, <a href='#SkRect_MakeEmpty'>where</a> <a href='#SkRect_MakeEmpty'>all</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>sides</a> <a href='SkRect_Reference#SkRect'>equal</a> <a href='SkRect_Reference#SkRect'>zero</a>.

Unlike <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(), <a href='#SkCanvas_getLocalClipBounds'>returned</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>not</a> <a href='SkIRect_Reference#SkIRect'>outset</a>.

### Return Value

bounds of clip in <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>coordinates</a>

### Example

<div><fiddle-embed name="556832ac5711af662a98c21c547185e9"><div>Initial bounds is <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>bounds</a>, <a href='undocumented#Device'>not</a> <a href='undocumented#Device'>outset</a>.
<a href='undocumented#Device'>Clipped</a> <a href='undocumented#Device'>bounds</a> <a href='undocumented#Device'>is</a> <a href='#SkCanvas_clipPath'>clipPath</a> <a href='#SkCanvas_clipPath'>bounds</a>, <a href='#SkCanvas_clipPath'>not</a> <a href='#SkCanvas_clipPath'>outset</a>.
<a href='#SkCanvas_clipPath'>Scaling</a> <a href='#SkCanvas_clipPath'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>by</a> 1/2 <a href='SkCanvas_Reference#Canvas'>on</a> <a href='SkCanvas_Reference#Canvas'>both</a> <a href='SkCanvas_Reference#Canvas'>axes</a> <a href='SkCanvas_Reference#Canvas'>scales</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>bounds</a> <a href='undocumented#Device'>by</a> 1/2
<a href='undocumented#Device'>on</a> <a href='undocumented#Device'>both</a> <a href='undocumented#Device'>axes</a>.
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
bool <a href='#SkCanvas_getDeviceClipBounds'>getDeviceClipBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>bounds</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>of</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>clip</a>, <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>unaffected</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='SkMatrix_Reference#SkMatrix'>clip</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>empty</a>,
return false, and set <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>to</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>, <a href='#SkRect_MakeEmpty'>where</a> <a href='#SkRect_MakeEmpty'>all</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>sides</a> <a href='SkRect_Reference#SkRect'>equal</a> <a href='SkRect_Reference#SkRect'>zero</a>.

Unlike <a href='#SkCanvas_getLocalClipBounds'>getLocalClipBounds</a>(), <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>is</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>not</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>outset</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_getDeviceClipBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>clip</a> <a href='SkRect_Reference#SkRect'>in</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>coordinates</a></td>
  </tr>
</table>

### Return Value

true if clip <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>bounds</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>is</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>not</a> <a href='#SkCanvas_getDeviceClipBounds_2_bounds'>empty</a>

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
void <a href='#SkCanvas_drawColor'>drawColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a> = <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrcOver'>kSrcOver</a>)
</pre>

Fills clip with  <a href='#SkCanvas_drawColor_color'>color color</a>.
<a href='#SkCanvas_drawColor_mode'>mode</a> <a href='#SkCanvas_drawColor_mode'>determines</a> <a href='#SkCanvas_drawColor_mode'>how</a> <a href='#SkCanvas_drawColor_mode'>ARGB</a> <a href='#SkCanvas_drawColor_mode'>is</a> <a href='#SkCanvas_drawColor_mode'>combined</a> <a href='#SkCanvas_drawColor_mode'>with</a> <a href='#SkCanvas_drawColor_mode'>destination</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawColor_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='undocumented#Unpremultiply'>ARGB</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawColor_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>used</a> <a href='SkBlendMode_Reference#SkBlendMode'>to</a> <a href='SkBlendMode_Reference#SkBlendMode'>combine</a> <a href='SkBlendMode_Reference#SkBlendMode'>source</a> <a href='#SkCanvas_drawColor_color'>color</a> <a href='#SkCanvas_drawColor_color'>and</a> <a href='#SkCanvas_drawColor_color'>destination</a></td>
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

Fills clip with  <a href='#SkCanvas_clear_color'>color color</a> <a href='#SkCanvas_clear_color'>using</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kSrc'>kSrc</a>.
This has the effect of replacing all pixels contained by clip with <a href='#SkCanvas_clear_color'>color</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_clear_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='undocumented#Unpremultiply'>ARGB</a></td>
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

Makes <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>contents</a> <a href='SkCanvas_Reference#SkCanvas'>undefined</a>. <a href='SkCanvas_Reference#SkCanvas'>Subsequent</a> <a href='SkCanvas_Reference#SkCanvas'>calls</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>read</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>pixels</a>,
such as drawing with <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>return</a> <a href='SkBlendMode_Reference#SkBlendMode'>undefined</a> <a href='SkBlendMode_Reference#SkBlendMode'>results</a>. <a href='#SkCanvas_discard'>discard()</a> <a href='#SkCanvas_discard'>does</a>
not change clip or <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

<a href='#SkCanvas_discard'>discard()</a> <a href='#SkCanvas_discard'>may</a> <a href='#SkCanvas_discard'>do</a> <a href='#SkCanvas_discard'>nothing</a>, <a href='#SkCanvas_discard'>depending</a> <a href='#SkCanvas_discard'>on</a> <a href='#SkCanvas_discard'>the</a> <a href='#SkCanvas_discard'>implementation</a> <a href='#SkCanvas_discard'>of</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>or</a> <a href='undocumented#SkBaseDevice'>SkBaseDevice</a>
that created <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.

<a href='#SkCanvas_discard'>discard()</a> <a href='#SkCanvas_discard'>allows</a> <a href='#SkCanvas_discard'>optimized</a> <a href='#SkCanvas_discard'>performance</a> <a href='#SkCanvas_discard'>on</a> <a href='#SkCanvas_discard'>subsequent</a> <a href='#SkCanvas_discard'>draws</a> <a href='#SkCanvas_discard'>by</a> <a href='#SkCanvas_discard'>removing</a>
cached <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>associated</a> <a href='undocumented#Data'>with</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>or</a> <a href='undocumented#SkBaseDevice'>SkBaseDevice</a>.
It is not necessary to call <a href='#SkCanvas_discard'>discard()</a> <a href='#SkCanvas_discard'>once</a> <a href='#SkCanvas_discard'>done</a> <a href='#SkCanvas_discard'>with</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>;
any cached <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>deleted</a> <a href='undocumented#Data'>when</a> <a href='undocumented#Data'>owning</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>or</a> <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>is</a> <a href='undocumented#SkBaseDevice'>deleted</a>.

### See Also

<a href='#SkCanvas_flush'>flush()</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a> <a href='undocumented#GrContext'>GrContext</a>::<a href='#GrContext_abandonContext'>abandonContext</a>

<a name='SkCanvas_drawPaint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPaint'>drawPaint</a>(<a href='#SkCanvas_drawPaint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Fills clip with <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPaint_paint'>paint</a>. <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>components</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>affect</a> <a href='SkBlendMode_Reference#SkBlendMode'>drawing</a>;
<a href='undocumented#SkPathEffect'>SkPathEffect</a> <a href='undocumented#SkPathEffect'>in</a> <a href='#SkCanvas_drawPaint_paint'>paint</a> <a href='#SkCanvas_drawPaint_paint'>is</a> <a href='#SkCanvas_drawPaint_paint'>ignored</a>.

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

Selects if an array of <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>drawn</a> <a href='SkPoint_Reference#Point'>as</a> <a href='SkPoint_Reference#Point'>discrete</a> <a href='SkPoint_Reference#Point'>points</a>, <a href='SkPoint_Reference#Point'>as</a> <a href='undocumented#Line'>lines</a>, <a href='undocumented#Line'>or</a> <a href='undocumented#Line'>as</a>
<a href='undocumented#Line'>an</a> <a href='undocumented#Line'>open</a> <a href='undocumented#Line'>polygon</a>.

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
<a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>upper</a> <a href='SkPoint_Reference#Point'>right</a> <a href='SkPoint_Reference#Point'>corner</a> <a href='SkPoint_Reference#Point'>shows</a> <a href='SkPoint_Reference#Point'>one</a> <a href='undocumented#Line'>line</a>; <a href='undocumented#Line'>when</a> <a href='undocumented#Line'>drawn</a> <a href='undocumented#Line'>as</a> <a href='undocumented#Line'>lines</a>, <a href='undocumented#Line'>two</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>required</a> <a href='SkPoint_Reference#Point'>per</a> <a href='undocumented#Line'>line</a>.
<a href='undocumented#Line'>The</a> <a href='undocumented#Line'>lower</a> <a href='undocumented#Line'>right</a> <a href='undocumented#Line'>corner</a> <a href='undocumented#Line'>shows</a> <a href='undocumented#Line'>two</a> <a href='undocumented#Line'>lines</a>; <a href='undocumented#Line'>when</a> <a href='undocumented#Line'>draw</a> <a href='undocumented#Line'>as</a> <a href='undocumented#Line'>polygon</a>, <a href='undocumented#Line'>no</a> <a href='undocumented#Line'>miter</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>drawn</a> <a href='undocumented#Line'>at</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>corner</a>.
<a href='undocumented#Line'>The</a> <a href='undocumented#Line'>lower</a> <a href='undocumented#Line'>left</a> <a href='undocumented#Line'>corner</a> <a href='undocumented#Line'>shows</a> <a href='undocumented#Line'>two</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>with</a> <a href='undocumented#Line'>a</a> <a href='undocumented#Line'>miter</a> <a href='undocumented#Line'>when</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>contains</a> <a href='SkPath_Reference#Path'>polygon</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoints'>drawPoints</a>(<a href='#SkCanvas_PointMode'>PointMode</a> <a href='#SkCanvas_PointMode'>mode</a>, <a href='#SkCanvas_PointMode'>size_t</a> <a href='#SkCanvas_PointMode'>count</a>, <a href='#SkCanvas_PointMode'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawPoints_pts'>pts</a> <a href='#SkCanvas_drawPoints_pts'>using</a> <a href='#SkCanvas_drawPoints_pts'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPoints_paint'>paint</a>.
<a href='#SkCanvas_drawPoints_count'>count</a> <a href='#SkCanvas_drawPoints_count'>is</a> <a href='#SkCanvas_drawPoints_count'>the</a> <a href='#SkCanvas_drawPoints_count'>number</a> <a href='#SkCanvas_drawPoints_count'>of</a> <a href='SkPoint_Reference#Point'>points</a>; <a href='SkPoint_Reference#Point'>if</a> <a href='#SkCanvas_drawPoints_count'>count</a> <a href='#SkCanvas_drawPoints_count'>is</a> <a href='#SkCanvas_drawPoints_count'>less</a> <a href='#SkCanvas_drawPoints_count'>than</a> <a href='#SkCanvas_drawPoints_count'>one</a>, <a href='#SkCanvas_drawPoints_count'>has</a> <a href='#SkCanvas_drawPoints_count'>no</a> <a href='#SkCanvas_drawPoints_count'>effect</a>.
<a href='#SkCanvas_drawPoints_mode'>mode</a> <a href='#SkCanvas_drawPoints_mode'>may</a> <a href='#SkCanvas_drawPoints_mode'>be</a> <a href='#SkCanvas_drawPoints_mode'>one</a> <a href='#SkCanvas_drawPoints_mode'>of</a>: <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>, <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>, <a href='#SkCanvas_kLines_PointMode'>or</a> <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>.

If <a href='#SkCanvas_drawPoints_mode'>mode</a> <a href='#SkCanvas_drawPoints_mode'>is</a> <a href='#SkCanvas_kPoints_PointMode'>kPoints_PointMode</a>, <a href='#SkCanvas_kPoints_PointMode'>the</a> <a href='#SkCanvas_kPoints_PointMode'>shape</a> <a href='#SkCanvas_kPoints_PointMode'>of</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>drawn</a> <a href='SkPoint_Reference#Point'>depends</a> <a href='SkPoint_Reference#Point'>on</a> <a href='#SkCanvas_drawPoints_paint'>paint</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a>. <a href='#SkPaint_Cap'>If</a> <a href='#SkCanvas_drawPoints_paint'>paint</a> <a href='#SkCanvas_drawPoints_paint'>is</a> <a href='#SkCanvas_drawPoints_paint'>set</a> <a href='#SkCanvas_drawPoints_paint'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kRound_Cap'>each</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>draws</a> <a href='SkPoint_Reference#Point'>a</a>
<a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>diameter</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>. <a href='SkPaint_Reference#SkPaint'>If</a> <a href='#SkCanvas_drawPoints_paint'>paint</a> <a href='#SkCanvas_drawPoints_paint'>is</a> <a href='#SkCanvas_drawPoints_paint'>set</a> <a href='#SkCanvas_drawPoints_paint'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a>
or <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kButt_Cap'>kButt_Cap</a>, <a href='#SkPaint_kButt_Cap'>each</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>draws</a> <a href='SkPoint_Reference#Point'>a</a> <a href='SkPoint_Reference#Point'>square</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>width</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>height</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>.

If <a href='#SkCanvas_drawPoints_mode'>mode</a> <a href='#SkCanvas_drawPoints_mode'>is</a> <a href='#SkCanvas_kLines_PointMode'>kLines_PointMode</a>, <a href='#SkCanvas_kLines_PointMode'>each</a> <a href='#SkCanvas_kLines_PointMode'>pair</a> <a href='#SkCanvas_kLines_PointMode'>of</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>draws</a> <a href='SkPoint_Reference#Point'>a</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a>.
One <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>drawn</a> <a href='undocumented#Line'>for</a> <a href='undocumented#Line'>every</a> <a href='undocumented#Line'>two</a> <a href='SkPoint_Reference#Point'>points</a>; <a href='SkPoint_Reference#Point'>each</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>used</a> <a href='SkPoint_Reference#Point'>once</a>. <a href='SkPoint_Reference#Point'>If</a> <a href='#SkCanvas_drawPoints_count'>count</a> <a href='#SkCanvas_drawPoints_count'>is</a> <a href='#SkCanvas_drawPoints_count'>odd</a>,
the final <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>ignored</a>.

If <a href='#SkCanvas_drawPoints_mode'>mode</a> <a href='#SkCanvas_drawPoints_mode'>is</a> <a href='#SkCanvas_kPolygon_PointMode'>kPolygon_PointMode</a>, <a href='#SkCanvas_kPolygon_PointMode'>each</a> <a href='#SkCanvas_kPolygon_PointMode'>adjacent</a> <a href='#SkCanvas_kPolygon_PointMode'>pair</a> <a href='#SkCanvas_kPolygon_PointMode'>of</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>draws</a> <a href='SkPoint_Reference#Point'>a</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a>.
<a href='#SkCanvas_drawPoints_count'>count</a> <a href='#SkCanvas_drawPoints_count'>minus</a> <a href='#SkCanvas_drawPoints_count'>one</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>are</a> <a href='undocumented#Line'>drawn</a>; <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>first</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>last</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>used</a> <a href='SkPoint_Reference#Point'>once</a>.

Each <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>respects</a> <a href='#SkCanvas_drawPoints_paint'>paint</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>ignored</a>, <a href='#SkPaint_Style'>as</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>were</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

Always draws each element one at a time; is not affected by
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a>, <a href='#SkPaint_Join'>and</a> <a href='#SkPaint_Join'>unlike</a> <a href='#SkCanvas_drawPath'>drawPath</a>(), <a href='#SkCanvas_drawPath'>does</a> <a href='#SkCanvas_drawPath'>not</a> <a href='#SkCanvas_drawPath'>create</a> <a href='#SkCanvas_drawPath'>a</a> <a href='#SkCanvas_drawPath'>mask</a> <a href='#SkCanvas_drawPath'>from</a> <a href='#SkCanvas_drawPath'>all</a> <a href='SkPoint_Reference#Point'>points</a>
and <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>before</a> <a href='undocumented#Line'>drawing</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoints_mode'><code><strong>mode</strong></code></a></td>
    <td>whether <a href='#SkCanvas_drawPoints_pts'>pts</a> <a href='#SkCanvas_drawPoints_pts'>draws</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Line'>lines</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>in</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>array</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_pts'><code><strong>pts</strong></code></a></td>
    <td>array of <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoints_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="635d54b4716e226e93dfbc21ad40e77d"><div>

<table>  <tr>
    <td>The first column draws <a href='SkPoint_Reference#Point'>points</a>.</td>
  </tr>  <tr>
    <td>The second column draws <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>as</a> <a href='undocumented#Line'>lines</a>.</td>
  </tr>  <tr>
    <td>The third column draws <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>as</a> <a href='SkPoint_Reference#Point'>a</a> <a href='SkPoint_Reference#Point'>polygon</a>.</td>
  </tr>  <tr>
    <td>The fourth column draws <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>as</a> <a href='SkPoint_Reference#Point'>a</a> <a href='SkPoint_Reference#Point'>polygonal</a> <a href='SkPath_Reference#Path'>path</a>.</td>
  </tr>  <tr>
    <td>The first row uses a round cap and round join.</td>
  </tr>  <tr>
    <td>The second row uses a square cap and a miter join.</td>
  </tr>  <tr>
    <td>The third row uses a butt cap and a bevel join.</td>
  </tr>
</table>

The transparent <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>makes</a> <a href='SkColor_Reference#Color'>multiple</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>draws</a> <a href='undocumented#Line'>visible</a>;
<a href='undocumented#Line'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>drawn</a> <a href='SkPath_Reference#Path'>all</a> <a href='SkPath_Reference#Path'>at</a> <a href='SkPath_Reference#Path'>once</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (<a href='#SkCanvas_drawPoint_x'>x</a>, <a href='#SkCanvas_drawPoint_y'>y</a>) <a href='#SkCanvas_drawPoint_y'>using</a> <a href='#SkCanvas_drawPoint_y'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPoint_paint'>paint</a>.

The shape of <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>drawn</a> <a href='SkPoint_Reference#Point'>depends</a> <a href='SkPoint_Reference#Point'>on</a> <a href='#SkCanvas_drawPoint_paint'>paint</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a>.
If <a href='#SkCanvas_drawPoint_paint'>paint</a> <a href='#SkCanvas_drawPoint_paint'>is</a> <a href='#SkCanvas_drawPoint_paint'>set</a> <a href='#SkCanvas_drawPoint_paint'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kRound_Cap'>draw</a> <a href='#SkPaint_kRound_Cap'>a</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>diameter</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>. <a href='SkPaint_Reference#SkPaint'>If</a> <a href='#SkCanvas_drawPoint_paint'>paint</a> <a href='#SkCanvas_drawPoint_paint'>is</a> <a href='#SkCanvas_drawPoint_paint'>set</a> <a href='#SkCanvas_drawPoint_paint'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> <a href='#SkPaint_kSquare_Cap'>or</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
draw a square of width and height <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>ignored</a>, <a href='#SkPaint_Style'>as</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>were</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoint_x'><code><strong>x</strong></code></a></td>
    <td>left edge of <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>or</a> <a href='undocumented#Circle'>square</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_y'><code><strong>y</strong></code></a></td>
    <td>top edge of <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>or</a> <a href='undocumented#Circle'>square</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3476b553e7b547b604a3f6969f02d933"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoints'>drawPoints</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawPoint_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPoint'>drawPoint</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPoint_Reference#Point'>point</a> <a href='#SkCanvas_drawPoint_2_p'>p</a> <a href='#SkCanvas_drawPoint_2_p'>using</a> <a href='#SkCanvas_drawPoint_2_p'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPoint_2_paint'>paint</a>.

The shape of <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>drawn</a> <a href='SkPoint_Reference#Point'>depends</a> <a href='SkPoint_Reference#Point'>on</a> <a href='#SkCanvas_drawPoint_2_paint'>paint</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a>.
If <a href='#SkCanvas_drawPoint_2_paint'>paint</a> <a href='#SkCanvas_drawPoint_2_paint'>is</a> <a href='#SkCanvas_drawPoint_2_paint'>set</a> <a href='#SkCanvas_drawPoint_2_paint'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>, <a href='#SkPaint_kRound_Cap'>draw</a> <a href='#SkPaint_kRound_Cap'>a</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>diameter</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>. <a href='SkPaint_Reference#SkPaint'>If</a> <a href='#SkCanvas_drawPoint_2_paint'>paint</a> <a href='#SkCanvas_drawPoint_2_paint'>is</a> <a href='#SkCanvas_drawPoint_2_paint'>set</a> <a href='#SkCanvas_drawPoint_2_paint'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> <a href='#SkPaint_kSquare_Cap'>or</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kButt_Cap'>kButt_Cap</a>,
draw a square of width and height <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a>.
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>ignored</a>, <a href='#SkPaint_Style'>as</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>were</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPoint_2_p'><code><strong>p</strong></code></a></td>
    <td>top-left edge of <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>or</a> <a href='undocumented#Circle'>square</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPoint_2_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1a0a839061c69d870acca2bcfbdf1a41"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoints'>drawPoints</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawLine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>from</a> (<a href='#SkCanvas_drawLine_x0'>x0</a>, <a href='#SkCanvas_drawLine_y0'>y0</a>) <a href='#SkCanvas_drawLine_y0'>to</a> (<a href='#SkCanvas_drawLine_x1'>x1</a>, <a href='#SkCanvas_drawLine_y1'>y1</a>) <a href='#SkCanvas_drawLine_y1'>using</a> <a href='#SkCanvas_drawLine_y1'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawLine_paint'>paint</a>.
In <a href='#SkCanvas_drawLine_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>draws</a> <a href='#SkPaint_Cap'>the</a> <a href='#SkPaint_Cap'>end</a> <a href='#SkPaint_Cap'>rounded</a> <a href='#SkPaint_Cap'>or</a> <a href='#SkPaint_Cap'>square</a>;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>ignored</a>, <a href='#SkPaint_Style'>as</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>were</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawLine_x0'><code><strong>x0</strong></code></a></td>
    <td>start of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_y0'><code><strong>y0</strong></code></a></td>
    <td>start of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_x1'><code><strong>x1</strong></code></a></td>
    <td>end of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_y1'><code><strong>y1</strong></code></a></td>
    <td>end of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d10ee4a265f278d02afe11ad889b293b"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawLine_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawLine'>drawLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p0</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>from</a> <a href='#SkCanvas_drawLine_2_p0'>p0</a> <a href='#SkCanvas_drawLine_2_p0'>to</a> <a href='#SkCanvas_drawLine_2_p1'>p1</a> <a href='#SkCanvas_drawLine_2_p1'>using</a> <a href='#SkCanvas_drawLine_2_p1'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawLine_2_paint'>paint</a>.
In <a href='#SkCanvas_drawLine_2_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>draws</a> <a href='#SkPaint_Cap'>the</a> <a href='#SkPaint_Cap'>end</a> <a href='#SkPaint_Cap'>rounded</a> <a href='#SkPaint_Cap'>or</a> <a href='#SkPaint_Cap'>square</a>;
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>ignored</a>, <a href='#SkPaint_Style'>as</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>were</a> <a href='#SkPaint_Style'>set</a> <a href='#SkPaint_Style'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawLine_2_p0'><code><strong>p0</strong></code></a></td>
    <td>start of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_2_p1'><code><strong>p1</strong></code></a></td>
    <td>end of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawLine_2_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f8525816cb596dde1a3855446792c8e0"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRect'>drawRect</a>(<a href='#SkCanvas_drawRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawRect_rect'>rect</a> <a href='#SkCanvas_drawRect_rect'>using</a> <a href='#SkCanvas_drawRect_rect'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRect_paint'>paint</a>.
In <a href='#SkCanvas_drawRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>rectangle</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>stroked</a> <a href='#SkPaint_Style'>or</a> <a href='#SkPaint_Style'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>, <a href='undocumented#Line'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>draws</a> <a href='#SkPaint_Join'>the</a> <a href='#SkPaint_Join'>corners</a> <a href='#SkPaint_Join'>rounded</a> <a href='#SkPaint_Join'>or</a> <a href='#SkPaint_Join'>square</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRect_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="871b0da9b4a23de11ae7a772ce14aed3"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawIRect'>drawIRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRegion'>drawRegion</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawIRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawIRect'>drawIRect</a>(<a href='#SkCanvas_drawIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawIRect_rect'>rect</a> <a href='#SkCanvas_drawIRect_rect'>using</a> <a href='#SkCanvas_drawIRect_rect'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawIRect_paint'>paint</a>.
In <a href='#SkCanvas_drawIRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>rectangle</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>stroked</a> <a href='#SkPaint_Style'>or</a> <a href='#SkPaint_Style'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>, <a href='undocumented#Line'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>draws</a> <a href='#SkPaint_Join'>the</a> <a href='#SkPaint_Join'>corners</a> <a href='#SkPaint_Join'>rounded</a> <a href='#SkPaint_Join'>or</a> <a href='#SkPaint_Join'>square</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawIRect_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawIRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke or fill, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d3d8ca584134560750b1efa4a4c6e138"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRegion'>drawRegion</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRegion'>drawRegion</a>(<a href='#SkCanvas_drawRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkCanvas_drawRegion_region'>region</a> <a href='#SkCanvas_drawRegion_region'>using</a> <a href='#SkCanvas_drawRegion_region'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRegion_paint'>paint</a>.
In <a href='#SkCanvas_drawRegion_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='#SkPaint_Style'>rectangle</a> <a href='#SkPaint_Style'>is</a> <a href='#SkPaint_Style'>stroked</a> <a href='#SkPaint_Style'>or</a> <a href='#SkPaint_Style'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>, <a href='undocumented#Line'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>draws</a> <a href='#SkPaint_Join'>the</a> <a href='#SkPaint_Join'>corners</a> <a href='#SkPaint_Join'>rounded</a> <a href='#SkPaint_Join'>or</a> <a href='#SkPaint_Join'>square</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRegion_region'><code><strong>region</strong></code></a></td>
    <td><a href='#SkCanvas_drawRegion_region'>region</a> <a href='#SkCanvas_drawRegion_region'>to</a> <a href='#SkCanvas_drawRegion_region'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRegion_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="80309e0deca0f8add616cec7bec634ca"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawIRect'>drawIRect</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawOval'>drawOval</a>(<a href='#SkCanvas_drawOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Oval'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawOval_oval'>oval</a> <a href='#SkCanvas_drawOval_oval'>oval</a> <a href='#SkCanvas_drawOval_oval'>using</a> <a href='#SkCanvas_drawOval_oval'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>.
In <a href='#SkCanvas_drawOval_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='#SkCanvas_drawOval_oval'>oval</a> <a href='#SkCanvas_drawOval_oval'>is</a> <a href='#SkCanvas_drawOval_oval'>stroked</a> <a href='#SkCanvas_drawOval_oval'>or</a> <a href='#SkCanvas_drawOval_oval'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawOval_oval'><code><strong>oval</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawOval_oval'>oval</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawOval_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8b6b86f8a022811cd29a9c6ab771df12"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>

<a name='SkCanvas_drawRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawRRect'>drawRRect</a>(<a href='#SkCanvas_drawRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawRRect_rrect'>rrect</a> <a href='#SkCanvas_drawRRect_rrect'>using</a> <a href='#SkCanvas_drawRRect_rrect'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRRect_paint'>paint</a>.
In <a href='#SkCanvas_drawRRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='#SkCanvas_drawRRect_rrect'>rrect</a> <a href='#SkCanvas_drawRRect_rrect'>is</a> <a href='#SkCanvas_drawRRect_rrect'>stroked</a> <a href='#SkCanvas_drawRRect_rrect'>or</a> <a href='#SkCanvas_drawRRect_rrect'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>.

<a href='#SkCanvas_drawRRect_rrect'>rrect</a> <a href='#SkCanvas_drawRRect_rrect'>may</a> <a href='#SkCanvas_drawRRect_rrect'>represent</a> <a href='#SkCanvas_drawRRect_rrect'>a</a> <a href='#SkCanvas_drawRRect_rrect'>rectangle</a>, <a href='undocumented#Circle'>circle</a>, <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Oval'>uniformly</a> <a href='undocumented#Oval'>rounded</a> <a href='undocumented#Oval'>rectangle</a>, <a href='undocumented#Oval'>or</a>
may have any combination of positive non-square radii for the four corners.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>with</a> <a href='SkRRect_Reference#SkRRect'>up</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>eight</a> <a href='SkRRect_Reference#SkRRect'>corner</a> <a href='SkRRect_Reference#SkRRect'>radii</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="90fed1bb11efb43aada94113338c63d8"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawDRRect'>drawDRRect</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawDRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDRRect'>drawDRRect</a>(<a href='#SkCanvas_drawDRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>outer</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>inner</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawDRRect_outer'>outer</a> <a href='#SkCanvas_drawDRRect_outer'>and</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a>
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawDRRect_paint'>paint</a>.
<a href='#SkCanvas_drawDRRect_outer'>outer</a> <a href='#SkCanvas_drawDRRect_outer'>must</a> <a href='#SkCanvas_drawDRRect_outer'>contain</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a> <a href='#SkCanvas_drawDRRect_inner'>or</a> <a href='#SkCanvas_drawDRRect_inner'>the</a> <a href='#SkCanvas_drawDRRect_inner'>drawing</a> <a href='#SkCanvas_drawDRRect_inner'>is</a> <a href='#SkCanvas_drawDRRect_inner'>undefined</a>.
In <a href='#SkCanvas_drawDRRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>stroked</a> <a href='SkRRect_Reference#SkRRect'>or</a> <a href='SkRRect_Reference#SkRRect'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>.
If stroked and <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>corner</a> <a href='SkRRect_Reference#SkRRect'>has</a>  <a href='SkRRect_Reference#SkRRect'>zero length</a> <a href='SkRRect_Reference#SkRRect'>radii</a>, <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>can</a>
draw corners rounded or square.

GPU-backed platforms optimize drawing when both <a href='#SkCanvas_drawDRRect_outer'>outer</a> <a href='#SkCanvas_drawDRRect_outer'>and</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a> <a href='#SkCanvas_drawDRRect_inner'>are</a>
concave and <a href='#SkCanvas_drawDRRect_outer'>outer</a> <a href='#SkCanvas_drawDRRect_outer'>contains</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a>. <a href='#SkCanvas_drawDRRect_inner'>These</a> <a href='#SkCanvas_drawDRRect_inner'>platforms</a> <a href='#SkCanvas_drawDRRect_inner'>may</a> <a href='#SkCanvas_drawDRRect_inner'>not</a> <a href='#SkCanvas_drawDRRect_inner'>be</a> <a href='#SkCanvas_drawDRRect_inner'>able</a> <a href='#SkCanvas_drawDRRect_inner'>to</a> <a href='#SkCanvas_drawDRRect_inner'>draw</a>
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>built</a> <a href='SkPath_Reference#SkPath'>with</a> <a href='SkPath_Reference#SkPath'>identical</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>as</a> <a href='undocumented#Data'>fast</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDRRect_outer'><code><strong>outer</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawDRRect_outer'>outer</a> <a href='#SkCanvas_drawDRRect_outer'>bounds</a> <a href='#SkCanvas_drawDRRect_outer'>to</a> <a href='#SkCanvas_drawDRRect_outer'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDRRect_inner'><code><strong>inner</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a> <a href='#SkCanvas_drawDRRect_inner'>bounds</a> <a href='#SkCanvas_drawDRRect_inner'>to</a> <a href='#SkCanvas_drawDRRect_inner'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDRRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="02e33141f13da2f19aef7feb7117b541"></fiddle-embed></div>

### Example

<div><fiddle-embed name="30823cb4edf884d330285ea161664931"><div>Outer <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>has</a> <a href='SkRect_Reference#Rect'>no</a> <a href='SkRect_Reference#Rect'>corner</a> <a href='SkRect_Reference#Rect'>radii</a>, <a href='SkRect_Reference#Rect'>but</a> <a href='SkRect_Reference#Rect'>stroke</a> <a href='SkRect_Reference#Rect'>join</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>rounded</a>.
<a href='SkRect_Reference#Rect'>Inner</a> <a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>corner</a> <a href='#RRect'>radii</a>; <a href='#RRect'>outset</a> <a href='#RRect'>stroke</a> <a href='#RRect'>increases</a> <a href='#RRect'>radii</a> <a href='#RRect'>of</a> <a href='#RRect'>corners</a>.
<a href='#RRect'>Stroke</a> <a href='#RRect'>join</a> <a href='#RRect'>does</a> <a href='#RRect'>not</a> <a href='#RRect'>affect</a> <a href='#SkCanvas_drawDRRect_inner'>inner</a> <a href='#RRect'>Round_Rect</a> <a href='#RRect'>since</a> <a href='#RRect'>it</a> <a href='#RRect'>has</a> <a href='#RRect'>no</a> <a href='#RRect'>sharp</a> <a href='#RRect'>corners</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawPath'>drawPath</a>

<a name='SkCanvas_drawCircle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>at</a> (<a href='#SkCanvas_drawCircle_cx'>cx</a>, <a href='#SkCanvas_drawCircle_cy'>cy</a>) <a href='#SkCanvas_drawCircle_cy'>with</a> <a href='#SkCanvas_drawCircle_radius'>radius</a> <a href='#SkCanvas_drawCircle_radius'>using</a> <a href='#SkCanvas_drawCircle_radius'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawCircle_paint'>paint</a>.
If <a href='#SkCanvas_drawCircle_radius'>radius</a> <a href='#SkCanvas_drawCircle_radius'>is</a> <a href='#SkCanvas_drawCircle_radius'>zero</a> <a href='#SkCanvas_drawCircle_radius'>or</a> <a href='#SkCanvas_drawCircle_radius'>less</a>, <a href='#SkCanvas_drawCircle_radius'>nothing</a> <a href='#SkCanvas_drawCircle_radius'>is</a> <a href='#SkCanvas_drawCircle_radius'>drawn</a>.
In <a href='#SkCanvas_drawCircle_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>is</a> <a href='undocumented#Circle'>stroked</a> <a href='undocumented#Circle'>or</a> <a href='undocumented#Circle'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawCircle_cx'><code><strong>cx</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>center</a> <a href='undocumented#Circle'>on</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_cy'><code><strong>cy</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>center</a> <a href='undocumented#Circle'>on</a> <a href='undocumented#Circle'>the</a> <a href='undocumented#Circle'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_radius'><code><strong>radius</strong></code></a></td>
    <td>half the diameter of <a href='undocumented#Circle'>circle</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="841229e25ca9dfb68bd0dc4dfff356eb"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawCircle_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawCircle'>drawCircle</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>center</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>at</a> <a href='#SkCanvas_drawCircle_2_center'>center</a> <a href='#SkCanvas_drawCircle_2_center'>with</a> <a href='#SkCanvas_drawCircle_2_radius'>radius</a> <a href='#SkCanvas_drawCircle_2_radius'>using</a> <a href='#SkCanvas_drawCircle_2_radius'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawCircle_2_paint'>paint</a>.
If <a href='#SkCanvas_drawCircle_2_radius'>radius</a> <a href='#SkCanvas_drawCircle_2_radius'>is</a> <a href='#SkCanvas_drawCircle_2_radius'>zero</a> <a href='#SkCanvas_drawCircle_2_radius'>or</a> <a href='#SkCanvas_drawCircle_2_radius'>less</a>, <a href='#SkCanvas_drawCircle_2_radius'>nothing</a> <a href='#SkCanvas_drawCircle_2_radius'>is</a> <a href='#SkCanvas_drawCircle_2_radius'>drawn</a>.
In <a href='#SkCanvas_drawCircle_2_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>is</a> <a href='undocumented#Circle'>stroked</a> <a href='undocumented#Circle'>or</a> <a href='undocumented#Circle'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawCircle_2_center'><code><strong>center</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> <a href='#SkCanvas_drawCircle_2_center'>center</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_2_radius'><code><strong>radius</strong></code></a></td>
    <td>half the diameter of <a href='undocumented#Circle'>circle</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawCircle_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9303ffae45ddd0b0a1f93d816a1762f4"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkCanvas_drawRRect'>drawRRect</a> <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a> <a href='#SkCanvas_drawPath'>drawPath</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawPoint'>drawPoint</a> <a href='#SkCanvas_drawLine'>drawLine</a>

<a name='SkCanvas_drawArc'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawArc'>drawArc</a>(<a href='#SkCanvas_drawArc'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>startAngle</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sweepAngle</a>, <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>useCenter</a>,
             <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>using</a> <a href='undocumented#Arc'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawArc_paint'>paint</a>.

<a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>part</a> <a href='undocumented#Arc'>of</a> <a href='#SkCanvas_drawArc_oval'>oval</a> <a href='#SkCanvas_drawArc_oval'>bounded</a> <a href='#SkCanvas_drawArc_oval'>by</a> <a href='#SkCanvas_drawArc_oval'>oval</a>, <a href='#SkCanvas_drawArc_oval'>sweeping</a> <a href='#SkCanvas_drawArc_oval'>from</a> <a href='#SkCanvas_drawArc_startAngle'>startAngle</a> <a href='#SkCanvas_drawArc_startAngle'>to</a> <a href='#SkCanvas_drawArc_startAngle'>startAngle</a> <a href='#SkCanvas_drawArc_startAngle'>plus</a>
<a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a>. <a href='#SkCanvas_drawArc_startAngle'>startAngle</a> <a href='#SkCanvas_drawArc_startAngle'>and</a> <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> <a href='#SkCanvas_drawArc_sweepAngle'>are</a> <a href='#SkCanvas_drawArc_sweepAngle'>in</a> <a href='#SkCanvas_drawArc_sweepAngle'>degrees</a>.

<a href='#SkCanvas_drawArc_startAngle'>startAngle</a> <a href='#SkCanvas_drawArc_startAngle'>of</a> <a href='#SkCanvas_drawArc_startAngle'>zero</a> <a href='#SkCanvas_drawArc_startAngle'>places</a> <a href='#SkCanvas_drawArc_startAngle'>start</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>right</a> <a href='SkPoint_Reference#Point'>middle</a> <a href='SkPoint_Reference#Point'>edge</a> <a href='SkPoint_Reference#Point'>of</a> <a href='#SkCanvas_drawArc_oval'>oval</a>.
A positive <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> <a href='#SkCanvas_drawArc_sweepAngle'>places</a> <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>end</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>clockwise</a> <a href='SkPoint_Reference#Point'>from</a> <a href='SkPoint_Reference#Point'>start</a> <a href='SkPoint_Reference#Point'>point</a>;
a negative <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> <a href='#SkCanvas_drawArc_sweepAngle'>places</a> <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>end</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>counterclockwise</a> <a href='SkPoint_Reference#Point'>from</a> <a href='SkPoint_Reference#Point'>start</a> <a href='SkPoint_Reference#Point'>point</a>.
<a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> <a href='#SkCanvas_drawArc_sweepAngle'>may</a> <a href='#SkCanvas_drawArc_sweepAngle'>exceed</a> 360 <a href='#SkCanvas_drawArc_sweepAngle'>degrees</a>, <a href='#SkCanvas_drawArc_sweepAngle'>a</a> <a href='#SkCanvas_drawArc_sweepAngle'>full</a> <a href='undocumented#Circle'>circle</a>.
If <a href='#SkCanvas_drawArc_useCenter'>useCenter</a> <a href='#SkCanvas_drawArc_useCenter'>is</a> <a href='#SkCanvas_drawArc_useCenter'>true</a>, <a href='#SkCanvas_drawArc_useCenter'>draw</a> <a href='#SkCanvas_drawArc_useCenter'>a</a> <a href='#SkCanvas_drawArc_useCenter'>wedge</a> <a href='#SkCanvas_drawArc_useCenter'>that</a> <a href='#SkCanvas_drawArc_useCenter'>includes</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>from</a> <a href='#SkCanvas_drawArc_oval'>oval</a>
center to <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>end</a> <a href='SkPoint_Reference#Point'>points</a>. <a href='SkPoint_Reference#Point'>If</a> <a href='#SkCanvas_drawArc_useCenter'>useCenter</a> <a href='#SkCanvas_drawArc_useCenter'>is</a> <a href='#SkCanvas_drawArc_useCenter'>false</a>, <a href='#SkCanvas_drawArc_useCenter'>draw</a> <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>between</a> <a href='undocumented#Arc'>end</a> <a href='SkPoint_Reference#Point'>points</a>.

If <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawArc_oval'>oval</a> <a href='#SkCanvas_drawArc_oval'>is</a> <a href='#SkCanvas_drawArc_oval'>empty</a> <a href='#SkCanvas_drawArc_oval'>or</a> <a href='#SkCanvas_drawArc_sweepAngle'>sweepAngle</a> <a href='#SkCanvas_drawArc_sweepAngle'>is</a> <a href='#SkCanvas_drawArc_sweepAngle'>zero</a>, <a href='#SkCanvas_drawArc_sweepAngle'>nothing</a> <a href='#SkCanvas_drawArc_sweepAngle'>is</a> <a href='#SkCanvas_drawArc_sweepAngle'>drawn</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawArc_oval'><code><strong>oval</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawArc_oval'>oval</a> <a href='#SkCanvas_drawArc_oval'>containing</a> <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='undocumented#Arc'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>angle in degrees where <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>begins</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep angle in degrees; positive is clockwise</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_useCenter'><code><strong>useCenter</strong></code></a></td>
    <td>if true, include the center of the <a href='#SkCanvas_drawArc_oval'>oval</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawArc_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>or</a> <a href='SkPaint_Reference#SkPaint'>fill</a>, <a href='SkPaint_Reference#SkPaint'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
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
void <a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>(<a href='#SkCanvas_drawRoundRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>bounded</a> <a href='SkRRect_Reference#SkRRect'>by</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawRoundRect_rect'>rect</a>, <a href='#SkCanvas_drawRoundRect_rect'>with</a> <a href='#SkCanvas_drawRoundRect_rect'>corner</a> <a href='#SkCanvas_drawRoundRect_rect'>radii</a> (<a href='#SkCanvas_drawRoundRect_rx'>rx</a>, <a href='#SkCanvas_drawRoundRect_ry'>ry</a>) <a href='#SkCanvas_drawRoundRect_ry'>using</a> <a href='#SkCanvas_drawRoundRect_ry'>clip</a>,
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawRoundRect_paint'>paint</a>.

In <a href='#SkCanvas_drawRoundRect_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>stroked</a> <a href='SkRRect_Reference#SkRRect'>or</a> <a href='SkRRect_Reference#SkRRect'>filled</a>;
if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>.
If <a href='#SkCanvas_drawRoundRect_rx'>rx</a> <a href='#SkCanvas_drawRoundRect_rx'>or</a> <a href='#SkCanvas_drawRoundRect_ry'>ry</a> <a href='#SkCanvas_drawRoundRect_ry'>are</a> <a href='#SkCanvas_drawRoundRect_ry'>less</a> <a href='#SkCanvas_drawRoundRect_ry'>than</a> <a href='#SkCanvas_drawRoundRect_ry'>zero</a>, <a href='#SkCanvas_drawRoundRect_ry'>they</a> <a href='#SkCanvas_drawRoundRect_ry'>are</a> <a href='#SkCanvas_drawRoundRect_ry'>treated</a> <a href='#SkCanvas_drawRoundRect_ry'>as</a> <a href='#SkCanvas_drawRoundRect_ry'>if</a> <a href='#SkCanvas_drawRoundRect_ry'>they</a> <a href='#SkCanvas_drawRoundRect_ry'>are</a> <a href='#SkCanvas_drawRoundRect_ry'>zero</a>.
If <a href='#SkCanvas_drawRoundRect_rx'>rx</a> <a href='#SkCanvas_drawRoundRect_rx'>plus</a> <a href='#SkCanvas_drawRoundRect_ry'>ry</a> <a href='#SkCanvas_drawRoundRect_ry'>exceeds</a> <a href='#SkCanvas_drawRoundRect_rect'>rect</a> <a href='#SkCanvas_drawRoundRect_rect'>width</a> <a href='#SkCanvas_drawRoundRect_rect'>or</a> <a href='#SkCanvas_drawRoundRect_rect'>rect</a> <a href='#SkCanvas_drawRoundRect_rect'>height</a>, <a href='#SkCanvas_drawRoundRect_rect'>radii</a> <a href='#SkCanvas_drawRoundRect_rect'>are</a> <a href='#SkCanvas_drawRoundRect_rect'>scaled</a> <a href='#SkCanvas_drawRoundRect_rect'>down</a> <a href='#SkCanvas_drawRoundRect_rect'>to</a> <a href='#SkCanvas_drawRoundRect_rect'>fit</a>.
If <a href='#SkCanvas_drawRoundRect_rx'>rx</a> <a href='#SkCanvas_drawRoundRect_rx'>and</a> <a href='#SkCanvas_drawRoundRect_ry'>ry</a> <a href='#SkCanvas_drawRoundRect_ry'>are</a> <a href='#SkCanvas_drawRoundRect_ry'>zero</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>drawn</a> <a href='SkRRect_Reference#SkRRect'>as</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>stroked</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>affected</a> <a href='SkRect_Reference#SkRect'>by</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawRoundRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_rx'><code><strong>rx</strong></code></a></td>
    <td>axis length on x-axis of <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>describing</a> <a href='undocumented#Oval'>rounded</a> <a href='undocumented#Oval'>corners</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_ry'><code><strong>ry</strong></code></a></td>
    <td>axis length on y-axis of <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>describing</a> <a href='undocumented#Oval'>rounded</a> <a href='undocumented#Oval'>corners</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawRoundRect_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
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
void <a href='#SkCanvas_drawPath'>drawPath</a>(<a href='#SkCanvas_drawPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkCanvas_drawPath_path'>path</a> <a href='#SkCanvas_drawPath_path'>using</a> <a href='#SkCanvas_drawPath_path'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPath_paint'>paint</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>an</a> <a href='SkPath_Reference#SkPath'>array</a> <a href='SkPath_Reference#SkPath'>of</a>  <a href='SkPath_Overview#Contour'>path contour</a>, <a href='#SkCanvas_drawPath_path'>each</a> <a href='#SkCanvas_drawPath_path'>of</a> <a href='#SkCanvas_drawPath_path'>which</a> <a href='#SkCanvas_drawPath_path'>may</a> <a href='#SkCanvas_drawPath_path'>be</a> <a href='#SkCanvas_drawPath_path'>open</a> <a href='#SkCanvas_drawPath_path'>or</a> <a href='#SkCanvas_drawPath_path'>closed</a>.

In <a href='#SkCanvas_drawPath_paint'>paint</a>: <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPaint_Style'>determines</a> <a href='#SkPaint_Style'>if</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>stroked</a> <a href='SkRRect_Reference#SkRRect'>or</a> <a href='SkRRect_Reference#SkRRect'>filled</a>:
if filled, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>determines</a> <a href='#SkPath_FillType'>whether</a>  <a href='SkPath_Overview#Contour'>path contour</a> <a href='#SkCanvas_drawPath_path'>describes</a> <a href='#SkCanvas_drawPath_path'>inside</a> <a href='#SkCanvas_drawPath_path'>or</a>
outside of fill; if stroked, <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>stroke</a> <a href='SkPaint_Reference#SkPaint'>width</a> <a href='SkPaint_Reference#SkPaint'>describes</a> <a href='SkPaint_Reference#SkPaint'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>thickness</a>,
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>describes</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>ends</a>, <a href='undocumented#Line'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>describes</a> <a href='#SkPaint_Join'>how</a>
corners are drawn.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPath_paint'><code><strong>paint</strong></code></a></td>
    <td>stroke, blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="fe2294131f422b8d6752f6a880f98ad9"><div>Top rows draw stroked <a href='#SkCanvas_drawPath_path'>path</a> <a href='#SkCanvas_drawPath_path'>with</a> <a href='#SkCanvas_drawPath_path'>combinations</a> <a href='#SkCanvas_drawPath_path'>of</a> <a href='#SkCanvas_drawPath_path'>joins</a> <a href='#SkCanvas_drawPath_path'>and</a> <a href='#SkCanvas_drawPath_path'>caps</a>. <a href='#SkCanvas_drawPath_path'>The</a> <a href='#SkCanvas_drawPath_path'>open</a> <a href='SkPath_Overview#Contour'>contour</a>
<a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>affected</a> <a href='SkPath_Overview#Contour'>by</a> <a href='SkPath_Overview#Contour'>caps</a>; <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>closed</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>affected</a> <a href='SkPath_Overview#Contour'>by</a> <a href='SkPath_Overview#Contour'>joins</a>.
<a href='SkPath_Overview#Contour'>Bottom</a> <a href='SkPath_Overview#Contour'>row</a> <a href='SkPath_Overview#Contour'>draws</a> <a href='SkPath_Overview#Contour'>fill</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>same</a> <a href='SkPath_Overview#Contour'>for</a> <a href='SkPath_Overview#Contour'>open</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>closed</a> <a href='SkPath_Overview#Contour'>contour</a>.
<a href='SkPath_Overview#Contour'>First</a> <a href='SkPath_Overview#Contour'>bottom</a> <a href='SkPath_Overview#Contour'>column</a> <a href='SkPath_Overview#Contour'>shows</a> <a href='SkPath_Overview#Contour'>winding</a> <a href='SkPath_Overview#Contour'>fills</a> <a href='SkPath_Overview#Contour'>overlap</a>.
<a href='SkPath_Overview#Contour'>Second</a> <a href='SkPath_Overview#Contour'>bottom</a> <a href='SkPath_Overview#Contour'>column</a> <a href='SkPath_Overview#Contour'>shows</a> <a href='SkPath_Overview#Contour'>even</a> <a href='SkPath_Overview#Contour'>odd</a> <a href='SkPath_Overview#Contour'>fills</a> <a href='SkPath_Overview#Contour'>exclude</a> <a href='SkPath_Overview#Contour'>overlap</a>.
<a href='SkPath_Overview#Contour'>Third</a> <a href='SkPath_Overview#Contour'>bottom</a> <a href='SkPath_Overview#Contour'>column</a> <a href='SkPath_Overview#Contour'>shows</a> <a href='SkPath_Overview#Contour'>inverse</a> <a href='SkPath_Overview#Contour'>winding</a> <a href='SkPath_Overview#Contour'>fills</a> <a href='SkPath_Overview#Contour'>area</a> <a href='SkPath_Overview#Contour'>outside</a> <a href='SkPath_Overview#Contour'>both</a> <a href='SkPath_Overview#Contour'>contours</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='#SkCanvas_drawLine'>drawLine</a> <a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkCanvas_drawPoints'>drawPoints</a>

<a name='Draw_Image'></a>

<a href='#SkCanvas_drawImage'>drawImage</a>, <a href='#SkCanvas_drawImageRect'>drawImageRect</a>, <a href='#SkCanvas_drawImageRect'>and</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_drawImageNine'>can</a> <a href='#SkCanvas_drawImageNine'>be</a> <a href='#SkCanvas_drawImageNine'>called</a> <a href='#SkCanvas_drawImageNine'>with</a> <a href='#SkCanvas_drawImageNine'>a</a> <a href='#SkCanvas_drawImageNine'>bare</a> <a href='#SkCanvas_drawImageNine'>pointer</a> <a href='#SkCanvas_drawImageNine'>or</a>
<a href='#SkCanvas_drawImageNine'>a</a>  <a href='undocumented#Smart_Pointer'>smart pointer</a> <a href='#SkCanvas_drawImageNine'>as</a> <a href='#SkCanvas_drawImageNine'>a</a> <a href='#SkCanvas_drawImageNine'>convenience</a>. <a href='#SkCanvas_drawImageNine'>The</a> <a href='#SkCanvas_drawImageNine'>pairs</a> <a href='#SkCanvas_drawImageNine'>of</a> <a href='#SkCanvas_drawImageNine'>calls</a> <a href='#SkCanvas_drawImageNine'>are</a> <a href='#SkCanvas_drawImageNine'>otherwise</a> <a href='#SkCanvas_drawImageNine'>identical</a>.

<a name='SkCanvas_drawImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImage'>drawImage</a>(<a href='#SkCanvas_drawImage'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImage_image'>image</a>, <a href='#SkCanvas_drawImage_image'>with</a> <a href='#SkCanvas_drawImage_image'>its</a> <a href='#SkCanvas_drawImage_image'>top-left</a> <a href='#SkCanvas_drawImage_image'>corner</a> <a href='#SkCanvas_drawImage_image'>at</a> (<a href='#SkCanvas_drawImage_left'>left</a>, <a href='#SkCanvas_drawImage_top'>top</a>),
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImage_paint'>paint</a>.

If <a href='#SkCanvas_drawImage_paint'>paint</a> <a href='#SkCanvas_drawImage_paint'>is</a> <a href='#SkCanvas_drawImage_paint'>supplied</a>, <a href='#SkCanvas_drawImage_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>,
and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImage_image'>image</a> <a href='#SkCanvas_drawImage_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImage_paint'>paint</a> <a href='#SkCanvas_drawImage_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImage_image'>image</a> <a href='#SkCanvas_drawImage_image'>bounds</a>. <a href='#SkCanvas_drawImage_image'>If</a> <a href='#SkCanvas_drawImage_image'>generated</a>
mask extends beyond <a href='#SkCanvas_drawImage_image'>image</a> <a href='#SkCanvas_drawImage_image'>bounds</a>, <a href='#SkCanvas_drawImage_image'>replicate</a> <a href='#SkCanvas_drawImage_image'>image</a> <a href='#SkCanvas_drawImage_image'>edge</a> <a href='#SkCanvas_drawImage_image'>colors</a>, <a href='#SkCanvas_drawImage_image'>just</a> <a href='#SkCanvas_drawImage_image'>as</a> <a href='undocumented#SkShader'>SkShader</a>
made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a>
<a href='#SkCanvas_drawImage_image'>image</a> <a href='#SkCanvas_drawImage_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImage_image'><code><strong>image</strong></code></a></td>
    <td>uncompressed rectangular map of pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_left'>left side</a> <a href='#SkCanvas_drawImage_left'>of</a> <a href='#SkCanvas_drawImage_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_top'><code><strong>top</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_top'>top</a> <a href='#SkCanvas_drawImage_top'>side</a> <a href='#SkCanvas_drawImage_top'>of</a> <a href='#SkCanvas_drawImage_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
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
void <a href='#SkCanvas_drawImage'>drawImage</a>(<a href='#SkCanvas_drawImage'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#Image'>image</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>,
               <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImage_2_image'>image</a>, <a href='#SkCanvas_drawImage_2_image'>with</a> <a href='#SkCanvas_drawImage_2_image'>its</a> <a href='#SkCanvas_drawImage_2_image'>top-left</a> <a href='#SkCanvas_drawImage_2_image'>corner</a> <a href='#SkCanvas_drawImage_2_image'>at</a> (<a href='#SkCanvas_drawImage_2_left'>left</a>, <a href='#SkCanvas_drawImage_2_top'>top</a>),
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImage_2_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImage_2_paint'>paint</a> <a href='#SkCanvas_drawImage_2_paint'>is</a> <a href='#SkCanvas_drawImage_2_paint'>supplied</a>, <a href='#SkCanvas_drawImage_2_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImage_2_image'>image</a> <a href='#SkCanvas_drawImage_2_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImage_2_paint'>paint</a> <a href='#SkCanvas_drawImage_2_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImage_2_image'>image</a> <a href='#SkCanvas_drawImage_2_image'>bounds</a>. <a href='#SkCanvas_drawImage_2_image'>If</a> <a href='#SkCanvas_drawImage_2_image'>generated</a>
mask extends beyond <a href='#SkCanvas_drawImage_2_image'>image</a> <a href='#SkCanvas_drawImage_2_image'>bounds</a>, <a href='#SkCanvas_drawImage_2_image'>replicate</a> <a href='#SkCanvas_drawImage_2_image'>image</a> <a href='#SkCanvas_drawImage_2_image'>edge</a> <a href='#SkCanvas_drawImage_2_image'>colors</a>, <a href='#SkCanvas_drawImage_2_image'>just</a> <a href='#SkCanvas_drawImage_2_image'>as</a> <a href='undocumented#SkShader'>SkShader</a>
made from <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a>
<a href='#SkCanvas_drawImage_2_image'>image</a> <a href='#SkCanvas_drawImage_2_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImage_2_image'><code><strong>image</strong></code></a></td>
    <td>uncompressed rectangular map of pixels</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawImage_2_left'>left side</a> <a href='#SkCanvas_drawImage_2_left'>of</a> <a href='#SkCanvas_drawImage_2_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_top'><code><strong>top</strong></code></a></td>
    <td>pop side of <a href='#SkCanvas_drawImage_2_image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImage_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
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

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>controls</a> <a href='#SkCanvas_SrcRectConstraint'>the</a> <a href='#SkCanvas_SrcRectConstraint'>behavior</a> <a href='#SkCanvas_SrcRectConstraint'>at</a> <a href='#SkCanvas_SrcRectConstraint'>the</a> <a href='#SkCanvas_SrcRectConstraint'>edge</a> <a href='#SkCanvas_SrcRectConstraint'>of</a> <a href='#SkCanvas_SrcRectConstraint'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>,
<a href='SkRect_Reference#Rect'>provided</a> <a href='SkRect_Reference#Rect'>to</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>, <a href='#SkCanvas_drawImageRect'>trading</a> <a href='#SkCanvas_drawImageRect'>off</a> <a href='#SkCanvas_drawImageRect'>speed</a> <a href='#SkCanvas_drawImageRect'>for</a> <a href='#SkCanvas_drawImageRect'>precision</a>.

<a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>in</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>may</a> <a href='SkPaint_Reference#Paint'>sample</a> <a href='SkPaint_Reference#Paint'>multiple</a> <a href='SkPaint_Reference#Paint'>pixels</a> <a href='SkPaint_Reference#Paint'>in</a> <a href='SkPaint_Reference#Paint'>the</a> <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>restricts</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>bounds</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>that</a> <a href='SkRect_Reference#Rect'>may</a> <a href='SkRect_Reference#Rect'>be</a> <a href='SkRect_Reference#Rect'>read</a>. <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>may</a> <a href='#Image_Filter'>slow</a> <a href='#Image_Filter'>down</a> <a href='#Image_Filter'>if</a>
<a href='#Image_Filter'>it</a> <a href='#Image_Filter'>cannot</a> <a href='#Image_Filter'>read</a> <a href='#Image_Filter'>outside</a> <a href='#Image_Filter'>the</a> <a href='#Image_Filter'>bounds</a>, <a href='#Image_Filter'>when</a> <a href='#Image_Filter'>sampling</a> <a href='#Image_Filter'>near</a> <a href='#Image_Filter'>the</a> <a href='#Image_Filter'>edge</a> <a href='#Image_Filter'>of</a> <a href='#Image_Filter'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>.
<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>specifies</a> <a href='#SkCanvas_SrcRectConstraint'>whether</a> <a href='#SkCanvas_SrcRectConstraint'>an</a> <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>is</a> <a href='#Image_Filter'>allowed</a> <a href='#Image_Filter'>to</a> <a href='#Image_Filter'>read</a> <a href='#Image_Filter'>pixels</a>
<a href='#Image_Filter'>outside</a> <a href='#Image_Filter'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kStrict_SrcRectConstraint'><code>SkCanvas::kStrict_SrcRectConstraint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Requires <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>to</a> <a href='#Image_Filter'>respect</a> <a href='#Image_Filter'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>,
<a href='SkRect_Reference#Rect'>sampling</a> <a href='SkRect_Reference#Rect'>only</a> <a href='SkRect_Reference#Rect'>inside</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>its</a> <a href='SkRect_Reference#Rect'>bounds</a>, <a href='SkRect_Reference#Rect'>possibly</a> <a href='SkRect_Reference#Rect'>with</a> <a href='SkRect_Reference#Rect'>a</a> <a href='SkRect_Reference#Rect'>performance</a> <a href='SkRect_Reference#Rect'>penalty</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kFast_SrcRectConstraint'><code>SkCanvas::kFast_SrcRectConstraint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Permits <a href='#Image_Filter'>Image_Filter</a> <a href='#Image_Filter'>to</a> <a href='#Image_Filter'>sample</a> <a href='#Image_Filter'>outside</a> <a href='#Image_Filter'>of</a> <a href='#Image_Filter'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>by</a> <a href='SkRect_Reference#Rect'>half</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>width</a> <a href='SkRect_Reference#Rect'>of</a> <a href='#Image_Filter'>Image_Filter</a>, <a href='#Image_Filter'>permitting</a> <a href='#Image_Filter'>it</a> <a href='#Image_Filter'>to</a> <a href='#Image_Filter'>run</a> <a href='#Image_Filter'>faster</a> <a href='#Image_Filter'>but</a> <a href='#Image_Filter'>with</a>
<a href='#Image_Filter'>error</a> <a href='#Image_Filter'>at</a> <a href='#Image_Filter'>the</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>edges</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5df49d1f4da37275a1f10ef7f1a749f0"><div>redBorder contains a black and white checkerboard bordered by red.
redBorder is drawn scaled by 16 on the left.
The middle and right <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>are</a> <a href='SkBitmap_Reference#Bitmap'>filtered</a> <a href='SkBitmap_Reference#Bitmap'>checkerboards</a>.
<a href='SkBitmap_Reference#Bitmap'>Drawing</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>checkerboard</a> <a href='SkBitmap_Reference#Bitmap'>with</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>shows</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>only</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>a</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>blur</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>of</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>black</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>and</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>white</a>.
<a href='#SkCanvas_kStrict_SrcRectConstraint'>Drawing</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>the</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>checkerboard</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>with</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>red</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>bleed</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>in</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>the</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>corners</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setImageFilter'>setImageFilter</a>

<a name='SkCanvas_drawImageRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                   <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_src'>src</a> <a href='#SkCanvas_drawImageRect_src'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_image'>image</a>, <a href='#SkCanvas_drawImageRect_image'>scaled</a> <a href='#SkCanvas_drawImageRect_image'>and</a> <a href='#SkCanvas_drawImageRect_image'>translated</a> <a href='#SkCanvas_drawImageRect_image'>to</a> <a href='#SkCanvas_drawImageRect_image'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_dst'>dst</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_paint'>paint</a> <a href='#SkCanvas_drawImageRect_paint'>is</a> <a href='#SkCanvas_drawImageRect_paint'>supplied</a>, <a href='#SkCanvas_drawImageRect_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_paint'>paint</a> <a href='#SkCanvas_drawImageRect_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>bounds</a>, <a href='#SkCanvas_drawImageRect_image'>replicate</a> <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>edge</a> <a href='#SkCanvas_drawImageRect_image'>colors</a>, <a href='#SkCanvas_drawImageRect_image'>just</a>
as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
replicates the <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

<a href='#SkCanvas_drawImageRect_constraint'>constraint</a> <a href='#SkCanvas_drawImageRect_constraint'>set</a> <a href='#SkCanvas_drawImageRect_constraint'>to</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawImageRect_src'>src</a>; <a href='#SkCanvas_drawImageRect_src'>set</a> <a href='#SkCanvas_drawImageRect_src'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>dimensions</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>to</a> <a href='#SkCanvas_drawImageRect_image'>draw</a> <a href='#SkCanvas_drawImageRect_image'>from</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_image'>image</a> <a href='#SkCanvas_drawImageRect_image'>to</a> <a href='#SkCanvas_drawImageRect_image'>draw</a> <a href='#SkCanvas_drawImageRect_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_src'>src</a> <a href='#SkCanvas_drawImageRect_src'>or</a> <a href='#SkCanvas_drawImageRect_src'>draw</a> <a href='#SkCanvas_drawImageRect_src'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bfd18e9cac896cdf94c9f154ccf94be8"><div>The left <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>draws</a> <a href='SkBitmap_Reference#Bitmap'>with</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>default</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kNone_SkFilterQuality'>and</a> <a href='undocumented#kNone_SkFilterQuality'>stays</a> <a href='undocumented#kNone_SkFilterQuality'>within</a>
<a href='undocumented#kNone_SkFilterQuality'>its</a> <a href='undocumented#kNone_SkFilterQuality'>bounds</a>; <a href='undocumented#kNone_SkFilterQuality'>there</a> <a href='undocumented#kNone_SkFilterQuality'>is</a> <a href='undocumented#kNone_SkFilterQuality'>no</a> <a href='undocumented#kNone_SkFilterQuality'>bleeding</a> <a href='undocumented#kNone_SkFilterQuality'>with</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a>.
<a href='#SkCanvas_kFast_SrcRectConstraint'>the</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>middle</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>and</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>right</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>with</a> <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>; <a href='undocumented#kLow_SkFilterQuality'>with</a>
<a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>, <a href='#SkCanvas_kStrict_SrcRectConstraint'>the</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>filter</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>remains</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>within</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>the</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>checkerboard</a>, <a href='#SkCanvas_kStrict_SrcRectConstraint'>and</a>
<a href='#SkCanvas_kStrict_SrcRectConstraint'>with</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>red</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>bleeds</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>on</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>the</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>edges</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>isrc</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                   <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> <a href='#SkCanvas_drawImageRect_2_isrc'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a>, <a href='#SkCanvas_drawImageRect_2_image'>scaled</a> <a href='#SkCanvas_drawImageRect_2_image'>and</a> <a href='#SkCanvas_drawImageRect_2_image'>translated</a> <a href='#SkCanvas_drawImageRect_2_image'>to</a> <a href='#SkCanvas_drawImageRect_2_image'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_2_dst'>dst</a>.
Note that <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> <a href='#SkCanvas_drawImageRect_2_isrc'>is</a> <a href='#SkCanvas_drawImageRect_2_isrc'>on</a> <a href='#SkCanvas_drawImageRect_2_isrc'>integer</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>boundaries</a>; <a href='#SkCanvas_drawImageRect_2_dst'>dst</a> <a href='#SkCanvas_drawImageRect_2_dst'>may</a> <a href='#SkCanvas_drawImageRect_2_dst'>include</a> <a href='#SkCanvas_drawImageRect_2_dst'>fractional</a>
boundaries. Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>
<a href='#SkCanvas_drawImageRect_2_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_2_paint'>paint</a> <a href='#SkCanvas_drawImageRect_2_paint'>is</a> <a href='#SkCanvas_drawImageRect_2_paint'>supplied</a>, <a href='#SkCanvas_drawImageRect_2_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_2_paint'>paint</a> <a href='#SkCanvas_drawImageRect_2_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>bounds</a>, <a href='#SkCanvas_drawImageRect_2_image'>replicate</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>edge</a> <a href='#SkCanvas_drawImageRect_2_image'>colors</a>, <a href='#SkCanvas_drawImageRect_2_image'>just</a>
as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
replicates the <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

<a href='#SkCanvas_drawImageRect_2_constraint'>constraint</a> <a href='#SkCanvas_drawImageRect_2_constraint'>set</a> <a href='#SkCanvas_drawImageRect_2_constraint'>to</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a>; <a href='#SkCanvas_drawImageRect_2_isrc'>set</a> <a href='#SkCanvas_drawImageRect_2_isrc'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_2_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>dimensions</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>to</a> <a href='#SkCanvas_drawImageRect_2_image'>draw</a> <a href='#SkCanvas_drawImageRect_2_image'>from</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_2_image'>image</a> <a href='#SkCanvas_drawImageRect_2_image'>to</a> <a href='#SkCanvas_drawImageRect_2_image'>draw</a> <a href='#SkCanvas_drawImageRect_2_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_2_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_2_isrc'>isrc</a> <a href='#SkCanvas_drawImageRect_2_isrc'>or</a> <a href='#SkCanvas_drawImageRect_2_isrc'>draw</a> <a href='#SkCanvas_drawImageRect_2_isrc'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f92cd5c9b9f4b1ac3cd933b08037bfe"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a>, <a href='#SkCanvas_drawImageRect_3_image'>scaled</a> <a href='#SkCanvas_drawImageRect_3_image'>and</a> <a href='#SkCanvas_drawImageRect_3_image'>translated</a> <a href='#SkCanvas_drawImageRect_3_image'>to</a> <a href='#SkCanvas_drawImageRect_3_image'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_3_dst'>dst</a>, <a href='#SkCanvas_drawImageRect_3_dst'>using</a> <a href='#SkCanvas_drawImageRect_3_dst'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>,
and optional <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_3_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_3_paint'>paint</a> <a href='#SkCanvas_drawImageRect_3_paint'>is</a> <a href='#SkCanvas_drawImageRect_3_paint'>supplied</a>, <a href='#SkCanvas_drawImageRect_3_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a> <a href='#SkCanvas_drawImageRect_3_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_3_paint'>paint</a> <a href='#SkCanvas_drawImageRect_3_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a> <a href='#SkCanvas_drawImageRect_3_image'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_3_image'>image</a> <a href='#SkCanvas_drawImageRect_3_image'>bounds</a>, <a href='#SkCanvas_drawImageRect_3_image'>replicate</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a> <a href='#SkCanvas_drawImageRect_3_image'>edge</a> <a href='#SkCanvas_drawImageRect_3_image'>colors</a>, <a href='#SkCanvas_drawImageRect_3_image'>just</a>
as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
replicates the <a href='#SkCanvas_drawImageRect_3_image'>image</a> <a href='#SkCanvas_drawImageRect_3_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_3_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>dimensions</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_3_image'>image</a> <a href='#SkCanvas_drawImageRect_3_image'>to</a> <a href='#SkCanvas_drawImageRect_3_image'>draw</a> <a href='#SkCanvas_drawImageRect_3_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
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
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_4_src'>src</a> <a href='#SkCanvas_drawImageRect_4_src'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a>, <a href='#SkCanvas_drawImageRect_4_image'>scaled</a> <a href='#SkCanvas_drawImageRect_4_image'>and</a> <a href='#SkCanvas_drawImageRect_4_image'>translated</a> <a href='#SkCanvas_drawImageRect_4_image'>to</a> <a href='#SkCanvas_drawImageRect_4_image'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_4_dst'>dst</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_4_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_4_paint'>paint</a> <a href='#SkCanvas_drawImageRect_4_paint'>is</a> <a href='#SkCanvas_drawImageRect_4_paint'>supplied</a>, <a href='#SkCanvas_drawImageRect_4_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_4_paint'>paint</a> <a href='#SkCanvas_drawImageRect_4_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>bounds</a>, <a href='#SkCanvas_drawImageRect_4_image'>replicate</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>edge</a> <a href='#SkCanvas_drawImageRect_4_image'>colors</a>, <a href='#SkCanvas_drawImageRect_4_image'>just</a>
as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
replicates the <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_4_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>dimensions</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>to</a> <a href='#SkCanvas_drawImageRect_4_image'>draw</a> <a href='#SkCanvas_drawImageRect_4_image'>from</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_4_image'>image</a> <a href='#SkCanvas_drawImageRect_4_image'>to</a> <a href='#SkCanvas_drawImageRect_4_image'>draw</a> <a href='#SkCanvas_drawImageRect_4_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_4_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_4_src'>src</a> <a href='#SkCanvas_drawImageRect_4_src'>or</a> <a href='#SkCanvas_drawImageRect_4_src'>draw</a> <a href='#SkCanvas_drawImageRect_4_src'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d4b35a9d24c32c042bd1f529b8de3c0d"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>scales</a> <a href='SkCanvas_Reference#Canvas'>and</a> <a href='SkCanvas_Reference#Canvas'>translates</a>; <a href='SkCanvas_Reference#Canvas'>transformation</a> <a href='SkCanvas_Reference#Canvas'>from</a> <a href='#SkCanvas_drawImageRect_4_src'>src</a> <a href='#SkCanvas_drawImageRect_4_src'>to</a> <a href='#SkCanvas_drawImageRect_4_dst'>dst</a> <a href='#SkCanvas_drawImageRect_4_dst'>also</a> <a href='#SkCanvas_drawImageRect_4_dst'>scales</a>.
<a href='#SkCanvas_drawImageRect_4_dst'>The</a> <a href='#SkCanvas_drawImageRect_4_dst'>two</a> <a href='SkMatrix_Reference#Matrix'>matrices</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>concatenated</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>create</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>final</a> <a href='SkMatrix_Reference#Matrix'>transformation</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_5'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>isrc</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawImageRect_5_isrc'>isrc</a> <a href='#SkCanvas_drawImageRect_5_isrc'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a>, <a href='#SkCanvas_drawImageRect_5_image'>scaled</a> <a href='#SkCanvas_drawImageRect_5_image'>and</a> <a href='#SkCanvas_drawImageRect_5_image'>translated</a> <a href='#SkCanvas_drawImageRect_5_image'>to</a> <a href='#SkCanvas_drawImageRect_5_image'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_5_dst'>dst</a>.
<a href='#SkCanvas_drawImageRect_5_isrc'>isrc</a> <a href='#SkCanvas_drawImageRect_5_isrc'>is</a> <a href='#SkCanvas_drawImageRect_5_isrc'>on</a> <a href='#SkCanvas_drawImageRect_5_isrc'>integer</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>boundaries</a>; <a href='#SkCanvas_drawImageRect_5_dst'>dst</a> <a href='#SkCanvas_drawImageRect_5_dst'>may</a> <a href='#SkCanvas_drawImageRect_5_dst'>include</a> <a href='#SkCanvas_drawImageRect_5_dst'>fractional</a> <a href='#SkCanvas_drawImageRect_5_dst'>boundaries</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_5_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_5_paint'>paint</a> <a href='#SkCanvas_drawImageRect_5_paint'>is</a> <a href='#SkCanvas_drawImageRect_5_paint'>supplied</a>, <a href='#SkCanvas_drawImageRect_5_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_5_paint'>paint</a> <a href='#SkCanvas_drawImageRect_5_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>bounds</a>, <a href='#SkCanvas_drawImageRect_5_image'>replicate</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>edge</a> <a href='#SkCanvas_drawImageRect_5_image'>colors</a>, <a href='#SkCanvas_drawImageRect_5_image'>just</a>
as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
replicates the <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

<a href='#SkCanvas_drawImageRect_5_constraint'>constraint</a> <a href='#SkCanvas_drawImageRect_5_constraint'>set</a> <a href='#SkCanvas_drawImageRect_5_constraint'>to</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawImageRect_5_image'>image</a>; <a href='#SkCanvas_drawImageRect_5_image'>set</a> <a href='#SkCanvas_drawImageRect_5_image'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_5_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>dimensions</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>to</a> <a href='#SkCanvas_drawImageRect_5_image'>draw</a> <a href='#SkCanvas_drawImageRect_5_image'>from</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>to</a> <a href='#SkCanvas_drawImageRect_5_image'>draw</a> <a href='#SkCanvas_drawImageRect_5_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_5_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_5_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawImageRect_5_image'>image</a> <a href='#SkCanvas_drawImageRect_5_image'>or</a> <a href='#SkCanvas_drawImageRect_5_image'>draw</a> <a href='#SkCanvas_drawImageRect_5_image'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d307e7e1237f39fb54d80723e5449857"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a>

<a name='SkCanvas_drawImageRect_6'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageRect'>drawImageRect</a>(<a href='#SkCanvas_drawImageRect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a>, <a href='#SkCanvas_drawImageRect_6_image'>scaled</a> <a href='#SkCanvas_drawImageRect_6_image'>and</a> <a href='#SkCanvas_drawImageRect_6_image'>translated</a> <a href='#SkCanvas_drawImageRect_6_image'>to</a> <a href='#SkCanvas_drawImageRect_6_image'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawImageRect_6_dst'>dst</a>,
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_6_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawImageRect_6_paint'>paint</a> <a href='#SkCanvas_drawImageRect_6_paint'>is</a> <a href='#SkCanvas_drawImageRect_6_paint'>supplied</a>, <a href='#SkCanvas_drawImageRect_6_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a> <a href='#SkCanvas_drawImageRect_6_image'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawImageRect_6_paint'>paint</a> <a href='#SkCanvas_drawImageRect_6_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a> <a href='#SkCanvas_drawImageRect_6_image'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawImageRect_6_image'>image</a> <a href='#SkCanvas_drawImageRect_6_image'>bounds</a>, <a href='#SkCanvas_drawImageRect_6_image'>replicate</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a> <a href='#SkCanvas_drawImageRect_6_image'>edge</a> <a href='#SkCanvas_drawImageRect_6_image'>colors</a>, <a href='#SkCanvas_drawImageRect_6_image'>just</a>
as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
replicates the <a href='#SkCanvas_drawImageRect_6_image'>image</a> <a href='#SkCanvas_drawImageRect_6_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

constraint set to <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawImageRect_6_image'>image</a>; <a href='#SkCanvas_drawImageRect_6_image'>set</a> <a href='#SkCanvas_drawImageRect_6_image'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageRect_6_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>dimensions</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawImageRect_6_image'>image</a> <a href='#SkCanvas_drawImageRect_6_image'>to</a> <a href='#SkCanvas_drawImageRect_6_image'>draw</a> <a href='#SkCanvas_drawImageRect_6_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageRect_6_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
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
void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(<a href='#SkCanvas_drawImageNine'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>center</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>stretched</a> <a href='#SkCanvas_drawImageNine_image'>proportionally</a> <a href='#SkCanvas_drawImageNine_image'>to</a> <a href='#SkCanvas_drawImageNine_image'>fit</a> <a href='#SkCanvas_drawImageNine_image'>into</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageNine_dst'>dst</a>.
<a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawImageNine_center'>center</a> <a href='#SkCanvas_drawImageNine_center'>divides</a> <a href='#SkCanvas_drawImageNine_center'>the</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>into</a> <a href='#SkCanvas_drawImageNine_image'>nine</a> <a href='#SkCanvas_drawImageNine_image'>sections</a>: <a href='#SkCanvas_drawImageNine_image'>four</a> <a href='#SkCanvas_drawImageNine_image'>sides</a>, <a href='#SkCanvas_drawImageNine_image'>four</a> <a href='#SkCanvas_drawImageNine_image'>corners</a>, <a href='#SkCanvas_drawImageNine_image'>and</a>
<a href='#SkCanvas_drawImageNine_image'>the</a> <a href='#SkCanvas_drawImageNine_center'>center</a>. <a href='#SkCanvas_drawImageNine_center'>Corners</a> <a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>unmodified</a> <a href='#SkCanvas_drawImageNine_center'>or</a> <a href='#SkCanvas_drawImageNine_center'>scaled</a> <a href='#SkCanvas_drawImageNine_center'>down</a> <a href='#SkCanvas_drawImageNine_center'>proportionately</a> <a href='#SkCanvas_drawImageNine_center'>if</a> <a href='#SkCanvas_drawImageNine_center'>their</a> <a href='#SkCanvas_drawImageNine_center'>sides</a>
<a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>larger</a> <a href='#SkCanvas_drawImageNine_center'>than</a> <a href='#SkCanvas_drawImageNine_dst'>dst</a>; <a href='#SkCanvas_drawImageNine_center'>center</a> <a href='#SkCanvas_drawImageNine_center'>and</a> <a href='#SkCanvas_drawImageNine_center'>four</a> <a href='#SkCanvas_drawImageNine_center'>sides</a> <a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>scaled</a> <a href='#SkCanvas_drawImageNine_center'>to</a> <a href='#SkCanvas_drawImageNine_center'>fit</a> <a href='#SkCanvas_drawImageNine_center'>remaining</a> <a href='#SkCanvas_drawImageNine_center'>space</a>, <a href='#SkCanvas_drawImageNine_center'>if</a> <a href='#SkCanvas_drawImageNine_center'>any</a>.

<a href='#SkCanvas_drawImageNine_center'>Additionally</a> <a href='#SkCanvas_drawImageNine_center'>transform</a> <a href='#SkCanvas_drawImageNine_center'>draw</a> <a href='#SkCanvas_drawImageNine_center'>using</a> <a href='#SkCanvas_drawImageNine_center'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>optional</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#SkCanvas_drawImageNine_paint'>is</a> <a href='#SkCanvas_drawImageNine_paint'>supplied</a>, <a href='#SkCanvas_drawImageNine_paint'>apply</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Blend_Mode'>and</a> <a href='#Draw_Looper'>Draw_Looper</a>. <a href='#Draw_Looper'>If</a> image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#Shader'>Shader</a>.
<a href='undocumented#Shader'>If</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#SkCanvas_drawImageNine_paint'>contains</a> <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Mask_Filter'>generate</a> <a href='#Mask_Filter'>mask</a> <a href='#Mask_Filter'>from</a> image bounds. If <a href='#SkCanvas_drawImageNine_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>set</a> <a href='#Filter_Quality'>to</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kNone_SkFilterQuality'>disable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>filtering</a>. <a href='undocumented#Pixel'>For</a> <a href='undocumented#Pixel'>all</a>
<a href='undocumented#Pixel'>other</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>of</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, <a href='#Filter_Quality'>use</a> <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>to</a> <a href='undocumented#kLow_SkFilterQuality'>filter</a> <a href='undocumented#kLow_SkFilterQuality'>pixels</a>.
<a href='undocumented#kLow_SkFilterQuality'>Any</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>on</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#SkCanvas_drawImageNine_paint'>is</a> <a href='#SkCanvas_drawImageNine_paint'>ignored</a> <a href='#SkCanvas_drawImageNine_paint'>as</a> <a href='#SkCanvas_drawImageNine_paint'>is</a> <a href='#SkCanvas_drawImageNine_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> <a href='#Paint_Anti_Alias'>state</a>.
If generated mask extends beyond <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>bounds</a>, <a href='#SkCanvas_drawImageNine_image'>replicate</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>edge</a> <a href='#SkCanvas_drawImageNine_image'>colors</a>, <a href='#SkCanvas_drawImageNine_image'>just</a>
<a href='#SkCanvas_drawImageNine_image'>as</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>made</a> <a href='undocumented#Shader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
<a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageNine_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>containing</a> <a href='SkImage_Reference#Image'>pixels</a>, <a href='SkImage_Reference#Image'>dimensions</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>edge</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>corners</a> <a href='#SkCanvas_drawImageNine_image'>and</a> <a href='#SkCanvas_drawImageNine_image'>sides</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>to</a> <a href='#SkCanvas_drawImageNine_image'>draw</a> <a href='#SkCanvas_drawImageNine_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>containing</a> <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Image_Filter'>and</a> <a href='#Image_Filter'>so</a> <a href='#Image_Filter'>on</a>; <a href='#Image_Filter'>or</a> <a href='#Image_Filter'>nullptr</a>
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4f153cf1d0dbe1a95acf5badeec14dae"><div>The leftmost <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>is</a> <a href='#SkCanvas_drawImageNine_image'>smaller</a> <a href='#SkCanvas_drawImageNine_image'>than</a> <a href='#SkCanvas_drawImageNine_center'>center</a>; <a href='#SkCanvas_drawImageNine_center'>only</a> <a href='#SkCanvas_drawImageNine_center'>corners</a> <a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>drawn</a>, <a href='#SkCanvas_drawImageNine_center'>all</a> <a href='#SkCanvas_drawImageNine_center'>scaled</a> <a href='#SkCanvas_drawImageNine_center'>to</a> <a href='#SkCanvas_drawImageNine_center'>fit</a>.
<a href='#SkCanvas_drawImageNine_center'>The</a> <a href='#SkCanvas_drawImageNine_center'>second</a> <a href='#SkCanvas_drawImageNine_image'>image</a> <a href='#SkCanvas_drawImageNine_image'>equals</a> <a href='#SkCanvas_drawImageNine_image'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkCanvas_drawImageNine_center'>center</a>; <a href='#SkCanvas_drawImageNine_center'>only</a> <a href='#SkCanvas_drawImageNine_center'>corners</a> <a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>drawn</a> <a href='#SkCanvas_drawImageNine_center'>without</a> <a href='#SkCanvas_drawImageNine_center'>scaling</a>.
<a href='#SkCanvas_drawImageNine_center'>The</a> <a href='#SkCanvas_drawImageNine_center'>remaining</a> <a href='#SkCanvas_drawImageNine_center'>images</a> <a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>larger</a> <a href='#SkCanvas_drawImageNine_center'>than</a> <a href='#SkCanvas_drawImageNine_center'>center</a>. <a href='#SkCanvas_drawImageNine_center'>All</a> <a href='#SkCanvas_drawImageNine_center'>corners</a> <a href='#SkCanvas_drawImageNine_center'>draw</a> <a href='#SkCanvas_drawImageNine_center'>without</a> <a href='#SkCanvas_drawImageNine_center'>scaling</a>.
<a href='#SkCanvas_drawImageNine_center'>The</a> <a href='#SkCanvas_drawImageNine_center'>sides</a> <a href='#SkCanvas_drawImageNine_center'>and</a> <a href='#SkCanvas_drawImageNine_center'>center</a> <a href='#SkCanvas_drawImageNine_center'>are</a> <a href='#SkCanvas_drawImageNine_center'>scaled</a> <a href='#SkCanvas_drawImageNine_center'>if</a> <a href='#SkCanvas_drawImageNine_center'>needed</a> <a href='#SkCanvas_drawImageNine_center'>to</a> <a href='#SkCanvas_drawImageNine_center'>take</a> <a href='#SkCanvas_drawImageNine_center'>up</a> <a href='#SkCanvas_drawImageNine_center'>the</a> <a href='#SkCanvas_drawImageNine_center'>remaining</a> <a href='#SkCanvas_drawImageNine_center'>space</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>

<a name='SkCanvas_drawImageNine_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageNine'>drawImageNine</a>(<a href='#SkCanvas_drawImageNine'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>center</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                   <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>stretched</a> <a href='#SkCanvas_drawImageNine_2_image'>proportionally</a> <a href='#SkCanvas_drawImageNine_2_image'>to</a> <a href='#SkCanvas_drawImageNine_2_image'>fit</a> <a href='#SkCanvas_drawImageNine_2_image'>into</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageNine_2_dst'>dst</a>.
<a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawImageNine_2_center'>center</a> <a href='#SkCanvas_drawImageNine_2_center'>divides</a> <a href='#SkCanvas_drawImageNine_2_center'>the</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>into</a> <a href='#SkCanvas_drawImageNine_2_image'>nine</a> <a href='#SkCanvas_drawImageNine_2_image'>sections</a>: <a href='#SkCanvas_drawImageNine_2_image'>four</a> <a href='#SkCanvas_drawImageNine_2_image'>sides</a>, <a href='#SkCanvas_drawImageNine_2_image'>four</a> <a href='#SkCanvas_drawImageNine_2_image'>corners</a>, <a href='#SkCanvas_drawImageNine_2_image'>and</a>
<a href='#SkCanvas_drawImageNine_2_image'>the</a> <a href='#SkCanvas_drawImageNine_2_center'>center</a>. <a href='#SkCanvas_drawImageNine_2_center'>Corners</a> <a href='#SkCanvas_drawImageNine_2_center'>are</a> <a href='#SkCanvas_drawImageNine_2_center'>not</a> <a href='#SkCanvas_drawImageNine_2_center'>scaled</a>, <a href='#SkCanvas_drawImageNine_2_center'>or</a> <a href='#SkCanvas_drawImageNine_2_center'>scaled</a> <a href='#SkCanvas_drawImageNine_2_center'>down</a> <a href='#SkCanvas_drawImageNine_2_center'>proportionately</a> <a href='#SkCanvas_drawImageNine_2_center'>if</a> <a href='#SkCanvas_drawImageNine_2_center'>their</a> <a href='#SkCanvas_drawImageNine_2_center'>sides</a>
<a href='#SkCanvas_drawImageNine_2_center'>are</a> <a href='#SkCanvas_drawImageNine_2_center'>larger</a> <a href='#SkCanvas_drawImageNine_2_center'>than</a> <a href='#SkCanvas_drawImageNine_2_dst'>dst</a>; <a href='#SkCanvas_drawImageNine_2_center'>center</a> <a href='#SkCanvas_drawImageNine_2_center'>and</a> <a href='#SkCanvas_drawImageNine_2_center'>four</a> <a href='#SkCanvas_drawImageNine_2_center'>sides</a> <a href='#SkCanvas_drawImageNine_2_center'>are</a> <a href='#SkCanvas_drawImageNine_2_center'>scaled</a> <a href='#SkCanvas_drawImageNine_2_center'>to</a> <a href='#SkCanvas_drawImageNine_2_center'>fit</a> <a href='#SkCanvas_drawImageNine_2_center'>remaining</a> <a href='#SkCanvas_drawImageNine_2_center'>space</a>, <a href='#SkCanvas_drawImageNine_2_center'>if</a> <a href='#SkCanvas_drawImageNine_2_center'>any</a>.

<a href='#SkCanvas_drawImageNine_2_center'>Additionally</a> <a href='#SkCanvas_drawImageNine_2_center'>transform</a> <a href='#SkCanvas_drawImageNine_2_center'>draw</a> <a href='#SkCanvas_drawImageNine_2_center'>using</a> <a href='#SkCanvas_drawImageNine_2_center'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>optional</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>is</a> <a href='#SkCanvas_drawImageNine_2_paint'>supplied</a>, <a href='#SkCanvas_drawImageNine_2_paint'>apply</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Blend_Mode'>and</a> <a href='#Draw_Looper'>Draw_Looper</a>. <a href='#Draw_Looper'>If</a> image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#Shader'>Shader</a>.
<a href='undocumented#Shader'>If</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>contains</a> <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Mask_Filter'>generate</a> <a href='#Mask_Filter'>mask</a> <a href='#Mask_Filter'>from</a> image bounds. If <a href='#SkCanvas_drawImageNine_2_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>set</a> <a href='#Filter_Quality'>to</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kNone_SkFilterQuality'>disable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>filtering</a>. <a href='undocumented#Pixel'>For</a> <a href='undocumented#Pixel'>all</a>
<a href='undocumented#Pixel'>other</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>of</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, <a href='#Filter_Quality'>use</a> <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>to</a> <a href='undocumented#kLow_SkFilterQuality'>filter</a> <a href='undocumented#kLow_SkFilterQuality'>pixels</a>.
<a href='undocumented#kLow_SkFilterQuality'>Any</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>on</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#SkCanvas_drawImageNine_2_paint'>is</a> <a href='#SkCanvas_drawImageNine_2_paint'>ignored</a> <a href='#SkCanvas_drawImageNine_2_paint'>as</a> <a href='#SkCanvas_drawImageNine_2_paint'>is</a> <a href='#SkCanvas_drawImageNine_2_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> <a href='#Paint_Anti_Alias'>state</a>.
If generated mask extends beyond <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>bounds</a>, <a href='#SkCanvas_drawImageNine_2_image'>replicate</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>edge</a> <a href='#SkCanvas_drawImageNine_2_image'>colors</a>, <a href='#SkCanvas_drawImageNine_2_image'>just</a>
<a href='#SkCanvas_drawImageNine_2_image'>as</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>made</a> <a href='undocumented#Shader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_makeShader'>makeShader</a> <a href='#SkImage_makeShader'>with</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a>
<a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a> <a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageNine_2_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>containing</a> <a href='SkImage_Reference#Image'>pixels</a>, <a href='SkImage_Reference#Image'>dimensions</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>edge</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>corners</a> <a href='#SkCanvas_drawImageNine_2_image'>and</a> <a href='#SkCanvas_drawImageNine_2_image'>sides</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>to</a> <a href='#SkCanvas_drawImageNine_2_image'>draw</a> <a href='#SkCanvas_drawImageNine_2_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageNine_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>containing</a> <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Image_Filter'>and</a> <a href='#Image_Filter'>so</a> <a href='#Image_Filter'>on</a>; <a href='#Image_Filter'>or</a> <a href='#Image_Filter'>nullptr</a>
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d597d9af8d17fd93e634dd12017058e2"><div>The two leftmost images has four corners and sides to the left and right of <a href='#SkCanvas_drawImageNine_2_center'>center</a>.
<a href='#SkCanvas_drawImageNine_2_center'>The</a> <a href='#SkCanvas_drawImageNine_2_center'>leftmost</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>scales</a> <a href='#SkCanvas_drawImageNine_2_image'>the</a> <a href='#SkCanvas_drawImageNine_2_image'>width</a> <a href='#SkCanvas_drawImageNine_2_image'>of</a> <a href='#SkCanvas_drawImageNine_2_image'>corners</a> <a href='#SkCanvas_drawImageNine_2_image'>proportionately</a> <a href='#SkCanvas_drawImageNine_2_image'>to</a> <a href='#SkCanvas_drawImageNine_2_image'>fit</a>.
<a href='#SkCanvas_drawImageNine_2_image'>The</a> <a href='#SkCanvas_drawImageNine_2_image'>third</a> <a href='#SkCanvas_drawImageNine_2_image'>and</a> <a href='#SkCanvas_drawImageNine_2_image'>fourth</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>corners</a> <a href='#SkCanvas_drawImageNine_2_image'>are</a> <a href='#SkCanvas_drawImageNine_2_image'>not</a> <a href='#SkCanvas_drawImageNine_2_image'>scaled</a>; <a href='#SkCanvas_drawImageNine_2_image'>the</a> <a href='#SkCanvas_drawImageNine_2_image'>sides</a> <a href='#SkCanvas_drawImageNine_2_image'>and</a> <a href='#SkCanvas_drawImageNine_2_center'>center</a> <a href='#SkCanvas_drawImageNine_2_center'>are</a> <a href='#SkCanvas_drawImageNine_2_center'>scaled</a> <a href='#SkCanvas_drawImageNine_2_center'>to</a>
<a href='#SkCanvas_drawImageNine_2_center'>fill</a> <a href='#SkCanvas_drawImageNine_2_center'>the</a> <a href='#SkCanvas_drawImageNine_2_center'>remaining</a> <a href='#SkCanvas_drawImageNine_2_center'>space</a>.
<a href='#SkCanvas_drawImageNine_2_center'>The</a> <a href='#SkCanvas_drawImageNine_2_center'>rightmost</a> <a href='#SkCanvas_drawImageNine_2_image'>image</a> <a href='#SkCanvas_drawImageNine_2_image'>has</a> <a href='#SkCanvas_drawImageNine_2_image'>four</a> <a href='#SkCanvas_drawImageNine_2_image'>corners</a> <a href='#SkCanvas_drawImageNine_2_image'>scaled</a> <a href='#SkCanvas_drawImageNine_2_image'>vertically</a> <a href='#SkCanvas_drawImageNine_2_image'>to</a> <a href='#SkCanvas_drawImageNine_2_image'>fit</a>, <a href='#SkCanvas_drawImageNine_2_image'>and</a> <a href='#SkCanvas_drawImageNine_2_image'>uses</a> <a href='#SkCanvas_drawImageNine_2_image'>sides</a> <a href='#SkCanvas_drawImageNine_2_image'>above</a>
<a href='#SkCanvas_drawImageNine_2_image'>and</a> <a href='#SkCanvas_drawImageNine_2_image'>below</a> <a href='#SkCanvas_drawImageNine_2_center'>center</a> <a href='#SkCanvas_drawImageNine_2_center'>to</a> <a href='#SkCanvas_drawImageNine_2_center'>fill</a> <a href='#SkCanvas_drawImageNine_2_center'>the</a> <a href='#SkCanvas_drawImageNine_2_center'>remaining</a> <a href='#SkCanvas_drawImageNine_2_center'>space</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawImageRect'>drawImageRect</a>

<a name='SkCanvas_drawBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmap'>drawBitmap</a>(<a href='#SkCanvas_drawBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a>, <a href='#SkCanvas_drawBitmap_bitmap'>with</a> <a href='#SkCanvas_drawBitmap_bitmap'>its</a> <a href='#SkCanvas_drawBitmap_bitmap'>top-left</a> <a href='#SkCanvas_drawBitmap_bitmap'>corner</a> <a href='#SkCanvas_drawBitmap_bitmap'>at</a> (<a href='#SkCanvas_drawBitmap_left'>left</a>, <a href='#SkCanvas_drawBitmap_top'>top</a>),
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmap_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmap_paint'>paint</a> <a href='#SkCanvas_drawBitmap_paint'>is</a> <a href='#SkCanvas_drawBitmap_paint'>not</a> <a href='#SkCanvas_drawBitmap_paint'>nullptr</a>, <a href='#SkCanvas_drawBitmap_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmap_paint'>paint</a> <a href='#SkCanvas_drawBitmap_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>bounds</a>, <a href='#SkCanvas_drawBitmap_bitmap'>replicate</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>edge</a> <a href='#SkCanvas_drawBitmap_bitmap'>colors</a>,
just as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmap_bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
outside of its bounds.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>containing</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a>, <a href='SkBitmap_Reference#SkBitmap'>dimensions</a>, <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='SkBitmap_Reference#SkBitmap'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_left'><code><strong>left</strong></code></a></td>
    <td><a href='#SkCanvas_drawBitmap_left'>left side</a> <a href='#SkCanvas_drawBitmap_left'>of</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_top'><code><strong>top</strong></code></a></td>
    <td><a href='#SkCanvas_drawBitmap_top'>top</a> <a href='#SkCanvas_drawBitmap_top'>side</a> <a href='#SkCanvas_drawBitmap_top'>of</a> <a href='#SkCanvas_drawBitmap_bitmap'>bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmap_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
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
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(<a href='#SkCanvas_drawBitmapRect'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                    <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_src'>src</a> <a href='#SkCanvas_drawBitmapRect_src'>of</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a>, <a href='#SkCanvas_drawBitmapRect_bitmap'>scaled</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>and</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>translated</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>to</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_dst'>dst</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_paint'>paint</a> <a href='#SkCanvas_drawBitmapRect_paint'>is</a> <a href='#SkCanvas_drawBitmapRect_paint'>supplied</a>, <a href='#SkCanvas_drawBitmapRect_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmapRect_paint'>paint</a> <a href='#SkCanvas_drawBitmapRect_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bounds</a>, <a href='#SkCanvas_drawBitmapRect_bitmap'>replicate</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>edge</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>colors</a>,
just as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
outside of its bounds.

<a href='#SkCanvas_drawBitmapRect_constraint'>constraint</a> <a href='#SkCanvas_drawBitmapRect_constraint'>set</a> <a href='#SkCanvas_drawBitmapRect_constraint'>to</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawBitmapRect_src'>src</a>; <a href='#SkCanvas_drawBitmapRect_src'>set</a> <a href='#SkCanvas_drawBitmapRect_src'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>containing</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a>, <a href='SkBitmap_Reference#SkBitmap'>dimensions</a>, <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='SkBitmap_Reference#SkBitmap'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>from</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawBitmapRect_src'>src</a> <a href='#SkCanvas_drawBitmapRect_src'>or</a> <a href='#SkCanvas_drawBitmapRect_src'>draw</a> <a href='#SkCanvas_drawBitmapRect_src'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7d04932f2a259cc70d6e45cd25a6feb6"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

<a name='SkCanvas_drawBitmapRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(<a href='#SkCanvas_drawBitmapRect'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>isrc</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                    <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>of</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a>, <a href='#SkCanvas_drawBitmapRect_2_bitmap'>scaled</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>and</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>translated</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>to</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_2_dst'>dst</a>.
<a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>is</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>on</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>integer</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>boundaries</a>; <a href='#SkCanvas_drawBitmapRect_2_dst'>dst</a> <a href='#SkCanvas_drawBitmapRect_2_dst'>may</a> <a href='#SkCanvas_drawBitmapRect_2_dst'>include</a> <a href='#SkCanvas_drawBitmapRect_2_dst'>fractional</a> <a href='#SkCanvas_drawBitmapRect_2_dst'>boundaries</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>paint</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>is</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>supplied</a>, <a href='#SkCanvas_drawBitmapRect_2_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmapRect_2_paint'>paint</a> <a href='#SkCanvas_drawBitmapRect_2_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bounds</a>, <a href='#SkCanvas_drawBitmapRect_2_bitmap'>replicate</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>edge</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>colors</a>,
just as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_2_bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
outside of its bounds.

<a href='#SkCanvas_drawBitmapRect_2_constraint'>constraint</a> <a href='#SkCanvas_drawBitmapRect_2_constraint'>set</a> <a href='#SkCanvas_drawBitmapRect_2_constraint'>to</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a>; <a href='#SkCanvas_drawBitmapRect_2_isrc'>set</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>containing</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a>, <a href='SkBitmap_Reference#SkBitmap'>dimensions</a>, <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='SkBitmap_Reference#SkBitmap'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_isrc'><code><strong>isrc</strong></code></a></td>
    <td>source <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>from</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_2_constraint'><code><strong>constraint</strong></code></a></td>
    <td>sample strictly within <a href='#SkCanvas_drawBitmapRect_2_isrc'>isrc</a>, <a href='#SkCanvas_drawBitmapRect_2_isrc'>or</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>draw</a> <a href='#SkCanvas_drawBitmapRect_2_isrc'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0a3c6d2459566e58cee7d4910655ee21"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

<a name='SkCanvas_drawBitmapRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>(<a href='#SkCanvas_drawBitmapRect'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                    <a href='#SkCanvas_SrcRectConstraint'>SrcRectConstraint</a> <a href='#SkCanvas_SrcRectConstraint'>constraint</a> = <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a>)
</pre>

Draws <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a>, <a href='#SkCanvas_drawBitmapRect_3_bitmap'>scaled</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>and</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>translated</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>to</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>fill</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawBitmapRect_3_dst'>dst</a>.
<a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bounds</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>is</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>on</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>integer</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>boundaries</a>; <a href='#SkCanvas_drawBitmapRect_3_dst'>dst</a> <a href='#SkCanvas_drawBitmapRect_3_dst'>may</a> <a href='#SkCanvas_drawBitmapRect_3_dst'>include</a> <a href='#SkCanvas_drawBitmapRect_3_dst'>fractional</a> <a href='#SkCanvas_drawBitmapRect_3_dst'>boundaries</a>.
Additionally transform draw using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>paint</a>.

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>paint</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>is</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>supplied</a>, <a href='#SkCanvas_drawBitmapRect_3_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>. <a href='undocumented#SkDrawLooper'>If</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#SkShader'>SkShader</a>.
If <a href='#SkCanvas_drawBitmapRect_3_paint'>paint</a> <a href='#SkCanvas_drawBitmapRect_3_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>generate</a> <a href='undocumented#SkMaskFilter'>mask</a> <a href='undocumented#SkMaskFilter'>from</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bounds</a>.

If generated mask extends beyond <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bounds</a>, <a href='#SkCanvas_drawBitmapRect_3_bitmap'>replicate</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>edge</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>colors</a>,
just as <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>made</a> <a href='undocumented#SkShader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
outside of its bounds.

<a href='#SkCanvas_drawBitmapRect_3_constraint'>constraint</a> <a href='#SkCanvas_drawBitmapRect_3_constraint'>set</a> <a href='#SkCanvas_drawBitmapRect_3_constraint'>to</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>kStrict_SrcRectConstraint</a> <a href='#SkCanvas_kStrict_SrcRectConstraint'>limits</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>to</a>
sample within <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a>; <a href='#SkCanvas_drawBitmapRect_3_bitmap'>set</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>to</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>kFast_SrcRectConstraint</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>allows</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>sampling</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>outside</a> <a href='#SkCanvas_kFast_SrcRectConstraint'>to</a>
improve performance.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>containing</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a>, <a href='SkBitmap_Reference#SkBitmap'>dimensions</a>, <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='SkBitmap_Reference#SkBitmap'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapRect_3_constraint'><code><strong>constraint</strong></code></a></td>
    <td>filter strictly within <a href='#SkCanvas_drawBitmapRect_3_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>or</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>draw</a> <a href='#SkCanvas_drawBitmapRect_3_bitmap'>faster</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bdbeac3c97f60a63987b1cc8e1f1e91e"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageRect'>drawImageRect</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>

<a name='SkCanvas_drawBitmapNine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>(<a href='#SkCanvas_drawBitmapNine'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>center</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                    <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>stretched</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>proportionally</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>to</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>fit</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>into</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapNine_dst'>dst</a>.
<a href='SkIRect_Reference#IRect'>IRect</a> <a href='#SkCanvas_drawBitmapNine_center'>center</a> <a href='#SkCanvas_drawBitmapNine_center'>divides</a> <a href='#SkCanvas_drawBitmapNine_center'>the</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>into</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>nine</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>sections</a>: <a href='#SkCanvas_drawBitmapNine_bitmap'>four</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>sides</a>, <a href='#SkCanvas_drawBitmapNine_bitmap'>four</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>corners</a>,
<a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>the</a> <a href='#SkCanvas_drawBitmapNine_center'>center</a>. <a href='#SkCanvas_drawBitmapNine_center'>Corners</a> <a href='#SkCanvas_drawBitmapNine_center'>are</a> <a href='#SkCanvas_drawBitmapNine_center'>not</a> <a href='#SkCanvas_drawBitmapNine_center'>scaled</a>, <a href='#SkCanvas_drawBitmapNine_center'>or</a> <a href='#SkCanvas_drawBitmapNine_center'>scaled</a> <a href='#SkCanvas_drawBitmapNine_center'>down</a> <a href='#SkCanvas_drawBitmapNine_center'>proportionately</a> <a href='#SkCanvas_drawBitmapNine_center'>if</a> <a href='#SkCanvas_drawBitmapNine_center'>their</a>
<a href='#SkCanvas_drawBitmapNine_center'>sides</a> <a href='#SkCanvas_drawBitmapNine_center'>are</a> <a href='#SkCanvas_drawBitmapNine_center'>larger</a> <a href='#SkCanvas_drawBitmapNine_center'>than</a> <a href='#SkCanvas_drawBitmapNine_dst'>dst</a>; <a href='#SkCanvas_drawBitmapNine_center'>center</a> <a href='#SkCanvas_drawBitmapNine_center'>and</a> <a href='#SkCanvas_drawBitmapNine_center'>four</a> <a href='#SkCanvas_drawBitmapNine_center'>sides</a> <a href='#SkCanvas_drawBitmapNine_center'>are</a> <a href='#SkCanvas_drawBitmapNine_center'>scaled</a> <a href='#SkCanvas_drawBitmapNine_center'>to</a> <a href='#SkCanvas_drawBitmapNine_center'>fit</a> <a href='#SkCanvas_drawBitmapNine_center'>remaining</a>
<a href='#SkCanvas_drawBitmapNine_center'>space</a>, <a href='#SkCanvas_drawBitmapNine_center'>if</a> <a href='#SkCanvas_drawBitmapNine_center'>any</a>.

<a href='#SkCanvas_drawBitmapNine_center'>Additionally</a> <a href='#SkCanvas_drawBitmapNine_center'>transform</a> <a href='#SkCanvas_drawBitmapNine_center'>draw</a> <a href='#SkCanvas_drawBitmapNine_center'>using</a> <a href='#SkCanvas_drawBitmapNine_center'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>optional</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>is</a> <a href='#SkCanvas_drawBitmapNine_paint'>supplied</a>, <a href='#SkCanvas_drawBitmapNine_paint'>apply</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Blend_Mode'>and</a> <a href='#Draw_Looper'>Draw_Looper</a>. <a href='#Draw_Looper'>If</a> bitmap is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#Shader'>Shader</a>.
<a href='undocumented#Shader'>If</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>contains</a> <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Mask_Filter'>generate</a> <a href='#Mask_Filter'>mask</a> <a href='#Mask_Filter'>from</a> bitmap bounds. If <a href='#SkCanvas_drawBitmapNine_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>set</a> <a href='#Filter_Quality'>to</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kNone_SkFilterQuality'>disable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>filtering</a>. <a href='undocumented#Pixel'>For</a> <a href='undocumented#Pixel'>all</a>
<a href='undocumented#Pixel'>other</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>of</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, <a href='#Filter_Quality'>use</a> <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>to</a> <a href='undocumented#kLow_SkFilterQuality'>filter</a> <a href='undocumented#kLow_SkFilterQuality'>pixels</a>.
<a href='undocumented#kLow_SkFilterQuality'>Any</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>on</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#SkCanvas_drawBitmapNine_paint'>is</a> <a href='#SkCanvas_drawBitmapNine_paint'>ignored</a> <a href='#SkCanvas_drawBitmapNine_paint'>as</a> <a href='#SkCanvas_drawBitmapNine_paint'>is</a> <a href='#SkCanvas_drawBitmapNine_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> <a href='#Paint_Anti_Alias'>state</a>.
If generated mask extends beyond <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bounds</a>, <a href='#SkCanvas_drawBitmapNine_bitmap'>replicate</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>edge</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>colors</a>,
<a href='#SkCanvas_drawBitmapNine_bitmap'>just</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>as</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>made</a> <a href='undocumented#Shader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
<a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapNine_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>containing</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a>, <a href='SkBitmap_Reference#Bitmap'>dimensions</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_center'><code><strong>center</strong></code></a></td>
    <td><a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>edge</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>corners</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>sides</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapNine_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>containing</a> <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Image_Filter'>and</a> <a href='#Image_Filter'>so</a> <a href='#Image_Filter'>on</a>; <a href='#Image_Filter'>or</a> <a href='#Image_Filter'>nullptr</a>
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e99e7be0d8f67dfacbecf85df585433d"><div>The two leftmost <a href='#SkCanvas_drawBitmapNine_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>draws</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>has</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>four</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>sides</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>to</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>the</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>left</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>right</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>of</a> <a href='#SkCanvas_drawBitmapNine_center'>center</a>.
<a href='#SkCanvas_drawBitmapNine_center'>The</a> <a href='#SkCanvas_drawBitmapNine_center'>leftmost</a>  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>scales</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>the</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>width</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>of</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>proportionately</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>to</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>fit</a>.
<a href='#SkCanvas_drawBitmapNine_bitmap'>The</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>third</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>fourth</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>draw</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>are</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>not</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>scaled</a>; <a href='#SkCanvas_drawBitmapNine_bitmap'>the</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>sides</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_center'>center</a> <a href='#SkCanvas_drawBitmapNine_center'>are</a> <a href='#SkCanvas_drawBitmapNine_center'>scaled</a> <a href='#SkCanvas_drawBitmapNine_center'>to</a>
<a href='#SkCanvas_drawBitmapNine_center'>fill</a> <a href='#SkCanvas_drawBitmapNine_center'>the</a> <a href='#SkCanvas_drawBitmapNine_center'>remaining</a> <a href='#SkCanvas_drawBitmapNine_center'>space</a>.
<a href='#SkCanvas_drawBitmapNine_center'>The</a> <a href='#SkCanvas_drawBitmapNine_center'>rightmost</a>  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>has</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>four</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>scaled</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>vertically</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>to</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>fit</a>, <a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>uses</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>sides</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>above</a>
<a href='#SkCanvas_drawBitmapNine_bitmap'>and</a> <a href='#SkCanvas_drawBitmapNine_bitmap'>below</a> <a href='#SkCanvas_drawBitmapNine_center'>center</a> <a href='#SkCanvas_drawBitmapNine_center'>to</a> <a href='#SkCanvas_drawBitmapNine_center'>fill</a> <a href='#SkCanvas_drawBitmapNine_center'>the</a> <a href='#SkCanvas_drawBitmapNine_center'>remaining</a> <a href='#SkCanvas_drawBitmapNine_center'>space</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawBitmapRect'>drawBitmapRect</a>

<a name='Draw_Image_Lattice'></a>

<a name='SkCanvas_Lattice'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkCanvas_Lattice'>Lattice</a> {
        <a href='#SkCanvas_Lattice'>enum</a> <a href='#SkCanvas_Lattice_RectType'>RectType</a> : <a href='#SkCanvas_Lattice_RectType'>uint8_t</a> {
            <a href='#SkCanvas_Lattice_kDefault'>kDefault</a> = 0,
            <a href='#SkCanvas_Lattice_kTransparent'>kTransparent</a>,
            <a href='#SkCanvas_Lattice_kFixedColor'>kFixedColor</a>,
        };

        <a href='#SkCanvas_Lattice_kFixedColor'>const</a> <a href='#SkCanvas_Lattice_kFixedColor'>int</a>* <a href='#SkCanvas_Lattice_fXDivs'>fXDivs</a>;
        <a href='#SkCanvas_Lattice_fXDivs'>const</a> <a href='#SkCanvas_Lattice_fXDivs'>int</a>* <a href='#SkCanvas_Lattice_fYDivs'>fYDivs</a>;
        <a href='#SkCanvas_Lattice_fYDivs'>const</a> <a href='#SkCanvas_Lattice_RectType'>RectType</a>* <a href='#SkCanvas_Lattice_fRectTypes'>fRectTypes</a>;
        <a href='#SkCanvas_Lattice_fRectTypes'>int</a> <a href='#SkCanvas_Lattice_fXCount'>fXCount</a>;
        <a href='#SkCanvas_Lattice_fXCount'>int</a> <a href='#SkCanvas_Lattice_fYCount'>fYCount</a>;
        <a href='#SkCanvas_Lattice_fYCount'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='#SkCanvas_Lattice_fBounds'>fBounds</a>;
        <a href='#SkCanvas_Lattice_fBounds'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a>* <a href='#SkCanvas_Lattice_fColors'>fColors</a>;

    };
</pre>

<a href='#SkCanvas_Lattice'>Lattice</a> <a href='#SkCanvas_Lattice'>divides</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>or</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>into</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>rectangular</a> <a href='SkImage_Reference#Image'>grid</a>.
<a href='SkImage_Reference#Image'>Grid</a> <a href='SkImage_Reference#Image'>entries</a> <a href='SkImage_Reference#Image'>on</a> <a href='SkImage_Reference#Image'>even</a> <a href='SkImage_Reference#Image'>columns</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>even</a> <a href='SkImage_Reference#Image'>rows</a> <a href='SkImage_Reference#Image'>are</a> <a href='SkImage_Reference#Image'>fixed</a>; <a href='SkImage_Reference#Image'>these</a> <a href='SkImage_Reference#Image'>entries</a> <a href='SkImage_Reference#Image'>are</a>
<a href='SkImage_Reference#Image'>always</a> <a href='SkImage_Reference#Image'>drawn</a> <a href='SkImage_Reference#Image'>at</a> <a href='SkImage_Reference#Image'>their</a> <a href='SkImage_Reference#Image'>original</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>if</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>destination</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>large</a> <a href='undocumented#Size'>enough</a>.
<a href='undocumented#Size'>If</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>destination</a> <a href='undocumented#Size'>side</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>too</a> <a href='undocumented#Size'>small</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>hold</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>fixed</a> <a href='undocumented#Size'>entries</a>, <a href='undocumented#Size'>all</a> <a href='undocumented#Size'>fixed</a>
<a href='undocumented#Size'>entries</a> <a href='undocumented#Size'>are</a> <a href='undocumented#Size'>proportionately</a> <a href='undocumented#Size'>scaled</a> <a href='undocumented#Size'>down</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>fit</a>.
<a href='undocumented#Size'>The</a> <a href='undocumented#Size'>grid</a> <a href='undocumented#Size'>entries</a> <a href='undocumented#Size'>not</a> <a href='undocumented#Size'>on</a> <a href='undocumented#Size'>even</a> <a href='undocumented#Size'>columns</a> <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>rows</a> <a href='undocumented#Size'>are</a> <a href='undocumented#Size'>scaled</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>fit</a> <a href='undocumented#Size'>the</a>
<a href='undocumented#Size'>remaining</a> <a href='undocumented#Size'>space</a>, <a href='undocumented#Size'>if</a> <a href='undocumented#Size'>any</a>.

<a name='SkCanvas_Lattice_RectType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        enum <a href='#SkCanvas_Lattice_RectType'>RectType</a> : <a href='#SkCanvas_Lattice_RectType'>uint8_t</a> {
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
Array of x-axis values that divide the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>vertically</a>.
<a href='SkBitmap_Reference#Bitmap'>Array</a> <a href='SkBitmap_Reference#Bitmap'>entries</a> <a href='SkBitmap_Reference#Bitmap'>must</a> <a href='SkBitmap_Reference#Bitmap'>be</a> <a href='SkBitmap_Reference#Bitmap'>unique</a>, <a href='SkBitmap_Reference#Bitmap'>increasing</a>, <a href='SkBitmap_Reference#Bitmap'>greater</a> <a href='SkBitmap_Reference#Bitmap'>than</a> <a href='SkBitmap_Reference#Bitmap'>or</a> <a href='SkBitmap_Reference#Bitmap'>equal</a> <a href='SkBitmap_Reference#Bitmap'>to</a>
<a href='#SkCanvas_Lattice_fBounds'>fBounds</a> <a href='#SkCanvas_Lattice_fBounds'>left</a> <a href='#SkCanvas_Lattice_fBounds'>edge</a>, <a href='#SkCanvas_Lattice_fBounds'>and</a> <a href='#SkCanvas_Lattice_fBounds'>less</a> <a href='#SkCanvas_Lattice_fBounds'>than</a> <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> <a href='#SkCanvas_Lattice_fBounds'>right</a> <a href='#SkCanvas_Lattice_fBounds'>edge</a>.
<a href='#SkCanvas_Lattice_fBounds'>Set</a> <a href='#SkCanvas_Lattice_fBounds'>the</a> <a href='#SkCanvas_Lattice_fBounds'>first</a> <a href='#SkCanvas_Lattice_fBounds'>element</a> <a href='#SkCanvas_Lattice_fBounds'>to</a> <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> <a href='#SkCanvas_Lattice_fBounds'>left</a> <a href='#SkCanvas_Lattice_fBounds'>to</a> <a href='#SkCanvas_Lattice_fBounds'>collapse</a> <a href='#SkCanvas_Lattice_fBounds'>the</a> <a href='#SkCanvas_Lattice_fBounds'>left</a> <a href='#SkCanvas_Lattice_fBounds'>column</a> <a href='#SkCanvas_Lattice_fBounds'>of</a>
<a href='#SkCanvas_Lattice_fBounds'>fixed</a> <a href='#SkCanvas_Lattice_fBounds'>grid</a> <a href='#SkCanvas_Lattice_fBounds'>entries</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;int*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fYDivs'><code>fYDivs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Array of y-axis values that divide the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>horizontally</a>.
<a href='SkBitmap_Reference#Bitmap'>Array</a> <a href='SkBitmap_Reference#Bitmap'>entries</a> <a href='SkBitmap_Reference#Bitmap'>must</a> <a href='SkBitmap_Reference#Bitmap'>be</a> <a href='SkBitmap_Reference#Bitmap'>unique</a>, <a href='SkBitmap_Reference#Bitmap'>increasing</a>, <a href='SkBitmap_Reference#Bitmap'>greater</a> <a href='SkBitmap_Reference#Bitmap'>than</a> <a href='SkBitmap_Reference#Bitmap'>or</a> <a href='SkBitmap_Reference#Bitmap'>equal</a> <a href='SkBitmap_Reference#Bitmap'>to</a>
<a href='#SkCanvas_Lattice_fBounds'>fBounds</a> <a href='#SkCanvas_Lattice_fBounds'>top</a> <a href='#SkCanvas_Lattice_fBounds'>edge</a>, <a href='#SkCanvas_Lattice_fBounds'>and</a> <a href='#SkCanvas_Lattice_fBounds'>less</a> <a href='#SkCanvas_Lattice_fBounds'>than</a> <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> <a href='#SkCanvas_Lattice_fBounds'>bottom</a> <a href='#SkCanvas_Lattice_fBounds'>edge</a>.
<a href='#SkCanvas_Lattice_fBounds'>Set</a> <a href='#SkCanvas_Lattice_fBounds'>the</a> <a href='#SkCanvas_Lattice_fBounds'>first</a> <a href='#SkCanvas_Lattice_fBounds'>element</a> <a href='#SkCanvas_Lattice_fBounds'>to</a> <a href='#SkCanvas_Lattice_fBounds'>fBounds</a> <a href='#SkCanvas_Lattice_fBounds'>top</a> <a href='#SkCanvas_Lattice_fBounds'>to</a> <a href='#SkCanvas_Lattice_fBounds'>collapse</a> <a href='#SkCanvas_Lattice_fBounds'>the</a> <a href='#SkCanvas_Lattice_fBounds'>top</a> <a href='#SkCanvas_Lattice_fBounds'>row</a> <a href='#SkCanvas_Lattice_fBounds'>of</a> <a href='#SkCanvas_Lattice_fBounds'>fixed</a>
<a href='#SkCanvas_Lattice_fBounds'>grid</a> <a href='#SkCanvas_Lattice_fBounds'>entries</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;RectType*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fRectTypes'><code>fRectTypes</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional array of fill types, one per rectangular grid entry:
array length must be <code>(<a href='#SkCanvas_Lattice_fXCount'>fXCount</a> + 1) * (<a href='#SkCanvas_Lattice_fYCount'>fYCount</a> + 1)</code>.

Each <a href='#SkCanvas_Lattice_RectType'>RectType</a> <a href='#SkCanvas_Lattice_RectType'>is</a> <a href='#SkCanvas_Lattice_RectType'>one</a> <a href='#SkCanvas_Lattice_RectType'>of</a>: <a href='#SkCanvas_Lattice_kDefault'>kDefault</a>, <a href='#SkCanvas_Lattice_kTransparent'>kTransparent</a>, <a href='#SkCanvas_Lattice_kFixedColor'>kFixedColor</a>.

<a href='#SkCanvas_Lattice_kFixedColor'>Array</a> <a href='#SkCanvas_Lattice_kFixedColor'>entries</a> <a href='#SkCanvas_Lattice_kFixedColor'>correspond</a> <a href='#SkCanvas_Lattice_kFixedColor'>to</a> <a href='#SkCanvas_Lattice_kFixedColor'>the</a> <a href='#SkCanvas_Lattice_kFixedColor'>rectangular</a> <a href='#SkCanvas_Lattice_kFixedColor'>grid</a> <a href='#SkCanvas_Lattice_kFixedColor'>entries</a>, <a href='#SkCanvas_Lattice_kFixedColor'>ascending</a>
<a href='#SkCanvas_Lattice_kFixedColor'>left</a> <a href='#SkCanvas_Lattice_kFixedColor'>to</a> <a href='#SkCanvas_Lattice_kFixedColor'>right</a> <a href='#SkCanvas_Lattice_kFixedColor'>and</a> <a href='#SkCanvas_Lattice_kFixedColor'>then</a> <a href='#SkCanvas_Lattice_kFixedColor'>top</a> <a href='#SkCanvas_Lattice_kFixedColor'>to</a> <a href='#SkCanvas_Lattice_kFixedColor'>bottom</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fXCount'><code>fXCount</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Number of entries in <a href='#SkCanvas_Lattice_fXDivs'>fXDivs</a> <a href='#SkCanvas_Lattice_fXDivs'>array</a>; <a href='#SkCanvas_Lattice_fXDivs'>one</a> <a href='#SkCanvas_Lattice_fXDivs'>less</a> <a href='#SkCanvas_Lattice_fXDivs'>than</a> <a href='#SkCanvas_Lattice_fXDivs'>the</a> <a href='#SkCanvas_Lattice_fXDivs'>number</a> <a href='#SkCanvas_Lattice_fXDivs'>of</a>
<a href='#SkCanvas_Lattice_fXDivs'>horizontal</a> <a href='#SkCanvas_Lattice_fXDivs'>divisions</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fYCount'><code>fYCount</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Number of entries in <a href='#SkCanvas_Lattice_fYDivs'>fYDivs</a> <a href='#SkCanvas_Lattice_fYDivs'>array</a>; <a href='#SkCanvas_Lattice_fYDivs'>one</a> <a href='#SkCanvas_Lattice_fYDivs'>less</a> <a href='#SkCanvas_Lattice_fYDivs'>than</a> <a href='#SkCanvas_Lattice_fYDivs'>the</a> <a href='#SkCanvas_Lattice_fYDivs'>number</a> <a href='#SkCanvas_Lattice_fYDivs'>of</a> <a href='#SkCanvas_Lattice_fYDivs'>vertical</a>
<a href='#SkCanvas_Lattice_fYDivs'>divisions</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>const&nbsp;SkIRect*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_Lattice_fBounds'><code>fBounds</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Optional subset <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>source</a> <a href='SkIRect_Reference#IRect'>to</a> <a href='SkIRect_Reference#IRect'>draw</a> <a href='SkIRect_Reference#IRect'>from</a>.
<a href='SkIRect_Reference#IRect'>If</a> <a href='SkIRect_Reference#IRect'>nullptr</a>, <a href='SkIRect_Reference#IRect'>source</a> <a href='SkIRect_Reference#IRect'>bounds</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>dimensions</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>or</a> <a href='SkImage_Reference#Image'>Image</a>.
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
void <a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a>(<a href='#SkCanvas_drawBitmapLattice'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>const</a> <a href='#SkCanvas_Lattice'>Lattice</a>& <a href='#SkCanvas_Lattice'>lattice</a>, <a href='#SkCanvas_Lattice'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                       <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>stretched</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>proportionally</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fit</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>into</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawBitmapLattice_dst'>dst</a>.

<a href='#SkCanvas_Lattice'>Lattice</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>lattice</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>divides</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>into</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>a</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>rectangular</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>grid</a>.
<a href='#SkCanvas_drawBitmapLattice_bitmap'>Each</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>intersection</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>of</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>an</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>even-numbered</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>row</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>column</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>is</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fixed</a>; <a href='#SkCanvas_drawBitmapLattice_bitmap'>like</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>corners</a>
<a href='#SkCanvas_drawBitmapLattice_bitmap'>of</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>, <a href='#SkCanvas_drawBitmapNine'>fixed</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>lattice</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>elements</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>never</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>scale</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>larger</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>than</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>their</a> <a href='#SkCanvas_drawBitmapLattice_lattice'>initial</a>
<a href='undocumented#Size'>size</a> <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>shrink</a> <a href='undocumented#Size'>proportionately</a> <a href='undocumented#Size'>when</a> <a href='undocumented#Size'>all</a> <a href='undocumented#Size'>fixed</a> <a href='undocumented#Size'>elements</a> <a href='undocumented#Size'>exceed</a> <a href='undocumented#Size'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a>
<a href='#SkCanvas_drawBitmapLattice_bitmap'>dimension</a>. <a href='#SkCanvas_drawBitmapLattice_bitmap'>All</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>other</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>grid</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>elements</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>scale</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fill</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>available</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>space</a>, <a href='#SkCanvas_drawBitmapLattice_bitmap'>if</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>any</a>.

<a href='#SkCanvas_drawBitmapLattice_bitmap'>Additionally</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>transform</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>draw</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>using</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>optional</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>is</a> <a href='#SkCanvas_drawBitmapLattice_paint'>supplied</a>, <a href='#SkCanvas_drawBitmapLattice_paint'>apply</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Blend_Mode'>and</a> <a href='#Draw_Looper'>Draw_Looper</a>. <a href='#Draw_Looper'>If</a> bitmap is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#Shader'>Shader</a>.
<a href='undocumented#Shader'>If</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>contains</a> <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Mask_Filter'>generate</a> <a href='#Mask_Filter'>mask</a> <a href='#Mask_Filter'>from</a> bitmap bounds. If <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>set</a> <a href='#Filter_Quality'>to</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kNone_SkFilterQuality'>disable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>filtering</a>. <a href='undocumented#Pixel'>For</a> <a href='undocumented#Pixel'>all</a>
<a href='undocumented#Pixel'>other</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>of</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, <a href='#Filter_Quality'>use</a> <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>to</a> <a href='undocumented#kLow_SkFilterQuality'>filter</a> <a href='undocumented#kLow_SkFilterQuality'>pixels</a>.
<a href='undocumented#kLow_SkFilterQuality'>Any</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>on</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#SkCanvas_drawBitmapLattice_paint'>is</a> <a href='#SkCanvas_drawBitmapLattice_paint'>ignored</a> <a href='#SkCanvas_drawBitmapLattice_paint'>as</a> <a href='#SkCanvas_drawBitmapLattice_paint'>is</a> <a href='#SkCanvas_drawBitmapLattice_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> <a href='#Paint_Anti_Alias'>state</a>.
If generated mask extends beyond <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bounds</a>, <a href='#SkCanvas_drawBitmapLattice_bitmap'>replicate</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>edge</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>colors</a>,
<a href='#SkCanvas_drawBitmapLattice_bitmap'>just</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>as</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>made</a> <a href='undocumented#Shader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
<a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawBitmapLattice_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>containing</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a>, <a href='SkBitmap_Reference#Bitmap'>dimensions</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_lattice'><code><strong>lattice</strong></code></a></td>
    <td>division of <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>into</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fixed</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>variable</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>rectangles</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawBitmapLattice_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>containing</a> <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Image_Filter'>and</a> <a href='#Image_Filter'>so</a> <a href='#Image_Filter'>on</a>; <a href='#Image_Filter'>or</a> <a href='#Image_Filter'>nullptr</a>
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c5bfa944e17ba4a4400dc799f032069c"><div>The two leftmost <a href='#SkCanvas_drawBitmapLattice_bitmap'>bitmap</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>draws</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>has</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>four</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>sides</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>left</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>right</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>of</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>center</a>.
<a href='#SkCanvas_drawBitmapLattice_bitmap'>The</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>leftmost</a>  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>scales</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>width</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>of</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>proportionately</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fit</a>.
<a href='#SkCanvas_drawBitmapLattice_bitmap'>The</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>third</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fourth</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>draw</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>are</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>not</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>scaled</a>; <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>sides</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>are</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>scaled</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a>
<a href='#SkCanvas_drawBitmapLattice_bitmap'>fill</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>remaining</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>space</a>; <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>center</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>is</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>transparent</a>.
<a href='#SkCanvas_drawBitmapLattice_bitmap'>The</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>rightmost</a>  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>has</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>four</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>corners</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>scaled</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>vertically</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fit</a>, <a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>uses</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>sides</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>above</a>
<a href='#SkCanvas_drawBitmapLattice_bitmap'>and</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>below</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>center</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>to</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>fill</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>the</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>remaining</a> <a href='#SkCanvas_drawBitmapLattice_bitmap'>space</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawImageLattice'>drawImageLattice</a> <a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a> <a href='#SkCanvas_Lattice'>Lattice</a>

<a name='SkCanvas_drawImageLattice'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawImageLattice'>drawImageLattice</a>(<a href='#SkCanvas_drawImageLattice'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>const</a> <a href='#SkCanvas_Lattice'>Lattice</a>& <a href='#SkCanvas_Lattice'>lattice</a>, <a href='#SkCanvas_Lattice'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>,
                      <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = <a href='SkPaint_Reference#Paint'>nullptr</a>)
</pre>

Draws <a href='SkImage_Reference#Image'>Image</a> <a href='#SkCanvas_drawImageLattice_image'>image</a> <a href='#SkCanvas_drawImageLattice_image'>stretched</a> <a href='#SkCanvas_drawImageLattice_image'>proportionally</a> <a href='#SkCanvas_drawImageLattice_image'>to</a> <a href='#SkCanvas_drawImageLattice_image'>fit</a> <a href='#SkCanvas_drawImageLattice_image'>into</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkCanvas_drawImageLattice_dst'>dst</a>.

<a href='#SkCanvas_Lattice'>Lattice</a> <a href='#SkCanvas_drawImageLattice_lattice'>lattice</a> <a href='#SkCanvas_drawImageLattice_lattice'>divides</a> <a href='#SkCanvas_drawImageLattice_image'>image</a> <a href='#SkCanvas_drawImageLattice_image'>into</a> <a href='#SkCanvas_drawImageLattice_image'>a</a> <a href='#SkCanvas_drawImageLattice_image'>rectangular</a> <a href='#SkCanvas_drawImageLattice_image'>grid</a>.
<a href='#SkCanvas_drawImageLattice_image'>Each</a> <a href='#SkCanvas_drawImageLattice_image'>intersection</a> <a href='#SkCanvas_drawImageLattice_image'>of</a> <a href='#SkCanvas_drawImageLattice_image'>an</a> <a href='#SkCanvas_drawImageLattice_image'>even-numbered</a> <a href='#SkCanvas_drawImageLattice_image'>row</a> <a href='#SkCanvas_drawImageLattice_image'>and</a> <a href='#SkCanvas_drawImageLattice_image'>column</a> <a href='#SkCanvas_drawImageLattice_image'>is</a> <a href='#SkCanvas_drawImageLattice_image'>fixed</a>; <a href='#SkCanvas_drawImageLattice_image'>like</a> <a href='#SkCanvas_drawImageLattice_image'>the</a> <a href='#SkCanvas_drawImageLattice_image'>corners</a>
<a href='#SkCanvas_drawImageLattice_image'>of</a> <a href='#SkCanvas_drawBitmapNine'>drawBitmapNine</a>, <a href='#SkCanvas_drawBitmapNine'>fixed</a> <a href='#SkCanvas_drawImageLattice_lattice'>lattice</a> <a href='#SkCanvas_drawImageLattice_lattice'>elements</a> <a href='#SkCanvas_drawImageLattice_lattice'>never</a> <a href='#SkCanvas_drawImageLattice_lattice'>scale</a> <a href='#SkCanvas_drawImageLattice_lattice'>larger</a> <a href='#SkCanvas_drawImageLattice_lattice'>than</a> <a href='#SkCanvas_drawImageLattice_lattice'>their</a> <a href='#SkCanvas_drawImageLattice_lattice'>initial</a>
<a href='undocumented#Size'>size</a> <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>shrink</a> <a href='undocumented#Size'>proportionately</a> <a href='undocumented#Size'>when</a> <a href='undocumented#Size'>all</a> <a href='undocumented#Size'>fixed</a> <a href='undocumented#Size'>elements</a> <a href='undocumented#Size'>exceed</a> <a href='undocumented#Size'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>
<a href='SkBitmap_Reference#Bitmap'>dimension</a>. <a href='SkBitmap_Reference#Bitmap'>All</a> <a href='SkBitmap_Reference#Bitmap'>other</a> <a href='SkBitmap_Reference#Bitmap'>grid</a> <a href='SkBitmap_Reference#Bitmap'>elements</a> <a href='SkBitmap_Reference#Bitmap'>scale</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>fill</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>available</a> <a href='SkBitmap_Reference#Bitmap'>space</a>, <a href='SkBitmap_Reference#Bitmap'>if</a> <a href='SkBitmap_Reference#Bitmap'>any</a>.

<a href='SkBitmap_Reference#Bitmap'>Additionally</a> <a href='SkBitmap_Reference#Bitmap'>transform</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>using</a> <a href='SkBitmap_Reference#Bitmap'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>optional</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a>.
If <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#SkCanvas_drawImageLattice_paint'>is</a> <a href='#SkCanvas_drawImageLattice_paint'>supplied</a>, <a href='#SkCanvas_drawImageLattice_paint'>apply</a> <a href='#Color_Filter'>Color_Filter</a>, <a href='#Color_Alpha'>Color_Alpha</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Blend_Mode'>and</a> <a href='#Draw_Looper'>Draw_Looper</a>. <a href='#Draw_Looper'>If</a> image is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>apply</a> <a href='undocumented#Shader'>Shader</a>.
<a href='undocumented#Shader'>If</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#SkCanvas_drawImageLattice_paint'>contains</a> <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Mask_Filter'>generate</a> <a href='#Mask_Filter'>mask</a> <a href='#Mask_Filter'>from</a> image bounds. If <a href='#SkCanvas_drawImageLattice_paint'>paint</a>
<a href='#Filter_Quality'>Filter_Quality</a> <a href='#Filter_Quality'>set</a> <a href='#Filter_Quality'>to</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kNone_SkFilterQuality'>disable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>filtering</a>. <a href='undocumented#Pixel'>For</a> <a href='undocumented#Pixel'>all</a>
<a href='undocumented#Pixel'>other</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>of</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#Filter_Quality'>Filter_Quality</a>, <a href='#Filter_Quality'>use</a> <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>to</a> <a href='undocumented#kLow_SkFilterQuality'>filter</a> <a href='undocumented#kLow_SkFilterQuality'>pixels</a>.
<a href='undocumented#kLow_SkFilterQuality'>Any</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>on</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#SkCanvas_drawImageLattice_paint'>is</a> <a href='#SkCanvas_drawImageLattice_paint'>ignored</a> <a href='#SkCanvas_drawImageLattice_paint'>as</a> <a href='#SkCanvas_drawImageLattice_paint'>is</a> <a href='#SkCanvas_drawImageLattice_paint'>paint</a> <a href='#Paint_Anti_Alias'>Anti_Aliasing</a> <a href='#Paint_Anti_Alias'>state</a>.
If generated mask extends beyond <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>bounds</a>, <a href='SkBitmap_Reference#Bitmap'>replicate</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>edge</a> <a href='SkBitmap_Reference#Bitmap'>colors</a>,
<a href='SkBitmap_Reference#Bitmap'>just</a> <a href='SkBitmap_Reference#Bitmap'>as</a> <a href='undocumented#Shader'>Shader</a> <a href='undocumented#Shader'>made</a> <a href='undocumented#Shader'>from</a> <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_MakeBitmapShader'>MakeBitmapShader</a> <a href='#SkShader_MakeBitmapShader'>with</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>set</a> <a href='#SkShader_kClamp_TileMode'>replicates</a> <a href='#SkShader_kClamp_TileMode'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>edge</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>when</a> <a href='SkColor_Reference#Color'>it</a> <a href='SkColor_Reference#Color'>samples</a>
<a href='SkColor_Reference#Color'>outside</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>its</a> <a href='SkColor_Reference#Color'>bounds</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawImageLattice_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>containing</a> <a href='SkImage_Reference#Image'>pixels</a>, <a href='SkImage_Reference#Image'>dimensions</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>format</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_lattice'><code><strong>lattice</strong></code></a></td>
    <td>division of <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>into</a> <a href='SkBitmap_Reference#Bitmap'>fixed</a> <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>variable</a> <a href='SkBitmap_Reference#Bitmap'>rectangles</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='#SkCanvas_drawImageLattice_image'>image</a> <a href='#SkCanvas_drawImageLattice_image'>to</a> <a href='#SkCanvas_drawImageLattice_image'>draw</a> <a href='#SkCanvas_drawImageLattice_image'>to</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawImageLattice_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>containing</a> <a href='#Blend_Mode'>Blend_Mode</a>, <a href='#Color_Filter'>Color_Filter</a>, <a href='#Image_Filter'>Image_Filter</a>,
<a href='#Image_Filter'>and</a> <a href='#Image_Filter'>so</a> <a href='#Image_Filter'>on</a>; <a href='#Image_Filter'>or</a> <a href='#Image_Filter'>nullptr</a>
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4f153cf1d0dbe1a95acf5badeec14dae"><div>The leftmost <a href='#SkCanvas_drawImageLattice_image'>image</a> <a href='#SkCanvas_drawImageLattice_image'>is</a> <a href='#SkCanvas_drawImageLattice_image'>smaller</a> <a href='#SkCanvas_drawImageLattice_image'>than</a> <a href='#SkCanvas_drawImageLattice_image'>center</a>; <a href='#SkCanvas_drawImageLattice_image'>only</a> <a href='#SkCanvas_drawImageLattice_image'>corners</a> <a href='#SkCanvas_drawImageLattice_image'>are</a> <a href='#SkCanvas_drawImageLattice_image'>drawn</a>, <a href='#SkCanvas_drawImageLattice_image'>all</a> <a href='#SkCanvas_drawImageLattice_image'>scaled</a> <a href='#SkCanvas_drawImageLattice_image'>to</a> <a href='#SkCanvas_drawImageLattice_image'>fit</a>.
<a href='#SkCanvas_drawImageLattice_image'>The</a> <a href='#SkCanvas_drawImageLattice_image'>second</a> <a href='#SkCanvas_drawImageLattice_image'>image</a> <a href='#SkCanvas_drawImageLattice_image'>equals</a> <a href='#SkCanvas_drawImageLattice_image'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>center</a>; <a href='undocumented#Size'>only</a> <a href='undocumented#Size'>corners</a> <a href='undocumented#Size'>are</a> <a href='undocumented#Size'>drawn</a> <a href='undocumented#Size'>without</a> <a href='undocumented#Size'>scaling</a>.
<a href='undocumented#Size'>The</a> <a href='undocumented#Size'>remaining</a> <a href='undocumented#Size'>images</a> <a href='undocumented#Size'>are</a> <a href='undocumented#Size'>larger</a> <a href='undocumented#Size'>than</a> <a href='undocumented#Size'>center</a>. <a href='undocumented#Size'>All</a> <a href='undocumented#Size'>corners</a> <a href='undocumented#Size'>draw</a> <a href='undocumented#Size'>without</a> <a href='undocumented#Size'>scaling</a>. <a href='undocumented#Size'>The</a> <a href='undocumented#Size'>sides</a>
<a href='undocumented#Size'>are</a> <a href='undocumented#Size'>scaled</a> <a href='undocumented#Size'>if</a> <a href='undocumented#Size'>needed</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>take</a> <a href='undocumented#Size'>up</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>remaining</a> <a href='undocumented#Size'>space</a>; <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>center</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>transparent</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmapLattice'>drawBitmapLattice</a> <a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkCanvas_drawImageNine'>drawImageNine</a> <a href='#SkCanvas_Lattice'>Lattice</a>

<a name='SkCanvas_QuadAAFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkCanvas_QuadAAFlags'>QuadAAFlags</a> : <a href='#SkCanvas_QuadAAFlags'>unsigned</a> {
        <a href='#SkCanvas_kLeft_QuadAAFlag'>kLeft_QuadAAFlag</a> = 0<a href='#SkCanvas_kLeft_QuadAAFlag'>b0001</a>,
        <a href='#SkCanvas_kTop_QuadAAFlag'>kTop_QuadAAFlag</a> = 0<a href='#SkCanvas_kTop_QuadAAFlag'>b0010</a>,
        <a href='#SkCanvas_kRight_QuadAAFlag'>kRight_QuadAAFlag</a> = 0<a href='#SkCanvas_kRight_QuadAAFlag'>b0100</a>,
        <a href='#SkCanvas_kBottom_QuadAAFlag'>kBottom_QuadAAFlag</a> = 0<a href='#SkCanvas_kBottom_QuadAAFlag'>b1000</a>,
        <a href='#SkCanvas_kNone_QuadAAFlags'>kNone_QuadAAFlags</a> = 0<a href='#SkCanvas_kNone_QuadAAFlags'>b0000</a>,
        <a href='#SkCanvas_kAll_QuadAAFlags'>kAll_QuadAAFlags</a> = 0<a href='#SkCanvas_kAll_QuadAAFlags'>b1111</a>,
    };
</pre>

Private: Do not use.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kLeft_QuadAAFlag'><code>SkCanvas::kLeft_QuadAAFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialias the left edge</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kTop_QuadAAFlag'><code>SkCanvas::kTop_QuadAAFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialias the top edge</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kRight_QuadAAFlag'><code>SkCanvas::kRight_QuadAAFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialias the right edge</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kBottom_QuadAAFlag'><code>SkCanvas::kBottom_QuadAAFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialias the bottom edge</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kNone_QuadAAFlags'><code>SkCanvas::kNone_QuadAAFlags</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialias none of the edges</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_kAll_QuadAAFlags'><code>SkCanvas::kAll_QuadAAFlags</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>15</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialias all of the edges</td>
  </tr>
</table>

<a name='SkCanvas_ImageSetEntry'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkCanvas_ImageSetEntry'>ImageSetEntry</a> {
        <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#sk_sp'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkCanvas_ImageSetEntry_fImage'>fImage</a>;
        <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_ImageSetEntry_fSrcRect'>fSrcRect</a>;
        <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_ImageSetEntry_fDstRect'>fDstRect</a>;
        <a href='#SkCanvas_ImageSetEntry_fDstRect'>unsigned</a> <a href='#SkCanvas_ImageSetEntry_fAAFlags'>fAAFlags</a>;

    };
</pre>

Private: Do not use.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkImage*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_ImageSetEntry_fImage'><code>fImage</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
image to draw</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkRect</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_ImageSetEntry_fSrcRect'><code>fSrcRect</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
image src rectangle</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkRect</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_ImageSetEntry_fDstRect'><code>fDstRect</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
local space rectangle</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>unsigned</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkCanvas_ImageSetEntry_fAAFlags'><code>fAAFlags</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
antialiasing flags</td>
  </tr>
</table>

<a name='SkCanvas_experimental_DrawImageSetV0'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_experimental_DrawImageSetV0'>experimental_DrawImageSetV0</a>(<a href='#SkCanvas_experimental_DrawImageSetV0'>const</a> <a href='#SkCanvas_ImageSetEntry'>ImageSetEntry</a> <a href='#SkCanvas_ImageSetEntry'>imageSet</a>[], <a href='#SkCanvas_ImageSetEntry'>int</a> <a href='#SkCanvas_ImageSetEntry'>cnt</a>, <a href='#SkCanvas_ImageSetEntry'>float</a> <a href='SkColor_Reference#Alpha'>alpha</a>,
                                 <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>quality</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>) ;
</pre>

Private: Do not use.

Draws a set of images. Do not use this method.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_experimental_DrawImageSetV0_imageSet'><code><strong>imageSet</strong></code></a></td>
    <td>images</td>
  </tr>
  <tr>    <td><a name='SkCanvas_experimental_DrawImageSetV0_cnt'><code><strong>cnt</strong></code></a></td>
    <td>number of images</td>
  </tr>
  <tr>    <td><a name='SkCanvas_experimental_DrawImageSetV0_alpha'><code><strong>alpha</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>alpha</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_experimental_DrawImageSetV0_quality'><code><strong>quality</strong></code></a></td>
    <td>filter quality</td>
  </tr>
  <tr>    <td><a name='SkCanvas_experimental_DrawImageSetV0_mode'><code><strong>mode</strong></code></a></td>
    <td>blend mode</td>
  </tr>
</table>

<a name='Draw_Text'></a>

<a name='SkCanvas_drawText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawText'>drawText</a>(<a href='#SkCanvas_drawText'>const</a> <a href='#SkCanvas_drawText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawText_text'>text</a>, <a href='#SkCanvas_drawText_text'>with</a> <a href='#SkCanvas_drawText_text'>origin</a> <a href='#SkCanvas_drawText_text'>at</a> (<a href='#SkCanvas_drawText_x'>x</a>, <a href='#SkCanvas_drawText_y'>y</a>), <a href='#SkCanvas_drawText_y'>using</a> <a href='#SkCanvas_drawText_y'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawText_paint'>paint</a>.

<a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>meaning</a> <a href='#SkCanvas_drawText_text'>depends</a> <a href='#SkCanvas_drawText_text'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>; <a href='#SkPaint_TextEncoding'>by</a> <a href='#SkPaint_TextEncoding'>default</a>, <a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>is</a> <a href='#SkCanvas_drawText_text'>encoded</a> <a href='#SkCanvas_drawText_text'>as</a>
UTF-8.

<a href='#SkCanvas_drawText_x'>x</a> <a href='#SkCanvas_drawText_x'>and</a> <a href='#SkCanvas_drawText_y'>y</a> <a href='#SkCanvas_drawText_y'>meaning</a> <a href='#SkCanvas_drawText_y'>depends</a> <a href='#SkCanvas_drawText_y'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='SkPaint_Reference#SkPaint'>Align</a> <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>vertical</a> <a href='#SkCanvas_drawText_text'>text</a>; <a href='#SkCanvas_drawText_text'>by</a> <a href='#SkCanvas_drawText_text'>default</a>
<a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>draws</a> <a href='#SkCanvas_drawText_text'>left</a> <a href='#SkCanvas_drawText_text'>to</a> <a href='#SkCanvas_drawText_text'>right</a>, <a href='#SkCanvas_drawText_text'>positioning</a> <a href='#SkCanvas_drawText_text'>the</a> <a href='#SkCanvas_drawText_text'>first</a> <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> <a href='undocumented#Glyph'>at</a> <a href='#SkCanvas_drawText_x'>x</a>
and its baseline at <a href='#SkCanvas_drawText_y'>y</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>affected</a> <a href='undocumented#Size'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawText_text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawText_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; <a href='undocumented#SkDrawLooper'>apply</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='#SkCanvas_drawText_text'>text</a>. <a href='#SkCanvas_drawText_text'>By</a> <a href='#SkCanvas_drawText_text'>default</a>, <a href='#SkCanvas_drawText_text'>draws</a>
filled 12 <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>black</a> <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawText_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>array</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>on</a> <a href='#SkCanvas_drawText_text'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>on</a> <a href='#SkCanvas_drawText_text'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawText_text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="55f5e59350622c5e2834d1c85789f732"><div>The same <a href='#SkCanvas_drawText_text'>text</a> <a href='#SkCanvas_drawText_text'>is</a> <a href='#SkCanvas_drawText_text'>drawn</a> <a href='#SkCanvas_drawText_text'>varying</a> <a href='#Paint_Text_Size'>Paint_Text_Size</a> <a href='#Paint_Text_Size'>and</a> <a href='#Paint_Text_Size'>varying</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawString'>drawString</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawString'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawString'>drawString</a>(<a href='#SkCanvas_drawString'>const</a> <a href='#SkCanvas_drawString'>char</a>* <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws null terminated <a href='#SkCanvas_drawString_string'>string</a>, <a href='#SkCanvas_drawString_string'>with</a> <a href='#SkCanvas_drawString_string'>origin</a> <a href='#SkCanvas_drawString_string'>at</a> (<a href='#SkCanvas_drawString_x'>x</a>, <a href='#SkCanvas_drawString_y'>y</a>), <a href='#SkCanvas_drawString_y'>using</a> <a href='#SkCanvas_drawString_y'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawString_paint'>paint</a>.

<a href='#SkCanvas_drawString_string'>string</a> <a href='#SkCanvas_drawString_string'>meaning</a> <a href='#SkCanvas_drawString_string'>depends</a> <a href='#SkCanvas_drawString_string'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>; <a href='#SkPaint_TextEncoding'>by</a> <a href='#SkPaint_TextEncoding'>default</a>, <a href='#SkPaint_TextEncoding'>strings</a> <a href='#SkPaint_TextEncoding'>are</a> <a href='#SkPaint_TextEncoding'>encoded</a>
as UTF-8. Other values of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>are</a> <a href='#SkPaint_TextEncoding'>unlikely</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>produce</a> <a href='#SkPaint_TextEncoding'>the</a> <a href='#SkPaint_TextEncoding'>desired</a>
results, since zero bytes may be embedded in the <a href='#SkCanvas_drawString_string'>string</a>.

<a href='#SkCanvas_drawString_x'>x</a> <a href='#SkCanvas_drawString_x'>and</a> <a href='#SkCanvas_drawString_y'>y</a> <a href='#SkCanvas_drawString_y'>meaning</a> <a href='#SkCanvas_drawString_y'>depends</a> <a href='#SkCanvas_drawString_y'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='SkPaint_Reference#SkPaint'>Align</a> <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>vertical</a> <a href='undocumented#Text'>text</a>; <a href='undocumented#Text'>by</a> <a href='undocumented#Text'>default</a>
<a href='#SkCanvas_drawString_string'>string</a> <a href='#SkCanvas_drawString_string'>draws</a> <a href='#SkCanvas_drawString_string'>left</a> <a href='#SkCanvas_drawString_string'>to</a> <a href='#SkCanvas_drawString_string'>right</a>, <a href='#SkCanvas_drawString_string'>positioning</a> <a href='#SkCanvas_drawString_string'>the</a> <a href='#SkCanvas_drawString_string'>first</a> <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> <a href='undocumented#Glyph'>at</a> <a href='#SkCanvas_drawString_x'>x</a>
and its baseline at <a href='#SkCanvas_drawString_y'>y</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>affected</a> <a href='undocumented#Size'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawString_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; <a href='undocumented#SkDrawLooper'>apply</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='undocumented#Text'>text</a>. <a href='undocumented#Text'>By</a> <a href='undocumented#Text'>default</a>, <a href='undocumented#Text'>draws</a>
filled 12 <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>black</a> <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>drawn</a>,</td>
  </tr>
</table>

ending with a char value of zero

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_string'>string</a> <a href='#SkCanvas_drawString_string'>on</a> <a href='#SkCanvas_drawString_string'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_string'>string</a> <a href='#SkCanvas_drawString_string'>on</a> <a href='#SkCanvas_drawString_string'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="85442cf8d0bce6b5a777853bc36a4dc4"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawString_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawString'>drawString</a>(<a href='#SkCanvas_drawString'>const</a> <a href='undocumented#SkString'>SkString</a>& <a href='undocumented#String'>string</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws null terminated <a href='#SkCanvas_drawString_2_string'>string</a>, <a href='#SkCanvas_drawString_2_string'>with</a> <a href='#SkCanvas_drawString_2_string'>origin</a> <a href='#SkCanvas_drawString_2_string'>at</a> (<a href='#SkCanvas_drawString_2_x'>x</a>, <a href='#SkCanvas_drawString_2_y'>y</a>), <a href='#SkCanvas_drawString_2_y'>using</a> <a href='#SkCanvas_drawString_2_y'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawString_2_paint'>paint</a>.

<a href='#SkCanvas_drawString_2_string'>string</a> <a href='#SkCanvas_drawString_2_string'>meaning</a> <a href='#SkCanvas_drawString_2_string'>depends</a> <a href='#SkCanvas_drawString_2_string'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>; <a href='#SkPaint_TextEncoding'>by</a> <a href='#SkPaint_TextEncoding'>default</a>, <a href='#SkPaint_TextEncoding'>strings</a> <a href='#SkPaint_TextEncoding'>are</a> <a href='#SkPaint_TextEncoding'>encoded</a>
as UTF-8. Other values of <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>are</a> <a href='#SkPaint_TextEncoding'>unlikely</a> <a href='#SkPaint_TextEncoding'>to</a> <a href='#SkPaint_TextEncoding'>produce</a> <a href='#SkPaint_TextEncoding'>the</a> <a href='#SkPaint_TextEncoding'>desired</a>
results, since zero bytes may be embedded in the <a href='#SkCanvas_drawString_2_string'>string</a>.

<a href='#SkCanvas_drawString_2_x'>x</a> <a href='#SkCanvas_drawString_2_x'>and</a> <a href='#SkCanvas_drawString_2_y'>y</a> <a href='#SkCanvas_drawString_2_y'>meaning</a> <a href='#SkCanvas_drawString_2_y'>depends</a> <a href='#SkCanvas_drawString_2_y'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='SkPaint_Reference#SkPaint'>Align</a> <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>vertical</a> <a href='undocumented#Text'>text</a>; <a href='undocumented#Text'>by</a> <a href='undocumented#Text'>default</a>
<a href='#SkCanvas_drawString_2_string'>string</a> <a href='#SkCanvas_drawString_2_string'>draws</a> <a href='#SkCanvas_drawString_2_string'>left</a> <a href='#SkCanvas_drawString_2_string'>to</a> <a href='#SkCanvas_drawString_2_string'>right</a>, <a href='#SkCanvas_drawString_2_string'>positioning</a> <a href='#SkCanvas_drawString_2_string'>the</a> <a href='#SkCanvas_drawString_2_string'>first</a> <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> <a href='undocumented#Glyph'>at</a> <a href='#SkCanvas_drawString_2_x'>x</a>
and its baseline at <a href='#SkCanvas_drawString_2_y'>y</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>affected</a> <a href='undocumented#Size'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawString_2_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; <a href='undocumented#SkDrawLooper'>apply</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='undocumented#Text'>text</a>. <a href='undocumented#Text'>By</a> <a href='undocumented#Text'>default</a>, <a href='undocumented#Text'>draws</a>
filled 12 <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>black</a> <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_2_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>drawn</a>,</td>
  </tr>
</table>

ending with a char value of zero

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawString_2_x'><code><strong>x</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_2_string'>string</a> <a href='#SkCanvas_drawString_2_string'>on</a> <a href='#SkCanvas_drawString_2_string'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_y'><code><strong>y</strong></code></a></td>
    <td>start of <a href='#SkCanvas_drawString_2_string'>string</a> <a href='#SkCanvas_drawString_2_string'>on</a> <a href='#SkCanvas_drawString_2_string'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawString_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="435178c09feb3bfec5e35d983609a013"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawPosText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPosText'>drawPosText</a>(<a href='#SkCanvas_drawPosText'>const</a> <a href='#SkCanvas_drawPosText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pos</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws each <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>in</a> <a href='#SkCanvas_drawPosText_text'>text</a> <a href='#SkCanvas_drawPosText_text'>with</a> <a href='#SkCanvas_drawPosText_text'>the</a> <a href='#SkCanvas_drawPosText_text'>origin</a> <a href='#SkCanvas_drawPosText_text'>in</a> <a href='#SkCanvas_drawPosText_pos'>pos</a> <a href='#SkCanvas_drawPosText_pos'>array</a>, <a href='#SkCanvas_drawPosText_pos'>using</a> <a href='#SkCanvas_drawPosText_pos'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosText_paint'>paint</a>. <a href='#SkCanvas_drawPosText_paint'>The</a> <a href='#SkCanvas_drawPosText_paint'>number</a> <a href='#SkCanvas_drawPosText_paint'>of</a> <a href='#SkCanvas_drawPosText_paint'>entries</a> <a href='#SkCanvas_drawPosText_paint'>in</a> <a href='#SkCanvas_drawPosText_pos'>pos</a> <a href='#SkCanvas_drawPosText_pos'>array</a> <a href='#SkCanvas_drawPosText_pos'>must</a> <a href='#SkCanvas_drawPosText_pos'>match</a> <a href='#SkCanvas_drawPosText_pos'>the</a> <a href='#SkCanvas_drawPosText_pos'>number</a> <a href='#SkCanvas_drawPosText_pos'>of</a> <a href='undocumented#Glyph'>glyphs</a>
described by <a href='#SkCanvas_drawPosText_byteLength'>byteLength</a> <a href='#SkCanvas_drawPosText_byteLength'>of</a> <a href='#SkCanvas_drawPosText_text'>text</a>.

<a href='#SkCanvas_drawPosText_text'>text</a> <a href='#SkCanvas_drawPosText_text'>meaning</a> <a href='#SkCanvas_drawPosText_text'>depends</a> <a href='#SkCanvas_drawPosText_text'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>; <a href='#SkPaint_TextEncoding'>by</a> <a href='#SkPaint_TextEncoding'>default</a>, <a href='#SkCanvas_drawPosText_text'>text</a> <a href='#SkCanvas_drawPosText_text'>is</a> <a href='#SkCanvas_drawPosText_text'>encoded</a> <a href='#SkCanvas_drawPosText_text'>as</a>
UTF-8. <a href='#SkCanvas_drawPosText_pos'>pos</a> <a href='#SkCanvas_drawPosText_pos'>elements</a> <a href='#SkCanvas_drawPosText_pos'>meaning</a> <a href='#SkCanvas_drawPosText_pos'>depends</a> <a href='#SkCanvas_drawPosText_pos'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>vertical</a> <a href='#SkCanvas_drawPosText_text'>text</a>; <a href='#SkCanvas_drawPosText_text'>by</a> <a href='#SkCanvas_drawPosText_text'>default</a>
<a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>baseline</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>relative</a> <a href='undocumented#Glyph'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkCanvas_drawPosText_pos'>pos</a> <a href='#SkCanvas_drawPosText_pos'>array</a>.
<a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>affected</a> <a href='undocumented#Size'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosText_text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawPosText_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; <a href='undocumented#SkDrawLooper'>apply</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='#SkCanvas_drawPosText_text'>text</a>. <a href='#SkCanvas_drawPosText_text'>By</a> <a href='#SkCanvas_drawPosText_text'>default</a>, <a href='#SkCanvas_drawPosText_text'>draws</a>
filled 12 <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>black</a> <a href='undocumented#Glyph'>glyphs</a>.

Layout engines such as Harfbuzz typically position each <a href='undocumented#Glyph'>glyph</a>
rather than using the  <a href='undocumented#Font_Advance'>font advance</a> <a href='undocumented#Font'>widths</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPosText_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawPosText_text'>text</a> <a href='#SkCanvas_drawPosText_text'>array</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_pos'><code><strong>pos</strong></code></a></td>
    <td>array of <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>origins</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawPosText_text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bf0b2402533a23b6392e0676b7a8414c"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawPosTextH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>(<a href='#SkCanvas_drawPosTextH'>const</a> <a href='#SkCanvas_drawPosTextH'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xpos</a>[], <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>constY</a>,
                  <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws each <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>in</a> <a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='#SkCanvas_drawPosTextH_text'>with</a> <a href='#SkCanvas_drawPosTextH_text'>its</a> <a href='#SkCanvas_drawPosTextH_text'>origin</a> <a href='#SkCanvas_drawPosTextH_text'>composed</a> <a href='#SkCanvas_drawPosTextH_text'>from</a> <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> <a href='#SkCanvas_drawPosTextH_xpos'>array</a> <a href='#SkCanvas_drawPosTextH_xpos'>and</a>
<a href='#SkCanvas_drawPosTextH_constY'>constY</a>, <a href='#SkCanvas_drawPosTextH_constY'>using</a> <a href='#SkCanvas_drawPosTextH_constY'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosTextH_paint'>paint</a>. <a href='#SkCanvas_drawPosTextH_paint'>The</a> <a href='#SkCanvas_drawPosTextH_paint'>number</a> <a href='#SkCanvas_drawPosTextH_paint'>of</a> <a href='#SkCanvas_drawPosTextH_paint'>entries</a> <a href='#SkCanvas_drawPosTextH_paint'>in</a> <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> <a href='#SkCanvas_drawPosTextH_xpos'>array</a>
must match the number of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>described</a> <a href='undocumented#Glyph'>by</a> <a href='#SkCanvas_drawPosTextH_byteLength'>byteLength</a> <a href='#SkCanvas_drawPosTextH_byteLength'>of</a> <a href='#SkCanvas_drawPosTextH_text'>text</a>.

<a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='#SkCanvas_drawPosTextH_text'>meaning</a> <a href='#SkCanvas_drawPosTextH_text'>depends</a> <a href='#SkCanvas_drawPosTextH_text'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a>; <a href='#SkPaint_TextEncoding'>by</a> <a href='#SkPaint_TextEncoding'>default</a>, <a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='#SkCanvas_drawPosTextH_text'>is</a> <a href='#SkCanvas_drawPosTextH_text'>encoded</a> <a href='#SkCanvas_drawPosTextH_text'>as</a>
UTF-8. <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> <a href='#SkCanvas_drawPosTextH_xpos'>elements</a> <a href='#SkCanvas_drawPosTextH_xpos'>meaning</a> <a href='#SkCanvas_drawPosTextH_xpos'>depends</a> <a href='#SkCanvas_drawPosTextH_xpos'>on</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>vertical</a> <a href='#SkCanvas_drawPosTextH_text'>text</a>;
by default each <a href='undocumented#Glyph'>glyph</a>   <a href='undocumented#Left_Side_Bearing'>left side bearing</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>positioned</a> <a href='undocumented#Glyph'>at</a> <a href='undocumented#Glyph'>an</a> <a href='#SkCanvas_drawPosTextH_xpos'>xpos</a> <a href='#SkCanvas_drawPosTextH_xpos'>element</a> <a href='#SkCanvas_drawPosTextH_xpos'>and</a>
its baseline is positioned at <a href='#SkCanvas_drawPosTextH_constY'>constY</a>. <a href='undocumented#Text'>Text</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>affected</a> <a href='undocumented#Size'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='undocumented#Size'>size</a>.

All elements of <a href='#SkCanvas_drawPosTextH_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; <a href='undocumented#SkDrawLooper'>apply</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='#SkCanvas_drawPosTextH_text'>text</a>. <a href='#SkCanvas_drawPosTextH_text'>By</a> <a href='#SkCanvas_drawPosTextH_text'>default</a>, <a href='#SkCanvas_drawPosTextH_text'>draws</a>
filled 12 <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>black</a> <a href='undocumented#Glyph'>glyphs</a>.

Layout engines such as Harfbuzz typically position each <a href='undocumented#Glyph'>glyph</a>
rather than using the  <a href='undocumented#Font_Advance'>font advance</a> <a href='undocumented#Font'>widths</a> <a href='undocumented#Font'>if</a> <a href='undocumented#Font'>all</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>share</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>same</a>
baseline.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPosTextH_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='#SkCanvas_drawPosTextH_text'>array</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_xpos'><code><strong>xpos</strong></code></a></td>
    <td>array of x-axis positions, used to position each <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_constY'><code><strong>constY</strong></code></a></td>
    <td>shared y-axis value for all of x-axis positions</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPosTextH_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawPosTextH_text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="95c6a7ef82993a8d2add676080e9438a"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a> <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>

<a name='SkCanvas_drawTextRSXform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextRSXform'>drawTextRSXform</a>(<a href='#SkCanvas_drawTextRSXform'>const</a> <a href='#SkCanvas_drawTextRSXform'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[],
                     <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#SkCanvas_drawTextRSXform_text'>text</a>, <a href='#SkCanvas_drawTextRSXform_text'>transforming</a> <a href='#SkCanvas_drawTextRSXform_text'>each</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>corresponding</a> <a href='undocumented#SkRSXform'>SkRSXform</a>,
using clip, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawTextRSXform_paint'>paint</a>.

<a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawTextRSXform_xform'>xform</a> <a href='#SkCanvas_drawTextRSXform_xform'>array</a> <a href='#SkCanvas_drawTextRSXform_xform'>specifies</a> <a href='#SkCanvas_drawTextRSXform_xform'>a</a> <a href='#SkCanvas_drawTextRSXform_xform'>separate</a> <a href='#SkCanvas_drawTextRSXform_xform'>square</a> <a href='#SkCanvas_drawTextRSXform_xform'>scale</a>, <a href='#SkCanvas_drawTextRSXform_xform'>rotation</a>, <a href='#SkCanvas_drawTextRSXform_xform'>and</a> <a href='#SkCanvas_drawTextRSXform_xform'>translation</a>
for each <a href='undocumented#Glyph'>glyph</a>. <a href='#SkCanvas_drawTextRSXform_xform'>xform</a> <a href='#SkCanvas_drawTextRSXform_xform'>does</a> <a href='#SkCanvas_drawTextRSXform_xform'>not</a> <a href='#SkCanvas_drawTextRSXform_xform'>affect</a> <a href='#SkCanvas_drawTextRSXform_paint'>paint</a> <a href='undocumented#SkShader'>SkShader</a>.

Optional <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>cullRect</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>is</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>a</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>conservative</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>bounds</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>of</a> <a href='#SkCanvas_drawTextRSXform_text'>text</a>, <a href='#SkCanvas_drawTextRSXform_text'>taking</a> <a href='#SkCanvas_drawTextRSXform_text'>into</a> <a href='#SkCanvas_drawTextRSXform_text'>account</a>
<a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>and</a> <a href='#SkCanvas_drawTextRSXform_paint'>paint</a>. <a href='#SkCanvas_drawTextRSXform_paint'>If</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>cullRect</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>is</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>outside</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>of</a> <a href='#SkCanvas_drawTextRSXform_cullRect'>clip</a>, <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>skip</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>.

All elements of <a href='#SkCanvas_drawTextRSXform_paint'>paint</a>: <a href='undocumented#SkPathEffect'>SkPathEffect</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkShader'>SkShader</a>,
<a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>; <a href='undocumented#SkDrawLooper'>apply</a> <a href='undocumented#SkDrawLooper'>to</a> <a href='#SkCanvas_drawTextRSXform_text'>text</a>. <a href='#SkCanvas_drawTextRSXform_text'>By</a> <a href='#SkCanvas_drawTextRSXform_text'>default</a>, <a href='#SkCanvas_drawTextRSXform_text'>draws</a>
filled 12 <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>black</a> <a href='undocumented#Glyph'>glyphs</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextRSXform_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkCanvas_drawTextRSXform_text'>text</a> <a href='#SkCanvas_drawTextRSXform_text'>array</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>rotates</a>, <a href='undocumented#SkRSXform'>scales</a>, <a href='undocumented#SkRSXform'>and</a> <a href='undocumented#SkRSXform'>translates</a> <a href='undocumented#SkRSXform'>each</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>individually</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='#SkCanvas_drawTextRSXform_text'>text</a> <a href='#SkCanvas_drawTextRSXform_text'>for</a> <a href='#SkCanvas_drawTextRSXform_text'>efficient</a> <a href='#SkCanvas_drawTextRSXform_text'>clipping</a>; <a href='#SkCanvas_drawTextRSXform_text'>or</a> <a href='#SkCanvas_drawTextRSXform_text'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextRSXform_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkCanvas_drawTextRSXform_text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>blend</a>, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="935c8f8b9782d297a73d7186f6ef7945"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>

<a name='SkCanvas_drawTextBlob'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(<a href='#SkCanvas_drawTextBlob'>const</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>* <a href='SkTextBlob_Reference#SkTextBlob'>blob</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#Text_Blob'>Text_Blob</a> <a href='#SkCanvas_drawTextBlob_blob'>blob</a> <a href='#SkCanvas_drawTextBlob_blob'>at</a> (<a href='#SkCanvas_drawTextBlob_x'>x</a>, <a href='#SkCanvas_drawTextBlob_y'>y</a>), <a href='#SkCanvas_drawTextBlob_y'>using</a> <a href='#SkCanvas_drawTextBlob_y'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawTextBlob_paint'>paint</a>.

<a href='#SkCanvas_drawTextBlob_blob'>blob</a> <a href='#SkCanvas_drawTextBlob_blob'>contains</a> <a href='undocumented#Glyph'>Glyphs</a>, <a href='undocumented#Glyph'>their</a> <a href='undocumented#Glyph'>positions</a>, <a href='undocumented#Glyph'>and</a> <a href='#SkCanvas_drawTextBlob_paint'>paint</a> <a href='#SkCanvas_drawTextBlob_paint'>attributes</a> <a href='#SkCanvas_drawTextBlob_paint'>specific</a> <a href='#SkCanvas_drawTextBlob_paint'>to</a> <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
<a href='#Paint_Linear_Text'>and</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a> <a href='#Paint_Text_Encoding'>must</a> <a href='#Paint_Text_Encoding'>be</a> <a href='#Paint_Text_Encoding'>set</a> <a href='#Paint_Text_Encoding'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>.

<a href='#SkPaint_kGlyphID_TextEncoding'>Elements</a> <a href='#SkPaint_kGlyphID_TextEncoding'>of</a> <a href='#SkCanvas_drawTextBlob_paint'>paint</a>: <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Blend_Mode'>Blend_Mode</a>, <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>including</a> <a href='#Color_Alpha'>Color_Alpha</a>,
<a href='#Color_Filter'>Color_Filter</a>, <a href='#Paint_Dither'>Paint_Dither</a>, <a href='#Draw_Looper'>Draw_Looper</a>, <a href='#Mask_Filter'>Mask_Filter</a>, <a href='#Path_Effect'>Path_Effect</a>, <a href='undocumented#Shader'>Shader</a>, <a href='undocumented#Shader'>and</a>
<a href='#Paint_Style'>Paint_Style</a>; <a href='#Paint_Style'>apply</a> <a href='#Paint_Style'>to</a> <a href='#SkCanvas_drawTextBlob_blob'>blob</a>. <a href='#SkCanvas_drawTextBlob_blob'>If</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>contains</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>:
<a href='#Paint_Miter_Limit'>Paint_Miter_Limit</a>, <a href='#Paint_Stroke_Cap'>Paint_Stroke_Cap</a>, <a href='#Paint_Stroke_Join'>Paint_Stroke_Join</a>, <a href='#Paint_Stroke_Join'>and</a> <a href='#Paint_Stroke_Width'>Paint_Stroke_Width</a>;
<a href='#Paint_Stroke_Width'>apply</a> <a href='#Paint_Stroke_Width'>to</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>created</a> <a href='SkPath_Reference#Path'>from</a> <a href='#SkCanvas_drawTextBlob_blob'>blob</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextBlob_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a>, <a href='undocumented#Glyph'>positions</a>, <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>their</a> <a href='undocumented#Glyph'>paints</a>' <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>, <a href='undocumented#Typeface'>and</a> <a href='undocumented#Typeface'>so</a> <a href='undocumented#Typeface'>on</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset applied to <a href='#SkCanvas_drawTextBlob_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset applied to <a href='#SkCanvas_drawTextBlob_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_paint'><code><strong>paint</strong></code></a></td>
    <td>blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>stroking</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5f823814ec9df1f912a2ea943bedfca1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>

<a name='SkCanvas_drawTextBlob_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>(<a href='#SkCanvas_drawTextBlob'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&<a href='SkTextBlob_Reference#SkTextBlob'>gt</a>;& <a href='SkTextBlob_Reference#SkTextBlob'>blob</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='#Text_Blob'>Text_Blob</a> <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a> <a href='#SkCanvas_drawTextBlob_2_blob'>at</a> (<a href='#SkCanvas_drawTextBlob_2_x'>x</a>, <a href='#SkCanvas_drawTextBlob_2_y'>y</a>), <a href='#SkCanvas_drawTextBlob_2_y'>using</a> <a href='#SkCanvas_drawTextBlob_2_y'>Clip</a>, <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a>.

<a href='#SkCanvas_drawTextBlob_2_blob'>blob</a> <a href='#SkCanvas_drawTextBlob_2_blob'>contains</a> <a href='undocumented#Glyph'>Glyphs</a>, <a href='undocumented#Glyph'>their</a> <a href='undocumented#Glyph'>positions</a>, <a href='undocumented#Glyph'>and</a> <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a> <a href='#SkCanvas_drawTextBlob_2_paint'>attributes</a> <a href='#SkCanvas_drawTextBlob_2_paint'>specific</a> <a href='#SkCanvas_drawTextBlob_2_paint'>to</a> <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
<a href='#Paint_Linear_Text'>and</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a> <a href='#Paint_Text_Encoding'>must</a> <a href='#Paint_Text_Encoding'>be</a> <a href='#Paint_Text_Encoding'>set</a> <a href='#Paint_Text_Encoding'>to</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kGlyphID_TextEncoding'>kGlyphID_TextEncoding</a>.

<a href='#SkPaint_kGlyphID_TextEncoding'>Elements</a> <a href='#SkPaint_kGlyphID_TextEncoding'>of</a> <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a>: <a href='#Path_Effect'>Path_Effect</a>, <a href='#Mask_Filter'>Mask_Filter</a>, <a href='undocumented#Shader'>Shader</a>, <a href='#Color_Filter'>Color_Filter</a>,
<a href='#Image_Filter'>Image_Filter</a>, <a href='#Image_Filter'>and</a> <a href='#Draw_Looper'>Draw_Looper</a>; <a href='#Draw_Looper'>apply</a> <a href='#Draw_Looper'>to</a> <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawTextBlob_2_blob'><code><strong>blob</strong></code></a></td>
    <td><a href='undocumented#Glyph'>Glyphs</a>, <a href='undocumented#Glyph'>positions</a>, <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>their</a> <a href='undocumented#Glyph'>paints</a>' <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>, <a href='undocumented#Typeface'>and</a> <a href='undocumented#Typeface'>so</a> <a href='undocumented#Typeface'>on</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset applied to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset applied to <a href='#SkCanvas_drawTextBlob_2_blob'>blob</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawTextBlob_2_paint'><code><strong>paint</strong></code></a></td>
    <td>blend, <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>stroking</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>, <a href='SkColor_Reference#Color'>used</a> <a href='SkColor_Reference#Color'>to</a> <a href='SkColor_Reference#Color'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="af4c69fbbd165c8b0eb0c9bd49ccbd8d"><div><a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>attributes</a> <a href='SkPaint_Reference#Paint'>unrelated</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>like</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>have</a> <a href='SkColor_Reference#Color'>no</a> <a href='SkColor_Reference#Color'>effect</a> <a href='SkColor_Reference#Color'>on</a> <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a> <a href='#SkCanvas_drawTextBlob_2_paint'>in</a> <a href='#SkCanvas_drawTextBlob_2_paint'>allocated</a> <a href='#Text_Blob'>Text_Blob</a>.
<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>attributes</a> <a href='SkPaint_Reference#Paint'>related</a> <a href='SkPaint_Reference#Paint'>to</a> <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>like</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>have</a> <a href='undocumented#Size'>no</a> <a href='undocumented#Size'>effect</a> <a href='undocumented#Size'>on</a> <a href='#SkCanvas_drawTextBlob_2_paint'>paint</a> <a href='#SkCanvas_drawTextBlob_2_paint'>passed</a> <a href='#SkCanvas_drawTextBlob_2_paint'>to</a> <a href='#SkCanvas_drawTextBlob'>drawTextBlob</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawText'>drawText</a> <a href='#SkCanvas_drawPosText'>drawPosText</a> <a href='#SkCanvas_drawPosTextH'>drawPosTextH</a>

<a name='SkCanvas_drawPicture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_picture'>picture</a>, <a href='#SkCanvas_drawPicture_picture'>using</a> <a href='#SkCanvas_drawPicture_picture'>clip</a> <a href='#SkCanvas_drawPicture_picture'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>are</a> <a href='SkMatrix_Reference#SkMatrix'>unchanged</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_drawPicture_picture'>picture</a> <a href='#SkCanvas_drawPicture_picture'>contents</a>, <a href='#SkCanvas_drawPicture_picture'>as</a> <a href='#SkCanvas_drawPicture_picture'>if</a>
<a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>was</a> <a href='#SkCanvas_save'>called</a> <a href='#SkCanvas_save'>before</a> <a href='#SkCanvas_save'>and</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>was</a> <a href='#SkCanvas_restore'>called</a> <a href='#SkCanvas_restore'>after</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>().

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>records</a> <a href='SkPicture_Reference#SkPicture'>a</a> <a href='SkPicture_Reference#SkPicture'>series</a> <a href='SkPicture_Reference#SkPicture'>of</a> <a href='SkPicture_Reference#SkPicture'>draw</a> <a href='SkPicture_Reference#SkPicture'>commands</a> <a href='SkPicture_Reference#SkPicture'>for</a> <a href='SkPicture_Reference#SkPicture'>later</a> <a href='SkPicture_Reference#SkPicture'>playback</a>.

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
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>;& <a href='SkPicture_Reference#Picture'>picture</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_2_picture'>picture</a>, <a href='#SkCanvas_drawPicture_2_picture'>using</a> <a href='#SkCanvas_drawPicture_2_picture'>clip</a> <a href='#SkCanvas_drawPicture_2_picture'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>are</a> <a href='SkMatrix_Reference#SkMatrix'>unchanged</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkCanvas_drawPicture_2_picture'>picture</a> <a href='#SkCanvas_drawPicture_2_picture'>contents</a>, <a href='#SkCanvas_drawPicture_2_picture'>as</a> <a href='#SkCanvas_drawPicture_2_picture'>if</a>
<a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>was</a> <a href='#SkCanvas_save'>called</a> <a href='#SkCanvas_save'>before</a> <a href='#SkCanvas_save'>and</a> <a href='#SkCanvas_restore'>restore()</a> <a href='#SkCanvas_restore'>was</a> <a href='#SkCanvas_restore'>called</a> <a href='#SkCanvas_restore'>after</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>().

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>records</a> <a href='SkPicture_Reference#SkPicture'>a</a> <a href='SkPicture_Reference#SkPicture'>series</a> <a href='SkPicture_Reference#SkPicture'>of</a> <a href='SkPicture_Reference#SkPicture'>draw</a> <a href='SkPicture_Reference#SkPicture'>commands</a> <a href='SkPicture_Reference#SkPicture'>for</a> <a href='SkPicture_Reference#SkPicture'>later</a> <a href='SkPicture_Reference#SkPicture'>playback</a>.

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
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>* <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_3_picture'>picture</a>, <a href='#SkCanvas_drawPicture_3_picture'>using</a> <a href='#SkCanvas_drawPicture_3_picture'>clip</a> <a href='#SkCanvas_drawPicture_3_picture'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>; <a href='SkMatrix_Reference#SkMatrix'>transforming</a> <a href='#SkCanvas_drawPicture_3_picture'>picture</a> <a href='#SkCanvas_drawPicture_3_picture'>with</a>
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkCanvas_drawPicture_3_matrix'>matrix</a>, <a href='#SkCanvas_drawPicture_3_matrix'>if</a> <a href='#SkCanvas_drawPicture_3_matrix'>provided</a>; <a href='#SkCanvas_drawPicture_3_matrix'>and</a> <a href='#SkCanvas_drawPicture_3_matrix'>use</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>  <a href='#SkCanvas_drawPicture_3_paint'>paint alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>if</a> <a href='SkBlendMode_Reference#SkBlendMode'>provided</a>.

<a href='#SkCanvas_drawPicture_3_matrix'>matrix</a> <a href='#SkCanvas_drawPicture_3_matrix'>transformation</a> <a href='#SkCanvas_drawPicture_3_matrix'>is</a> <a href='#SkCanvas_drawPicture_3_matrix'>equivalent</a> <a href='#SkCanvas_drawPicture_3_matrix'>to</a>: <a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.
<a href='#SkCanvas_drawPicture_3_paint'>paint</a> <a href='#SkCanvas_drawPicture_3_paint'>use</a> <a href='#SkCanvas_drawPicture_3_paint'>is</a> <a href='#SkCanvas_drawPicture_3_paint'>equivalent</a> <a href='#SkCanvas_drawPicture_3_paint'>to</a>: <a href='#SkCanvas_saveLayer'>saveLayer</a>(), <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_3_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_3_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a>, <a href='SkMatrix_Reference#SkMatrix'>scale</a>, <a href='SkMatrix_Reference#SkMatrix'>translate</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>so</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>; <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>apply</a> <a href='SkPaint_Reference#SkPaint'>transparency</a>, <a href='SkPaint_Reference#SkPaint'>filtering</a>, <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>so</a> <a href='SkPaint_Reference#SkPaint'>on</a>; <a href='SkPaint_Reference#SkPaint'>may</a> <a href='SkPaint_Reference#SkPaint'>be</a> <a href='SkPaint_Reference#SkPaint'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="759e4e5bac680838added8f70884dcdc"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>

<a name='SkCanvas_drawPicture_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPicture'>drawPicture</a>(<a href='#SkCanvas_drawPicture'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>;& <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='#SkCanvas_drawPicture_4_picture'>picture</a>, <a href='#SkCanvas_drawPicture_4_picture'>using</a> <a href='#SkCanvas_drawPicture_4_picture'>clip</a> <a href='#SkCanvas_drawPicture_4_picture'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>; <a href='SkMatrix_Reference#SkMatrix'>transforming</a> <a href='#SkCanvas_drawPicture_4_picture'>picture</a> <a href='#SkCanvas_drawPicture_4_picture'>with</a>
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkCanvas_drawPicture_4_matrix'>matrix</a>, <a href='#SkCanvas_drawPicture_4_matrix'>if</a> <a href='#SkCanvas_drawPicture_4_matrix'>provided</a>; <a href='#SkCanvas_drawPicture_4_matrix'>and</a> <a href='#SkCanvas_drawPicture_4_matrix'>use</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>  <a href='#SkCanvas_drawPicture_4_paint'>paint alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>if</a> <a href='SkBlendMode_Reference#SkBlendMode'>provided</a>.

<a href='#SkCanvas_drawPicture_4_matrix'>matrix</a> <a href='#SkCanvas_drawPicture_4_matrix'>transformation</a> <a href='#SkCanvas_drawPicture_4_matrix'>is</a> <a href='#SkCanvas_drawPicture_4_matrix'>equivalent</a> <a href='#SkCanvas_drawPicture_4_matrix'>to</a>: <a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_concat'>concat()</a>, <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.
<a href='#SkCanvas_drawPicture_4_paint'>paint</a> <a href='#SkCanvas_drawPicture_4_paint'>use</a> <a href='#SkCanvas_drawPicture_4_paint'>is</a> <a href='#SkCanvas_drawPicture_4_paint'>equivalent</a> <a href='#SkCanvas_drawPicture_4_paint'>to</a>: <a href='#SkCanvas_saveLayer'>saveLayer</a>(), <a href='#SkCanvas_drawPicture'>drawPicture</a>(), <a href='#SkCanvas_restore'>restore()</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPicture_4_picture'><code><strong>picture</strong></code></a></td>
    <td>recorded drawing commands to play</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_4_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a>, <a href='SkMatrix_Reference#SkMatrix'>scale</a>, <a href='SkMatrix_Reference#SkMatrix'>translate</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>so</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>; <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPicture_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>apply</a> <a href='SkPaint_Reference#SkPaint'>transparency</a>, <a href='SkPaint_Reference#SkPaint'>filtering</a>, <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>so</a> <a href='SkPaint_Reference#SkPaint'>on</a>; <a href='SkPaint_Reference#SkPaint'>may</a> <a href='SkPaint_Reference#SkPaint'>be</a> <a href='SkPaint_Reference#SkPaint'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c4ff59439dd2fc871925d4eeb0c84ca1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawDrawable'>drawDrawable</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>

<a name='SkCanvas_drawVertices'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_vertices'>a</a> <a href='#SkCanvas_drawVertices_vertices'>triangle</a> <a href='#SkCanvas_drawVertices_vertices'>mesh</a>, <a href='#SkCanvas_drawVertices_vertices'>using</a> <a href='#SkCanvas_drawVertices_vertices'>clip</a> <a href='#SkCanvas_drawVertices_vertices'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> <a href='#SkCanvas_drawVertices_vertices'>and</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_vertices'>are</a> <a href='#SkCanvas_drawVertices_vertices'>defined</a> <a href='#SkCanvas_drawVertices_vertices'>in</a> <a href='#SkCanvas_drawVertices_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_vertices'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_mode'>mode</a> <a href='#SkCanvas_drawVertices_mode'>combines</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_mode'><code><strong>mode</strong></code></a></td>
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>if</a> <a href='undocumented#SkShader'>both</a> <a href='undocumented#SkShader'>are</a> <a href='undocumented#SkShader'>present</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>used</a> <a href='undocumented#SkShader'>as</a> <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>; <a href='undocumented#Texture'>may</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f48b22eaad1bb7adcc3faaa321754af6"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawVertices_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkVertices'>SkVertices</a>&<a href='undocumented#SkVertices'>gt</a>;& <a href='undocumented#Vertices'>vertices</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_2_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_2_vertices'>a</a> <a href='#SkCanvas_drawVertices_2_vertices'>triangle</a> <a href='#SkCanvas_drawVertices_2_vertices'>mesh</a>, <a href='#SkCanvas_drawVertices_2_vertices'>using</a> <a href='#SkCanvas_drawVertices_2_vertices'>clip</a> <a href='#SkCanvas_drawVertices_2_vertices'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> <a href='#SkCanvas_drawVertices_2_vertices'>and</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_2_vertices'>are</a> <a href='#SkCanvas_drawVertices_2_vertices'>defined</a> <a href='#SkCanvas_drawVertices_2_vertices'>in</a> <a href='#SkCanvas_drawVertices_2_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_2_vertices'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_2_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_2_mode'>mode</a> <a href='#SkCanvas_drawVertices_2_mode'>combines</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_2_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawVertices_2_vertices'><code><strong>vertices</strong></code></a></td>
    <td>triangle mesh to draw</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_2_mode'><code><strong>mode</strong></code></a></td>
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_2_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>if</a> <a href='undocumented#SkShader'>both</a> <a href='undocumented#SkShader'>are</a> <a href='undocumented#SkShader'>present</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_2_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>used</a> <a href='undocumented#SkShader'>as</a> <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>may</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e8bdae9bea3227758989028424fcac3d"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawVertices_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>* <a href='undocumented#Vertices'>vertices</a>, <a href='undocumented#Vertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> <a href='#SkVertices_Bone'>bones</a>[], <a href='#SkVertices_Bone'>int</a> <a href='#SkVertices_Bone'>boneCount</a>,
                  <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_3_vertices'>a</a> <a href='#SkCanvas_drawVertices_3_vertices'>triangle</a> <a href='#SkCanvas_drawVertices_3_vertices'>mesh</a>, <a href='#SkCanvas_drawVertices_3_vertices'>using</a> <a href='#SkCanvas_drawVertices_3_vertices'>clip</a> <a href='#SkCanvas_drawVertices_3_vertices'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Bone</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>used</a> <a href='undocumented#Data'>to</a>
deform <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a> <a href='#SkCanvas_drawVertices_3_vertices'>with</a> <a href='#SkCanvas_drawVertices_3_vertices'>bone</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> <a href='#SkCanvas_drawVertices_3_vertices'>and</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_3_vertices'>are</a> <a href='#SkCanvas_drawVertices_3_vertices'>defined</a> <a href='#SkCanvas_drawVertices_3_vertices'>in</a> <a href='#SkCanvas_drawVertices_3_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_3_vertices'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_3_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_3_mode'>mode</a> <a href='#SkCanvas_drawVertices_3_mode'>combines</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_3_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>.
The first element of <a href='#SkCanvas_drawVertices_3_bones'>bones</a> <a href='#SkCanvas_drawVertices_3_bones'>should</a> <a href='#SkCanvas_drawVertices_3_bones'>be</a> <a href='#SkCanvas_drawVertices_3_bones'>an</a> <a href='#SkCanvas_drawVertices_3_bones'>object</a> <a href='#SkCanvas_drawVertices_3_bones'>to</a> <a href='#SkCanvas_drawVertices_3_bones'>world</a> <a href='#SkCanvas_drawVertices_3_bones'>space</a> <a href='#SkCanvas_drawVertices_3_bones'>transformation</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a>
will be applied before performing mesh deformations. If no such transformation is needed,
it should be the identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
<a href='#SkCanvas_drawVertices_3_boneCount'>boneCount</a> <a href='#SkCanvas_drawVertices_3_boneCount'>must</a> <a href='#SkCanvas_drawVertices_3_boneCount'>be</a> <a href='#SkCanvas_drawVertices_3_boneCount'>at</a> <a href='#SkCanvas_drawVertices_3_boneCount'>most</a> 80, <a href='#SkCanvas_drawVertices_3_boneCount'>and</a> <a href='#SkCanvas_drawVertices_3_boneCount'>thus</a> <a href='#SkCanvas_drawVertices_3_boneCount'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkCanvas_drawVertices_3_bones'>bones</a> <a href='#SkCanvas_drawVertices_3_bones'>should</a> <a href='#SkCanvas_drawVertices_3_bones'>be</a> <a href='#SkCanvas_drawVertices_3_bones'>at</a> <a href='#SkCanvas_drawVertices_3_bones'>most</a> 80.

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
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_3_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>if</a> <a href='undocumented#SkShader'>both</a> <a href='undocumented#SkShader'>are</a> <a href='undocumented#SkShader'>present</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_3_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>used</a> <a href='undocumented#SkShader'>as</a> <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>may</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>nullptr</a></td>
  </tr>
</table>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawVertices_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawVertices'>drawVertices</a>(<a href='#SkCanvas_drawVertices'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkVertices'>SkVertices</a>&<a href='undocumented#SkVertices'>gt</a>;& <a href='undocumented#Vertices'>vertices</a>, <a href='undocumented#Vertices'>const</a> <a href='undocumented#SkVertices'>SkVertices</a>::<a href='#SkVertices_Bone'>Bone</a> <a href='#SkVertices_Bone'>bones</a>[], <a href='#SkVertices_Bone'>int</a> <a href='#SkVertices_Bone'>boneCount</a>,
                  <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='undocumented#SkVertices'>SkVertices</a> <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_4_vertices'>a</a> <a href='#SkCanvas_drawVertices_4_vertices'>triangle</a> <a href='#SkCanvas_drawVertices_4_vertices'>mesh</a>, <a href='#SkCanvas_drawVertices_4_vertices'>using</a> <a href='#SkCanvas_drawVertices_4_vertices'>clip</a> <a href='#SkCanvas_drawVertices_4_vertices'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Bone</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>used</a> <a href='undocumented#Data'>to</a>
deform <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a> <a href='#SkCanvas_drawVertices_4_vertices'>with</a> <a href='#SkCanvas_drawVertices_4_vertices'>bone</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>.
If  <a href='undocumented#Vertices_Texs'>vertices texs</a> <a href='#SkCanvas_drawVertices_4_vertices'>and</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_4_vertices'>are</a> <a href='#SkCanvas_drawVertices_4_vertices'>defined</a> <a href='#SkCanvas_drawVertices_4_vertices'>in</a> <a href='#SkCanvas_drawVertices_4_vertices'>vertices</a>, <a href='#SkCanvas_drawVertices_4_vertices'>and</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawVertices_4_paint'>paint</a>
contains <a href='undocumented#SkShader'>SkShader</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawVertices_4_mode'>mode</a> <a href='#SkCanvas_drawVertices_4_mode'>combines</a>  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_4_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>.
The first element of <a href='#SkCanvas_drawVertices_4_bones'>bones</a> <a href='#SkCanvas_drawVertices_4_bones'>should</a> <a href='#SkCanvas_drawVertices_4_bones'>be</a> <a href='#SkCanvas_drawVertices_4_bones'>an</a> <a href='#SkCanvas_drawVertices_4_bones'>object</a> <a href='#SkCanvas_drawVertices_4_bones'>to</a> <a href='#SkCanvas_drawVertices_4_bones'>world</a> <a href='#SkCanvas_drawVertices_4_bones'>space</a> <a href='#SkCanvas_drawVertices_4_bones'>transformation</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a>
will be applied before performing mesh deformations. If no such transformation is needed,
it should be the identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
<a href='#SkCanvas_drawVertices_4_boneCount'>boneCount</a> <a href='#SkCanvas_drawVertices_4_boneCount'>must</a> <a href='#SkCanvas_drawVertices_4_boneCount'>be</a> <a href='#SkCanvas_drawVertices_4_boneCount'>at</a> <a href='#SkCanvas_drawVertices_4_boneCount'>most</a> 80, <a href='#SkCanvas_drawVertices_4_boneCount'>and</a> <a href='#SkCanvas_drawVertices_4_boneCount'>thus</a> <a href='#SkCanvas_drawVertices_4_boneCount'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkCanvas_drawVertices_4_bones'>bones</a> <a href='#SkCanvas_drawVertices_4_bones'>should</a> <a href='#SkCanvas_drawVertices_4_bones'>be</a> <a href='#SkCanvas_drawVertices_4_bones'>at</a> <a href='#SkCanvas_drawVertices_4_bones'>most</a> 80.

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
    <td>combines  <a href='undocumented#Vertices_Colors'>vertices colors</a> <a href='#SkCanvas_drawVertices_4_vertices'>with</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>if</a> <a href='undocumented#SkShader'>both</a> <a href='undocumented#SkShader'>are</a> <a href='undocumented#SkShader'>present</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawVertices_4_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies the <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkShader'>used</a> <a href='undocumented#SkShader'>as</a> <a href='undocumented#SkVertices'>SkVertices</a> <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>may</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>nullptr</a></td>
  </tr>
</table>

### See Also

<a href='#SkCanvas_drawPatch'>drawPatch</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawPatch'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPatch'>drawPatch</a>(<a href='#SkCanvas_drawPatch'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], <a href='SkPath_Reference#Cubic'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[4], <a href='SkColor_Reference#SkColor'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>texCoords</a>[4],
               <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a  <a href='undocumented#Coons_Patch'>Coons patch</a>: the interpolation of four <a href='#SkCanvas_drawPatch_cubics'>cubics</a> <a href='#SkCanvas_drawPatch_cubics'>with</a> <a href='#SkCanvas_drawPatch_cubics'>shared</a> <a href='#SkCanvas_drawPatch_cubics'>corners</a>,
associating a <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>optionally</a> <a href='SkColor_Reference#Color'>a</a> <a href='undocumented#Texture'>texture</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>with</a> <a href='SkPoint_Reference#SkPoint'>each</a> <a href='SkPoint_Reference#SkPoint'>corner</a>.

<a href='undocumented#Coons_Patch'>Coons patch</a> uses clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='#SkCanvas_drawPatch_paint'>paint</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>. <a href='SkBlendMode_Reference#SkBlendMode'>If</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>is</a> <a href='undocumented#SkShader'>provided</a> <a href='undocumented#SkShader'>it</a> <a href='undocumented#SkShader'>is</a> <a href='undocumented#SkShader'>treated</a>
as  <a href='undocumented#Coons_Patch'>Coons patch</a> <a href='undocumented#Texture'>texture</a>; <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawPatch_mode'>mode</a> <a href='#SkCanvas_drawPatch_mode'>combines</a> <a href='SkColor_Reference#Color'>color</a> <a href='#SkCanvas_drawPatch_colors'>colors</a> <a href='#SkCanvas_drawPatch_colors'>and</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>if</a>
both are provided.

<a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_cubics'>cubics</a> <a href='#SkCanvas_drawPatch_cubics'>specifies</a> <a href='#SkCanvas_drawPatch_cubics'>four</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>starting</a> <a href='SkPath_Reference#Cubic'>at</a> <a href='SkPath_Reference#Cubic'>the</a> <a href='SkPath_Reference#Cubic'>top-left</a> <a href='SkPath_Reference#Cubic'>corner</a>,
in clockwise order, sharing every fourth <a href='SkPoint_Reference#Point'>point</a>. <a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>ends</a> <a href='SkPath_Reference#Cubic'>at</a> <a href='SkPath_Reference#Cubic'>the</a>
first <a href='SkPoint_Reference#Point'>point</a>.

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>array</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>associates</a> <a href='#SkCanvas_drawPatch_colors'>colors</a> <a href='#SkCanvas_drawPatch_colors'>with</a> <a href='#SkCanvas_drawPatch_colors'>corners</a> <a href='#SkCanvas_drawPatch_colors'>in</a> <a href='#SkCanvas_drawPatch_colors'>top-left</a>, <a href='#SkCanvas_drawPatch_colors'>top-right</a>,
bottom-right, bottom-left order.

If <a href='#SkCanvas_drawPatch_paint'>paint</a> <a href='#SkCanvas_drawPatch_paint'>contains</a> <a href='undocumented#SkShader'>SkShader</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_texCoords'>texCoords</a> <a href='#SkCanvas_drawPatch_texCoords'>maps</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>as</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>to</a>
corners in top-left, top-right, bottom-right, bottom-left order.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_cubics'><code><strong>cubics</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>array</a>, <a href='SkPath_Reference#Cubic'>sharing</a> <a href='SkPath_Reference#Cubic'>common</a> <a href='SkPoint_Reference#Point'>points</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_colors'><code><strong>colors</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>array</a>, <a href='SkColor_Reference#Color'>one</a> <a href='SkColor_Reference#Color'>for</a> <a href='SkColor_Reference#Color'>each</a> <a href='SkColor_Reference#Color'>corner</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_texCoords'><code><strong>texCoords</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>coordinates</a>, <a href='undocumented#Texture'>mapping</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>to</a> <a href='undocumented#SkShader'>corners</a>;</td>
  </tr>
</table>

may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>for</a> <a href='#SkCanvas_drawPatch_colors'>colors</a>, <a href='#SkCanvas_drawPatch_colors'>and</a> <a href='#SkCanvas_drawPatch_colors'>for</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>if</a> <a href='#SkCanvas_drawPatch_paint'>paint</a> <a href='#SkCanvas_drawPatch_paint'>has</a> <a href='#SkCanvas_drawPatch_paint'>one</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>used</a> <a href='SkBlendMode_Reference#SkBlendMode'>to</a> <a href='SkBlendMode_Reference#SkBlendMode'>draw</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="accb545d67984ced168f5be6ab824795"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawVertices'>drawVertices</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawPatch_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawPatch'>drawPatch</a>(<a href='#SkCanvas_drawPatch'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Cubic'>cubics</a>[12], <a href='SkPath_Reference#Cubic'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[4], <a href='SkColor_Reference#SkColor'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>texCoords</a>[4],
               <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a>  <a href='undocumented#Coons_Patch'>Coons patch</a>: <a href='SkPath_Reference#Cubic'>the</a> <a href='SkPath_Reference#Cubic'>interpolation</a> <a href='SkPath_Reference#Cubic'>of</a> <a href='SkPath_Reference#Cubic'>four</a> <a href='#SkCanvas_drawPatch_2_cubics'>cubics</a> <a href='#SkCanvas_drawPatch_2_cubics'>with</a> <a href='#SkCanvas_drawPatch_2_cubics'>shared</a> <a href='#SkCanvas_drawPatch_2_cubics'>corners</a>,
associating a <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>optionally</a> <a href='SkColor_Reference#Color'>a</a> <a href='undocumented#Texture'>texture</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>with</a> <a href='SkPoint_Reference#SkPoint'>each</a> <a href='SkPoint_Reference#SkPoint'>corner</a>.

<a href='undocumented#Coons_Patch'>Coons patch</a> uses clip and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='#SkCanvas_drawPatch_2_paint'>paint</a> <a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>,
<a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>. <a href='SkBlendMode_Reference#SkBlendMode'>If</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>is</a> <a href='undocumented#SkShader'>provided</a> <a href='undocumented#SkShader'>it</a> <a href='undocumented#SkShader'>is</a> <a href='undocumented#SkShader'>treated</a>
as  <a href='undocumented#Coons_Patch'>Coons patch</a> <a href='undocumented#Texture'>texture</a>; <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a> <a href='SkBlendMode_Reference#SkBlendMode'>combines</a> <a href='SkColor_Reference#Color'>color</a> <a href='#SkCanvas_drawPatch_2_colors'>colors</a> <a href='#SkCanvas_drawPatch_2_colors'>and</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>if</a>
both are provided.

<a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_2_cubics'>cubics</a> <a href='#SkCanvas_drawPatch_2_cubics'>specifies</a> <a href='#SkCanvas_drawPatch_2_cubics'>four</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>starting</a> <a href='SkPath_Reference#Cubic'>at</a> <a href='SkPath_Reference#Cubic'>the</a> <a href='SkPath_Reference#Cubic'>top-left</a> <a href='SkPath_Reference#Cubic'>corner</a>,
in clockwise order, sharing every fourth <a href='SkPoint_Reference#Point'>point</a>. <a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>ends</a> <a href='SkPath_Reference#Cubic'>at</a> <a href='SkPath_Reference#Cubic'>the</a>
first <a href='SkPoint_Reference#Point'>point</a>.

<a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>array</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>associates</a> <a href='#SkCanvas_drawPatch_2_colors'>colors</a> <a href='#SkCanvas_drawPatch_2_colors'>with</a> <a href='#SkCanvas_drawPatch_2_colors'>corners</a> <a href='#SkCanvas_drawPatch_2_colors'>in</a> <a href='#SkCanvas_drawPatch_2_colors'>top-left</a>, <a href='#SkCanvas_drawPatch_2_colors'>top-right</a>,
bottom-right, bottom-left order.

If <a href='#SkCanvas_drawPatch_2_paint'>paint</a> <a href='#SkCanvas_drawPatch_2_paint'>contains</a> <a href='undocumented#SkShader'>SkShader</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='#SkCanvas_drawPatch_2_texCoords'>texCoords</a> <a href='#SkCanvas_drawPatch_2_texCoords'>maps</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>as</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>to</a>
corners in top-left, top-right, bottom-right, bottom-left order.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_2_cubics'><code><strong>cubics</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>array</a>, <a href='SkPath_Reference#Cubic'>sharing</a> <a href='SkPath_Reference#Cubic'>common</a> <a href='SkPoint_Reference#Point'>points</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_colors'><code><strong>colors</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>array</a>, <a href='SkColor_Reference#Color'>one</a> <a href='SkColor_Reference#Color'>for</a> <a href='SkColor_Reference#Color'>each</a> <a href='SkColor_Reference#Color'>corner</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawPatch_2_texCoords'><code><strong>texCoords</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>coordinates</a>, <a href='undocumented#Texture'>mapping</a> <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>to</a> <a href='undocumented#SkShader'>corners</a>;</td>
  </tr>
</table>

may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawPatch_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkShader'>SkShader</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>used</a> <a href='SkBlendMode_Reference#SkBlendMode'>to</a> <a href='SkBlendMode_Reference#SkBlendMode'>draw</a></td>
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
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[],
               <a href='SkRect_Reference#SkRect'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[], <a href='SkColor_Reference#SkColor'>int</a> <a href='SkColor_Reference#SkColor'>count</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>,
               <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>from</a> <a href='#SkCanvas_drawAtlas_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_atlas'>using</a> <a href='#SkCanvas_drawAtlas_atlas'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_paint'>paint</a> <a href='#SkCanvas_drawAtlas_paint'>uses</a> <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_tex'>tex</a> <a href='#SkCanvas_drawAtlas_tex'>locates</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>in</a>
<a href='#SkCanvas_drawAtlas_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_atlas'>and</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_xform'>xform</a> <a href='#SkCanvas_drawAtlas_xform'>transforms</a> <a href='#SkCanvas_drawAtlas_xform'>it</a> <a href='#SkCanvas_drawAtlas_xform'>into</a> <a href='#SkCanvas_drawAtlas_xform'>destination</a> <a href='#SkCanvas_drawAtlas_xform'>space</a>.

<a href='#SkCanvas_drawAtlas_xform'>xform</a>, <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>and</a> <a href='#SkCanvas_drawAtlas_colors'>colors</a> <a href='#SkCanvas_drawAtlas_colors'>if</a> <a href='#SkCanvas_drawAtlas_colors'>present</a>, <a href='#SkCanvas_drawAtlas_colors'>must</a> <a href='#SkCanvas_drawAtlas_colors'>contain</a> <a href='#SkCanvas_drawAtlas_count'>count</a> <a href='#SkCanvas_drawAtlas_count'>entries</a>.
Optional <a href='#SkCanvas_drawAtlas_colors'>colors</a> <a href='#SkCanvas_drawAtlas_colors'>are</a> <a href='#SkCanvas_drawAtlas_colors'>applied</a> <a href='#SkCanvas_drawAtlas_colors'>for</a> <a href='#SkCanvas_drawAtlas_colors'>each</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>using</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='#SkCanvas_drawAtlas_mode'>mode</a>, <a href='#SkCanvas_drawAtlas_mode'>treating</a>
<a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>as</a> <a href='undocumented#Sprite'>source</a> <a href='undocumented#Sprite'>and</a> <a href='#SkCanvas_drawAtlas_colors'>colors</a> <a href='#SkCanvas_drawAtlas_colors'>as</a> <a href='#SkCanvas_drawAtlas_colors'>destination</a>.
Optional <a href='#SkCanvas_drawAtlas_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_cullRect'>a</a> <a href='#SkCanvas_drawAtlas_cullRect'>conservative</a> <a href='#SkCanvas_drawAtlas_cullRect'>bounds</a> <a href='#SkCanvas_drawAtlas_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_cullRect'>all</a> <a href='#SkCanvas_drawAtlas_cullRect'>transformed</a> <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_cullRect'>outside</a> <a href='#SkCanvas_drawAtlas_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_cullRect'>clip</a>, <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>skip</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>mappings</a> <a href='undocumented#SkRSXform'>for</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>locations</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_colors'><code><strong>colors</strong></code></a></td>
    <td>one per <a href='undocumented#Sprite'>sprite</a>, <a href='undocumented#Sprite'>blended</a> <a href='undocumented#Sprite'>with</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>using</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>; <a href='SkBlendMode_Reference#SkBlendMode'>may</a> <a href='SkBlendMode_Reference#SkBlendMode'>be</a> <a href='SkBlendMode_Reference#SkBlendMode'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>to</a> <a href='undocumented#Sprite'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>combining</a> <a href='#SkCanvas_drawAtlas_colors'>colors</a> <a href='#SkCanvas_drawAtlas_colors'>and</a> <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>for</a> <a href='undocumented#Sprite'>efficient</a> <a href='undocumented#Sprite'>clipping</a>; <a href='undocumented#Sprite'>may</a> <a href='undocumented#Sprite'>be</a> <a href='undocumented#Sprite'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>so</a> <a href='SkBlendMode_Reference#SkBlendMode'>on</a>; <a href='SkBlendMode_Reference#SkBlendMode'>may</a> <a href='SkBlendMode_Reference#SkBlendMode'>be</a> <a href='SkBlendMode_Reference#SkBlendMode'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1df575f9b8132306ce0552a2554ed132"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawAtlas_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[],
               <a href='SkRect_Reference#SkRect'>const</a> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>colors</a>[], <a href='SkColor_Reference#SkColor'>int</a> <a href='SkColor_Reference#SkColor'>count</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>mode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>,
               <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>from</a> <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_2_atlas'>using</a> <a href='#SkCanvas_drawAtlas_2_atlas'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_2_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_2_paint'>paint</a> <a href='#SkCanvas_drawAtlas_2_paint'>uses</a> <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_2_tex'>tex</a> <a href='#SkCanvas_drawAtlas_2_tex'>locates</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>in</a>
<a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_2_atlas'>and</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_2_xform'>xform</a> <a href='#SkCanvas_drawAtlas_2_xform'>transforms</a> <a href='#SkCanvas_drawAtlas_2_xform'>it</a> <a href='#SkCanvas_drawAtlas_2_xform'>into</a> <a href='#SkCanvas_drawAtlas_2_xform'>destination</a> <a href='#SkCanvas_drawAtlas_2_xform'>space</a>.

<a href='#SkCanvas_drawAtlas_2_xform'>xform</a>, <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>and</a> <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> <a href='#SkCanvas_drawAtlas_2_colors'>if</a> <a href='#SkCanvas_drawAtlas_2_colors'>present</a>, <a href='#SkCanvas_drawAtlas_2_colors'>must</a> <a href='#SkCanvas_drawAtlas_2_colors'>contain</a> <a href='#SkCanvas_drawAtlas_2_count'>count</a> <a href='#SkCanvas_drawAtlas_2_count'>entries</a>.
Optional <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> <a href='#SkCanvas_drawAtlas_2_colors'>is</a> <a href='#SkCanvas_drawAtlas_2_colors'>applied</a> <a href='#SkCanvas_drawAtlas_2_colors'>for</a> <a href='#SkCanvas_drawAtlas_2_colors'>each</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>using</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>.
Optional <a href='#SkCanvas_drawAtlas_2_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>a</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>conservative</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>bounds</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>all</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>transformed</a> <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_2_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>outside</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_2_cullRect'>clip</a>, <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>skip</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_2_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>mappings</a> <a href='undocumented#SkRSXform'>for</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>locations</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_2_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_colors'><code><strong>colors</strong></code></a></td>
    <td>one per <a href='undocumented#Sprite'>sprite</a>, <a href='undocumented#Sprite'>blended</a> <a href='undocumented#Sprite'>with</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>using</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>; <a href='SkBlendMode_Reference#SkBlendMode'>may</a> <a href='SkBlendMode_Reference#SkBlendMode'>be</a> <a href='SkBlendMode_Reference#SkBlendMode'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>to</a> <a href='undocumented#Sprite'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a> <a href='SkBlendMode_Reference#SkBlendMode'>combining</a> <a href='#SkCanvas_drawAtlas_2_colors'>colors</a> <a href='#SkCanvas_drawAtlas_2_colors'>and</a> <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>for</a> <a href='undocumented#Sprite'>efficient</a> <a href='undocumented#Sprite'>clipping</a>; <a href='undocumented#Sprite'>may</a> <a href='undocumented#Sprite'>be</a> <a href='undocumented#Sprite'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_2_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>so</a> <a href='SkBlendMode_Reference#SkBlendMode'>on</a>; <a href='SkBlendMode_Reference#SkBlendMode'>may</a> <a href='SkBlendMode_Reference#SkBlendMode'>be</a> <a href='SkBlendMode_Reference#SkBlendMode'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e66a8f230a8d531bcef9f5ebdc5aac1"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawAtlas_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='SkImage_Reference#SkImage'>SkImage</a>* <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[], <a href='SkRect_Reference#SkRect'>int</a> <a href='SkRect_Reference#SkRect'>count</a>,
               <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>from</a> <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_3_atlas'>using</a> <a href='#SkCanvas_drawAtlas_3_atlas'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_3_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_3_paint'>paint</a> <a href='#SkCanvas_drawAtlas_3_paint'>uses</a> <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_3_tex'>tex</a> <a href='#SkCanvas_drawAtlas_3_tex'>locates</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>in</a>
<a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_3_atlas'>and</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_3_xform'>xform</a> <a href='#SkCanvas_drawAtlas_3_xform'>transforms</a> <a href='#SkCanvas_drawAtlas_3_xform'>it</a> <a href='#SkCanvas_drawAtlas_3_xform'>into</a> <a href='#SkCanvas_drawAtlas_3_xform'>destination</a> <a href='#SkCanvas_drawAtlas_3_xform'>space</a>.

<a href='#SkCanvas_drawAtlas_3_xform'>xform</a> <a href='#SkCanvas_drawAtlas_3_xform'>and</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>must</a> <a href='undocumented#Text'>contain</a> <a href='#SkCanvas_drawAtlas_3_count'>count</a> <a href='#SkCanvas_drawAtlas_3_count'>entries</a>.
Optional <a href='#SkCanvas_drawAtlas_3_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>a</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>conservative</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>bounds</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>all</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>transformed</a> <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_3_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>outside</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_3_cullRect'>clip</a>, <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>skip</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_3_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>mappings</a> <a href='undocumented#SkRSXform'>for</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>locations</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_3_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>to</a> <a href='undocumented#Sprite'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>for</a> <a href='undocumented#Sprite'>efficient</a> <a href='undocumented#Sprite'>clipping</a>; <a href='undocumented#Sprite'>may</a> <a href='undocumented#Sprite'>be</a> <a href='undocumented#Sprite'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_3_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>so</a> <a href='SkBlendMode_Reference#SkBlendMode'>on</a>; <a href='SkBlendMode_Reference#SkBlendMode'>may</a> <a href='SkBlendMode_Reference#SkBlendMode'>be</a> <a href='SkBlendMode_Reference#SkBlendMode'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8dc0d0fdeab20bbc21cac6874ddbefcd"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawAtlas_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAtlas'>drawAtlas</a>(<a href='#SkCanvas_drawAtlas'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>;& <a href='SkImage_Reference#SkImage'>atlas</a>, <a href='SkImage_Reference#SkImage'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>xform</a>[], <a href='undocumented#SkRSXform'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>tex</a>[], <a href='SkRect_Reference#SkRect'>int</a> <a href='SkRect_Reference#SkRect'>count</a>,
               <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>cullRect</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws a set of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>from</a> <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_4_atlas'>using</a> <a href='#SkCanvas_drawAtlas_4_atlas'>clip</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>optional</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkCanvas_drawAtlas_4_paint'>paint</a>.
<a href='#SkCanvas_drawAtlas_4_paint'>paint</a> <a href='#SkCanvas_drawAtlas_4_paint'>uses</a> <a href='SkPaint_Reference#Anti_Alias'>anti-alias</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='undocumented#SkImageFilter'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>
to draw, if present. For each entry in the array, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkCanvas_drawAtlas_4_tex'>tex</a> <a href='#SkCanvas_drawAtlas_4_tex'>locates</a> <a href='undocumented#Sprite'>sprite</a> <a href='undocumented#Sprite'>in</a>
<a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a>, <a href='#SkCanvas_drawAtlas_4_atlas'>and</a> <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='#SkCanvas_drawAtlas_4_xform'>xform</a> <a href='#SkCanvas_drawAtlas_4_xform'>transforms</a> <a href='#SkCanvas_drawAtlas_4_xform'>it</a> <a href='#SkCanvas_drawAtlas_4_xform'>into</a> <a href='#SkCanvas_drawAtlas_4_xform'>destination</a> <a href='#SkCanvas_drawAtlas_4_xform'>space</a>.

<a href='#SkCanvas_drawAtlas_4_xform'>xform</a> <a href='#SkCanvas_drawAtlas_4_xform'>and</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>must</a> <a href='undocumented#Text'>contain</a> <a href='#SkCanvas_drawAtlas_4_count'>count</a> <a href='#SkCanvas_drawAtlas_4_count'>entries</a>.
Optional <a href='#SkCanvas_drawAtlas_4_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>a</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>conservative</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>bounds</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>all</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>transformed</a> <a href='undocumented#Sprite'>sprites</a>.
If <a href='#SkCanvas_drawAtlas_4_cullRect'>cullRect</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>is</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>outside</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>of</a> <a href='#SkCanvas_drawAtlas_4_cullRect'>clip</a>, <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>skip</a> <a href='SkCanvas_Reference#Canvas'>drawing</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAtlas_4_atlas'><code><strong>atlas</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>containing</a> <a href='undocumented#Sprite'>sprites</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_xform'><code><strong>xform</strong></code></a></td>
    <td><a href='undocumented#SkRSXform'>SkRSXform</a> <a href='undocumented#SkRSXform'>mappings</a> <a href='undocumented#SkRSXform'>for</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_tex'><code><strong>tex</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>locations</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>in</a> <a href='#SkCanvas_drawAtlas_4_atlas'>atlas</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>to</a> <a href='undocumented#Sprite'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_cullRect'><code><strong>cullRect</strong></code></a></td>
    <td>bounds of transformed <a href='undocumented#Sprite'>sprites</a> <a href='undocumented#Sprite'>for</a> <a href='undocumented#Sprite'>efficient</a> <a href='undocumented#Sprite'>clipping</a>; <a href='undocumented#Sprite'>may</a> <a href='undocumented#Sprite'>be</a> <a href='undocumented#Sprite'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAtlas_4_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>, <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='SkBlendMode_Reference#SkBlendMode'>so</a> <a href='SkBlendMode_Reference#SkBlendMode'>on</a>; <a href='SkBlendMode_Reference#SkBlendMode'>may</a> <a href='SkBlendMode_Reference#SkBlendMode'>be</a> <a href='SkBlendMode_Reference#SkBlendMode'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c093c2b14bd3e6171ede7cd4049d9b57"></fiddle-embed></div>

### See Also

<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='#SkCanvas_drawImage'>drawImage</a>

<a name='SkCanvas_drawDrawable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, <a href='undocumented#Drawable'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a> = <a href='SkMatrix_Reference#Matrix'>nullptr</a>)
</pre>

Draws <a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawDrawable_drawable'>drawable</a> <a href='#SkCanvas_drawDrawable_drawable'>using</a> <a href='#SkCanvas_drawDrawable_drawable'>clip</a> <a href='#SkCanvas_drawDrawable_drawable'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>concatenated</a> <a href='SkMatrix_Reference#SkMatrix'>with</a>
optional <a href='#SkCanvas_drawDrawable_matrix'>matrix</a>.

If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>has</a> <a href='SkCanvas_Reference#SkCanvas'>an</a> <a href='SkCanvas_Reference#SkCanvas'>asynchronous</a> <a href='SkCanvas_Reference#SkCanvas'>implementation</a>, <a href='SkCanvas_Reference#SkCanvas'>as</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkCanvas_Reference#SkCanvas'>case</a>
when it is recording into <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, <a href='SkPicture_Reference#SkPicture'>then</a> <a href='#SkCanvas_drawDrawable_drawable'>drawable</a> <a href='#SkCanvas_drawDrawable_drawable'>will</a> <a href='#SkCanvas_drawDrawable_drawable'>be</a> <a href='#SkCanvas_drawDrawable_drawable'>referenced</a>,
so that <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> <a href='#SkDrawable_draw'>can</a> <a href='#SkDrawable_draw'>be</a> <a href='#SkDrawable_draw'>called</a> <a href='#SkDrawable_draw'>when</a> <a href='#SkDrawable_draw'>the</a> <a href='#SkDrawable_draw'>operation</a> <a href='#SkDrawable_draw'>is</a> <a href='#SkDrawable_draw'>finalized</a>. <a href='#SkDrawable_draw'>To</a> <a href='#SkDrawable_draw'>force</a>
immediate drawing, call <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> <a href='#SkDrawable_draw'>instead</a>.

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
void <a href='#SkCanvas_drawDrawable'>drawDrawable</a>(<a href='undocumented#SkDrawable'>SkDrawable</a>* <a href='undocumented#Drawable'>drawable</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Draws <a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawDrawable_2_drawable'>drawable</a> <a href='#SkCanvas_drawDrawable_2_drawable'>using</a> <a href='#SkCanvas_drawDrawable_2_drawable'>clip</a> <a href='#SkCanvas_drawDrawable_2_drawable'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>offset</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkCanvas_drawDrawable_2_x'>x</a>, <a href='#SkCanvas_drawDrawable_2_y'>y</a>).

If <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>has</a> <a href='SkCanvas_Reference#SkCanvas'>an</a> <a href='SkCanvas_Reference#SkCanvas'>asynchronous</a> <a href='SkCanvas_Reference#SkCanvas'>implementation</a>, <a href='SkCanvas_Reference#SkCanvas'>as</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkCanvas_Reference#SkCanvas'>case</a>
when it is recording into <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, <a href='SkPicture_Reference#SkPicture'>then</a> <a href='#SkCanvas_drawDrawable_2_drawable'>drawable</a> <a href='#SkCanvas_drawDrawable_2_drawable'>will</a> <a href='#SkCanvas_drawDrawable_2_drawable'>be</a> <a href='#SkCanvas_drawDrawable_2_drawable'>referenced</a>,
so that <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> <a href='#SkDrawable_draw'>can</a> <a href='#SkDrawable_draw'>be</a> <a href='#SkDrawable_draw'>called</a> <a href='#SkDrawable_draw'>when</a> <a href='#SkDrawable_draw'>the</a> <a href='#SkDrawable_draw'>operation</a> <a href='#SkDrawable_draw'>is</a> <a href='#SkDrawable_draw'>finalized</a>. <a href='#SkDrawable_draw'>To</a> <a href='#SkDrawable_draw'>force</a>
immediate drawing, call <a href='undocumented#SkDrawable'>SkDrawable</a>::<a href='#SkDrawable_draw'>draw()</a> <a href='#SkDrawable_draw'>instead</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawDrawable_2_drawable'><code><strong>drawable</strong></code></a></td>
    <td>custom struct encapsulating drawing commands</td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_2_x'><code><strong>x</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>writable</a> <a href='SkCanvas_Reference#SkCanvas'>pixels</a> <a href='SkCanvas_Reference#SkCanvas'>on</a> <a href='SkCanvas_Reference#SkCanvas'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawDrawable_2_y'><code><strong>y</strong></code></a></td>
    <td>offset into <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>writable</a> <a href='SkCanvas_Reference#SkCanvas'>pixels</a> <a href='SkCanvas_Reference#SkCanvas'>on</a> <a href='SkCanvas_Reference#SkCanvas'>y-axis</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1bdc07ad3b154c89b771722c2fcaee3f"></fiddle-embed></div>

### See Also

<a href='undocumented#SkDrawable'>SkDrawable</a> <a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkCanvas_drawAnnotation'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(<a href='#SkCanvas_drawAnnotation'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkRect_Reference#Rect'>char</a> <a href='SkRect_Reference#Rect'>key</a>[], <a href='undocumented#SkData'>SkData</a>* <a href='undocumented#SkData'>value</a>)
</pre>

Associates <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>on</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>with</a> <a href='SkCanvas_Reference#SkCanvas'>an</a> <a href='SkCanvas_Reference#SkCanvas'>annotation</a>; <a href='SkCanvas_Reference#SkCanvas'>a</a> <a href='SkCanvas_Reference#SkCanvas'>key-value</a> <a href='SkCanvas_Reference#SkCanvas'>pair</a>, <a href='SkCanvas_Reference#SkCanvas'>where</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='#SkCanvas_drawAnnotation_key'>key</a> <a href='#SkCanvas_drawAnnotation_key'>is</a>
a null-terminated UTF-8 <a href='undocumented#String'>string</a>, <a href='undocumented#String'>and</a> <a href='undocumented#String'>optional</a> <a href='#SkCanvas_drawAnnotation_value'>value</a> <a href='#SkCanvas_drawAnnotation_value'>is</a> <a href='#SkCanvas_drawAnnotation_value'>stored</a> <a href='#SkCanvas_drawAnnotation_value'>as</a> <a href='undocumented#SkData'>SkData</a>.

Only some <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>implementations</a>, <a href='SkCanvas_Reference#Canvas'>such</a> <a href='SkCanvas_Reference#Canvas'>as</a> <a href='SkCanvas_Reference#Canvas'>recording</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, <a href='SkPicture_Reference#SkPicture'>or</a> <a href='SkPicture_Reference#SkPicture'>drawing</a> <a href='SkPicture_Reference#SkPicture'>to</a>
<a href='undocumented#Document_PDF'>document PDF</a>, <a href='undocumented#Document'>use</a> <a href='undocumented#Document'>annotations</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAnnotation_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>extent</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>annotate</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_key'><code><strong>key</strong></code></a></td>
    <td><a href='undocumented#String'>string</a> <a href='undocumented#String'>used</a> <a href='undocumented#String'>for</a> <a href='undocumented#String'>lookup</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_value'><code><strong>value</strong></code></a></td>
    <td><a href='undocumented#Data'>data</a> <a href='undocumented#Data'>holding</a> <a href='#SkCanvas_drawAnnotation_value'>value</a> <a href='#SkCanvas_drawAnnotation_value'>stored</a> <a href='#SkCanvas_drawAnnotation_value'>in</a> <a href='#SkCanvas_drawAnnotation_value'>annotation</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="00b430bd80d740e19c6d020a940f56d5"></fiddle-embed></div>

### See Also

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#SkDocument'>SkDocument</a>

<a name='SkCanvas_drawAnnotation_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkCanvas_drawAnnotation'>drawAnnotation</a>(<a href='#SkCanvas_drawAnnotation'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkRect_Reference#Rect'>char</a> <a href='SkRect_Reference#Rect'>key</a>[], <a href='SkRect_Reference#Rect'>const</a> <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>;& <a href='undocumented#SkData'>value</a>)
</pre>

Associates <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>on</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>when</a> <a href='SkCanvas_Reference#SkCanvas'>an</a> <a href='SkCanvas_Reference#SkCanvas'>annotation</a>; <a href='SkCanvas_Reference#SkCanvas'>a</a> <a href='SkCanvas_Reference#SkCanvas'>key-value</a> <a href='SkCanvas_Reference#SkCanvas'>pair</a>, <a href='SkCanvas_Reference#SkCanvas'>where</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='#SkCanvas_drawAnnotation_2_key'>key</a> <a href='#SkCanvas_drawAnnotation_2_key'>is</a>
a null-terminated UTF-8 <a href='undocumented#String'>string</a>, <a href='undocumented#String'>and</a> <a href='undocumented#String'>optional</a> <a href='#SkCanvas_drawAnnotation_2_value'>value</a> <a href='#SkCanvas_drawAnnotation_2_value'>is</a> <a href='#SkCanvas_drawAnnotation_2_value'>stored</a> <a href='#SkCanvas_drawAnnotation_2_value'>as</a> <a href='undocumented#SkData'>SkData</a>.

Only some <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>implementations</a>, <a href='SkCanvas_Reference#Canvas'>such</a> <a href='SkCanvas_Reference#Canvas'>as</a> <a href='SkCanvas_Reference#Canvas'>recording</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, <a href='SkPicture_Reference#SkPicture'>or</a> <a href='SkPicture_Reference#SkPicture'>drawing</a> <a href='SkPicture_Reference#SkPicture'>to</a>
<a href='undocumented#Document_PDF'>document PDF</a>, <a href='undocumented#Document'>use</a> <a href='undocumented#Document'>annotations</a>.

### Parameters

<table>  <tr>    <td><a name='SkCanvas_drawAnnotation_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>extent</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>annotate</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_2_key'><code><strong>key</strong></code></a></td>
    <td><a href='undocumented#String'>string</a> <a href='undocumented#String'>used</a> <a href='undocumented#String'>for</a> <a href='undocumented#String'>lookup</a></td>
  </tr>
  <tr>    <td><a name='SkCanvas_drawAnnotation_2_value'><code><strong>value</strong></code></a></td>
    <td><a href='undocumented#Data'>data</a> <a href='undocumented#Data'>holding</a> <a href='#SkCanvas_drawAnnotation_2_value'>value</a> <a href='#SkCanvas_drawAnnotation_2_value'>stored</a> <a href='#SkCanvas_drawAnnotation_2_value'>in</a> <a href='#SkCanvas_drawAnnotation_2_value'>annotation</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="00b430bd80d740e19c6d020a940f56d5"></fiddle-embed></div>

### See Also

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#SkDocument'>SkDocument</a>

<a name='SkCanvas_isClipEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkCanvas_isClipEmpty'>isClipEmpty</a>() <a href='#SkCanvas_isClipEmpty'>const</a>
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
virtual bool <a href='#SkCanvas_isClipRect'>isClipRect</a>() <a href='#SkCanvas_isClipRect'>const</a>
</pre>

Returns true if clip is <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>empty</a>.
Returns false if the clip is empty, or if it is not <a href='SkRect_Reference#SkRect'>SkRect</a>.

### Return Value

true if clip is <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>empty</a>

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

