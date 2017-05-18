function interp(A, B, t) {
    return A + (B - A) * t;
}

function interp_cubic_coords(x1, x2, x3, x4, t)
{
    var ab = interp(x1, x2, t);
    var bc = interp(x2, x3, t);
    var cd = interp(x3, x4, t);
    var abc = interp(ab, bc, t);
    var bcd = interp(bc, cd, t);
    var abcd = interp(abc, bcd, t);
    return abcd;
}

// FIXME : only works for path with single cubic
function path_partial(value, path) {
    assert(isArray(path));
    var out = [];
    for (var cIndex = 0; cIndex < path.length; ++cIndex) {
        out[cIndex] = {};
        var curveKey = Object.keys(path[cIndex])[0];
        var curve = path[cIndex][curveKey];
        var outArray;
        switch (curveKey) {
            case "cubic":
                var x1 = curve[0], y1 = curve[1], x2 = curve[2], y2 = curve[3];
                var x3 = curve[4], y3 = curve[5], x4 = curve[6], y4 = curve[7];
                var t1 = 0, t2 = value;
                var ax = interp_cubic_coords(x1, x2, x3, x4, t1);
                var ay = interp_cubic_coords(y1, y2, y3, y4, t1);
                var ex = interp_cubic_coords(x1, x2, x3, x4, (t1*2+t2)/3);
                var ey = interp_cubic_coords(y1, y2, y3, y4, (t1*2+t2)/3);
                var fx = interp_cubic_coords(x1, x2, x3, x4, (t1+t2*2)/3);
                var fy = interp_cubic_coords(y1, y2, y3, y4, (t1+t2*2)/3);
                var dx = interp_cubic_coords(x1, x2, x3, x4, t2);
                var dy = interp_cubic_coords(y1, y2, y3, y4, t2);
                var mx = ex * 27 - ax * 8 - dx;
                var my = ey * 27 - ay * 8 - dy;
                var nx = fx * 27 - ax - dx * 8;
                var ny = fy * 27 - ay - dy * 8;
                var bx = (mx * 2 - nx) / 18;
                var by = (my * 2 - ny) / 18;
                var cx = (nx * 2 - mx) / 18;
                var cy = (ny * 2 - my) / 18;
                outArray = [
                    ax, ay, bx, by, cx, cy, dx, dy
                ];
                break;
            default:
                assert(0);  // unimplemented
        }
        out[cIndex][curveKey] = outArray;
    }
    return out;
}

function interp_paths(value, paths) {
    assert(isArray(paths));
    assert(paths.length == 2);
    var curves0 = paths[0];
    assert(isArray(curves0));
    var curves1 = paths[1];
    assert(isArray(curves1));
    assert(curves0.length == curves1.length);
    var out = [];
    for (var cIndex = 0; cIndex < curves0.length; ++cIndex) {
        out[cIndex] = {};
        var curve0Key = Object.keys(curves0[cIndex])[0];
        var curve1Key = Object.keys(curves1[cIndex])[0];
        assert(curve0Key == curve1Key);
        var curve0 = curves0[cIndex][curve0Key];
        var curve1 = curves1[cIndex][curve1Key];
        assert(isArray(curve0));
        assert(isArray(curve1));
        assert(curve0.length == curve1.length);
        var outArray = [];
        for (var i = 0; i < curve1.length; ++i) {
            outArray[i] = curve0[i] + (curve1[i] - curve0[i]) * value;
        }
        out[cIndex][curve0Key] = outArray;
    }
    return out;
}
