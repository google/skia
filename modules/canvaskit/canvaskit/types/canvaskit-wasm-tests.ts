// This file is type-checked by the Typescript definitions. It is not actually executed.

import CanvasKitInit from "canvaskit-wasm/bin/canvaskit";
import {
    CanvasKit,
    SkCanvas,
    SkImage,
    SkImageInfo,
    SkSurface,
    SkPicture,
    TypedArray,
} from "canvaskit-wasm";

CanvasKitInit({locateFile: (file: string) => '/node_modules/canvaskit/bin/' + file}).then((CK: CanvasKit) => {
    colorTests(CK);
    mallocTests(CK);
    surfaceTests(CK);
    rectangleTests(CK);
});

function colorTests(CK: CanvasKit) {
    const colorOne = CK.Color(200, 200, 200, 0.8); // $ExpectType Float32Array
    const colorTwo = CK.Color4f(0.8, 0.8, 0.8, 0.7); // $ExpectType Float32Array
    const colorThree = CK.ColorAsInt(240, 230, 220); // $ExpectType number
    const colorFour = CK.parseColorString('#887766'); // $ExpectType Float32Array

    // Deprecated Color functions
    const [r, g, b, a] = CK.getColorComponents(colorTwo);
    const alphaChanged = CK.multiplyByAlpha(colorOne, 0.1);
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
