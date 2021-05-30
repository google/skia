
/*
 *  When calling makeShader, pass
 *      radius_scale    0.5
 *      center          x, y
 *      color0          r, g, b, a
 *      color1          r, g, b, a
 */
function MakeOffscreen(CanvasKit, x, y) {
    const surf = CanvasKit.MakeSurface(540, 540);

    return {
        surface: surf,
        canvas: surf.getCanvas(),
        loc_x: x,
        loc_y: y,

        getCanvas: function() {
            this.canvas.clear(CanvasKit.WHITE);
            return this.canvas;
        },

        draw: function(can) {
            const x = this.loc_x;
            const y = this.loc_y;

            const img = this.surface.makeImageSnapshot();
            can.drawImage(img, x, y, null);
            img.delete();

        },
    };
}
