const REPORT_URL = 'http://localhost:8081/report_perf_data'
// Set this to enforce that the perf server must be up.
// Typically used for debugging.
const fail_on_no_perf = false;

function benchmarkAndReport(benchName, setupFn, testFn, teardownFn) {
    try {
        let ctx = {};
        // warmup 3 times (arbitrary choice)
        setupFn(ctx);
        testFn(ctx);
        testFn(ctx);
        testFn(ctx);
        teardownFn(ctx);

        ctx = {};
        setupFn(ctx);
        let start = Date.now();
        let now = start;
        times = 0;
        // See how many times we can do it in 100ms (arbitrary choice)
        while (now - start < 100) {
            testFn(ctx);
            now = Date.now();
            times++;
        }

        teardownFn(ctx);

        // Try to make it go for 2 seconds (arbitrarily chosen)
        // Since the pre-try took 100ms, multiply by 20 to get
        // approximate tries in 2s (unless now - start >> 100 ms)
        let goalTimes = times * 20;
        ctx = {};
        setupFn(ctx);
        times = 0;
        start = Date.now();
        while (times < goalTimes) {
            testFn(ctx);
            times++;
        }
        const end = Date.now();
        teardownFn(ctx);

        const us = (end - start) * 1000 / times;
        console.log(benchName, `${us} microseconds`)
        return _report(us, benchName);
    } catch(e) {
        console.error('caught error', e);
        return Promise.reject(e);
    }
}


function _report(microseconds, benchName) {
    return fetch(REPORT_URL, {
        method: 'POST',
        mode: 'no-cors',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            'bench_name': benchName,
            'time_us': microseconds,
        })
    }).then(() => console.log(`Successfully reported ${benchName} to perf aggregator`));
}

function reportError(done) {
    return (e) => {
        console.log("Error with fetching. Likely could not connect to aggegator server", e.message);
        if (fail_on_no_perf) {
            expect(e).toBeUndefined();
        }
        done();
    };
}
