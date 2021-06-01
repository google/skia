// The size of the golden images (DMs)
const CANVAS_WIDTH = 600;
const CANVAS_HEIGHT = 600;

const _commonGM = (it, pause, name, callback, assetsToFetchOrPromisesToWaitOn) => {
    const fetchPromises = [];
    for (const assetOrPromise of assetsToFetchOrPromisesToWaitOn) {
        // https://stackoverflow.com/a/9436948
        if (typeof assetOrPromise === 'string' || assetOrPromise instanceof String) {
            const newPromise = fetchWithRetries(assetOrPromise)
                .then((response) => response.arrayBuffer())
                .catch((err) => {
                    console.error(err);
                    throw err;
                });
            fetchPromises.push(newPromise);
        } else if (typeof assetOrPromise.then === 'function') {
            fetchPromises.push(assetOrPromise);
        } else {
            throw 'Neither a string nor a promise ' + assetOrPromise;
        }
    }
    it('draws gm '+name, (done) => {
        const surface = CanvasKit.MakeCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface');
        if (!surface) {
            done();
            return;
        }
        // if fetchPromises is empty, the returned promise will
        // resolve right away and just call the callback.
        Promise.all(fetchPromises).then((values) => {
            try {
                // If callback returns a promise, the chained .then
                // will wait for it.
                return callback(surface.getCanvas(), values, surface);
            } catch (e) {
                console.log(`gm ${name} failed with error`, e);
                expect(e).toBeFalsy();
                debugger;
                done();
            }
        }).then(() => {
            surface.flush();
            if (pause) {
                reportSurface(surface, name, null);
                console.error('pausing due to pause_gm being invoked');
            } else {
                reportSurface(surface, name, done);
            }
        }).catch((e) => {
            console.log(`could not load assets for gm ${name}`, e);
            debugger;
            done();
        });
    })
};

const fetchWithRetries = (url) => {
    const MAX_ATTEMPTS = 3;
    const DELAY_AFTER_FAILURE = 1000;

    return new Promise((resolve, reject) => {
        let attempts = 0;
        const attemptFetch = () => {
            attempts++;
            fetch(url).then((resp) => resolve(resp))
                .catch((err) => {
                    if (attempts < MAX_ATTEMPTS) {
                        console.warn(`got error in fetching ${url}, retrying`, err);
                        retryAfterDelay();
                    } else {
                        console.error(`got error in fetching ${url} even after ${attempts} attempts`, err);
                        reject(err);
                    }
                });
        };
        const retryAfterDelay = () => {
            setTimeout(() => {
                attemptFetch();
            }, DELAY_AFTER_FAILURE);
        }
        attemptFetch();
    });

}

/**
 * Takes a name, a callback, and any number of assets or promises. It executes the
 * callback (presumably, the test) and reports the resulting surface to Gold.
 * @param name {string}
 * @param callback {Function}, has two params, the first is a CanvasKit.Canvas
 *    and the second is an array of results from the passed in assets or promises.
 *    If a given assetOrPromise was a string, the result will be an ArrayBuffer.
 * @param assetsToFetchOrPromisesToWaitOn {string|Promise}. If a string, it will
 *    be treated as a url to fetch and return an ArrayBuffer with the contents as
 *    a result in the callback. Otherwise, the promise will be waited on and its
 *    result will be whatever the promise resolves to.
 */
const gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(it, false, name, callback, assetsToFetchOrPromisesToWaitOn);
};

/**
 *  fgm is like gm, except only tests declared with fgm, force_gm, or fit will be
 *  executed. This mimics the behavior of Jasmine.js.
 */
const fgm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(fit, false, name, callback, assetsToFetchOrPromisesToWaitOn);
};

/**
 *  force_gm is like gm, except only tests declared with fgm, force_gm, or fit will be
 *  executed. This mimics the behavior of Jasmine.js.
 */
const force_gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    fgm(name, callback, assetsToFetchOrPromisesToWaitOn);
};

/**
 *  skip_gm does nothing. It is a convenient way to skip a test temporarily.
 */
const skip_gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    console.log(`Skipping gm ${name}`);
    // do nothing, skip the test for now
};

/**
 *  pause_gm is like fgm, except the test will not finish right away and clear,
 *  making it ideal for a human to manually inspect the results.
 */
const pause_gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(fit, true, name, callback, assetsToFetchOrPromisesToWaitOn);
};

