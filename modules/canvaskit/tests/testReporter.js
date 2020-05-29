const REPORT_URL = 'http://localhost:8081/report_gold_data'
// Set this to enforce that the gold server must be up.
// Typically used for debugging.
const fail_on_no_gold = false;

function reportCanvas(canvas, testname, outputType='canvas') {
    let b64 = canvas.toDataURL('image/png');
    return _report(b64, outputType, testname);
}

// data is a base64 encoded png, outputType is the value that goes with the
// key 'config' when reporting.
function _report(data, outputType, testname) {
    return fetch(REPORT_URL, {
        method: 'POST',
        mode: 'no-cors',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            'output_type': outputType,
            'data': data,
            'test_name': testname,
        })
    }).then(() => console.log(`Successfully reported ${testname} to gold aggregator`));
}

function reportError(testname, done) {
    return (e) => {
        console.log(`Error with fetching after ${testname}. Likely could not connect to aggregator server.`, e.message);
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

function standardizedCanvasSize(ctx) {
    setCanvasSize(ctx, 600, 600);
}

// A wrapper to catch and print a stacktrace to the logs.
// Exceptions normally shows up in the browser console,
// but not in the logs that appear on the bots AND a thrown
// exception will normally cause a test to time out.
// This wrapper mitigates both those pain points.
function catchException(done, fn) {
    return () => {
        try {
            fn()
        } catch (e) {
            console.log('Failed with the following error', e);
            expect(e).toBeFalsy();
            debugger;
            done();
        }
        // We don't call done with finally because
        // that would make the break the asynchronous nature
        // of fn().
    }
}