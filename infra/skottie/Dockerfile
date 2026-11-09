FROM gcr.io/skia-public/skottie-base@sha256:80de31c865d47495c270d1527cf898816d7107ea540844aab61803da7e188717

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]