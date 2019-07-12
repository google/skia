/**
 * Command line application to run Lottie-Web perf on a Lottie file in the
 * browser and then exporting that result.
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
    name: 'lottie_player',
    description: 'The path to lottie.min.js, defaults to a local npm install location.',
    type: String,
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
    header: 'Lottie-Web Perf',
    content: 'Command line application to run Lottie-Web perf.',
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
if (!options.lottie_player) {
  options.lottie_player = 'node_modules/lottie-web/build/player/lottie.min.js';
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
let lottieJS = fs.readFileSync(options.lottie_player, 'utf8');
let driverHTML = fs.readFileSync('lottie-web-perf.html', 'utf8');
let lottieJSON = fs.readFileSync(options.input, 'utf8');

// Find number of frames from the lottie JSON.
let lottieJSONContent = JSON.parse(lottieJSON);
const totalFrames = lottieJSONContent.op - lottieJSONContent.ip;
console.log('Total frames: ' + totalFrames);

const app = express();
app.get('/', (req, res) => res.send(driverHTML));
app.get('/res/lottie.js', (req, res) => res.send(lottieJS));
app.get('/res/lottie.json', (req, res) => res.send(lottieJSON));
app.listen(options.port, () => console.log('- Local web server started.'))

// Utility function.
async function wait(ms) {
    await new Promise(resolve => setTimeout(() => resolve(), ms));
    return ms;
}

const targetURL = "http://localhost:" + options.port + "/#" + totalFrames;

// Drive chrome to load the web page from the server we have running.
async function driveBrowser() {
  console.log('- Launching chrome for ' + options.input);
  let browser;
  let page;
  try {
    browser = await puppeteer.launch(
        {headless: true, args: ['--no-sandbox', '--disable-setuid-sandbox']});
    page = await browser.newPage();
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

    console.log('- Waiting 60s for run to be done.');
    await page.waitForFunction('window._lottieWebDone === true', {
      timeout: 60000,
    });

    // Stop trace.
    await page.tracing.stop();
  } catch(e) {
    console.log('Timed out while loading or drawing. Either the JSON file was ' +
                'too big or hit a bug in the player.', e);
    await browser.close();
    process.exit(1);
  }

  await browser.close();
  // Need to call exit() because the web server is still running.
  process.exit(0);
}

driveBrowser();
