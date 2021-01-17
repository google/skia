// This file is type-checked by the Typescript definitions. It is not actually executed.
// Test it by running `npm run dtslint` in the parent directory.
import {
    CanvasKitInit,
    AnimatedImage,
    Canvas,
    CanvasKit,
    ColorFilter,
    Font,
    FontMgr,
    Image,
    ImageFilter,
    ImageInfo,
    MaskFilter,
    Paint,
    Paragraph,
    Path,
    PathEffect,
    Shader,
    ShapedText,
    SkPicture,
    TextBlob,
    Typeface,
    Vertices,
} from "canvaskit-wasm";

CanvasKitInit({locateFile: (file: string) => '/node_modules/canvaskit/bin/' + file}).then((CK: CanvasKit) => {
    animatedImageTests(CK);
    canvasTests(CK);
    canvas2DTests(CK);
    colorFilterTests(CK);
    colorTests(CK);
    contourMeasureTests(CK);
    imageFilterTests(CK);
    imageTests(CK);
    fontTests(CK);
    fontMgrTests(CK);
    globalTests(CK);
    mallocTests(CK);
    maskFilterTests(CK);
    matrixTests(CK);
    paintTests(CK);
    paragraphTests(CK);
    paragraphBuilderTests(CK);
    particlesTests(CK);
    pathEffectTests(CK);
    pathTests(CK);
    pictureTests(CK);
    rectangleTests(CK);
    runtimeEffectTests(CK);
    skottieTests(CK);
    shaderTests(CK);
    shapedTextTests(CK);
    surfaceTests(CK);
    textBlobTests(CK);
    vectorTests(CK);
    verticesTests(CK);
});

function animatedImageTests(CK: CanvasKit) {
    const buff = new ArrayBuffer(10);
    const img = CK.MakeAnimatedImageFromEncoded(buff); // $ExpectType AnimatedImage | null
    if (!img) return;
    const n = img.decodeNextFrame(); // $ExpectType number
    const f = img.getFrameCount(); // $ExpectType number
    const r = img.getRepetitionCount(); // $ExpectType number
    const h = img.height(); // $ExpectType number
    const still = img.makeImageAtCurrentFrame(); // $ExpectType Image | null
    img.reset();
    const w = img.width(); // $ExpectType number
}

