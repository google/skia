
/*
 *  When calling makeShader, pass
 *      radius_scale    0.5
 *      center          x, y
 *      color0          r, g, b, a
 *      color1          r, g, b, a
 */
function MakeOffscreen(CanvasKit, x, y) {
    const w = 540;
    const h = 540;

    const surf = CanvasKit.MakeSurface(w, h);
/*
    drawPatch(cubics: InputFlattenedPointArray,
              colors?: ColorIntArray | Color[] | null,
              texs?: InputFlattenedPointArray | null,
              mode?: BlendMode | null,
              paint?: Paint): void;
*/
    const pts = new Float32Array([
        0, 0,
        w*0.33, 0,
        w*0.67, 0,
        w, 0,
        w, h*0.33,
        w, h*0.67,
        w, h,
        w*0.67, h,
        w*0.33, h,
        0, h,
        0, h*0.67,
        0, h*0.33,
    ]);

    const texs = new Float32Array([ 0, 0, w, 0, w, h, 0, h ]);

    const ptsPaint = new CanvasKit.Paint();
    ptsPaint.setStrokeWidth(8);
    ptsPaint.setStyle(CanvasKit.PaintStyle.Stroke);

    const obj = {
        _surface: surf,
        _canvas: surf.getCanvas(),
        _x: x,
        _y: y,

        _pts: pts,
        _texs: texs,
        _paint: new CanvasKit.Paint(),
        _ptsPaint: ptsPaint,

        getCanvas: function() {
            this._canvas.clear(CanvasKit.WHITE);
            return this._canvas;
        },

        draw: function(canvas) {
            canvas.save();
            canvas.translate(this._x, this._y);

            const img = this._surface.makeImageSnapshot();
            if (false) {
                canvas.drawImage(img, 0, 0, null);
            } else {
                const sh = img.makeShaderOptions(CanvasKit.TileMode.Repeat,
                                                 CanvasKit.TileMode.Repeat,
                                                 CanvasKit.FilterMode.Linear,
                                                 CanvasKit.MipmapMode.None);
                this._paint.setShader(sh);
                sh.delete();

                canvas.drawPatch(this._pts, null, this._texs, null, this._paint);
            }
            img.delete();

            canvas.drawPoints(CanvasKit.PointMode.Points, this._pts, this._ptsPaint);

            canvas.restore();
        },
    };

    return obj;
}
