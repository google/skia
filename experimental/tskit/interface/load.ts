/// <reference path="../bindings/embind.d.ts" />
declare const Module: embind.EmbindModule;
// eslint-disable-next-line @typescript-eslint/no-unused-vars
const CanvasKit = Module; // lets other files use this alias to declare new public APIs.
// eslint-disable-next-line @typescript-eslint/no-unused-vars
namespace load {
    type CallbackFn = () => void;
    const toLoad: CallbackFn[] = [];
    export const afterLoad = (callback: CallbackFn): void => {
      toLoad.push(callback);
    };

    Module.onRuntimeInitialized = () => {
      console.log('runtime initialized');
      toLoad.forEach(((callback) => callback()));
    };
}
