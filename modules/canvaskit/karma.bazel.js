const path = require('path');
const fs = require('fs')
// This should be a file created by gold_test_env.go which contains the port number
// on which it is listening. For whatever reason, karma was not happy serving the
// port file directly, but reading it in and then adding it as a proxy seems to
// work fine.
const testOnEnvPortPath = path.join(process.env['ENV_DIR'], 'port');
const port = fs.readFileSync(testOnEnvPortPath, 'utf8').toString();
console.log('test_on_env PORT:', port);

module.exports = function(config) {
  // http://karma-runner.github.io/6.3/config/configuration-file.html
  let cfg = {
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],

    proxies: {
      // The tests will make calls to /gold_rpc/whatever and they will be redirected
      // to the correct location.
      '/gold_rpc/': `http://localhost:${port}/`,
      // This makes it more convenient for tests to load the test assets.
      '/assets/': '/static/skia/modules/canvaskit/tests/assets/',
    },

    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['progress'],
    colors: true,
    logLevel: config.LOG_INFO,

    browserDisconnectTimeout: 20000,
    browserNoActivityTimeout: 20000,

    // How many browsers should be started simultaneous
    concurrency: Infinity,
  };

  // Bazel will inject some code here to add/change the following items:
  //  - files
  //  - proxies
  //  - browsers
  //  - basePath
  //  - singleRun
  //  - plugins
  BAZEL_APPLY_SETTINGS(cfg);

  config.set(cfg);
};
