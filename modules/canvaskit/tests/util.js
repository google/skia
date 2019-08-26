// The size of the golden images (DMs)
const CANVAS_WIDTH = 600;
const CANVAS_HEIGHT = 600;

function reportSurface(surface, testname, done) {
    // In docker, the webgl canvas is blank, but the surface has the pixel
    // data. So, we copy it out and draw it to a normal canvas to take a picture.
    // To be consistent across CPU and GPU, we just do it for all configurations
    // (even though the CPU canvas shows up after flush just fine).
    let pixels = surface.getCanvas().readPixels(0, 0, CANVAS_WIDTH, CANVAS_HEIGHT);
    pixels = new Uint8ClampedArray(pixels.buffer);
    const imageData = new ImageData(pixels, CANVAS_WIDTH, CANVAS_HEIGHT);

    let reportingCanvas =  document.getElementById('report');
    reportingCanvas.getContext('2d').putImageData(imageData, 0, 0);
    reportCanvas(reportingCanvas, testname).then(() => {
        done();
    }).catch(reportError(done));
}


function starPath(CanvasKit, X=128, Y=128, R=116) {
    let p = new CanvasKit.SkPath();
    p.moveTo(X + R, Y);
    for (let i = 1; i < 8; i++) {
      let a = 2.6927937 * i;
      p.lineTo(X + R * Math.cos(a), Y + R * Math.sin(a));
    }
    return p;
  }