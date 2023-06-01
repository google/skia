fn _outParamHelper_2_d_vi(i: ptr<function, i32>) {
    var _var0: i32 = (*i);
    d_vi(&_var0);
    (*i) = _var0;
}
fn _outParamHelper_4_c_vi(i: ptr<function, i32>) {
    var _var0: i32 = (*i);
    c_vi(&_var0);
    (*i) = _var0;
}
fn _outParamHelper_6_b_vi(i: ptr<function, i32>) {
    var _var0: i32 = (*i);
    b_vi(&_var0);
    (*i) = _var0;
}
fn _outParamHelper_8_a_vi(i: ptr<function, i32>) {
    var _var0: i32 = (*i);
    a_vi(&_var0);
    (*i) = _var0;
}
fn d_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                let _skTemp0 = &((*i));
                (*_skTemp0) += i32(1);
            } else {
                break;
            }
            continuing {
                let _skTemp1 = &(x);
                (*_skTemp1) += i32(1);
            }
        }
    }
}
fn c_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                _outParamHelper_2_d_vi(&(*i));
            } else {
                break;
            }
            continuing {
                let _skTemp3 = &(x);
                (*_skTemp3) += i32(1);
            }
        }
    }
}
fn b_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                _outParamHelper_4_c_vi(&(*i));
            } else {
                break;
            }
            continuing {
                let _skTemp5 = &(x);
                (*_skTemp5) += i32(1);
            }
        }
    }
}
fn a_vi(i: ptr<function, i32>) {
    {
        var x: i32 = 0;
        loop {
            if x < 10 {
                _outParamHelper_6_b_vi(&(*i));
            } else {
                break;
            }
            continuing {
                let _skTemp7 = &(x);
                (*_skTemp7) += i32(1);
            }
        }
    }
}
fn main(xy: vec2<f32>) -> vec4<f32> {
    var i: i32 = 0;
    _outParamHelper_8_a_vi(&i);
    return vec4<f32>(0.0);
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
    return main(_coords);
}
