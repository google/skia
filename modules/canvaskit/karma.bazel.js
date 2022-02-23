module.exports = function(config) {
  // http://karma-runner.github.io/6.3/config/configuration-file.html
  let cfg = {
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['jasmine'],

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
  BAZEL_APPLY_SETTINGS(cfg)

  config.set(cfg);
};
