// This is the legacy (non-Bazel) test setup.
const isDocker = require('is-docker')();

module.exports = function(config) {
  // Set the default values to be what are needed when testing the
  // WebAssembly build locally.
  let cfg = {
    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],

    // list of files / patterns to load in the browser
    files: [
      { pattern: 'build/canvaskit.wasm', included:false, served:true},
      { pattern: 'tests/assets/*', included:false, served:true},
      'build/canvaskit.js',
      'tests/legacy_init.js',
      'tests/util.js',
      'tests/legacy_test_reporter.js',
      'tests/*_test.js'
    ],

    proxies: {
      '/assets/': '/base/tests/assets/',
      '/build/': '/base/build/',
    },

    // test results reporter to use
    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['progress'],

    // web server port
    port: 4444,

    // enable / disable colors in the output (reporters and logs)
    colors: true,

    // level of logging
    // possible values: config.LOG_DISABLE || config.LOG_ERROR || config.LOG_WARN || config.LOG_INFO || config.LOG_DEBUG
    logLevel: config.LOG_INFO,

    // enable / disable watching file and executing tests whenever any file changes
    autoWatch: true,

    browserDisconnectTimeout: 20000,
    browserNoActivityTimeout: 20000,

    // start these browsers
    browsers: ['Chrome'],

    // Continuous Integration mode
    // if true, Karma captures browsers, runs the tests and exits
    singleRun: false,

    // Concurrency level
    // how many browser should be started simultaneous
    concurrency: Infinity,
  };

  if (isDocker || config.headless) {
    // See https://hackernoon.com/running-karma-tests-with-headless-chrome-inside-docker-ae4aceb06ed3
    cfg.browsers = ['ChromeHeadlessNoSandbox'],
    cfg.customLaunchers = {
        ChromeHeadlessNoSandbox: {
          base: 'ChromeHeadless',
          flags: [
            // Without this flag, we see an error:
            // Failed to move to new namespace: PID namespaces supported, Network namespace supported, but failed: errno = Operation not permitted
            '--no-sandbox',
            // may help tests be less flaky
            // https://peter.sh/experiments/chromium-command-line-switches/#browser-test
            '--browser-test',
            // This can also help avoid crashes/timeouts:
            // https://github.com/GoogleChrome/puppeteer/issues/1834
            '--disable-dev-shm-usage',
          ],
        },
    };
  } else {
    // Extra options that should only be applied locally

    // Measure test coverage and write output to coverage/ directory
    cfg.reporters.push('coverage');
    cfg.preprocessors = {
      // Measure test coverage of these source files
      // Since this file is a combination of our code, and emscripten's glue,
      // we'll never see 100% coverage, but this lets us measure improvements.
      'canvaskit/bin/canvaskit.js': ['coverage'],
    };
  }

  config.set(cfg);
}
