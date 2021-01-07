// Configuration for running the tests in google3
// The only thing to specify is the static file dependencies.
// For everything else the defaults for the karam_web_test_suite are used.

module.exports = function(config) {
  // By default this contains any srcs in the build rule, but we need to add our
  // generated js and wasm files, and they need to come first, hence unshift
  config.files.unshift(
    // pattern is a path relative to google3 root referring to any file
    // provided by a target in the data attribute of the js_library of the test.
    { pattern: 'third_party/skia/HEAD/modules/canvaskit/tests/util.js', included:true, served:false},
    { pattern: 'third_party/skia/HEAD/modules/canvaskit/canvaskit_wasm/canvaskit_cc.wasm', included:false, served:true},
    { pattern: 'third_party/skia/HEAD/modules/canvaskit/canvaskit_wasm/canvaskit_cc.js', included:true, served:false},
    { pattern: 'third_party/skia/HEAD/modules/canvaskit/tests/assets/*', included:false, served:true},
  );

  // proxies have the following form
  // {'/dir-to-serve/': '/base/dir-relative-to-google3-root'}
  config.proxies['/canvaskit/'] = '/base/third_party/skia/HEAD/modules/canvaskit/canvaskit_wasm/';
  config.proxies['/assets/'] = '/base/third_party/skia/HEAD/modules/canvaskit/tests/assets/';
};