// In an effort to keep these type-checking tests easy to read and understand, we can "inject"
// types instead of actually having to create them from scratch. To inject them, we define them
// as an optional parameter and then have a null check to make sure that optional-ness does not
// cause errors.
function canvasTests(CK: CanvasKit, canvas?: Canvas, paint?: Paint, path?: Path,
                     img?: Image, aImg?: AnimatedImage, para?: Paragraph,
                     skp?: SkPicture, font?: Font, shapedText?: ShapedText,
                     textBlob?: TextBlob, verts?: Vertices, imageInfo?: ImageInfo,
                     imgFilter?: ImageFilter) {
    if (!canvas || !paint || !path || !img || !aImg || !para || !skp || !font ||
        !shapedText || !textBlob || !verts || !imageInfo || !imgFilter) {
        return;
    }
    const someColor = [0.9, 0.8, 0.7, 0.6]; // Making sure arrays are accepted as colors.
    const someRect = [4, 3, 2, 1]; // Making sure arrays are accepted as rects.
    // Making sure arrays are accepted as rrects.
    const someRRect = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];
    const someMatrix = CK.Malloc(Float32Array, 16); // Make sure matrixes can be malloc'd.

    canvas.clear(CK.RED);
    canvas.clipPath(path, CK.ClipOp.Intersect, false);
    canvas.clipRect(someRect, CK.ClipOp.Intersect, true);
    canvas.clipRRect(CK.RRectXY(someRect, 10, 20), CK.ClipOp.Difference, true);
    canvas.concat([1, 0, 0, 0, 1, 0, 0, 0, 1]);
    canvas.concat(someMatrix);
    canvas.drawArc(someRect, 0, 90, true, paint);
    canvas.drawAtlas(img, [1, 2, 3, 4, 5, 6, 7, 8], [8, 7, 6, 5, 4, 3, 2, 1], paint);
    canvas.drawAtlas(img, [1, 2, 3, 4, 5, 6, 7, 8], [8, 7, 6, 5, 4, 3, 2, 1], paint,
                     CK.BlendMode.Darken,
                     [CK.ColorAsInt(100, 110, 120), CK.ColorAsInt(130, 140, 150)]);
    canvas.drawCircle(20, 20, 20, paint);
    canvas.drawColor(someColor);
    canvas.drawColor(someColor, CK.BlendMode.ColorDodge);
    canvas.drawColorComponents(0.2, 1.0, -0.02, 0.5);
    canvas.drawColorComponents(0.2, 1.0, -0.02, 0.5, CK.BlendMode.ColorDodge);
    canvas.drawColorInt(CK.ColorAsInt(100, 110, 120));
    canvas.drawColorInt(CK.ColorAsInt(100, 110, 120), CK.BlendMode.ColorDodge);
    canvas.drawDRRect(someRRect, CK.RRectXY(someRect, 10, 20), paint);
    canvas.drawImage(img, 0, -43);
    canvas.drawImage(img, 0, -43, paint);
    canvas.drawImageAtCurrentFrame(aImg, 0, -43);
    canvas.drawImageAtCurrentFrame(aImg, 0, -43, paint);
    canvas.drawImageCubic(img, 0, -43, 1 / 3, 1 / 4, null);
    canvas.drawImageOptions(img, 0, -43, CK.FilterMode.Nearest, CK.MipmapMode.Nearest, paint);
    canvas.drawImageNine(img, someRect, someRect, CK.FilterMode.Nearest);
    canvas.drawImageNine(img, CK.XYWHiRect(10, 20, 40, 40), someRect, CK.FilterMode.Linear, paint);
    canvas.drawImageRect(img, someRect, someRect, paint);
    canvas.drawImageRect(img, CK.XYWHRect(90, 90, 40, 40), someRect, paint);
    canvas.drawImageRect(img, someRect, someRect, paint, true);
    canvas.drawImageRectCubic(img, someRect, someRect, 1 / 5, 1 / 6);
    canvas.drawImageRectCubic(img, someRect, someRect, 1 / 5, 1 / 6, paint);
    canvas.drawImageRectOptions(img, someRect, someRect, CK.FilterMode.Linear, CK.MipmapMode.None);
    canvas.drawImageRectOptions(img, someRect, someRect, CK.FilterMode.Linear, CK.MipmapMode.None, paint);
    canvas.drawLine(1, 2, 3, 4, paint);
    canvas.drawOval(someRect, paint);
    canvas.drawPaint(paint);
    canvas.drawParagraph(para, 10, 7);
    canvas.drawPath(path, paint);
    canvas.drawPicture(skp);
    canvas.drawPoints(CK.PointMode.Lines, [1, 2, 3, 4, 5, 6], paint);
    canvas.drawRect(someRect, paint);
    canvas.drawRect4f(5, 6, 7, 8, paint);
    canvas.drawRRect(someRRect, paint);
    canvas.drawShadow(path, [1, 2, 3], [4, 5, 6], 7, someColor, CK.BLUE, 0);
    canvas.drawText('foo', 1, 2, paint, font);
    canvas.drawText(shapedText, 1, 2, paint, font);
    canvas.drawTextBlob(textBlob, 10, 20, paint);
    canvas.drawVertices(verts, CK.BlendMode.DstOut, paint);
    const matrOne = canvas.findMarkedCTM('thing'); // $ExpectType Float32Array | null
    const matrTwo = canvas.getLocalToDevice(); // $ExpectType Float32Array
    const sc = canvas.getSaveCount(); // $ExpectType number
    const matrThree = canvas.getTotalMatrix(); // $ExpectType number[]
    const surface = canvas.makeSurface(imageInfo); // $ExpectType Surface | null
    canvas.markCTM('more ctm');
    const pixels = canvas.readPixels(85, 1000, {// $Uint8Array | Float32Array | null
        width: 79,
        height: 205,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Unpremul,
        colorSpace: CK.ColorSpace.SRGB,
    });
    const m = CK.Malloc(Uint8Array, 10);
    img.readPixels(85, 1000, {
        width: 79,
        height: 205,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Unpremul,
        colorSpace: CK.ColorSpace.SRGB,
    }, m, 4 * 85);
    canvas.restore();
    canvas.restoreToCount(2);
    canvas.rotate(1, 2, 3);
    const height = canvas.save(); // $ExpectType number
    const h2 = canvas.saveLayer(); // $ExpectType number
    const h3 = canvas.saveLayer(paint); // $ExpectType number
    const h4 = canvas.saveLayer(paint, someRect);
    const h5 = canvas.saveLayer(paint, someRect, imgFilter, CK.SaveLayerF16ColorType);
    const h6 = canvas.saveLayer(paint, someRect, null, CK.SaveLayerInitWithPrevious);
    canvas.scale(5, 10);
    canvas.skew(10, 5);
    canvas.translate(20, 30);
    const ok = canvas.writePixels([1, 2, 3, 4], 1, 1, 10, 20); // $ExpectType boolean
    const ok2 = canvas.writePixels([1, 2, 3, 4], 1, 1, 10, 20, CK.AlphaType.Premul,
                                   CK.ColorType.Alpha_8, CK.ColorSpace.DISPLAY_P3);
}

function canvas2DTests(CK: CanvasKit) {
    const bytes = new ArrayBuffer(10);

    const canvas = CK.MakeCanvas(100, 200);
    const img = canvas.decodeImage(bytes);
    const ctx = canvas.getContext('2d');
    ctx!.lineTo(2, 4);
    canvas.loadFont(bytes, {
        family: 'BungeeNonSystem',
        style: 'normal',
        weight: '400',
    });
    const p2d = canvas.makePath2D();
    p2d.quadraticCurveTo(1, 2, 3, 4);
    const iData = new CK.ImageData(40, 50);
    const imgStr = canvas.toDataURL();
}

function colorTests(CK: CanvasKit) {
    const colorOne = CK.Color(200, 200, 200, 0.8); // $ExpectType Float32Array
    const colorTwo = CK.Color4f(0.8, 0.8, 0.8, 0.7); // $ExpectType Float32Array
    const colorThree = CK.ColorAsInt(240, 230, 220); // $ExpectType number
    const colorFour = CK.parseColorString('#887766'); // $ExpectType Float32Array
    const colors = CK.computeTonalColors({ // $ExpectType TonalColorsOutput
        ambient: colorOne,
        spot: [0.2, 0.4, 0.6, 0.8],
    });

    // Deprecated Color functions
    const [r, g, b, a] = CK.getColorComponents(colorTwo);
    const alphaChanged = CK.multiplyByAlpha(colorOne, 0.1);
}

