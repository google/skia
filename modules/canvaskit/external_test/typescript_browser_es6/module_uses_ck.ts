import { CanvasKitInit } from "./node_modules/canvaskit-wasm/bin/canvaskit.js";
async function init() {
    const CK = await CanvasKitInit({
        locateFile: (file: string) => __dirname + "FIXME/bin/" + file
    });
    const color = CK.Color(1,2,3,4);
    console.log(color);
}
init();
