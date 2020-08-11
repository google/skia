const DEFAULT_METHOD = 'SVG';

const worker = new Worker('worker.js');

const svgObjectElement = document.getElementById('svg');
document.getElementById('svg').addEventListener('load', () => {

    const svgElement = svgObjectElement.contentDocument;
    const svgData = svgToPathStringAndFillColorPairs(svgElement);

    // Send svgData and transfer an offscreenCanvas to the worker for Path2D and CanvasKit rendering
    const path2dCanvas =
        document.getElementById('Path2D-canvas').transferControlToOffscreen();
    worker.postMessage({
        svgData: svgData,
        offscreenCanvas: path2dCanvas,
        type: 'Path2D'
    }, [path2dCanvas]);
    const canvasKitCanvas =
        document.getElementById('CanvasKit-canvas').transferControlToOffscreen();
    worker.postMessage({
        svgData: svgData,
        offscreenCanvas: canvasKitCanvas,
        type: 'CanvasKit'
    }, [canvasKitCanvas]);

    // The Canvas2D and CanvasKit rendering methods are executed in a web worker to avoid blocking
    // the main thread. The SVG rendering method is executed in the main thread. SVG rendering is
    // not in a worker because it is not possible - the DOM cannot be accessed from a web worker.
    const svgAnimator = new Animator();
    svgAnimator.renderer = new SVGRenderer(svgObjectElement);
    switchRenderMethodCallback(DEFAULT_METHOD)();

    // Listen to framerate reports from the worker, and update framerate text
    worker.addEventListener('message', ({ data: {renderMethod, framesCount, totalFramesMs} }) => {
        const fps = fpsFromFramesInfo(framesCount, totalFramesMs);
        let textEl;
        if (renderMethod === 'Path2D') {
            textEl = document.getElementById('Path2D-fps');
        }
        if (renderMethod === 'CanvasKit') {
            textEl = document.getElementById('CanvasKit-fps');
        }
        textEl.innerText = `${fps.toFixed(2)} fps over ${framesCount} frames`;
    });
    // Update framerate text every second
    setInterval(() => {
        if (svgAnimator.framesCount > 0) {
            const fps = fpsFromFramesInfo(svgAnimator.framesCount, svgAnimator.totalFramesMs);
            document.getElementById('SVG-fps').innerText =
                `${fps.toFixed(2)} fps over ${svgAnimator.framesCount} frames`;
        }
    }, 1000);

    document.getElementById('SVG-input')
        .addEventListener('click', switchRenderMethodCallback('SVG'));
    document.getElementById('Path2D-input')
        .addEventListener('click', switchRenderMethodCallback('Path2D'));
    document.getElementById('CanvasKit-input')
        .addEventListener('click', switchRenderMethodCallback('CanvasKit'));

    function switchRenderMethodCallback(switchMethod) {
        return () => {
            // Hide all renderer elements and stop svgAnimator
            document.getElementById('CanvasKit-canvas').style.visibility = 'hidden';
            document.getElementById('Path2D-canvas').style.visibility = 'hidden';
            for (const svgEl of svgAnimator.renderer.svgElArray) {
                svgEl.style.visibility = 'hidden';
            }
            svgAnimator.stop();

            // Show only the active renderer element
            if (switchMethod === 'SVG') {
                svgAnimator.start();
                for (const svgEl of svgAnimator.renderer.svgElArray) {
                    svgEl.style.visibility = 'visible';
                }
            }
            if (switchMethod === 'CanvasKit') {
                document.getElementById('CanvasKit-canvas').style.visibility = 'visible';
            }
            if (switchMethod === 'Path2D') {
                document.getElementById('Path2D-canvas').style.visibility = 'visible';
            }
            worker.postMessage({ switchMethod });
        };
    }
});
// Add .data after the load listener so that the listener always fires an event
svgObjectElement.data = 'garbage.svg';

const EMPTY_SVG_PATH_STRING = 'M 0 0';
const COLOR_WHITE = '#000000';
function svgToPathStringAndFillColorPairs(svgElement) {
    const pathElements = Array.from(svgElement.getElementsByTagName('path'));
    return pathElements.map((path) => [
        path.getAttribute('d') ?? EMPTY_SVG_PATH_STRING,
        path.getAttribute('fill') ?? COLOR_WHITE
    ]);
}

const MS_IN_A_SECOND = 1000;
function fpsFromFramesInfo(framesCount, totalFramesMs) {
    const averageFrameTime = totalFramesMs / framesCount;
    return (1 / averageFrameTime) * MS_IN_A_SECOND;
}