const _commonMultipleCanvasGM = (it, pause, name, callback) => {
    it(`draws gm ${name} on both CanvasKit and using Canvas2D`, (done) => {
        const skcanvas = CanvasKit.MakeCanvas(CANVAS_WIDTH, CANVAS_HEIGHT);
        skcanvas._config = 'software_canvas';
        const realCanvas = document.getElementById('test');
        realCanvas._config = 'html_canvas';
        realCanvas.width = CANVAS_WIDTH;
        realCanvas.height = CANVAS_HEIGHT;

        if (pause) {
            console.log('debugging canvaskit version');
            callback(realCanvas);
            callback(skcanvas);
            const png = skcanvas.toDataURL();
            const img = document.createElement('img');
            document.body.appendChild(img);
            img.src = png;
            debugger;
            return;
        }

        const promises = [];

        for (const canvas of [skcanvas, realCanvas]) {
            callback(canvas);
            // canvas has .toDataURL (even though skcanvas is not a real Canvas)
            // so this will work.
            promises.push(reportCanvas(canvas, name, canvas._config));
        }
        Promise.all(promises).then(() => {
            skcanvas.dispose();
            done();
        }).catch(reportError(done));
    });
};

/**
 * Takes a name and a callback. It executes the callback (presumably, the test)
 * for both a CanvasKit.Canvas and a native Canvas2D. The result of both will be
 * uploaded to Gold.
 * @param name {string}
 * @param callback {Function}, has one param, either a CanvasKit.Canvas or a native
 *    Canvas2D object.
 */
const multipleCanvasGM = (name, callback) => {
    _commonMultipleCanvasGM(it, false, name, callback);
};

/**
 *  fmultipleCanvasGM is like multipleCanvasGM, except only tests declared with
 *  fmultipleCanvasGM, force_multipleCanvasGM, or fit will be executed. This
 *  mimics the behavior of Jasmine.js.
 */
const fmultipleCanvasGM = (name, callback) => {
    _commonMultipleCanvasGM(fit, false, name, callback);
};

/**
 *  force_multipleCanvasGM is like multipleCanvasGM, except only tests declared
 *  with fmultipleCanvasGM, force_multipleCanvasGM, or fit will be executed. This
 *  mimics the behavior of Jasmine.js.
 */
const force_multipleCanvasGM = (name, callback) => {
    fmultipleCanvasGM(name, callback);
};

/**
 *  pause_multipleCanvasGM is like fmultipleCanvasGM, except the test will not
 *  finish right away and clear, making it ideal for a human to manually inspect the results.
 */
const pause_multipleCanvasGM = (name, callback) => {
    _commonMultipleCanvasGM(fit, true, name, callback);
};

/**
 *  skip_multipleCanvasGM does nothing. It is a convenient way to skip a test temporarily.
 */
const skip_multipleCanvasGM = (name, callback) => {
    console.log(`Skipping multiple canvas gm ${name}`);
};


function reportSurface(surface, testname, done) {
    // In docker, the webgl canvas is blank, but the surface has the pixel
    // data. So, we copy it out and draw it to a normal canvas to take a picture.
    // To be consistent across CPU and GPU, we just do it for all configurations
    // (even though the CPU canvas shows up after flush just fine).
    let pixels = surface.getCanvas().readPixels(0, 0, {
        width: CANVAS_WIDTH,
        height: CANVAS_HEIGHT,
        colorType: CanvasKit.ColorType.RGBA_8888,
        alphaType: CanvasKit.AlphaType.Unpremul,
        colorSpace: CanvasKit.ColorSpace.SRGB,
    });
    if (!pixels) {
        throw 'Could not get pixels for test '+testname;
    }
    pixels = new Uint8ClampedArray(pixels.buffer);
    const imageData = new ImageData(pixels, CANVAS_WIDTH, CANVAS_HEIGHT);

    const reportingCanvas = document.getElementById('report');
    if (!reportingCanvas) {
        throw 'Reporting canvas not found';
    }
    reportingCanvas.getContext('2d').putImageData(imageData, 0, 0);
    if (!done) {
        return;
    }
    reportCanvas(reportingCanvas, testname).then(() => {
        surface.delete();
        done();
    }).catch(reportError(done));
}


function starPath(CanvasKit, X=128, Y=128, R=116) {
    const p = new CanvasKit.Path();
    p.moveTo(X + R, Y);
    for (let i = 1; i < 8; i++) {
      let a = 2.6927937 * i;
      p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
    }
    p.close();
    return p;
}
