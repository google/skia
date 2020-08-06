/**
 * Command line application to run CanvasKit benchmarks in webpages using puppeteer. Different
 * webpages can be specified to measure different aspects. The HTML page run contains the JS code
 * to run the scenario and either 1) produce the perf output as a JSON object or 2) defer to
 * puppeteer reading the tracing data.
 *
 */
const puppeteer = require('puppeteer');
const express = require('express');
const fs = require('fs');
const commandLineArgs = require('command-line-args');
const commandLineUsage= require('command-line-usage');

const opts = [
  {
    name: 'bench_html',
    typeLabel: '{underline file}',
    description: 'An HTML file containing the bench harness.'
  },
  {
    name: 'canvaskit_js',
    typeLabel: '{underline file}',
    description: '(required) The path to canvaskit.js.'
  },
  {
    name: 'canvaskit_wasm',
    typeLabel: '{underline file}',
    description: '(required) The path to canvaskit.wasm.'
  },
  {
    name: 'input_lottie',
    typeLabel: '{underline file}',
    description: 'The Lottie JSON file to process.'
  },
  {
    name: 'input_skp',
    typeLabel: '{underline file}',
    description: 'The SKP file to process.'
  },
  {
    name: 'assets',
    typeLabel: '{underline file}',
    description: 'A directory containing any assets needed by lottie files or tests (e.g. images/fonts).'
  },
  {
    name: 'output',
    typeLabel: '{underline file}',
    description: 'The perf file to write. Defaults to perf.json',
  },
  {
    name: 'chromium_executable_path',
    typeLabel: '{underline file}',
    description: 'The chromium executable to be used by puppeteer to run tests',
  },
  {
    name: 'merge_output_as',
    typeLabel: String,
    description: 'Overwrites a json property in an existing output file.',
  },
  {
    name: 'use_gpu',
    description: 'Whether we should run in non-headless mode with GPU.',
    type: Boolean,
  },
  {
    name: 'use_tracing',
    description: 'If non-empty, will be interpreted as the tracing categories that should be ' +
      'measured and returned in the output JSON. Example: "blink,cc,gpu"',
    type: String,
  },
  {
    name: 'enable_simd',
    description: 'enable execution of wasm SIMD operations in chromium',
    type: Boolean
  },
  {
    name: 'port',
    description: 'The port number to use, defaults to 8081.',
    type: Number,
  },
  {
    name: 'query_params',
    description: 'The query params to be added to the testing page URL. Useful for passing' +
      'options to the perf html page.',
    type: String,
    multiple: true
  },
  {
    name: 'help',
    alias: 'h',
    type: Boolean,
    description: 'Print this usage guide.'
  },
  {
    name: 'timeout',
    description: 'Number of seconds to allow test to run.',
    type: Number,
  },
];

const usage = [
  {
    header: 'Skia Web-based Performance Metrics of CanvasKit',
    content: "Command line application to capture performance metrics from a browser."
  },
  {
    header: 'Options',
    optionList: opts,
  },
];

// Parse and validate flags.
const options = commandLineArgs(opts);

if (!options.output) {
  options.output = 'perf.json';
}
if (!options.port) {
  options.port = 8081;
}
if (!options.timeout) {
  options.timeout = 60;
}

if (options.help) {
  console.log(commandLineUsage(usage));
  process.exit(0);
}

