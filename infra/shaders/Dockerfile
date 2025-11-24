FROM gcr.io/skia-public/shaders-base@sha256:77cefc24c76a774f8397b2f9cf0bced423e8327f7cd47b183817721ba643f4df

COPY canvaskit.js /usr/local/share/shaders/dist/canvaskit.js
COPY canvaskit.wasm /usr/local/share/shaders/dist/canvaskit.wasm
COPY version.js /usr/local/share/shaders/dist/version.js

CMD ["/usr/local/bin/shaders", "--help"]