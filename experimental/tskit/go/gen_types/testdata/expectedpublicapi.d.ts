declare namespace public_api {
export interface ExampleKit {
    /**
     * privateFunction2 does something with two numbers and boolean.
     * The boolean might flip the order.
     * @param x The first integer of great import.
     * @param y The first integer of lesser import.
     * @param b Controls which algorithm to use
     */
    privateFunction(x: number, y: number, b: boolean): number;

    /**
     * This function does another public thing.
     * @param input an ice cream flavor
     */
    publicFunction1(): boolean;

    /**
     * This function does a public thing.
     * @param input an ice cream flavor
     */
    publicFunction2(input: string): void;

    readonly AnotherClass: AnotherClassConstructor;
    readonly Something: SomethingConstructor;

    readonly SOME_FLAG: number;
    readonly hasBird: boolean;
    readonly optionalConst?: string;
}

export interface AnotherClassConstructor {
    /**
     * Initializes AnotherClass with default values.
     */
    new(): AnotherClass;

    /**
     * Initializes AnotherClass with the name and Something.
     * @param name
     * @param thing - will be used, I promise.
     */
    new(name: string, thing: Something): AnotherClass;
}

export interface SomethingConstructor {
    /**
     * Returns a Something with the provided name.
     * @param name
     */
    new(name: string): Something;
}

export interface AnotherClass extends EmbindObject<AnotherClass> {
    /**
     * Returns the associated thing.
     */
    get(): Something;
}

/**
 * The Something class is quite something. See SkSomething.h for more.
 */
export interface Something extends EmbindObject<Something> {
    /**
     * This calculates something about the rectangles.
     * It is an important calculation
     * @param nums flattened rectangles (who squished them?)
     */
    calculate(nums: InputFlattenedRectArray): boolean;
    /**
     * Returns the associated name.
     */
    getName(): string;
    /**
     * This sets the name twice for good measure.
     * @param name some param
     */
    setName(name: string): void;
}

export interface SomeValueObject {
    /**
     The number of columns that the frobulator needs.
     */
    columns?: number,
    /**
     *   The object associated with the frobulator.
     */
    isInteger: boolean,
    name: string,
    object: AnotherClass,
}

export type InputFlattenedRectArray = Float32Array | number[];

/**
 * CanvasKit is built with Emscripten and Embind. Embind adds the following methods to all objects
 * that are exposed with it.
 */
export interface EmbindObject<T extends EmbindObject<T>> {
    clone(): T;
    delete(): void;
    deleteAfter(): void;
    isAliasOf(other: any): boolean;
    isDeleted(): boolean;
}
}