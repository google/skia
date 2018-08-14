const REPORT_URL = 'http://localhost:8081/report_gold_data'
// Set this to enforce that the gold server must be up.
// Typically used for debugging.
const fail_on_no_gold = false;

function reportCanvas(canvas, testname) {
    let b64 = canvas.toDataURL('image/png');
    return _report(b64, 'png', testname);
}

function reportSVG(svg, testname) {
    let svgStr = svg.outerHTML;
    return _report(svgStr, 'svg', testname);
}

// For tests that just do a simple path and return it as a string, wrap it in
// a proper svg and send it off.  Supports fill (nofill means just stroke it).
function reportSVGString(svgstr, testname, fillRule='nofill') {
    let newPath = document.createElementNS('http://www.w3.org/2000/svg', 'path');
    newPath.setAttribute('stroke', 'black');
    if (fillRule !== 'nofill') {
        newPath.setAttribute('fill', 'orange');
        newPath.setAttribute('fill-rule', fillRule);
    } else {
        newPath.setAttribute('fill', 'white');
    }
    newPath.setAttribute('d', svgstr);
    let newSVG = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
    newSVG.appendChild(newPath);
    newSVG.setAttribute('width', 200);
    newSVG.setAttribute('height', 200);
    let svgStr = newSVG.outerHTML;
    return _report(svgStr, 'svg', testname);
}

function _report(data, ext, testname) {
    return fetch(REPORT_URL, {
        method: 'POST',
        mode: 'no-cors',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            'ext': ext,
            'data': data,
            'testname': testname,
        })
    }).then(() => console.log(`Successfully reported ${testname} to gold aggregator`));
}

function reportError(done) {
    return (e) => {
        console.log("Error with fetching. Likely could not connect to aggegator server", e.message);
        if (fail_on_no_gold) {
            expect(e).toBeUndefined();
        }
        done();
    };
}

function setCanvasSize(ctx, width, height) {
    ctx.canvas.width = width;
    ctx.canvas.height = height;
}