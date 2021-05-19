/// <reference path="load.ts" />
/// <reference path="../bindings/core.d.ts" />
/// <reference path="public_api.d.ts" />
// eslint-disable-next-line @typescript-eslint/no-unused-vars
namespace Core {
  // Module is the C++ module with the private (and some public) bindings on it.
  declare const Module: core.Bindings;
  declare const CanvasKit: public_api.CanvasKit;
  load.afterLoad(() => {
    /**
     * This function says hello
     *
     * @param x some number
     * @param y some other number
     * @ts sayHello(x: number, y: number): void;
     */
    CanvasKit.sayHello = (x: number, y: number) => {
      console.log('hello', Module._privateFunction(x, y));
    };

    /**
     * This sets the name twice for good measure.
     * @param name some param
     * @ts Something::setName(name: string): void;
     */
    CanvasKit.Something.prototype.setName = function setName(name: string) {
      this._setName(name + name);
    };
  });
}
