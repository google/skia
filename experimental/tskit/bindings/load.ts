/// <reference path="embind.d.ts" />
declare var Module: embind.EmbindModule;
namespace load {
    const toLoad = [];
    export const afterLoad = (callback: () => void) => {
        toLoad.push(callback);
    }

    Module.onRuntimeInitialized = function() {
        console.log("runtime initialized");
        toLoad.forEach((callback => callback()));
    }
}
const CanvasKit = Module;
