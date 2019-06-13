/**
 * Command line application to run Skottie-WASM perf on a Lottie file in the
 * browser and then exporting the result.
 *
 */
const puppeteer = require('puppeteer');
const canvaskit_wasm = require('canvaskit-wasm');

const express = require('express');
const fs = require('fs');
const commandLineArgs = require('command-line-args');
const commandLineUsage= require('command-line-usage');
const fetch = require('node-fetch');

const opts = [
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

if (!options.input) {
  console.error('You must supply a Lottie JSON filename.');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

// Start up a web server to serve the three files we need.
let driverHTML = fs.readFileSync('driver.html', 'utf8');
let lottieJSON = fs.readFileSync(options.input, 'utf8');

const app = express();
app.get('/', (req, res) => res.send(driverHTML));
app.get('/lottie.json', (req, res) => res.send(lottieJSON));
app.listen(options.port, () => console.log('- Local web server started.'))

// Utility function. Used?
async function wait(ms) {
    await new Promise(resolve => setTimeout(() => resolve(), ms));
    return ms;
}

const targetURL = "http://localhost:" + options.port + "/";

// Drive chrome to load the web page from the server we have running.
async function driveBrowser() {
  console.log('- Launching chrome in.');
  const browser = await puppeteer.launch({headless: false});
  const page = await browser.newPage();
  console.log("Loading " + targetURL);
  // return;
  try {
     // Is 20 seconds enough time?
    await page.goto(targetURL, {
      timeout: 20000,
      waitUntil: 'networkidle0'
    });
  } catch(e) {
    console.log('Timed out while loading or drawing. Either the JSON file was ' +
                'too big or hit a bug.', e);
    await browser.close();
    process.exit(0);
  }
  await browser.close();
  // Need to call exit() because the web server is still running.
  process.exit(0);
}

driveBrowser();