if (!options.bench_html) {
  console.error('You must supply the bench_html file to run.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}
const driverHTML = fs.readFileSync(options.bench_html, 'utf8');

// This express webserver will serve the HTML file running the benchmark and any additional assets
// needed to run the tests.
const app = express();
app.get('/', (req, res) => res.send(driverHTML));

if (!options.canvaskit_js) {
  console.error('You must supply path to canvaskit.js.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

if (!options.canvaskit_wasm) {
  console.error('You must supply path to canvaskit.wasm.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

const benchmarkJS = fs.readFileSync('benchmark.js', 'utf8');
const canvasPerfJS = fs.readFileSync('canvas_perf.js', 'utf8');
const canvasKitJS = fs.readFileSync(options.canvaskit_js, 'utf8');
const canvasKitWASM = fs.readFileSync(options.canvaskit_wasm, 'binary');

app.get('/static/benchmark.js', (req, res) => res.send(benchmarkJS));
app.get('/static/canvas_perf.js', (req, res) => res.send(canvasPerfJS));
app.get('/static/canvaskit.js', (req, res) => res.send(canvasKitJS));
app.get('/static/canvaskit.wasm', function(req, res) {
  // Set the MIME type so it can be streamed efficiently.
  res.type('application/wasm');
  res.send(new Buffer(canvasKitWASM, 'binary'));
});


if (options.input_lottie) {
  const lottieJSON = fs.readFileSync(options.input_lottie, 'utf8');
  app.get('/static/lottie.json', (req, res) => res.send(lottieJSON));
}
if (options.input_skp) {
  const skpBytes = fs.readFileSync(options.input_skp, 'binary');
  app.get('/static/test.skp', (req, res) => {
    res.send(new Buffer(skpBytes, 'binary'));
  });
}
if (options.assets) {
  app.use('/static/assets/', express.static(options.assets));
  console.log('assets served from', options.assets);
}

app.listen(options.port, () => console.log('- Local web server started.'));

let hash = "#cpu";
if (options.use_gpu) {
  hash = "#gpu";
}
let query_param_string = '?';
if (options.query_params) {
  for (const string of options.query_params) {
    query_param_string += string + '&';
  }
}
const targetURL = `http://localhost:${options.port}/${query_param_string}${hash}`;
const viewPort = {width: 1000, height: 1000};

// Drive chrome to load the web page from the server we have running.
async function driveBrowser() {
  console.log('- Launching chrome for ' + options.input);
  let browser;
  let page;
  const headless = !options.use_gpu;
  let browser_args = [
      '--no-sandbox',
      '--disable-setuid-sandbox',
      '--window-size=' + viewPort.width + ',' + viewPort.height,
      // The following two params allow Chrome to run at an unlimited fps. Note, if there is
      // already a chrome instance running, these arguments will have NO EFFECT, as the existing
      // Chrome instance will be used instead of puppeteer spinning up a new one.
      '--disable-frame-rate-limit',
      '--disable-gpu-vsync',
  ];
  if (options.enable_simd) {
    browser_args.push('--enable-features=WebAssemblySimd');
  }
  if (options.use_gpu) {
    browser_args.push('--ignore-gpu-blacklist');
    browser_args.push('--ignore-gpu-blocklist');
    browser_args.push('--enable-gpu-rasterization');
  }
  console.log("Running with headless: " + headless + " args: " + browser_args);
  try {
    browser = await puppeteer.launch({
      headless: headless,
      args: browser_args,
      executablePath: options.chromium_executable_path
    });
    page = await browser.newPage();
    await page.setViewport(viewPort);
  } catch (e) {
    console.log('Could not open the browser.', e);
    process.exit(1);
  }
  console.log("Loading " + targetURL);
  try {
    await page.goto(targetURL, {
      timeout: 60000,
      waitUntil: 'networkidle0'
    });

    // Page is mostly loaded, wait for benchmark page to report itself ready.
    console.log('Waiting 15s for benchmark to be ready');
    await page.waitForFunction(`(window._perfReady === true) || window._error`, {
      timeout: 15000,
    });

    let err = await page.evaluate('window._error');
    if (err) {
      console.log(`ERROR: ${err}`);
      process.exit(1);
    }

    // Start trace if requested
    if (options.use_tracing) {
      const categories = options.use_tracing.split(',');
      console.log('Collecting tracing data for categories', categories);
      await page.tracing.start({
        path: options.output,
        screenshots: false,
        categories: categories,
      });
    }

    // Benchmarks should have a button with id #start_bench to click (this also makes manual
    // debugging easier).
    await page.click('#start_bench');

    console.log(`Waiting ${options.timeout}s for run to be done`);
    await page.waitForFunction(`(window._perfDone === true) || window._error`, {
      timeout: options.timeout*1000,
    });

    err = await page.evaluate('window._error');
    if (err) {
      console.log(`ERROR: ${err}`);
      process.exit(1);
    }

    if (options.use_tracing) {
      // Stop Trace.
      await page.tracing.stop();
    } else {
      const perfResults = await page.evaluate('window._perfData');
      console.debug('Perf results: ', perfResults);

      if (options.merge_output_as) {
        const existing_output_file_contents = fs.readFileSync(options.output, 'utf8');
        let existing_dataset = {};
        try {
          existing_dataset = JSON.parse(existing_output_file_contents);
        } catch (e) {}

        existing_dataset[options.merge_output_as] = perfResults;
        fs.writeFileSync(options.output, JSON.stringify(existing_dataset));
      } else {
        fs.writeFileSync(options.output, JSON.stringify(perfResults));
      }
    }

  } catch(e) {
    console.log('Timed out while loading or drawing.', e);
    await browser.close();
    process.exit(1);
  }

  await browser.close();
  // Need to call exit() because the web server is still running.
  process.exit(0);
}

driveBrowser();
