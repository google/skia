
function ASSERT(pred) {
    console.assert(pred, 'assert failed');
}

function LOG(...args) {
    // comment out for non-debugging
//    console.log(args);
}

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
    return lines[lines.length - 1].textRange.last;
}

function runs_index_to_run(runs, index) {
    for (const r of runs) {
        if (index <= r.offsets[r.offsets.length-1]) {
            return r;
        }
    }
    return runs[runs.length-1];     // last run
}

function runs_index_to_x(runs, index) {
  const r = runs_index_to_run(runs, index);
  for (const i in r.offsets) {
      if (index == r.offsets[i]) {
          return r.positions[i*2];
      }
  }
  return r.positions[r.positions.length-2]; // last x
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

function make_default_paint() {
    const p = new CanvasKit.Paint();
    p.setAntiAlias(true);
    return p;
}

function make_default_font(tf) {
    const font = new CanvasKit.Font(tf);
    font.setSubpixel(true);
    return font;
}

function MakeStyle(length) {
    return {
        _length: length,
        typeface: null,
        size: null,
        color: null,
        bold: null,
        italic: null,
        underline: null,

        _check_toggle: function(src, dst) {
            if (src == 'toggle') {
                return !dst;
            } else {
                return src;
            }
        },

        // returns true if we changed something affecting layout
        mergeFrom: function(src) {
            let layoutChanged = false;

            if (src.typeface && this.typeface !== src.typeface) {
                this.typeface = src.typeface;
                layoutChanged = true;
            }
            if (src.size && this.size !== src.size) {
                this.size = src.size;
                layoutChanged = true;
            }
            if (src.color)    { this.color  = src.color; }

            if (src.bold) {
                this.bold = this._check_toggle(src.bold, this.bold);
            }
            if (src.italic) {
                this.italic = this._check_toggle(src.italic, this.italic);
            }
            if (src.underline) {
                this.underline = this._check_toggle(src.underline, this.underline);
            }

            if (src.size_add) {
                this.size += src.size_add;
                layoutChanged = true;
            }

            return layoutChanged;
        }
    };
}

function MakeEditor(text, style, cursor, width) {
    const ed = {
        _text: text,
        _lines: null,
        _cursor: cursor,
        _width: width,
        _index: { start: 0, end: 0 },
        _styles: null,
        // drawing
        _X: 0,
        _Y: 0,
        _paint: make_default_paint(),
        _font: make_default_font(style.typeface),

        getLines: function() { return this._lines; },

        width: function() {
            return this._width;
        },
        height: function() {
            return this._lines[this._lines.length-1].bottom;
        },
        bounds: function() {
            return [this._X, this._Y, this._X + this.width(), this._Y + this.height()];
        },
        setXY: function(x, y) {
            this._X = x;
            this._Y = y;
        },

        _rebuild_selection: function() {
            const a = this._index.start;
            const b = this._index.end;
            ASSERT(a >= 0 && a <= b && b <= this._text.length);
            if (a === b) {
                const l = lines_index_to_line(this._lines, a);
                const x = runs_index_to_x(l.runs, a);
                this._cursor.place(x, l.top, l.bottom);
            } else {
                this._cursor.setPath(lines_indices_to_path(this._lines, a, b, this._width));
            }
        },
        setIndex: function(i) {
            this._index.start = this._index.end = i;
            this._rebuild_selection();
        },
        setIndices: function(a, b) {
            if (a > b) { [a, b] = [b, a]; }
            this._index.start = a;
            this._index.end = b;
            this._rebuild_selection();
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

        _validateStyles: function() {
            let len = 0;
            for (const s of this._styles) {
                len += s._length;
            }
            ASSERT(len === this._text.length);
        },
        _validateBlocks: function(blocks) {
            let len = 0;
            for (const b of blocks) {
                len += b.length;
            }
            ASSERT(len === this._text.length);
        },

        _buildLines: function() {
            this._validateStyles();

            const build_sparse = true;
            const blocks = [];
            let block = null;
            for (const s of this._styles) {
                if (build_sparse) {
                if (!block || (block.typeface === s.typeface && block.size === s.size)) {
                    if (!block) {
                        block = { length: 0, typeface: s.typeface, size: s.size };
                    }
                    block.length += s._length;
                } else {
                    blocks.push(block);
                    block = { length: s._length, typeface: s.typeface, size: s.size };
                }
                } else {
                // force a block on every style boundary for now
                blocks.push({ length: s._length, typeface: s.typeface, size: s.size });
                }
            }
            if (build_sparse) {
                blocks.push(block);
            }
            this._validateBlocks(blocks);

            this._lines = CanvasKit.ParagraphBuilder.ShapeText(this._text, blocks, this._width);
            this._rebuild_selection();

            // add textRange to each run, to aid in drawing
            this._runs = [];
            for (const l of this._lines) {
                for (const r of l.runs) {
                    r.textRange = { start: r.offsets[0], end: r.offsets[r.offsets.length-1] };
                    this._runs.push(r);
                }
            }
        },

        // note: this does not rebuild lines/runs, or update the cursor,
        //       but it does edit the text and styles
        // returns true if it deleted anything
        _deleteRange: function(start, end) {
            ASSERT(start >= 0 && end <= this._text.length);
            ASSERT(start <= end);
            if (start === end) {
                return false;
            }

            this._delete_style_range(start, end);
            // Do this after shrink styles (we use text.length in an assert)
            this._text = string_del(this._text, start, end);
        },
        deleteSelection: function() {
            let start = this._index.start;
            if (start == this._index.end) {
                if (start == 0) {
                    return;     // nothing to do
                }
                this._deleteRange(start - 1, start);
                start -= 1;
            } else {
                this._deleteRange(start, this._index.end);
            }
            this._index.start = this._index.end = start;
            this._buildLines();
        },
        insert: function(charcode) {
            if (this._index.start != this._index.end) {
                this.deleteSelection();
            }
            const index = this._index.start;

            // do this before edit the text (we use text.length in an assert)
            const [i, prev_len] = this.find_style_index_and_prev_length(index);
            this._styles[i]._length += 1;

            // now grow the text
            this._text = this._text.slice(0, index) + charcode + this._text.slice(index);

            this._index.start = this._index.end = index + 1;
            this._buildLines();
        },

        draw: function(canvas) {
            canvas.save();
            canvas.translate(this._X, this._Y);

            this._cursor.draw_before(canvas);

            const runs = this._runs;
            const styles = this._styles;
            const f = this._font;
            const p = this._paint;

            let s = styles[0];
            let sindex = 0;
            let s_start = 0;
            let s_end = s._length;

            let r = runs[0];
            let rindex = 0;

            let start = 0;
            let end = 0;
            while (start < this._text.length) {
                while (r.textRange.end <= start) {
                    r = runs[++rindex];
                    if (!r) {
                        // ran out of runs, so the remaining text must just be WS
                        break;
                    }
                }
                if (!r) break;
                while (s_end <= start) {
                    s = styles[++sindex];
                    s_start = s_end;
                    s_end += s._length;
                }
                end = Math.min(r.textRange.end, s_end);

                LOG('New range: ', start, end,
                    'from run', r.textRange.start, r.textRange.end,
                    'style', s_start, s_end);

                // check that we have anything to draw
                if (r.textRange.start >= end) {
                    start = end;
                    continue;  // could be a span of WS with no glyphs
                }

//              f.setTypeface(r.typeface); // r.typeface is always null (for now)
                f.setSize(r.size);
                f.setEmbolden(s.bold);
                f.setSkewX(s.italic ? -0.2 : 0);
                p.setColor(s.color ? s.color : [0,0,0,1]);

                let gly = r.glyphs;
                let pos = r.positions;
                if (start > r.textRange.start || end < r.textRange.end) {
                    // search for the subset of glyphs to draw
                    let glyph_start, glyph_end;
                    for (let i = 0; i < r.offsets.length; ++i) {
                        if (r.offsets[i] >= start) {
                            glyph_start = i;
                            break;
                        }
                    }
                    for (let i = glyph_start+1; i < r.offsets.length; ++i) {
                        if (r.offsets[i] >= end) {
                            glyph_end = i;
                            break;
                        }
                    }
                    LOG('    glyph subrange', glyph_start, glyph_end);
                    gly = gly.slice(glyph_start, glyph_end);
                    // +2 at the end so we can see the trailing position (esp. for underlines)
                    pos = pos.slice(glyph_start*2, glyph_end*2 + 2);
                } else {
                    LOG('    use entire glyph run');
                }
                canvas.drawGlyphs(gly, pos, 0, 0, f, p);

                if (s.underline) {
                    const gap = 2;
                    const Y = pos[1];   // first Y
                    const lastX = pos[gly.length*2];
                    const sects = f.getGlyphIntercepts(gly, pos, Y+2, Y+4);

                    let x = pos[0];
                    for (let i = 0; i < sects.length; i += 2) {
                        const end = sects[i] - gap;
                        if (x < end) {
                            canvas.drawRect([x, Y+2, end, Y+4], p);
                        }
                        x = sects[i+1] + gap;
                    }
                    if (x < lastX) {
                        canvas.drawRect([x, Y+2, lastX, Y+4], p);
                    }
                }

                start = end;
            }

            this._cursor.draw_after(canvas);
            canvas.restore();
        },

        // Styling

        // returns [index, prev total length before this style]
        find_style_index_and_prev_length: function(index) {
            let len = 0;
            for (let i = 0; i < this._styles.length; ++i) {
                const l = this._styles[i]._length;
                len += l;
                // < favors the latter style if index is between two styles
                if (index < len) {
                    return [i, len - l];
                }
            }
            ASSERT(len === this._text.length);
            return [this._styles.length-1, len];
        },
        _delete_style_range: function(start, end) {
            // shrink/remove styles
            //
            // [.....][....][....][.....]  styles
            //    [..................]     start...end
            //
            // - trim the first style
            // - remove the middle styles
            // - trim the last style

            let N = end - start;
            let [i, prev_len] = this.find_style_index_and_prev_length(start);
            let s = this._styles[i];
            if (start > prev_len) {
                // we overlap the first style (but not entirely
                const skip = start - prev_len;
                ASSERT(skip < s._length);
                const shrink = Math.min(N, s._length - skip);
                ASSERT(shrink > 0);
                s._length -= shrink;
                N -= shrink;
                if (N === 0) {
                    return;
                }
                i += 1;
                ASSERT(i < this._styles.length);
            }
            while (N > 0) {
                s = this._styles[i];
                if (N >= s._length) {
                    N -= s._length;
                    this._styles.splice(i, 1);
                } else {
                    s._length -= N;
                    break;
                }
            }
        },

        applyStyleToRange: function(style, start, end) {
            if (start > end) { [start, end] = [end, start]; }
            ASSERT(start >= 0 && end <= this._text.length);
            if (start === end) {
                return;
            }

            LOG('trying to apply', style, start, end);
            let i;
            for (i = 0; i < this._styles.length; ++i) {
                if (start <= this._styles[i]._length) {
                    break;
                }
                start -= this._styles[i]._length;
                end -= this._styles[i]._length;
            }

            let s = this._styles[i];
            // do we need to fission off a clean subset for the head of s?
            if (start > 0) {
                const ns = Object.assign({}, s);
                s._length = start;
                ns._length -= start;
                LOG('initial splice', i, start, s._length, ns._length);
                i += 1;
                this._styles.splice(i, 0, ns);
                end -= start;
                // we don't use start any more
            }
            // merge into any/all whole styles we overlap
            let layoutChanged = false;
            while (end >= this._styles[i]._length) {
                LOG('whole run merging for style index', i)
                layoutChanged |= this._styles[i].mergeFrom(style);
                end -= this._styles[i]._length;
                i += 1;
                if (end == 0) {
                    break;
                }
            }
            // do we partially cover the last run
            if (end > 0) {
                s = this._styles[i];
                const ns = Object.assign({}, s);    // the new first half
                ns._length = end;
                s._length -= end;                   // trim the (unchanged) tail
                LOG('merging tail', i, ns._length, s._length);
                layoutChanged |= ns.mergeFrom(style);
                this._styles.splice(i, 0, ns);
            }

            this._validateStyles();
            LOG('after applying styles', this._styles);

            if (layoutChanged) {
                this._buildLines();
            }
        },
        applyStyleToSelection: function(style) {
            this.applyStyleToRange(style, this._index.start, this._index.end);
        },
    };

    const s = MakeStyle(ed._text.length);
    s.mergeFrom(style);
    ed._styles = [ s ];
    ed._buildLines();
    return ed;
}
