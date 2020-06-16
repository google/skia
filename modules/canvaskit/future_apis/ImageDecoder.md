# ImageDecoder API

Date Updated: June 16, 2020

## Summary and Links

The ImageDecoder API handles decoding of both still and animated images. The ImageDecoder API will fit well with `CanvasKit.MakeImageFromCanvasImageSource` for creating CanvasKit compatible `SkImages`. Similar to the larger [web codecs](https://github.com/WICG/web-codecs/blob/master/explainer.md) proposal which is focused more on video and audio.

- [Explainer](https://github.com/dalecurtis/image-decoder-api/blob/master/explainer.md)
- [Prototype](https://chromium-review.googlesource.com/c/chromium/src/+/2145133)
- [Discourse](https://discourse.wicg.io/t/proposal-imagedecoder-api-extension-for-webcodecs/4418)

Currently available as a prototype behind the `--enable-blink-features=WebCodecs` flag in Chrome Canary.

## Running the prototype

1. Download and install [Chrome Canary](https://www.google.com/chrome/canary/).
2. Run Chrome Canary with the `--enable-blink-features=WebCodecs` flag.

**MacOS**: Run `/applications/Google\ Chrome\ Canary.app/Contents/MacOS/Google\ Chrome\ Canary --enable-blink-features=WebCodecs`

**Windows, Linux**: [https://www.chromium.org/developers/how-tos/run-chromium-with-flags](https://www.chromium.org/developers/how-tos/run-chromium-with-flags)

3. Navigate to: [http://storage.googleapis.com/dalecurtis/test-gif.html?src=giphy.gif](http://storage.googleapis.com/dalecurtis/test-gif.html?src=giphy.gif)
4. You should see a cute animated cat illustration.

## Example API Usage with CanvasKit

```jsx
const response = await fetch(url);
const data = await response.arrayBuffer();

const imageDecoder = new ImageDecoder({ data });
const frameToDecode = 0; // If the image is not animated, then the image is in frame 0.
const imageBitmap = await imageDecoder.decode(frameToDecode);

const skImage = CanvasKit.MakeImageFromCanvasImageSource(imageBitmap);
```
