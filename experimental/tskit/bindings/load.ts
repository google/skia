/// <reference path="embind.d.ts" />
declare const Module: embind.EmbindModule;
// eslint-disable-next-line @typescript-eslint/no-unused-vars
const CanvasKit = Module; // lets other files use this alias to declare new public APIs.
// eslint-disable-next-line @typescript-eslint/no-unused-vars
namespace load {
    type callbackFn = () => void;
    const toLoad = [] as callbackFn[];
    export const afterLoad = (callback: callbackFn): void => {
        toLoad.push(callback);
    }

    Module.onRuntimeInitialized = function() {
        console.log("runtime initialized");
        toLoad.forEach((callback => callback()));
    }
}

