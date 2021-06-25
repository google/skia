// Shared benchmarking functions such as surface creation, measurement, publishing to perf.skia.org

function getSurface(CanvasKit, webglversion) {
  let surface;
  if (window.location.hash.indexOf('gpu') !== -1) {
    surface = CanvasKit.MakeWebGLCanvasSurface('anim', null /* colorspace */, {'majorVersion': webglversion});
    if (!surface) {
      window._error = 'Could not make GPU surface';
      return null;
    }
    let c = document.getElementById('anim');
    // If CanvasKit was unable to instantiate a WebGL context, it will fallback
    // to CPU and add a ck-replaced class to the canvas element.
    if (c.classList.contains('ck-replaced')) {
      window._error = 'fell back to CPU';
      return null;
    }
    if (webglversion !== surface.openGLversion) {
      window._error = 'Want WebGL version '+webglversion+' but got '+surface.openGLversion;
      return null;
    }
  } else {
    surface = CanvasKit.MakeSWCanvasSurface('anim');
    if (!surface) {
      window._error = 'Could not make CPU surface';
      return null;
    }
  }
  return surface;
}

// Time the drawing and flushing of a frame drawing function on a given surface.
// drawFn is expected to be a zero arg function making draw calls to a canvas
// warmupFrames - Run this number of frames before starting to measure things.
//   This allows us to make sure the noise from the first few renders (e.g shader
//   compilation, caches) is removed from the data we capture.
// Stops after timeoutMillis if provided
// Teturns a promise that resolves with the dict of measurements.
function startTimingFrames(drawFn, surface, warmupFrames, maxFrames, timeoutMillis) {
  return new Promise((resolve, reject) => {
    const totalFrame = new Float32Array(maxFrames);
    const withFlush = new Float32Array(maxFrames);
    const withoutFlush = new Float32Array(maxFrames);
    let warmUp = warmupFrames > 0;
    let idx = -1;
    let previousFrame;

    function drawFrame() {
      let start, afterDraw, end;
      try {
        start = performance.now();
        drawFn();
        afterDraw = performance.now();
        surface.flush();
        end = performance.now();
      } catch (e) {
        console.error(e);
        window._error = e.stack || e.toString();
        return;
      }

      if (warmUp) {
        idx++;
        if (idx >= warmupFrames) {
          idx = -1;
          warmUp = false;
        }
        window.requestAnimationFrame(drawFrame);
        return;
      }
      if (idx >= 0) {
        // Fill out total time the previous frame took to draw.
        totalFrame[idx] = start - previousFrame;
      }
      previousFrame = start;
      idx++;
      // If we have maxed out the frames we are measuring or have completed the animation,
      // we stop benchmarking.
      if (!window._perfData) {
        window._perfData = {};
      }
      if (idx >= withFlush.length) {
        resolve({
          // The total time elapsed between the same point during the drawing of each frame.
          // This is the most relevant measurement for normal drawing tests.
          'total_frame_ms': Array.from(totalFrame).slice(0, idx),
          // The time taken to run the code under test and call surface.flush()
          'with_flush_ms': Array.from(withFlush).slice(0, idx),
          // The time taken to run the code under test
          // This is the most relevant measurement for non-drawing tests such as matrix inversion.
          'without_flush_ms': Array.from(withoutFlush).slice(0, idx),
        });
        return;
      }

      // We can fill out this frame's intermediate steps.
      withFlush[idx] = end - start;
      withoutFlush[idx] = afterDraw - start;

      if (timeoutMillis && ((beginTest + timeoutMillis) < performance.now())) {
        console.log(`test aborted due to timeout after ${idx} frames`);
        reject(`test aborted due to timeout after ${idx} frames`);
        return;
      }
      window.requestAnimationFrame(drawFrame);
    }
    const beginTest = performance.now();
    window.requestAnimationFrame(drawFrame);
  }); // new promise
}
