/// <reference path="load.ts" />
/// <reference path="memory.ts" />
/// <reference path="../bindings/extension.d.ts" />
/// <reference path="public_api.d.ts" />
// eslint-disable-next-line @typescript-eslint/no-unused-vars
namespace Extension {
  // Module is the C++ module with the private (and some public) bindings on it.
  declare const Module: extension.Bindings;
  declare const CanvasKit: public_api.CanvasKit;
  load.afterLoad(() => {
    /**
     * publicExtension takes the number of rects and returns how
     * many of them have the point (5, 5) in them.
     * @param myRects
     * @ts publicExtension(myRects: InputFlattenedRectArray): void;
     */
    CanvasKit.publicExtension = (myRects: public_api.InputFlattenedRectArray) => {
      const rPtr = memory.copy1dArray(myRects, 'HEAPF32');
      const num = Module._privateExtension(rPtr, myRects.length / 4);
      memory.freeIfNecessary(rPtr, myRects);
      return num;
    };

    CanvasKit.withObject = (obj: public_api.CompoundObj) => {
      obj.gamma ||= 1.0;
      Module._withObject(obj);
    };
  });
}
