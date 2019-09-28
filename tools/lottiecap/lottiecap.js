/**
 * Command line application to build a 5x5 filmstrip from a Lottie file in the
 * browser and then exporting that filmstrip in a 1000x1000 PNG.
 *
 */
const puppeteer = require('puppeteer');
const express = require('express');
const fs = require('fs');
const commandLineArgs = require('command-line-args');
const commandLineUsage= require('command-line-usage');
const fetch = require('node-fetch');

// Valid values for the --renderer flag.
const RENDERERS = ['svg', 'canvas'];

const opts = [
  {
    name: 'input',
    typeLabel: '{underline file}',
    description: 'The Lottie JSON file to process.'
  },
  {
    name: 'output',
    typeLabel: '{underline file}',
    description: 'The captured filmstrip PNG file to write. Defaults to filmstrip.png',
  },
  {
    name: 'renderer',
    typeLabel: '{underline mode}',
    description: 'Which renderer to use, "svg" or "canvas". Defaults to "svg".',
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
    name: 'post_to',
    description: 'If set, the url to post results to for Gold Ingestion.',
    type: String,
  },
  {
    name: 'in_docker',
    description: 'Is this being run in docker, defaults to false',
    type: Boolean,
  },
  {
    name: 'skip_automation',
    description: 'If the automation of the screenshot taking should be skipped ' +
                 '(e.g. debugging). Defaults to false.',
    type: Boolean,
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
    header: 'Lottie Filmstrip Capture',
    content: `Command line application to build a 5x5 filmstrip
from a Lottie file in the browser and then export
that filmstrip in a 1000x1000 PNG.`
  },
  {
    header: 'Options',
    optionList: opts,
  },
];

// Parse and validate flags.
const options = commandLineArgs(opts);

if (!options.output) {
  options.output = 'filmstrip.png';
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

if (!options.renderer) {
  options.renderer = 'svg';
}

if (!RENDERERS.includes(options.renderer)) {
  console.error('The --renderer flag must have as a value one of: ', RENDERERS);
  console.log(commandLineUsage(usage));
  process.exit(1);
}

// Start up a web server to serve the three files we need.
let lottieJS = fs.readFileSync(options.lottie_player, 'utf8');
let driverHTML = fs.readFileSync('driver.html', 'utf8');
let lottieJSON = fs.readFileSync(options.input, 'utf8');

const app = express();
app.get('/', (req, res) => res.send(driverHTML));
app.get('/lottie.js', (req, res) => res.send(lottieJS));
app.get('/lottie.json', (req, res) => res.send(lottieJSON));
app.listen(options.port, () => console.log('- Local web server started.'))

// Utiltity function.
async function wait(ms) {
    await new Promise(resolve => setTimeout(() => resolve(), ms));
    return ms;
}

const targetURL = `http://localhost:${options.port}/#${options.renderer}`;

// Drive chrome to load the web page from the server we have running.
async function driveBrowser() {
  console.log('- Launching chrome in headless mode.');
  let browser = null;
  if (options.in_docker) {
    browser = await puppeteer.launch({
      'executablePath': '/usr/bin/google-chrome-stable',
      'args': ['--no-sandbox'],
    });
  } else {
    browser = await puppeteer.launch();
  }

  const page = await browser.newPage();
  console.log(`- Loading our Lottie exercising page for ${options.input}.`);
  try {
     // 20 seconds is plenty of time to wait for the json to be loaded once
     // This usually times out for super large json.
    await page.goto(targetURL, {
      timeout: 20000,
      waitUntil: 'networkidle0'
    });
    // 20 seconds is plenty of time to wait for the frames to be drawn.
    // This usually times out for json that causes errors in the player.
    console.log('- Waiting 15s for all the tiles to be drawn.');
    await page.waitForFunction('window._tileCount === 25', {
      timeout: 20000,
    });
  } catch(e) {
    console.log('Timed out while loading or drawing. Either the JSON file was ' +
                'too big or hit a bug in the player.', e);
    await browser.close();
    process.exit(0);
  }

  console.log('- Taking screenshot.');
  let encoding = 'binary';
  if (options.post_to) {
    encoding = 'base64';
    // prevent writing the image to disk
    options.output = '';
  }

  // See https://github.com/GoogleChrome/puppeteer/blob/v1.6.0/docs/api.md#pagescreenshotoptions
  let result = await page.screenshot({
    path: options.output,
    type: 'png',
    clip: {
      x: 0,
      y: 0,
      width: 1000,
      height: 1000,
    },
    encoding: encoding,
  });

  if (options.post_to) {
    console.log(`- Reporting ${options.input} to Gold server ${options.post_to}`);
    let shortenedName = options.input;
    let lastSlash = shortenedName.lastIndexOf('/');
    if (lastSlash !== -1) {
      shortenedName = shortenedName.slice(lastSlash+1);
    }
    await fetch(options.post_to, {
        method: 'POST',
        mode: 'no-cors',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            'data': result,
            'test_name': shortenedName,
        })
    });
  }

  await browser.close();
  // Need to call exit() because the web server is still running.
  process.exit(0);
}

if (!options.skip_automation) {
  driveBrowser();
} else {
  console.log(`open ${targetURL} to see the animation.`)
}