function colorFilterTests(CK: CanvasKit) {
    const cf = CK.ColorFilter; // less typing
    const filterOne = cf.MakeBlend(CK.CYAN, CK.BlendMode.ColorBurn); // $ExpectType ColorFilter
    const filterTwo = cf.MakeLinearToSRGBGamma(); // $ExpectType ColorFilter
    const filterThree = cf.MakeSRGBToLinearGamma(); // $ExpectType ColorFilter
    const filterFour = cf.MakeCompose(filterOne, filterTwo); // $ExpectType ColorFilter
    const filterFive = cf.MakeLerp(0.7, filterThree, filterFour); // $ExpectType ColorFilter

    const r = CK.ColorMatrix.rotated(0, .707, -.707);  // $ExpectType Float32Array
    const b = CK.ColorMatrix.rotated(2, .5, .866);
    const s = CK.ColorMatrix.scaled(0.9, 1.5, 0.8, 0.8);
    let cm = CK.ColorMatrix.concat(r, s);
    cm = CK.ColorMatrix.concat(cm, b);
    CK.ColorMatrix.postTranslate(cm, 20, 0, -10, 0);

    const filterSix = CK.ColorFilter.MakeMatrix(cm); // $ExpectType ColorFilter
}

function contourMeasureTests(CK: CanvasKit, path?: Path) {
    if (!path) return;
    const iter = new CK.ContourMeasureIter(path, true, 2); // $ExpectType ContourMeasureIter
    const contour = iter.next(); // $ExpectType ContourMeasure | null
    if (!contour) return;
    const pt = contour.getPosTan(2); // $ExpectType PosTan
    const segment = contour.getSegment(0, 20, true); // $ExpectType Path
    const closed = contour.isClosed(); // $ExpectType boolean
    const length = contour.length(); // $ExpectType number
}

function imageTests(CK: CanvasKit, imgElement?: HTMLImageElement) {
    if (!imgElement) return;
    const buff = new ArrayBuffer(10);
    const img = CK.MakeImageFromEncoded(buff); // $ExpectType Image | null
    const img2 = CK.MakeImageFromCanvasImageSource(imgElement); // $ExpectType Image
    const img3 = CK.MakeImage({ // $ExpectType Image | null
      width: 1,
      height: 1,
      alphaType: CK.AlphaType.Premul,
      colorType: CK.ColorType.RGBA_8888,
      colorSpace: CK.ColorSpace.SRGB
    }, Uint8Array.of(255, 0, 0, 250), 4);
    if (!img) return;
    const dOne = img.encodeToData(); // $ExpectType Data
    const dTwo = img.encodeToDataWithFormat(CK.ImageFormat.JPEG, 97);
    const bytes = CK.getDataBytes(dTwo); // $ExpectType Uint8Array
    const h = img.height();
    const w = img.width();
    const s1 = img.makeShaderCubic(CK.TileMode.Decal, CK.TileMode.Repeat, 1 / 3, 1 / 3); // $ExpectType Shader
    const mm = img.makeCopyWithDefaultMipmaps(); // $ExpectType Image
    const s2 = mm.makeShaderOptions(CK.TileMode.Decal, CK.TileMode.Repeat, // $ExpectType Shader
        CK.FilterMode.Nearest, CK.MipmapMode.Linear,
        CK.Matrix.identity());
    const pixels = img.readPixels(85, 1000, { // $ExpectType Float32Array | Uint8Array | null
        width: 79,
        height: 205,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Unpremul,
        colorSpace: CK.ColorSpace.SRGB,
    });
    const m = CK.Malloc(Uint8Array, 10);
    img.readPixels(85, 1000, {
        width: 79,
        height: 205,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Unpremul,
        colorSpace: CK.ColorSpace.SRGB,
    }, m, 4 * 85);
    const ii = img.getImageInfo(); // $ExpectType PartialImageInfo
    const cs = img.getColorSpace(); // $ExpectType ColorSpace
    cs.delete();
    img.delete();
}

function imageFilterTests(CK: CanvasKit, colorFilter?: ColorFilter) {
    if (!colorFilter) return;
    const imgf = CK.ImageFilter; // less typing
    const filter = imgf.MakeBlur(2, 4, CK.TileMode.Mirror, null); // $ExpectType ImageFilter
    const filter1 = imgf.MakeBlur(2, 4, CK.TileMode.Decal, filter); // $ExpectType ImageFilter
    const filter2 = imgf.MakeColorFilter(colorFilter, null); // $ExpectType ImageFilter
    const filter3 = imgf.MakeColorFilter(colorFilter, filter); // $ExpectType ImageFilter
    const filter4 = imgf.MakeCompose(null, filter2); // $ExpectType ImageFilter
    const filter5 = imgf.MakeCompose(filter3, null); // $ExpectType ImageFilter
    const filter6 = imgf.MakeCompose(filter4, filter2); // $ExpectType ImageFilter
    const filter7 = imgf.MakeMatrixTransform(CK.Matrix.scaled(2, 3, 10, 10),
                                             CK.FilterQuality.High, null);
    const filter8 = imgf.MakeMatrixTransform(CK.M44.identity(),
                                             CK.FilterQuality.None, filter6);
}

