const REPORT_URL = '/gold_rpc/report';
const pngPrefx = 'data:image/png;base64,'

function reportCanvas(canvas, testname, config) {
    // toDataURL returns a base64 encoded string with a data prefix. We only
    // want the PNG data itself, so we strip that off before submitting it.
    const b64 = canvas.toDataURL('image/png')
                      .substring(pngPrefx.length);
    return fetch(REPORT_URL, {
        method: 'POST',
        mode: 'no-cors',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            'b64_data': b64,
            'name': testname,
            'config': config,
        })
    }).then((resp) => {
        expect(resp.status).toEqual(201); // StatusCreated
        console.log(`${testname}: ${resp.statusText}`);
    });
}

function reportError(done) {
    return (e) => {
        fail(e);
        done();
    };
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
