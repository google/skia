// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, the code that emulates the HTML Canvas interface
// (which is called HTMLCanvas or similar to avoid confusion with
// SkCanvas).
(function() {

  // This allows us to expose internal functions (e.g. color
  // parsing) for unit-testing, even in the minified version.
  // Our tests are not minified like CanvasKit is, so the names
  // would get lost otherwise.
  CanvasKit._testing = {};

// This intentionally dangles because we want all the htmlcanvas
// JS code to be in the same scope, but JS doesn't support
// namespaces like C++ does. Thus, we simply include this
// preamble.js file, all the source .js files and then postamble.js
// to bundle everything in the same scope.