function fontTests(CK: CanvasKit, face?: Typeface, paint?: Paint) {
    if (!face || !paint) return;
    const font = new CK.Font(); // $ExpectType Font
    const f2 = new CK.Font(face); // $ExpectType Font
    const f3 = new CK.Font(null); // $ExpectType Font
    const f4 = new CK.Font(face, 20); // $ExpectType Font
    const f5 = new CK.Font(null, 20); // $ExpectType Font
    const f6 = new CK.Font(null, 20, 2, 3); // $ExpectType Font
    const f7 = new CK.Font(face, 20, 4, 5); // $ExpectType Font

    const glyphMalloc = CK.MallocGlyphIDs(20);
    const someGlyphs = [1, 2, 3, 4, 5];

    const glyphBounds = font.getGlyphBounds(glyphMalloc, paint); // $ExpectType Float32Array
    font.getGlyphBounds(someGlyphs, null, glyphBounds);

    const ids = font.getGlyphIDs('abcd');
    font.getGlyphIDs('efgh', 4, ids);

    const widths = font.getGlyphWidths(glyphMalloc, paint);
    font.getGlyphWidths(someGlyphs, null, widths);

    font.getScaleX();
    font.getSize();
    font.getSkewX();
    font.getTypeface();
    const w2 = font.getWidths('abcdefg'); // $ExpectType number[]
    const w = font.measureText('abc'); // $ExpectType number
    font.setEdging(CK.FontEdging.Alias);
    font.setEmbeddedBitmaps(true);
    font.setHinting(CK.FontHinting.Slight);
    font.setLinearMetrics(true);
    font.setScaleX(5);
    font.setSize(15);
    font.setSkewX(2);
    font.setSubpixel(true);
    font.setTypeface(null);
    font.setTypeface(face);
}

function fontMgrTests(CK: CanvasKit) {
    const fm = CK.FontMgr.RefDefault(); // $ExpectType FontMgr

    const buff1 = new ArrayBuffer(10);
    const buff2 = new ArrayBuffer(20);

    const fm2 = CK.FontMgr.FromData(buff1, buff2);
    fm.countFamilies();
    fm.getFamilyName(0);

    const tf = fm.makeTypefaceFromData(buff1); // $ExpectType Typeface
}

function globalTests(CK: CanvasKit, path?: Path) {
    if (!path) {
        return;
    }
    const ctx = CK.currentContext();
    CK.setCurrentContext(ctx);
    CK.deleteContext(ctx);
    const n = CK.getDecodeCacheLimitBytes();
    const u = CK.getDecodeCacheUsedBytes();
    CK.setDecodeCacheLimitBytes(1000);
    const matr = CK.Matrix.rotated(Math.PI / 6);
    const p = CK.getShadowLocalBounds(matr, path, [0, 0, 1], [500, 500, 20], 20,
        CK.ShadowDirectionalLight | CK.ShadowGeometricOnly | CK.ShadowDirectionalLight);
}

function paintTests(CK: CanvasKit, colorFilter?: ColorFilter, imageFilter?: ImageFilter,
                    maskFilter?: MaskFilter, pathEffect?: PathEffect, shader?: Shader) {
    if (!colorFilter || !colorFilter || !imageFilter || !maskFilter || !pathEffect || !shader) {
        return;
    }
    const paint = new CK.Paint(); // $ExpectType Paint
    const newPaint = paint.copy(); // $ExpectType Paint
    const bm = paint.getBlendMode();
    const color = paint.getColor(); // $ExpectType Float32Array
    const sc = paint.getStrokeCap();
    const sj = paint.getStrokeJoin();
    const limit = paint.getStrokeMiter(); // $ExpectType number
    const width = paint.getStrokeWidth(); // $ExpectType number
    paint.setAlphaf(0.8);
    paint.setAntiAlias(true);
    paint.setBlendMode(bm);
    paint.setColor(CK.RED);
    paint.setColor([0, 0, 1.2, 0.5], CK.ColorSpace.DISPLAY_P3);
    paint.setColorComponents(0, 0, 0.9, 1.0);
    paint.setColorComponents(0, 0, 1.2, 0.5, CK.ColorSpace.DISPLAY_P3);
    paint.setColorFilter(colorFilter);
    paint.setColorInt(CK.ColorAsInt(20, 30, 40));
    paint.setColorInt(CK.ColorAsInt(20, 30, 40), CK.ColorSpace.SRGB);
    paint.setImageFilter(imageFilter);
    paint.setMaskFilter(maskFilter);
    paint.setPathEffect(pathEffect);
    paint.setShader(shader);
    paint.setStrokeCap(CK.StrokeCap.Round);
    paint.setStrokeJoin(CK.StrokeJoin.Miter);
    paint.setStrokeMiter(10);
    paint.setStrokeWidth(20);
    paint.setStyle(CK.PaintStyle.Fill);
    paint.delete();
}

