function MakeCursor(CanvasKit) {
    const linePaint = new CanvasKit.Paint();
    linePaint.setColor([0,0,1,1]);
    linePaint.setStyle(CanvasKit.PaintStyle.Stroke);
    linePaint.setStrokeWidth(2);
    linePaint.setAntiAlias(true);

    const pathPaint = new CanvasKit.Paint();
    pathPaint.setColor([0,0,1,0.25]);
    linePaint.setAntiAlias(true);

    return {
        _line_paint: linePaint,    // wrap in weak-ref so we can delete it?
        _path_paint: pathPaint,
        _x: 0,
        _top: 0,
        _bottom: 0,
        _path: null,            // only use x,top,bottom if path is null
        _draws_per_sec: 2,

        // pass 0 for no-draw, pass inf. for always on
        setBlinkRate: function(blinks_per_sec) {
            this._draws_per_sec = blinks_per_sec;
        },
        place: function(x, top, bottom) {
            this._x = x;
            this._top = top;
            this._bottom = bottom;

            this._path = null;
        },
        setPath: function(path) {
            this._path = path;
        },
        draw_before: function(canvas) {
            if (this._path) {
                canvas.drawPath(this._path, this._path_paint);
            }
        },
        draw_after: function(canvas) {
            if (this._path) {
                return;
            }
            if (Math.floor(Date.now() * this._draws_per_sec / 1000) & 1) {
                canvas.drawLine(this._x, this._top, this._x, this._bottom, this._line_paint);
            }
        },
    };
}

function MakeMouse() {
    return {
        _start_x: 0, _start_y: 0,
        _curr_x:  0,  _curr_y: 0,
        _active: false,

        isActive: function() {
            return this._active;
        },
        setDown: function(x, y) {
            this._start_x = this._curr_x = x;
            this._start_y = this._curr_y = y;
            this._active = true;
        },
        setMove: function(x, y) {
            this._curr_x = x;
            this._curr_y = y;
        },
        setUp: function(x, y) {
            this._curr_x = x;
            this._curr_y = y;
            this._active = false;
        },
        getPos: function() {
            return [ this._start_x, this._start_y, this._curr_x, this._curr_y ];
        },
    };
}
