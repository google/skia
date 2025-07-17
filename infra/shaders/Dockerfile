FROM gcr.io/skia-public/shaders-base@sha256:1cdf37f3bad8e3b929d648d59bf2c1be13486effe58c33716fb2e87ccc59c04d

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]