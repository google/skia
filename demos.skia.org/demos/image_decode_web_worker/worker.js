

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