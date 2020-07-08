// Shared benchmarking functions such as surface creation, measurement, publishing to perf.skia.org

// TODO(kjlubick) make this configurable to return a WEBGL 1 or WEBGL 2 surface.
function getSurface(CanvasKit) {
  let surface;
  if (window.location.hash.indexOf('gpu') !== -1) {
    surface = CanvasKit.MakeWebGLCanvasSurface('anim');
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
  } else {
    surface = CanvasKit.MakeSWCanvasSurface('anim');
    if (!surface) {
      window._error = 'Could not make CPU surface';
      return null;
    }
  }
  return surface;
}

// time the drawing and flushing of a frame drawing function on a given surface.
// drawFn is expected to be a zero arg function making draw calls to a canvas
// warmupFrames - Run this number of frames before starting to measure things.
//   This allows us to make sure the noise from the first few renders (e.g shader
//   compilation, caches) is removed from the data we capture.
// Stops after timeoutMillis if provided
function startTimingFrames(drawFn, surface, warmupFrames, maxFrames, timeoutMillis) {
  const totalFrame = new Float32Array(maxFrames);
  const withFlush = new Float32Array(maxFrames);
  const withoutFlush = new Float32Array(maxFrames);
  let warmUp = warmupFrames > 0;
  let idx = -1;
  let previousFrame;

  function drawFrame() {
    const start = performance.now();
    drawFn();
    const afterDraw = performance.now();
    surface.flush();
    const end = performance.now();

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
      window._perfData.total_frame_ms = Array.from(totalFrame).slice(0, idx);
      window._perfData.with_flush_ms = Array.from(withFlush).slice(0, idx);
      window._perfData.without_flush_ms = Array.from(withoutFlush).slice(0, idx);
      window._perfDone = true;
      return;
    }

    // We can fill out this frame's intermediate steps.
    withFlush[idx] = end - start;
    withoutFlush[idx] = afterDraw - start;
    
    if (timeoutMillis && ((beginTest + timeoutMillis) < performance.now())) {
      console.log('test aborted due to timeout');
      return;
    }
    window.requestAnimationFrame(drawFrame);
  }
  const beginTest = performance.now();
  window.requestAnimationFrame(drawFrame);
}