function pathTests(CK: CanvasKit) {
    const path = new CK.Path();  // $ExpectType Path
    const p2 = CK.Path.MakeFromCmds([ // $ExpectType Path | null
        [CK.MOVE_VERB, 0, 10],
        [CK.LINE_VERB, 30, 40],
        [CK.QUAD_VERB, 20, 50, 45, 60],
    ]);
    const verbs = CK.Malloc(Uint8Array, 10);
    const points = CK.Malloc(Float32Array, 10);
    const p3 = CK.Path.MakeFromVerbsPointsWeights(verbs, [1, 2, 3, 4]); // $ExpectType Path
    const p4 = CK.Path.MakeFromVerbsPointsWeights([CK.CONIC_VERB], points, [2.3]);
    const p5 = CK.Path.MakeFromOp(p4, p2!, CK.PathOp.ReverseDifference); // $ExpectType Path | null
    const p6 = CK.Path.MakeFromSVGString('M 205,5 L 795,5 z'); // $ExpectType Path | null

    const someRect = CK.LTRBRect(10, 20, 30, 40);
    // Making sure arrays are accepted as rrects.
    const someRRect = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12];

    path.addArc(someRect, 0, 270);
    path.addOval(someRect);
    path.addOval(someRect, true, 3);
    path.addPath(p2);
    path.addPoly([20, 20,  40, 40,  20, 40], true);
    path.addRect(someRect);
    path.addRect(someRect, true);
    path.addRRect(someRRect);
    path.addRRect(someRRect, true);
    path.addVerbsPointsWeights(verbs, [1, 2, 3, 4]);
    path.addVerbsPointsWeights([CK.CONIC_VERB], points, [2.3]);
    path.arc(0, 0, 10, 0, Math.PI / 2);
    path.arc(0, 0, 10, 0, Math.PI / 2, true);
    path.arcToOval(someRect, 15, 60, true);
    path.arcToRotated(2, 4, 90, false, true, 0, 20);
    path.arcToTangent(20, 20, 40, 50, 2);
    path.close();
    let bounds = path.computeTightBounds(); // $ExpectType Float32Array
    path.computeTightBounds(bounds);
    path.conicTo(1, 2, 3, 4, 5);
    let ok = path.contains(10, 20); // $ExpectType boolean
    const pCopy = path.copy(); // $ExpectType Path
    const count = path.countPoints(); // $ExpectType number
    path.cubicTo(10, 10, 10, 10, 10, 10);
    ok = path.dash(8, 4, 1);
    ok = path.equals(pCopy);
    bounds = path.getBounds(); // $ExpectType Float32Array
    path.getBounds(bounds);
    const ft = path.getFillType();
    const pt = path.getPoint(7); // $ExpectType Point
    ok = path.isEmpty();
    ok = path.isVolatile();
    path.lineTo(10, -20);
    path.moveTo(-20, -30);
    path.offset(100, 100);
    ok = path.op(p2!, CK.PathOp.Difference);
    path.quadTo(10, 20, 30, 40);
    path.rArcTo(10, 10, 90, false, true, 2, 4);
    path.rConicTo(-1, 2, 4, 9, 3);
    path.rCubicTo(20, 30, 40, 50, 2, 1);
    path.reset();
    path.rewind();
    path.rLineTo(20, 30);
    path.rMoveTo(40, 80);
    path.rQuadTo(1, 2, 3, 4);
    path.setFillType(CK.FillType.EvenOdd);
    path.setIsVolatile(true);
    ok = path.simplify();
    path.stroke();
    path.stroke({});
    path.stroke({
        width: 20,
        miter_limit: 9,
        precision: 0.5,
        cap: CK.StrokeCap.Butt,
        join: CK.StrokeJoin.Miter,
    });
    const cmds = path.toCmds(); // $ExpectType PathCommand[]
    const str = path.toSVGString(); // $ExpectType string
    path.transform(CK.Matrix.identity());
    path.transform(1, 0, 0, 0, 1, 0, 0, 0, 1);
    path.trim(0.1, 0.7, false);
}

function paragraphTests(CK: CanvasKit, p?: Paragraph) {
    if (!p) return;
    const a = p.didExceedMaxLines(); // $ExpectType boolean
    const b = p.getAlphabeticBaseline(); // $ExpectType number
    const c = p.getGlyphPositionAtCoordinate(10, 3); // $ExpectType PositionWithAffinity
    const d = p.getHeight(); // $ExpectType number
    const e = p.getIdeographicBaseline(); // $ExpectType number
    const f = p.getLongestLine(); // $ExpectType number
    const g = p.getMaxIntrinsicWidth(); // $ExpectType number
    const h = p.getMaxWidth(); // $ExpectType number
    const i = p.getMinIntrinsicWidth(); // $ExpectType number
    const j = p.getRectsForPlaceholders(); // $ExpectType Float32Array
    const k = p.getRectsForRange(2, 10, CK.RectHeightStyle.Max,  // $ExpectType Float32Array
        CK.RectWidthStyle.Tight);
    const l = p.getWordBoundary(10); // $ExpectType URange
    p.layout(300);
    const m = p.getLineMetrics(); // $ExpectType LineMetrics[]
}

function paragraphBuilderTests(CK: CanvasKit, fontMgr?: FontMgr, paint?: Paint) {
    if (!fontMgr || !paint) return;
    const paraStyle = new CK.ParagraphStyle({ // $ExpectType ParagraphStyle
        textStyle: {
            color: CK.BLACK,
            fontFamilies: ['Noto Serif'],
            fontSize: 20,
        },
        textAlign: CK.TextAlign.Center,
        maxLines: 8,
        ellipsis: '.._.',
        strutStyle: {
            strutEnabled: true,
            fontFamilies: ['Roboto'],
            fontSize: 28,
            heightMultiplier: 1.5,
            forceStrutHeight: true,
        },
    });
    const blueText = new CK.TextStyle({ // $ExpectType TextStyle
        backgroundColor: CK.Color(234, 208, 232), // light pink
        color: CK.Color(48, 37, 199),
        fontFamilies: ['Noto Serif'],
        decoration: CK.LineThroughDecoration,
        decorationThickness: 1.5, // multiplier based on font size
        fontSize: 24,
        fontFeatures: [{name: 'smcp', value: 1}],
        shadows: [{color: CK.BLACK, blurRadius: 15},
                  {color: CK.RED, blurRadius: 5, offset: [10, 10]}],
    });

    const builder = CK.ParagraphBuilder.Make(paraStyle, fontMgr); // $ExpectType ParagraphBuilder

    builder.pushStyle(blueText);
    builder.addText('VAVAVAVAVAVAVA\nVAVA\n');
    builder.pop();
    const paragraph = builder.build(); // $ExpectType Paragraph

    const buf = new ArrayBuffer(10);
    const fontSrc = CK.TypefaceFontProvider.Make(); // $ExpectType TypefaceFontProvider
    fontSrc.registerFont(buf, 'sans-serif');
    const builder2 = CK.ParagraphBuilder.MakeFromFontProvider(// $ExpectType ParagraphBuilder
                                paraStyle, fontSrc);
    builder2.pushPaintStyle(blueText, paint, paint);
    builder2.addPlaceholder();
    builder2.addPlaceholder(10, 20, CK.PlaceholderAlignment.Top, CK.TextBaseline.Ideographic, 3);
}

