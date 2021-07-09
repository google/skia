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
         * privateFunction2 does something with two numbers and boolean.
         * The boolean might flip the order.
         * @param x The first integer of great import.
         * @param y The first integer of lesser import.
         * @param b Controls which algorithm to use
         * @ts privateFunction(x: number, y: number, b: boolean): number;
         */
        CanvasKit.privateFunction = (x: number, y: number, b: boolean): number => {
            if (b) {
                return CanvasKit._privateFunction2(y, x);
            }
            return CanvasKit._privateFunction1(x, y);
        };

        /**
         * This sets the name twice for good measure.
         * @param name some param
         * @ts Something::setName(name: string): void;
         */
        CanvasKit.Something.prototype.setName = function setName(name: string):void {
            this._setName(name + name);
        };

        /**
         * This calculates something about the rectangles.
         * It is an important calculation
         * @param nums flattened rectangles (who squished them?)
         * @ts Something::calculate(nums: InputFlattenedRectArray): boolean;
         */
        CanvasKit.Something.prototype.calculate = (nums: InputFlattenedRectArray):boolean => {
            let sum = 0;
            for (const i of nums) {
                sum += i;
            }
            return sum % 2 === 4;
        };
    });
}
