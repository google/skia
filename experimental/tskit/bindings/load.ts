/// <reference path="embind.d.ts" />
declare var Module: embind.EmbindModule;
namespace load {
    type callbackFn = () => void;
    const toLoad = [] as callbackFn[];
    export const afterLoad = (callback: callbackFn) => {
        toLoad.push(callback);
    }

    Module.onRuntimeInitialized = function() {
        console.log("runtime initialized");
        toLoad.forEach((callback => callback()));
    }
}
const CanvasKit = Module;
