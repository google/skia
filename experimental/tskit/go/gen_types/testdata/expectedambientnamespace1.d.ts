/// <reference path="embind.d.ts" />
declare namespace namespace_one {
	export interface Bindings {
		_privateFunction1(ptr: number): number;
		_privateFunction2(x: number, y: number): number;

		publicFunction1(): boolean;
		publicFunction2(input: string): void;

		readonly AnotherClass: AnotherClassConstructor;
		readonly Something: SomethingConstructor;

		hasBird: boolean;
		SOME_FLAG: number;
	}

	export interface AnotherClassConstructor {
		new(): AnotherClass;
		new(name: string, thing: Something): AnotherClass;
	}

	export interface SomethingConstructor {
		new(name: string): Something;
	}

	export interface AnotherClass extends embind.EmbindObject<AnotherClass> {
		get(): Something;
	}

	export interface Something extends embind.EmbindObject<Something> {
		getName(): string;

		_setName(name: string): void;
	}

	export interface SomeValueObject {
		columns: number,
		object: AnotherClass,
		name: string,
		isInteger: boolean,
	}
}
