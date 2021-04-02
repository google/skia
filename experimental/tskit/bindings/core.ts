import {CBindings} from './core.d'
import {CanvasKit} from '../npm_build/types'


// Module is the C++ module with the private (and some public) bindings on it.
declare var Module: CBindings;

console.log('hello from core');

(function(CModule: CBindings) {
    const CanvasKit = CModule as Partial<CanvasKit>;

    CanvasKit.sayHello = (n: number) => {
        console.log('hello', CModule.globalFunction(n));
    }

}(Module));