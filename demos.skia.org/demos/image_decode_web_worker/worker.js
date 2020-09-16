// This worker listens for a message that is the blob of data that is an encoded image.
// In principle, this worker could also load the image, but I didn't want to introduce
// network lag in this comparison. When it has decoded the image and converted it into
// unpremul bytes, it returns the width, height, and the pixels in an array buffer via
// a worker message.
self.addEventListener('message', (e) => {
    const blob = e.data;
    createImageBitmap(blob).then((bitmap) => {
        const oCanvas = new OffscreenCanvas(bitmap.width, bitmap.height);
        const ctx2d = oCanvas.getContext('2d');
        ctx2d.drawImage(bitmap, 0, 0);

        const imageData = ctx2d.getImageData(0, 0, bitmap.width, bitmap.height);
        const arrayBuffer = imageData.data.buffer;
        self.postMessage({
            width: bitmap.width,
            height: bitmap.height,
            decodedArrayBuffer: arrayBuffer
        }, [
          arrayBuffer // give up ownership of this object
        ]);
    });
});