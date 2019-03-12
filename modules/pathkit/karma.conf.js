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
      { pattern: 'npm-wasm/bin/test/pathkit.wasm', included:false, served:true},
      { pattern: 'tests/*.json', included:false, served:true},
      'tests/testReporter.js',
      'npm-wasm/bin/test/pathkit.js',
      'tests/pathkitinit.js',
      'tests/*.spec.js'
    ],

    proxies: {
      '/pathkit/': '/base/npm-wasm/bin/test/'
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

  if (isDocker) {
    // See https://hackernoon.com/running-karma-tests-with-headless-chrome-inside-docker-ae4aceb06ed3
    cfg.browsers = ['ChromeHeadlessNoSandbox'],
    cfg.customLaunchers = {
        ChromeHeadlessNoSandbox: {
            base: 'ChromeHeadless',
            flags: [
            // Without this flag, we see an error:
            // Failed to move to new namespace: PID namespaces supported, Network namespace supported, but failed: errno = Operation not permitted
                '--no-sandbox'
            ],
        },
    };
  }

  if (process.env.ASM_JS) {
    console.log('asm.js is under test');
    cfg.files = [
      { pattern: 'npm-asmjs/bin/test/pathkit.js.mem', included:false, served:true},
      { pattern: 'tests/*.json', included:false, served:true},
      'tests/testReporter.js',
      'npm-asmjs/bin/test/pathkit.js',
      'tests/pathkitinit.js',
      'tests/*.spec.js'
    ];

    cfg.proxies = {
      '/pathkit/': '/base/npm-asmjs/bin/test/'
    };
  } else {
    console.log('wasm is under test');
  }

  config.set(cfg);
}
