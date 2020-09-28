// This file is type-checked by the Typescript definitions. It is not actually executed.

import CanvasKitInit from "canvaskit-wasm/bin/canvaskit";
import {
    CanvasKit,
    TypedArray,
} from "canvaskit-wasm";

CanvasKitInit({locateFile: (file: string) => '/node_modules/canvaskit/bin/' + file}).then((CK: CanvasKit) => {
    const canvasEl = document.querySelector('canvas') as HTMLCanvasElement;

    // Color functions
    const colorOne = CK.Color(200, 200, 200, 0.8); // $ExpectType Float32Array
    const colorTwo = CK.Color4f(0.8, 0.8, 0.8, 0.7); // $ExpectType Float32Array
    const colorThree = CK.ColorAsInt(240, 230, 220); // $ExpectType number
    const colorFour = CK.parseColorString("#887766"); // $ExpectType Float32Array

    // Deprecated Color functions
    const [r, g, b, a] = CK.getColorComponents(colorTwo);
    const alphaChanged = CK.multiplyByAlpha(colorOne, 0.1);

    // Rectangle functions
    const rectOne = CK.LTRBRect(5, 10, 20, 30); // $ExpectType Float32Array
    const rectTwo = CK.XYWHRect(5, 10, 15, 20); // $ExpectType Float32Array
    const iRectOne = CK.LTRBiRect(105, 110, 120, 130); // $ExpectType Int32Array
    const iRectTwo = CK.XYWHiRect(105, 110, 15, 20); // $ExpectType Int32Array
    const rrectOne = CK.RRectXY(rectOne, 3, 7);  // $ExpectType Float32Array

    // Malloc functions
    const mFoo = CK.Malloc(Float32Array, 5);
    const mArray = mFoo.toTypedArray(); // $ExpectType TypedArray
    mArray[3] = 1.7;
    const mSubArray = mFoo.subarray(0, 2); // $ExpectType TypedArray
    mSubArray[0] = 2;
    CK.Free(mFoo);
});
