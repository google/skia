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
        getPos: function(dx, dy) {
            return [ this._start_x + dx, this._start_y + dy, this._curr_x + dx, this._curr_y + dy ];
        },
    };
}

function runs_x_to_index(runs, x) {
    for (const r of runs) {
        for (let i = 1; i < r.offsets.length; i += 1) {
            if (x < r.positions[i*2]) {
                const mid = (r.positions[i*2-2] + r.positions[i*2]) * 0.5;
                if (x <= mid) {
                    return r.offsets[i-1];
                } else {
                    return r.offsets[i];
                }
            }
        }
    }
    const r = runs[runs.length-1];
    return r.offsets[r.offsets.length-1];
}

function lines_pos_to_index(lines, x, y) {
    if (y < lines[0].top) {
        return 0;
    }
    for (const l of lines) {
        if (y <= l.bottom) {
            return runs_x_to_index(l.runs, x);
        }
    }
    return lines[lines.length - 1].textRange.last + 1;
}

function runs_index_to_run(runs, index) {
    for (const r of runs) {
        if (index <= r.offsets[r.offsets.length-1]) {
            return r;
        }
    }
    return null;
}

function runs_index_to_x(runs, index) {
  const r = runs_index_to_run(runs, index);
  for (const i in r.offsets) {
      if (index == r.offsets[i]) {
          return r.positions[i*2];
      }
  }
  return null;
}

function lines_index_to_line_index(lines, index) {
  let i = 0;
  for (const l of lines) {
      if (index <= l.textRange.last) {
          return i;
      }
      i += 1;
  }
  return lines.length-1;
}

function lines_index_to_line(lines, index) {
  return lines[lines_index_to_line_index(lines, index)];
}

function lines_index_to_x(lines, index) {
  for (const l of lines) {
      if (index <= l.textRange.last) {
          return runs_index_to_x(l.runs, index);
      }
  }
}

function lines_indices_to_path(lines, a, b, width) {
    if (a == b) {
        return null;
    }
    if (a > b) { [a, b] = [b, a]; }

    const path = new CanvasKit.Path();
    const la = lines_index_to_line(lines, a);
    const lb = lines_index_to_line(lines, b);
    const ax = runs_index_to_x(la.runs, a);
    const bx = runs_index_to_x(lb.runs, b);
    if (la == lb) {
        path.addRect([ax, la.top, bx, la.bottom]);
    } else {
        path.addRect([ax, la.top, width, la.bottom]);
        path.addRect([0, lb.top, bx, lb.bottom]);
        if (la.bottom < lb.top) {
            path.addRect([0, la.bottom, width, lb.top]);   // extra lines inbetween
        }
    }
    return path;
}

function string_del(str, start, end) {
    return str.slice(0, start) + str.slice(end, str.length);
}

function MakeEditor(text, cursor, width, builder) {
    const ed = {
        _text: text,
        _lines: null,
        _builder: builder,
        _cursor: cursor,
        _width: width,
        _index: { start: 0, end: 0 },

        getLines: function() { return this._lines; },

        width: function() {
            return this._width;
        },
        height: function() {
            return this._lines[this._lines.length-1].bottom;
        },

        setIndex: function(i) {
            this._index.start = this._index.end = i;
            const l = lines_index_to_line(this._lines, i);
            const x = runs_index_to_x(l.runs, i);
            this._cursor.place(x, l.top, l.bottom);
        },
        setIndices: function(a, b) {
            if (a > b) { [a, b] = [b, a]; }
            this._index.start = a;
            this._index.end = b;
            this._cursor.setPath(lines_indices_to_path(this._lines, a, b, this._width));
        },
        moveDX: function(dx) {
            let index;
            if (this._index.start == this._index.end) {
                // just adjust and pin
                index = Math.max(Math.min(this._index.start + dx, this._text.length), 0);
            } else {
                // 'deselect' the region, and turn it into just a single index
                index = dx < 0 ? this._index.start : this._index.end;
            }
            this.setIndex(index);
        },
        moveDY: function(dy) {
            let index = (dy < 0) ? this._index.start : this._index.end;
            const i = lines_index_to_line_index(this._lines, index);
            if (dy < 0 && i == 0) {
                index = 0;
            } else if (dy > 0 && i == this._lines.length - 1) {
                index = this._text.length;
            } else {
                const x = runs_index_to_x(this._lines[i].runs, index);
                // todo: statefully track "original" x when an up/down sequence started,
                //       so we can avoid drift.
                index = runs_x_to_index(this._lines[i+dy].runs, x);
            }
            this.setIndex(index);
        },
        _buildLines: function() {
            const builder = this._builder();
            builder.addText(this._text);
            const paragraph = builder.build();
            paragraph.layout(this._width);

            const rec = new CanvasKit.PictureRecorder();
            const can = rec.beginRecording([0,0,9999,9999]);
            can.drawParagraph(paragraph, 0, 0);
            rec.delete();

            this._lines = paragraph.getShapedLines();
            if (!this._lines) throw "null lines";

            paragraph.delete();
            builder.delete();
        },
        deleteSelection: function() {
            let start = this._index.start;
            if (start == this._index.end) {
                if (start > 0) {
                    this._text = string_del(this._text, start - 1, start);
                    start -= 1;
                }
            } else {
                this._text = string_del(this._text,  start, this._index.end);
            }
            this._buildLines();
            this.setIndex(start);
        },
        insert: function(charcode) {
            if (this._index.start != this._index.end) {
                this.deleteSelection();
            }
            const index = this._index.start;
            this._text = this._text.slice(0, index) + charcode + this._text.slice(index);
            this._buildLines();
            this.setIndex(index + 1);
        },
    };
    ed._buildLines();
    return ed;
}
