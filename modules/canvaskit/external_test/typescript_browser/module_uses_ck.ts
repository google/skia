import {CanvasKitInit as CKInit} from "canvaskit-wasm";

// Right now, CanvasKitInit is exported from canvaskit-wasm in value space, not type space.
// As such, we need to use "type of" to be able to describe the type of this function in the
// global scope.
declare const CanvasKitInit: typeof CKInit;

async function init() {
    const CK = await CanvasKitInit({
        locateFile: (file: string) => "node_modules/canvaskit-wasm/bin/" + file
    });
    const color = CK.Color(1,2,3,4);
    console.log(color);
}
init();
