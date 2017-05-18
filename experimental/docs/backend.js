function displayBackend(displayEngine, displayList) {
    switch (displayEngine) {
        case 'all':
            displayCanvas(displayList);
            displaySvg(displayList);
            break;
        case 'Canvas':
            displayCanvas(displayList);
            break;
        case 'SVG':
            displaySvg(displayList);
            break;
        default:
            assert(0);
    }
}

function keyframeBackendInit(displayEngine, displayList, first) {
    switch (displayEngine) {
        case 'all':
        case 'Canvas':
            keyframeCanvasInit(displayList, first);
            break;
        case 'SVG':
            break;
        default:
            assert(0);
    }
}

function setupBackend(displayEngine) {
    switch (displayEngine) {
        case 'all':
        case 'Canvas':
            setupCanvas();
            setupSvg();
            break;
        case 'SVG':
            setupSvg();
            break;
        default:
            assert(0);
    }
}
