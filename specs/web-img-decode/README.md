JS image decode
===============

Background
----------

It is currently cumbersome to go from an encoded Blob or ArrayBuffer of image bytes to
an ImageData (Uint8ClampedArray) for further image processing.
See current/index.html for an example where a user can select an image from disk
and have JS turn it into a grayscale version (no backend server).


Proposal
--------
We propose... See proposed/index.html for an API that makes this much cleaner.
It uses the CanvasKit WASM library under the hood to provide functionality, but
the intent is for Web Browsers to support this natively.