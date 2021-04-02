import {
    CBindings
} from "./core.d"

declare var Module: CBindings;

console.log('hello from core');

(function(CanvasKit: CBindings) {

    let hello = () => {
         console.log("here is module", CanvasKit);
    };
   hello();
    console.log("computed ", CanvasKit.globalFunction(10));

}(Module));