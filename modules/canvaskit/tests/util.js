// The size of the golden images (DMs)
const CANVAS_WIDTH = 600;
const CANVAS_HEIGHT = 600;

const _commonGM = (it, pause, name, callback, assetsToFetchOrPromisesToWaitOn) => {
    const fetchPromises = [];
    for (const assetOrPromise of assetsToFetchOrPromisesToWaitOn) {
        // https://stackoverflow.com/a/9436948
        if (typeof assetOrPromise === 'string' || assetOrPromise instanceof String) {
            const newPromise = fetch(assetOrPromise)
                .then((response) => response.arrayBuffer());
            fetchPromises.push(newPromise);
        } else if (typeof assetOrPromise.then === 'function') {
            fetchPromises.push(assetOrPromise);
        } else {
            throw 'Neither a string nor a promise ' + assetOrPromise;
        }
    }
    it('draws gm '+name, (done) => {
        const surface = CanvasKit.MakeCanvasSurface('test');
        expect(surface).toBeTruthy('Could not make surface')
        if (!surface) {
            done();
            return;
        }
        // if fetchPromises is empty, the returned promise will
         // resolve right away and just call the callback.
        Promise.all(fetchPromises).then((values) => {
            try {
                callback(surface.getCanvas(), values);
            } catch (e) {
                console.log(`gm ${name} failed with error`, e);
                expect(e).toBeFalsy();
                debugger;
                done();
            }
            surface.flush();
            if (pause) {
                reportSurface(surface, name, null);
            } else {
                reportSurface(surface, name, done);
            }
        }).catch((e) => {
            console.log(`could not load assets for gm ${name}`, e);
            debugger;
            done();
        });
    })
}

const gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(it, false, name, callback, assetsToFetchOrPromisesToWaitOn);
}

const fgm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(fit, false, name, callback, assetsToFetchOrPromisesToWaitOn);
}

const force_gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(fit, false, name, callback, assetsToFetchOrPromisesToWaitOn);
}

const skip_gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(xit, false, name, callback, assetsToFetchOrPromisesToWaitOn);
}

const pause_gm = (name, callback, ...assetsToFetchOrPromisesToWaitOn) => {
    _commonGM(fit, true, name, callback, assetsToFetchOrPromisesToWaitOn);
}

function reportSurface(surface, testname, done) {
    // In docker, the webgl canvas is blank, but the surface has the pixel
    // data. So, we copy it out and draw it to a normal canvas to take a picture.
    // To be consistent across CPU and GPU, we just do it for all configurations
    // (even though the CPU canvas shows up after flush just fine).
    let pixels = surface.getCanvas().readPixels(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    pixels = new Uint8ClampedArray(pixels.buffer);
    const imageData = new ImageData(pixels, CANVAS_WIDTH, CANVAS_HEIGHT);

    const reportingCanvas = document.getElementById('report');
    reportingCanvas.getContext('2d').putImageData(imageData, 0, 0);
    reportCanvas(reportingCanvas, testname).then(() => {
        // TODO(kjlubick): should we call surface.delete() here?
        done();
    }).catch(reportError(done));
}


function starPath(CanvasKit, X=128, Y=128, R=116) {
    const p = new CanvasKit.SkPath();
    p.moveTo(X + R, Y);
    for (let i = 1; i < 8; i++) {
      let a = 2.6927937 * i;
      p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
    }
    p.close();
    return p;
}