function particlesTests(CK: CanvasKit, canvas?: Canvas) {
    if (!canvas) return;

    const par = CK.MakeParticles('some json'); // $ExpectType Particles
    par.draw(canvas);
    par.effectUniforms()[0] = 1.2;
    const a = par.getEffectUniform(1); // $ExpectType ParticlesUniform
    const b = par.getEffectUniformCount(); // $ExpectType number
    const c = par.getEffectUniformFloatCount(); // $ExpectType number
    const d = par.getEffectUniformName(3); // $ExpectType string
    const e = par.getParticleUniform(3); // $ExpectType ParticlesUniform
    const f = par.getParticleUniformCount(); // $ExpectType number
    const g = par.getParticleUniformFloatCount(); // $ExpectType number
    const h = par.getParticleUniformName(3); // $ExpectType string
    par.particleUniforms()[2] = 4.5;
    par.setPosition([3, 5]);
    par.setRate(3);
    par.start(0, true);
    par.update(2);

    const buff = new ArrayBuffer(10);
    const par2 = CK.MakeParticles('other json', { // $ExpectType Particles
        'flightAnim.gif': buff,
    });
}

function pathEffectTests(CK: CanvasKit) {
    const pe1 = CK.PathEffect.MakeCorner(2); // $ExpectType PathEffect | null
    const pe2 = CK.PathEffect.MakeDash([2, 4]); // $ExpectType PathEffect
    const pe3 = CK.PathEffect.MakeDash([2, 4, 6, 8], 10); // $ExpectType PathEffect
    const pe4 = CK.PathEffect.MakeDiscrete(10, 2, 0); // $ExpectType PathEffect
}

function mallocTests(CK: CanvasKit) {
    const mFoo = CK.Malloc(Float32Array, 5);
    const mArray = mFoo.toTypedArray(); // $ExpectType TypedArray
    mArray[3] = 1.7;
    const mSubArray = mFoo.subarray(0, 2); // $ExpectType TypedArray
    mSubArray[0] = 2;
    CK.Free(mFoo);
}

function maskFilterTests(CK: CanvasKit) {
    const mf = CK.MaskFilter.MakeBlur(CK.BlurStyle.Solid, 8, false); // $ExpectType MaskFilter
}

function matrixTests(CK: CanvasKit) {
    const m33 = CK.Matrix; // less typing
    const matrA = m33.identity(); // $ExpectType number[]
    const matrB = m33.rotated(0.1); // $ExpectType number[]
    const matrC = m33.rotated(0.1, 15, 20); // $ExpectType number[]
    const matrD = m33.multiply(matrA, matrB); // $ExpectType number[]
    const matrE = m33.multiply(matrA, matrB, matrC, matrB, matrA); // $ExpectType number[]
    const matrF = m33.scaled(1, 2); // $ExpectType number[]
    const matrG = m33.scaled(1, 2, 3, 4); // $ExpectType number[]
    const matrH = m33.skewed(1, 2); // $ExpectType number[]
    const matrI = m33.skewed(1, 2, 3, 4); // $ExpectType number[]
    const matrJ = m33.translated(1, 2); // $ExpectType number[]
    const matrK = m33.invert(matrJ);

    const m44 = CK.M44;
    const matr1 = m44.identity(); // $ExpectType number[]
    const matr2 = m44.invert(matr1);
    const matr3 = m44.lookat([1, 2, 3], [4, 5, 6], [7, 8, 9]); // $ExpectType number[]
    const matr4 = m44.multiply(matr1, matr3); // $ExpectType number[]
    const matr5 = m44.mustInvert(matr1); // $ExpectType number[]
    const matr6 = m44.perspective(1, 8, 0.4); // $ExpectType number[]
    const matr7 = m44.rc(matr6, 0, 3); // $ExpectType number
    const matr8 = m44.rotated([2, 3, 4], -0.4); // $ExpectType number[]
    const matr9 = m44.rotatedUnitSinCos([4, 3, 2], 0.9, 0.1); // $ExpectType number[]
    const matr10 = m44.scaled([5, 5, 5]); // $ExpectType number[]
    const matr11 = m44.setupCamera(CK.LTRBRect(1, 2, 3, 4), 0.4, {
        eye: [0, 0, 1],
        coa: [0, 0, 0],
        up:  [0, 1, 0],
        near: 0.2,
        far: 4,
        angle: Math.PI / 12,
    });
    const matr12 = m44.translated([3, 2, 1]); // $ExpectType number[]
    const matr13 = m44.transpose([4, 5, 8]); // $ExpectType number[]
}

function pictureTests(CK: CanvasKit) {
    const recorder = new CK.PictureRecorder(); // $ExpectType PictureRecorder
    const canvas = recorder.beginRecording(CK.LTRBRect(0, 0, 100, 100));  // $ExpectType Canvas
    const pic = recorder.finishRecordingAsPicture(); // $ExpectType SkPicture
    const data = pic.serialize(); // $ExpectType Data
    const bytes = CK.getDataBytes(data);
    const pic2 = CK.MakePicture(bytes);
}

function rectangleTests(CK: CanvasKit) {
    const rectOne = CK.LTRBRect(5, 10, 20, 30); // $ExpectType Float32Array
    const rectTwo = CK.XYWHRect(5, 10, 15, 20); // $ExpectType Float32Array
    const iRectOne = CK.LTRBiRect(105, 110, 120, 130); // $ExpectType Int32Array
    const iRectTwo = CK.XYWHiRect(105, 110, 15, 20); // $ExpectType Int32Array
    const rrectOne = CK.RRectXY(rectOne, 3, 7);  // $ExpectType Float32Array
}

