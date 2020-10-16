/**
 * Command line application to test GMS and unit tests with puppeteer.
 * node run-wasm-gm-tests --js_file ../../out/wasm_gm_tests/wasm_gm_tests.js --wasm_file ../../out/wasm_gm_tests/wasm_gm_tests.wasm --known_hashes /tmp/gold2/tests/hashes.txt --output /tmp/gold2/tests/ --use_gpu --timeout 180
 */
const puppeteer = require('puppeteer');
const express = require('express');
const fs = require('fs');
const commandLineArgs = require('command-line-args');
const commandLineUsage= require('command-line-usage');

const opts = [
  {
    name: 'js_file',
    typeLabel: '{underline file}',
    description: '(required) The path to wasm_gm_tests.js.'
  },
  {
    name: 'wasm_file',
    typeLabel: '{underline file}',
    description: '(required) The path to wasm_gm_tests.wasm.'
  },
  {
    name: 'known_hashes',
    typeLabel: '{underline file}',
    description: '(required) A directory containing any assets needed by lottie' +
      ' files or tests (e.g. images/fonts).'
  },
  {
    name: 'output',
    typeLabel: '{underline file}',
    description: '(required) The directory to write the output JSON and images to.',
  },
  {
    name: 'use_gpu',
    description: 'Whether we should run in non-headless mode with GPU.',
    type: Boolean,
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
    header: 'Measruing correctness of Skia WASM code',
    content: 'Command line application to capture images drawn from tests',
  },
  {
    header: 'Options',
    optionList: opts,
  },
];

// Parse and validate flags.
const options = commandLineArgs(opts);

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

if (!options.output) {
  console.error('You must supply an output directory.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

const driverHTML = fs.readFileSync('run-wasm-gm-tests.html', 'utf8');

// This express webserver will serve the HTML file running the benchmark and any additional assets
// needed to run the tests.
const app = express();
app.get('/', (req, res) => res.send(driverHTML));

if (!options.js_file) {
  console.error('You must supply path to wasm_gm_tests.js.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

if (!options.wasm_file) {
  console.error('You must supply path to wasm_gm_tests.wasm.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

if (!options.known_hashes) {
  console.error('You must supply path to known_hashes.txt');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

const testJS = fs.readFileSync(options.js_file, 'utf8');
const testWASM = fs.readFileSync(options.wasm_file, 'binary');
const knownHashes = fs.readFileSync(options.known_hashes, 'utf8');

app.get('/static/hashes.txt', (req, res) => res.send(knownHashes));
app.get('/static/wasm_gm_tests.js', (req, res) => res.send(testJS));
app.get('/static/wasm_gm_tests.wasm', function(req, res) {
  // Set the MIME type so it can be streamed efficiently.
  res.type('application/wasm');
  res.send(new Buffer(testWASM, 'binary'));
});

app.listen(options.port, () => console.log('- Local web server started.'));

let hash = "#cpu";
if (options.use_gpu) {
  hash = "#gpu";
}
const targetURL = `http://localhost:${options.port}/${hash}`;
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
    await page.waitForFunction(`(window._testsReady === true) || window._error`, {
      timeout: 15000,
    });

    let err = await page.evaluate('window._error');
    if (err) {
      console.log(`ERROR: ${err}`);
      process.exit(1);
    }

    // Benchmarks should have a button with id #start_tests to click (this also makes manual
    // debugging easier).
    await page.click('#start_tests');

    console.log(`Waiting ${options.timeout}s for run to be done`);
    await page.waitForFunction(`(window.FIXME === true) || window._error`, {
      timeout: options.timeout*1000,
    });

    err = await page.evaluate('window._error');
    if (err) {
      console.log(`ERROR: ${err}`);
      process.exit(1);
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
