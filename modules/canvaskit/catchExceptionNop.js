// When running the jasmine tests in google3, this can be a nop because catching
// logs is handled already by the testing framework.
// original here http://shortn/_HeVXSB2tRh
function catchException(done, fn) {
    return fn;
}

// This function would normally upload results to gold, but we don't do that
// when running the test in google3. If necessary, the test could have scuba
// turned on to serve that purpose.
function reportSurface(foo, bar, done) {
  done();
}
