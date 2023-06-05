fn d_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                (*i) = (*i) + i32(1);
            } else {
                break;
            }
            continuing {
                x = x + i32(1);
            }
        }
    }
}
fn c_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                var _skTemp0: i32 = (*i);
                d_vi(&_skTemp0);
                (*i) = _skTemp0;
            } else {
                break;
            }
            continuing {
                x = x + i32(1);
            }
        }
    }
}
fn b_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                var _skTemp1: i32 = (*i);
                c_vi(&_skTemp1);
                (*i) = _skTemp1;
            } else {
                break;
            }
            continuing {
                x = x + i32(1);
            }
        }
    }
}
fn a_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                var _skTemp2: i32 = (*i);
                b_vi(&_skTemp2);
                (*i) = _skTemp2;
            } else {
                break;
            }
            continuing {
                x = x + i32(1);
            }
        }
    }
}
fn main(xy: vec2<f32>) -> vec4<f32> {
    var i: i32 = 0;
    var _skTemp3: i32 = i;
    a_vi(&_skTemp3);
    i = _skTemp3;
    return vec4<f32>(0.0);
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
    return main(_coords);
}
