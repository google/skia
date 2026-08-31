FROM gcr.io/skia-public/skottie-base@sha256:da37ba0ea52c9c3ac37d03af60f3b71b141d0d150cf2a279491f7b2a2a673507

COPY canvaskit.js /usr/local/share/skottie/canvaskit.js
COPY canvaskit.wasm /usr/local/share/skottie/canvaskit.wasm
COPY version.js /usr/local/share/skottie/version.js

CMD ["/usr/local/bin/skottie", "--help"]