function runtimeEffectTests(CK: CanvasKit) {
    const rt = CK.RuntimeEffect.Make('not real sksl code'); // $ExpectType RuntimeEffect | null
    if (!rt) return;
    const someMatr = CK.Matrix.translated(2, 60);
    const s1 = rt.makeShader([0, 1]); // $ExpectType Shader
    const s2 = rt.makeShader([0, 1], true, someMatr); // $ExpectType Shader
    const s3 = rt.makeShaderWithChildren([4, 5], true, [s1, s2]); // $ExpectType Shader
    const s4 = rt.makeShaderWithChildren([4, 5], true, [s1, s2], someMatr); // $ExpectType Shader
}

function skottieTests(CK: CanvasKit, canvas?: Canvas) {
    if (!canvas) return;

    const anim = CK.MakeAnimation('some json'); // $ExpectType SkottieAnimation
    const a = anim.duration(); // $ExpectType number
    const b = anim.fps(); // $ExpectType number
    const c = anim.version(); // $ExpectType string
    const d = anim.size(); // $ExpectType Point
    const rect = anim.seek(0.5);
    anim.seek(0.6, rect);
    const rect2 = anim.seekFrame(12.3);
    anim.seekFrame(12.3, rect2);
    anim.render(canvas);
    anim.render(canvas, rect);

    const buff = new ArrayBuffer(10);
    const mAnim = CK.MakeManagedAnimation('other json', { // $ExpectType ManagedSkottieAnimation
        'flightAnim.gif': buff,
    });
    mAnim.setColor('slider', CK.WHITE);
    mAnim.setOpacity('slider', 0.8);
    const e = mAnim.getMarkers();  // $ExpectType object[]
    const f = mAnim.getColorProps();  // $ExpectType object[]
    const g = mAnim.getOpacityProps();  // $ExpectType object[]
}

function shaderTests(CK: CanvasKit) {
    const s1 = CK.Shader.MakeColor([0.8, 0.2, 0.5, 0.9], // $ExpectType Shader
                                 CK.ColorSpace.SRGB);
    const s2 = CK.Shader.MakeBlend(CK.BlendMode.Src, s1, s1); // $ExpectType Shader
    const s3 = CK.Shader.MakeLerp(0.3, s1, s2); // $ExpectType Shader
    const s4 = CK.Shader.MakeLinearGradient(// $ExpectType Shader
        [0, 0], [50, 100],
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        [0, 0.65, 1.0],
        CK.TileMode.Mirror
    );
    const s5 = CK.Shader.MakeLinearGradient(// $ExpectType Shader
        [0, 0], [50, 100],
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        null,
        CK.TileMode.Clamp,
        CK.Matrix.rotated(Math.PI / 4, 0, 100),
        1,
        CK.ColorSpace.SRGB,
    );
    const s6 = CK.Shader.MakeRadialGradient(// $ExpectType Shader
        [0, 0], 50,
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        [0, 0.65, 1.0],
        CK.TileMode.Decal,
    );
    const s7 = CK.Shader.MakeRadialGradient(// $ExpectType Shader
        [0, 0], 50,
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        null,
        CK.TileMode.Clamp,
        CK.Matrix.skewed(3, -3),
        1,
        CK.ColorSpace.SRGB,
    );
    const s8 = CK.Shader.MakeTwoPointConicalGradient(// $ExpectType Shader
        [0, 0], 20,
        [50, 100], 60,
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        [0, 0.65, 1.0],
        CK.TileMode.Mirror
    );
    const s9 = CK.Shader.MakeTwoPointConicalGradient(// $ExpectType Shader
        [0, 0], 20,
        [50, 100], 60,
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        [0, 0.65, 1.0],
        CK.TileMode.Mirror,
        CK.Matrix.rotated(Math.PI / 4, 0, 100),
        1,
        CK.ColorSpace.SRGB,
    );
    const s10 = CK.Shader.MakeSweepGradient(// $ExpectType Shader
        0, 20,
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        [0, 0.65, 1.0],
        CK.TileMode.Mirror
    );
    const s11 = CK.Shader.MakeSweepGradient(// $ExpectType Shader
        0, 20,
        Float32Array.of(
            0, 1, 0, 0.8,
            1, 0, 0, 1,
            0, 0, 1, 0.5,
        ),
        null,
        CK.TileMode.Mirror,
        CK.Matrix.rotated(Math.PI / 4, 0, 100),
        1,
        15, 275, // start, end angle in degrees.
        CK.ColorSpace.SRGB,
    );
    const s12 = CK.Shader.MakeFractalNoise(0.1, 0.05, 2, 0, 80, 80); // $ExpectType Shader
    const s13 = CK.Shader.MakeTurbulence(0.1, 0.05, 2, 0, 80, 80); // $ExpectType Shader
}

function shapedTextTests(CK: CanvasKit, textFont?: Font) {
    if (!textFont) return;

    const shaped = new CK.ShapedText({ // $ExpectType ShapedText
       font: textFont,
       leftToRight: true,
       text: 'this is shaped',
       width: 20,
    });
    const bounds = shaped.getBounds();
    shaped.getBounds(bounds);
}

