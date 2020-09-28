import CanvasKitInit from "canvaskit-wasm/bin/canvaskit";
import {
    CanvasKit,
} from "canvaskit-wasm";

CanvasKitInit({locateFile: (file: string) => '/node_modules/canvaskit/bin/' + file}).then((canvasKit: CanvasKit) => {
    const canvasEl = document.querySelector('canvas') as HTMLCanvasElement;

    const cOne = canvasKit.Color(200, 200, 200, 0.8); // $ExpectType Float32Array
    const cTwo = canvasKit.Color4f(0.8, 0.8, 0.8, 0.7); // $ExpectType Float32Array
    const cThree = canvasKit.ColorAsInt(240, 230, 220); // $ExpectType number
});
