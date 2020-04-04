/**
 * Command line application to run Skottie-WASM perf on a Lottie file in the
 * browser and then exporting the result.
 *
 */
const puppeteer = require('puppeteer');
const express = require('express');
const fs = require('fs');
const commandLineArgs = require('command-line-args');
const commandLineUsage= require('command-line-usage');
const fetch = require('node-fetch');

const opts = [
  {
    name: 'canvaskit_js',
    typeLabel: '{underline file}',
    description: 'The path to canvaskit.js.'
  },
  {
    name: 'canvaskit_wasm',
    typeLabel: '{underline file}',
    description: 'The path to canvaskit.wasm.'
  },
  {
    name: 'input',
    typeLabel: '{underline file}',
    description: 'The Lottie JSON file to process.'
  },
  {
    name: 'output',
    typeLabel: '{underline file}',
    description: 'The perf file to write. Defaults to perf.json',
  },
  {
    name: 'use_gpu',
    description: 'Whether we should run in non-headless mode with GPU.',
    type: Boolean,
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
];

const usage = [
  {
    header: 'Skottie WASM Perf',
    content: "Command line application to run Skottie-WASM perf."
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

if (options.help) {
  console.log(commandLineUsage(usage));
  process.exit(0);
}

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

if (!options.input) {
  console.error('You must supply a Lottie JSON filename.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

// Start up a web server to serve the three files we need.
let canvasKitJS = fs.readFileSync(options.canvaskit_js, 'utf8');
let canvasKitWASM = fs.readFileSync(options.canvaskit_wasm, 'binary');
let driverHTML = fs.readFileSync('skottie-wasm-perf.html', 'utf8');
let lottieJSON = fs.readFileSync(options.input, 'utf8');

const app = express();
app.get('/', (req, res) => res.send(driverHTML));
app.get('/res/canvaskit.wasm', function(req, res) {
  res.type('application/wasm');
  res.send(new Buffer(canvasKitWASM, 'binary'));
});
app.get('/res/canvaskit.js', (req, res) => res.send(canvasKitJS));
app.get('/res/lottie.json', (req, res) => res.send(lottieJSON));
app.listen(options.port, () => console.log('- Local web server started.'))

// Utility function.
async function wait(ms) {
    await new Promise(resolve => setTimeout(() => resolve(), ms));
    return ms;
}

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
  ];
  if (options.use_gpu) {
    browser_args.push('--ignore-gpu-blacklist');
    browser_args.push('--ignore-gpu-blocklist');
    browser_args.push('--enable-gpu-rasterization');
  }
  console.log("Running with headless: " + headless + " args: " + browser_args);
  try {
    browser = await puppeteer.launch({headless: headless, args: browser_args});
    page = await browser.newPage();
    await page.setViewport(viewPort);
  } catch (e) {
    console.log('Could not open the browser.', e);
    process.exit(1);
  }
  console.log("Loading " + targetURL);
  try {
    // Start trace.
    await page.tracing.start({
      path: options.output,
      screenshots: false,
      categories: ["blink", "cc", "gpu"]
    });

    await page.goto(targetURL, {
      timeout: 60000,
      waitUntil: 'networkidle0'
    });

    console.log('Waiting 60s for run to be done');
    await page.waitForFunction(`(window._skottieDone === true) || window._error`, {
      timeout: 60000,
    });

    const err = await page.evaluate('window._error');
    if (err) {
      console.log(`ERROR: ${err}`)
      process.exit(1);
    }

    // Stop Trace.
    await page.tracing.stop();
  } catch(e) {
    console.log('Timed out while loading or drawing. Either the JSON file was ' +
                'too big or hit a bug.', e);
    await browser.close();
    process.exit(1);
  }

  await browser.close();
  // Need to call exit() because the web server is still running.
  process.exit(0);
}

driveBrowser();