function surfaceTests(CK: CanvasKit) {
    const canvasEl = document.querySelector('canvas') as HTMLCanvasElement;
    const surfaceOne = CK.MakeCanvasSurface(canvasEl)!; // $ExpectType Surface
    const surfaceTwo = CK.MakeCanvasSurface('my_canvas')!;
    const surfaceThree = CK.MakeSWCanvasSurface(canvasEl)!; // $ExpectType Surface
    const surfaceFour = CK.MakeSWCanvasSurface('my_canvas')!;
    const surfaceFive = CK.MakeWebGLCanvasSurface(canvasEl, // $ExpectType Surface
        CK.ColorSpace.SRGB, {
        majorVersion: 2,
        preferLowPowerToHighPerformance: 1,
    })!;
    const surfaceSix = CK.MakeWebGLCanvasSurface('my_canvas', CK.ColorSpace.DISPLAY_P3, {
        enableExtensionsByDefault: 2,
    })!;
    const surfaceSeven = CK.MakeSurface(200, 200)!; // $ExpectType Surface
    const m = CK.Malloc(Uint8Array, 5 * 5 * 4);
    const surfaceEight = CK.MakeRasterDirectSurface({
        width: 5,
        height: 5,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Premul,
        colorSpace: CK.ColorSpace.SRGB,
    }, m, 20);

    surfaceOne.flush();
    const canvas = surfaceTwo.getCanvas(); // $ExpectType Canvas
    const ii = surfaceThree.imageInfo(); // $ExpectType ImageInfo
    const h = surfaceFour.height(); // $ExpectType number
    const w = surfaceFive.width(); // $ExpectType number
    const subsurface = surfaceOne.makeSurface(ii); // $ExpectType Surface
    const isGPU = subsurface.reportBackendTypeIsGPU(); // $ExpectType boolean
    const count = surfaceThree.sampleCnt(); // $ExpectType number
    const img = surfaceFour.makeImageSnapshot([0, 3, 2, 5]); // $ExpectType Image
    const img2 = surfaceSix.makeImageSnapshot(); // $ExpectType Image

    surfaceSeven.delete();

    const ctx = CK.GetWebGLContext(canvasEl); // $ExpectType number
    const grCtx = CK.MakeGrContext(ctx);
    const surfaceNine = CK.MakeOnScreenGLSurface(grCtx, 100, 400, // $ExpectType Surface
        CK.ColorSpace.ADOBE_RGB)!;

    const rt = CK.MakeRenderTarget(grCtx, 100, 200); // $ExpectType Surface | null
    const rt2 = CK.MakeRenderTarget(grCtx, { // $ExpectType Surface | null
        width: 79,
        height: 205,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Premul,
        colorSpace: CK.ColorSpace.SRGB,
    });
}

function textBlobTests(CK: CanvasKit, font?: Font, path?: Path) {
    if (!font || !path) return;
    const tb = CK.TextBlob; // less typing
    const ids = font.getGlyphIDs('abc');
    const mXforms = CK.Malloc(Float32Array, ids.length * 4);

    const blob = tb.MakeFromGlyphs([5, 6, 7, 8], font); // $ExpectType TextBlob
    const blob1 = tb.MakeFromGlyphs(ids, font); // $ExpectType TextBlob
    const blob2 = tb.MakeFromRSXform('cdf', mXforms, font); // $ExpectType TextBlob
    const blob3 = tb.MakeFromRSXform('c', [-1, 0, 2, 3], font); // $ExpectType TextBlob
    const blob4 = tb.MakeFromRSXformGlyphs([3, 6], mXforms, font); // $ExpectType TextBlob
    const blob5 = tb.MakeFromRSXformGlyphs(ids, [-1, 0, 2, 3], font); // $ExpectType TextBlob
    const blob6 = tb.MakeFromText('xyz', font); // $ExpectType TextBlob
    const blob7 = tb.MakeOnPath('tuv', path, font); // $ExpectType TextBlob
    const blob8 = tb.MakeOnPath('tuv', path, font, 10); // $ExpectType TextBlob
}

function vectorTests(CK: CanvasKit) {
    const a = [1, 2, 3];
    const b = [4, 5, 6];

    const vec = CK.Vector; // less typing
    const v1 = vec.add(a, b); // $ExpectType VectorN
    const v2 = vec.cross(a, b); // $ExpectType Vector3
    const n1 = vec.dist(a, b); // $ExpectType number
    const n2 = vec.dot(a, b); // $ExpectType number
    const n3 = vec.length(a); // $ExpectType number
    const n4 = vec.lengthSquared(a); // $ExpectType number
    const v3 = vec.mulScalar(a, 10); // $ExpectType VectorN
    const v4 = vec.normalize(a); // $ExpectType VectorN
    const v5 = vec.sub(a, b); // $ExpectType VectorN
}

function verticesTests(CK: CanvasKit) {
    const points = [
         70, 170,   40, 90,  130, 150,  100, 50,
        225, 150,  225, 60,  310, 180,  330, 100,
    ];
    const textureCoordinates = [
          0, 240,    0, 0,   80, 240,   80, 0,
        160, 240,  160, 0,  240, 240,  240, 0,
    ];
    const vertices = CK.MakeVertices(CK.VertexMode.TrianglesStrip, // $ExpectType Vertices
        points, textureCoordinates);

    const points2 = new Float32Array(points);
    // 1d float color array
    const colors = Float32Array.of(
        1, 0, 0, 1, // red
        0, 1, 0, 1, // green
        0, 0, 1, 1, // blue
        1, 0, 1, 1); // purple
    const vertices2 = CK.MakeVertices(CK.VertexMode.TriangleFan,
        points2, null, colors, null, true);

    const rect = vertices.bounds(); // $ExpectType Float32Array
    vertices.bounds(rect);
    const id = vertices.uniqueID(); // $ExpectType number
}
