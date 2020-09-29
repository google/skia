// This file is type-checked by the Typescript definitions. It is not actually executed.

import CanvasKitInit from "canvaskit-wasm/bin/canvaskit";
import {
    CanvasKit,
    Paragraph,
    ShapedText,
    SkAnimatedImage,
    SkCanvas,
    SkImage,
    SkImageInfo,
    SkFont,
    SkSurface,
    SkPaint,
    SkPath,
    SkPicture,
    SkTextBlob,
    SkVertices,
    TypedArray,
} from "canvaskit-wasm";

CanvasKitInit({locateFile: (file: string) => '/node_modules/canvaskit/bin/' + file}).then((CK: CanvasKit) => {
    canvasTests(CK);
    colorTests(CK);
    imageTests(CK);
    mallocTests(CK);
    surfaceTests(CK);
    rectangleTests(CK);
});

// In an effort to keep these type-checking tests easy to read and understand, we can "inject"
// types instead of actually having to create them from scratch. To inject them, we define them
// as an optional parameter and then have a null check to make sure that optional-ness does not
// cause errors.
function canvasTests(CK: CanvasKit, canvas?: SkCanvas, paint?: SkPaint, path?: SkPath,
                     img?: SkImage, aImg?: SkAnimatedImage, para?: Paragraph,
                     skp?: SkPicture, font?: SkFont, shapedText?: ShapedText,
                     textBlob?: SkTextBlob, verts?: SkVertices, imageInfo?: SkImageInfo) {
    if (!canvas || !paint || !path || !img || !aImg || !para || !skp || !font ||
        !shapedText || !textBlob || !verts || !imageInfo) return;
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
    canvas.drawImageNine(img, someRect, someRect, paint);
    canvas.drawImageRect(img, someRect, someRect, paint);
    canvas.drawImageRect(img, someRect, someRect, paint, true);
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
    const matrThree = canvas.getTotalMatrix(); // $ExpectType Float32Array
    const surface = canvas.makeSurface(imageInfo); // $ExpectType SkSurface | null
    canvas.markCTM('more ctm');
    const pixels = canvas.readPixels(0, 1, 2, 3); // $ExpectType Uint8Array
    const pixelsTwo = canvas.readPixels(4, 5, 6, 7, CK.AlphaType.Opaque, CK.ColorType.RGBA_1010102,
                                        CK.SkColorSpace.DISPLAY_P3, 16);
    canvas.restore();
    canvas.restoreToCount(2);
    canvas.rotate(1, 2, 3);
    const height = canvas.save(); // $ExpectType number
    const h2 = canvas.saveLayer(); // $ExpectType number
    const h3 = canvas.saveLayer(paint); // $ExpectType number
    const h4 = canvas.saveLayer(paint, someRect); // $ExpectType number
    canvas.scale(5, 10);
    canvas.skew(10, 5);
    canvas.translate(20, 30);
    const ok = canvas.writePixels([1, 2, 3, 4], 1, 1, 10, 20); // $ExpectType boolean
    const ok2 = canvas.writePixels([1, 2, 3, 4], 1, 1, 10, 20, CK.AlphaType.Premul,
                                   CK.ColorType.Alpha_8, CK.SkColorSpace.DISPLAY_P3);
}

function colorTests(CK: CanvasKit) {
    const colorOne = CK.Color(200, 200, 200, 0.8); // $ExpectType Float32Array
    const colorTwo = CK.Color4f(0.8, 0.8, 0.8, 0.7); // $ExpectType Float32Array
    const colorThree = CK.ColorAsInt(240, 230, 220); // $ExpectType number
    const colorFour = CK.parseColorString('#887766'); // $ExpectType Float32Array

    // Deprecated Color functions
    const [r, g, b, a] = CK.getColorComponents(colorTwo);
    const alphaChanged = CK.multiplyByAlpha(colorOne, 0.1);
}

