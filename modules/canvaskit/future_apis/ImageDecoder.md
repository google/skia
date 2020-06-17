# ImageDecoder API

Date Updated: June 16, 2020

## Summary and Links

The [ImageDecoder API](https://github.com/dalecurtis/image-decoder-api/blob/master/explainer.md)
handles decoding of both still and animated images.
Similar to the larger [web codecs](https://github.com/WICG/web-codecs/blob/master/explainer.md)
proposal which is focused more on video and audio.
The ImageDecoder API could be used with `CanvasKit.MakeImageFromCanvasImageSource`
for creating CanvasKit compatible `SkImage`s.
For still images, the `createImageBitmap(blob)` API achieves the same result.

- [Explainer](https://github.com/dalecurtis/image-decoder-api/blob/master/explainer.md)
- [Prototype](https://chromium-review.googlesource.com/c/chromium/src/+/2145133)
- [Discourse](https://discourse.wicg.io/t/proposal-imagedecoder-api-extension-for-webcodecs/4418)

Currently available as a prototype behind the `--enable-blink-features=WebCodecs` flag
in Chrome Canary, works in version 85.0.4175.0.

## Running the prototype

1. Download and install [Chrome Canary](https://www.google.com/chrome/canary/). Verify that you
have version 85.0.4175.0 or later.
2. Close ALL open instances of chromium browsers, including chrome.
2. Run Chrome Canary with the `--enable-blink-features=WebCodecs` flag.

**MacOS**: Run `/applications/Google\ Chrome\ Canary.app/Contents/MacOS/Google\ Chrome\ Canary --enable-blink-features=WebCodecs`

**Windows, Linux**: [https://www.chromium.org/developers/how-tos/run-chromium-with-flags](https://www.chromium.org/developers/how-tos/run-chromium-with-flags)

3. Navigate to: [http://storage.googleapis.com/dalecurtis/test-gif.html?src=giphy.gif](http://storage.googleapis.com/dalecurtis/test-gif.html?src=giphy.gif)
4. You should see a cute animated cat illustration.

## Example API Usage with CanvasKit

With a still image:
```jsx
const response = await fetch(stillImageUrl); // e.g. png or jpeg
const data = await response.arrayBuffer();

const imageDecoder = new ImageDecoder({ data });
const imageBitmap = await imageDecoder.decode();

const skImage = CanvasKit.MakeImageFromCanvasImageSource(imageBitmap);
// do something with skImage, such as drawing it
```

With an animated image:
```jsx
const response = await fetch(animatedImageUrl); // e.g. gif or mjpeg
const data = await response.arrayBuffer();

const imageDecoder = new ImageDecoder({ data });

for (let frame = 0; frame < imageDecoder.frameCount; frame++) {
    const imageBitmap = await imageDecoder.decode(frame);
    const skImage = CanvasKit.MakeImageFromCanvasImageSource(imageBitmap);
    // do something with skImage, such as drawing it
}
```
