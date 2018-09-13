/*
 * This externs file prevents the Closure JS compiler from minifying away
 * names of objects created by Emscripten.
 * Basically, by defining empty objects and functions here, Closure will
 * know not to rename them.  This is needed because of our pre-js files,
 * that is, the JS we hand-write to bundle into the output.
 *
 * Emscripten does not support automatically generating an externs file, so we
 * do it by hand. The general process is to write some JS code, and then put any
 * calls to Skia or related things in here. Running ./compile.sh and then
 * looking at the minified results or running the Release trybot should
 * verify nothing was missed. Optionally, looking directly at the minified
 * pathkit.js can be useful when developing locally.
 *
 * Docs:
 *   https://github.com/cljsjs/packages/wiki/Creating-Externs
 *   https://github.com/google/closure-compiler/wiki/Types-in-the-Closure-Type-System
 *
 * Example externs:
 *   https://github.com/google/closure-compiler/tree/master/externs
 */

var Skia = {
	Color: function(r, g, b, a) {},

	_getWebGLSurface: function(htmlID, w, h) {},
}

// Not sure why this is needed - might be a bug in emsdk that this isn't properly declared.
function loadWebAssemblyModule() {}