function imageTests(CK: CanvasKit, img?: SkImage) {
    if (!img) return;
    const dOne = img.encodeToData(); // $ExpectType SkData
    const dTwo = img.encodeToDataWithFormat(CK.ImageFormat.JPEG, 97);
    const bytes = CK.getSkDataBytes(dTwo); // $ExpectType Uint8Array
    const h = img.height();
    const w = img.width();
    const shader = img.makeShader(CK.TileMode.Decal, CK.TileMode.Repeat); // $ExpectType SkShader
    const pixels = img.readPixels({
        width: 79,
        height: 205,
        colorType: CK.ColorType.RGBA_8888,
        alphaType: CK.AlphaType.Unpremul,
        colorSpace: CK.SkColorSpace.SRGB,
    }, 85, 1000);
}

function mallocTests(CK: CanvasKit) {
    const mFoo = CK.Malloc(Float32Array, 5);
    const mArray = mFoo.toTypedArray(); // $ExpectType TypedArray
    mArray[3] = 1.7;
    const mSubArray = mFoo.subarray(0, 2); // $ExpectType TypedArray
    mSubArray[0] = 2;
    CK.Free(mFoo);
}

function surfaceTests(CK: CanvasKit) {
    const canvasEl = document.querySelector('canvas') as HTMLCanvasElement;
    const surfaceOne = CK.MakeCanvasSurface(canvasEl)!; // $ExpectType SkSurface
    const surfaceTwo = CK.MakeCanvasSurface('my_canvas')!;
    const surfaceThree = CK.MakeSWCanvasSurface(canvasEl)!; // $ExpectType SkSurface
    const surfaceFour = CK.MakeSWCanvasSurface('my_canvas')!;
    const surfaceFive = CK.MakeWebGLCanvasSurface(canvasEl, // $ExpectType SkSurface
        CK.SkColorSpace.SRGB, {
        majorVersion: 2,
        preferLowPowerToHighPerformance: 1,
    })!;
    const surfaceSix = CK.MakeWebGLCanvasSurface('my_canvas', CK.SkColorSpace.DISPLAY_P3, {
        enableExtensionsByDefault: 2,
    })!;
    const surfaceSeven = CK.MakeSurface(200, 200)!; // $ExpectType SkSurface

    surfaceOne.flush();
    const canvas = surfaceTwo.getCanvas(); // $ExpectType SkCanvas
    const ii = surfaceThree.imageInfo(); // $ExpectType SkImageInfo
    const h = surfaceFour.height(); // $ExpectType number
    const w = surfaceFive.width(); // $ExpectType number
    const subsurface = surfaceOne.makeSurface(ii); // $ExpectType SkSurface
    const isGPU = subsurface.reportBackendTypeIsGPU(); // $ExpectType boolean
    const count = surfaceThree.sampleCnt(); // $ExpectType number
    const img = surfaceFour.makeImageSnapshot([0, 3, 2, 5]); // $ExpectType SkImage
    const img2 = surfaceSix.makeImageSnapshot(); // $ExpectType SkImage
    const pic = surfaceSeven.captureFrameAsSkPicture(// $ExpectType SkPicture
        (_: SkCanvas) => {});

    surfaceSeven.delete();

    const ctx = CK.GetWebGLContext(canvasEl); // $ExpectType number
    const grCtx = CK.MakeGrContext(ctx);
    const surfaceEight = CK.MakeOnScreenGLSurface(grCtx, 100, 400, // $ExpectType SkSurface
        CK.SkColorSpace.ADOBE_RGB)!;
}

function rectangleTests(CK: CanvasKit) {
    const rectOne = CK.LTRBRect(5, 10, 20, 30); // $ExpectType Float32Array
    const rectTwo = CK.XYWHRect(5, 10, 15, 20); // $ExpectType Float32Array
    const iRectOne = CK.LTRBiRect(105, 110, 120, 130); // $ExpectType Int32Array
    const iRectTwo = CK.XYWHiRect(105, 110, 15, 20); // $ExpectType Int32Array
    const rrectOne = CK.RRectXY(rectOne, 3, 7);  // $ExpectType Float32Array
}
