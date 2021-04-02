import {CBindings} from './core.d'
import {CanvasKit} from '../npm_build/types'


// Module is the C++ module with the private (and some public) bindings on it.
declare var Module: CBindings;

console.log('hello from core');

(function(CModule: CBindings) {
    const CanvasKit = CModule as Partial<CanvasKit>;

    /**
     * This function says hello
     *
     * @param n
     * @ts sayHello(n: number): void;
     */
    CanvasKit.sayHello = (n: number) => {
        console.log('hello', CModule._privateFunction(n));
    }

    /**
     * This function does a thing
     *
     * @param s you ice cream flavor
     */
    CanvasKit.myNewThing = function(s: string) {
        return CModule._myNewThing(s);
    }

}(Module));