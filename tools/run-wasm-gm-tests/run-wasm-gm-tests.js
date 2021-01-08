/**
 * Command line application to test GMS and unit tests with puppeteer.
 * node run-wasm-gm-tests --js_file ../../out/wasm_gm_tests/wasm_gm_tests.js --wasm_file ../../out/wasm_gm_tests/wasm_gm_tests.wasm --known_hashes /tmp/gold2/tests/hashes.txt --output /tmp/gold2/tests/ --use_gpu --timeout 180
 */
const puppeteer = require('puppeteer');
const express = require('express');
const path = require('path');
const bodyParser = require('body-parser');
const fs = require('fs');
const commandLineArgs = require('command-line-args');
const commandLineUsage = require('command-line-usage');

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
    description: '(required) The hashes that should not be written to disk.'
  },
  {
    name: 'output',
    typeLabel: '{underline file}',
    description: '(required) The directory to write the output JSON and images to.',
  },
  {
    name: 'resources',
    typeLabel: '{underline file}',
    description: '(required) The directory that test images are stored in.',
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
  {
    name: 'manual_mode',
    description: 'If set, tests will not run automatically.',
    type: Boolean,
  },
  {
    name: 'batch_size',
    description: 'Number of gms (or unit tests) to run in a batch. The main thread ' +
      'of the page is only unlocked between batches. Default: 50. Use 1 for debugging.',
    type: Number,
  }
];

const usage = [
  {
    header: 'Measuring correctness of Skia WASM code',
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
if (!options.batch_size) {
  options.batch_size = 50;
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

if (!options.resources) {
  console.error('You must supply resources directory');
  console.log(commandLineUsage(usage));
  process.exit(1);
}

const resourceBaseDir = path.resolve(options.resources)
// This executes recursively and synchronously.
const recursivelyListFiles = (dir) => {
  const absolutePaths = [];
  const files = fs.readdirSync(dir);
  files.forEach((file) => {
    const filepath = path.join(dir, file);
    const stats = fs.statSync(filepath);
    if (stats.isDirectory()) {
      absolutePaths.push(...recursivelyListFiles(filepath));
    } else if (stats.isFile()) {
      absolutePaths.push(path.relative(resourceBaseDir, filepath));
    }
  });
  return absolutePaths;
};

const resourceListing = recursivelyListFiles(options.resources);
console.log('Saw resources', resourceListing);

const driverHTML = fs.readFileSync('run-wasm-gm-tests.html', 'utf8');
const testJS = fs.readFileSync(options.js_file, 'utf8');
const testWASM = fs.readFileSync(options.wasm_file, 'binary');
const knownHashes = fs.readFileSync(options.known_hashes, 'utf8');

// This express webserver will serve the HTML file running the benchmark and any additional assets
// needed to run the tests.
const app = express();
app.get('/', (req, res) => res.send(driverHTML));

app.use('/static/resources/', express.static(resourceBaseDir));
console.log('resources served from', resourceBaseDir);

// This allows the server to receive POST requests of up to 10MB for image/png and read the body
// as raw bytes, housed in a buffer.
app.use(bodyParser.raw({ type: 'image/png', limit: '10mb' }));

app.get('/static/hashes.txt', (req, res) => res.send(knownHashes));
app.get('/static/resource_listing.json', (req, res) => res.send(JSON.stringify(resourceListing)));
app.get('/static/wasm_gm_tests.js', (req, res) => res.send(testJS));
app.get('/static/wasm_gm_tests.wasm', function(req, res) {
  // Set the MIME type so it can be streamed efficiently.
  res.type('application/wasm');
  res.send(new Buffer(testWASM, 'binary'));
});
app.post('/write_png', (req, res) => {
  const md5 = req.header('X-MD5-Hash');
  if (!md5) {
    res.sendStatus(400);
    return;
  }
  const data = req.body;
  const newFile = path.join(options.output, md5 + '.png');
  fs.writeFileSync(newFile, data, {
    encoding: 'binary',
  });
  res.sendStatus(200);
});

const server = app.listen(options.port, () => console.log('- Local web server started.'));

const hash = options.use_gpu? '#gpu': '#cpu';
const targetURL = `http://localhost:${options.port}/${hash}`;
const viewPort = {width: 1000, height: 1000};

// Drive chrome to load the web page from the server we have running.
async function driveBrowser() {
  console.log('- Launching chrome for ' + options.input);
  const browser_args = [
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
  const headless = !options.use_gpu;
  console.log("Running with headless: " + headless + " args: " + browser_args);
  let browser;
  let page;
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
  let failed = [];
  try {
    await page.goto(targetURL, {
      timeout: 60000,
      waitUntil: 'networkidle0'
    });

    if (options.manual_mode) {
      console.log('Manual mode detected. Will hang');
      // Wait a very long time, with the web server running.
      await page.waitForFunction(`window._abort_manual_mode`, {
        timeout: 1000000000,
        polling: 1000,
      });
    }

    // Page is mostly loaded, wait for test harness page to report itself ready. Some resources
    // may still be loading.
    console.log('Waiting 30s for test harness to be ready');
    await page.waitForFunction(`(window._testsReady === true) || window._error`, {
      timeout: 30000,
    });

    const err = await page.evaluate('window._error');
    if (err) {
      const log = await page.evaluate('window._log');
      console.info(log);
      console.error(`ERROR: ${err}`);
      process.exit(1);
    }

    // There is a button with id #start_tests to click (this also makes manual debugging easier).
    await page.click('#start_tests');

    // Rather than wait a long time for things to finish, we send progress updates every 50 tests.
    let batch = options.batch_size;
    while (true) {
      console.log(`Waiting ${options.timeout}s for ${options.batch_size} tests to complete`);
      await page.waitForFunction(`(window._testsProgress >= ${batch}) || window._testsDone || window._error`, {
        timeout: options.timeout*1000,
      });
      const progress = await page.evaluate(() => {
        return {
          err: window._error,
          done: window._testsDone,
          count: window._testsProgress,
        };
      });
      if (progress.err) {
        const log = await page.evaluate('window._log');
        console.info(log);
        console.error(`ERROR: ${progress.err}`);
        process.exit(1);
      }
      if (progress.done) {
        console.log(`Completed ${progress.count} tests. Finished.`);
        break;
      }
      console.log(`In Progress; completed ${progress.count} tests.`)
      batch = progress.count + options.batch_size;
    }
    const goldResults = await page.evaluate('window._results');
    failed = await(page.evaluate('window._failed'));

    const log = await page.evaluate('window._log');
    console.info(log);


    const jsonFile = path.join(options.output, 'gold_results.json');
    fs.writeFileSync(jsonFile, JSON.stringify(goldResults));
  } catch(e) {
    console.log('Timed out while loading, drawing, or writing to disk.', e);
    if (page) {
      const log = await page.evaluate('window._log');
      console.error(log);
    }
    await browser.close();
    await new Promise((resolve) => server.close(resolve));
    process.exit(1);
  }

  await browser.close();
  await new Promise((resolve) => server.close(resolve));

  if (failed.length > 0) {
    console.error('Failed tests', failed);
    process.exit(1);
  } else {
    process.exit(0);
  }
}

driveBrowser();
