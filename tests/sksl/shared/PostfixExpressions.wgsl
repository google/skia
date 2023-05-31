struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(c: vec2<f32>) -> vec4<f32> {
    var ok: bool = true;
    var i: i32 = 5;
    let _skTemp0 = &(i);
    (*_skTemp0) += i32(1);
    var _skTemp1: bool;
    if ok {
        let _skTemp2 = &(i);
        var _skTemp3: i32;
        _skTemp3 = (*_skTemp2);
        (*_skTemp2) += i32(1);
        _skTemp1 = _skTemp3 == 6;
    } else {
        _skTemp1 = false;
    }
    ok = _skTemp1;
    ok = ok && i == 7;
    var _skTemp4: bool;
    if ok {
        let _skTemp5 = &(i);
        var _skTemp6: i32;
        _skTemp6 = (*_skTemp5);
        (*_skTemp5) -= i32(1);
        _skTemp4 = _skTemp6 == 7;
    } else {
        _skTemp4 = false;
    }
    ok = _skTemp4;
    ok = ok && i == 6;
    let _skTemp7 = &(i);
    (*_skTemp7) -= i32(1);
    ok = ok && i == 5;
    var f: f32 = 0.5;
    let _skTemp8 = &(f);
    (*_skTemp8) += f32(1);
    var _skTemp9: bool;
    if ok {
        let _skTemp10 = &(f);
        var _skTemp11: f32;
        _skTemp11 = (*_skTemp10);
        (*_skTemp10) += f32(1);
        _skTemp9 = _skTemp11 == 1.5;
    } else {
        _skTemp9 = false;
    }
    ok = _skTemp9;
    ok = ok && f == 2.5;
    var _skTemp12: bool;
    if ok {
        let _skTemp13 = &(f);
        var _skTemp14: f32;
        _skTemp14 = (*_skTemp13);
        (*_skTemp13) -= f32(1);
        _skTemp12 = _skTemp14 == 2.5;
    } else {
        _skTemp12 = false;
    }
    ok = _skTemp12;
    ok = ok && f == 1.5;
    let _skTemp15 = &(f);
    (*_skTemp15) -= f32(1);
    ok = ok && f == 0.5;
    var f2: vec2<f32> = vec2<f32>(0.5);
    let _skTemp16 = &(f2);
    (*_skTemp16).x += f32(1);
    var _skTemp17: bool;
    if ok {
        let _skTemp18 = &(f2);
        var _skTemp19: f32;
        _skTemp19 = (*_skTemp18).x;
        (*_skTemp18).x += f32(1);
        _skTemp17 = _skTemp19 == 1.5;
    } else {
        _skTemp17 = false;
    }
    ok = _skTemp17;
    ok = ok && f2.x == 2.5;
    var _skTemp20: bool;
    if ok {
        let _skTemp21 = &(f2);
        var _skTemp22: f32;
        _skTemp22 = (*_skTemp21).x;
        (*_skTemp21).x -= f32(1);
        _skTemp20 = _skTemp22 == 2.5;
    } else {
        _skTemp20 = false;
    }
    ok = _skTemp20;
    ok = ok && f2.x == 1.5;
    let _skTemp23 = &(f2);
    (*_skTemp23).x -= f32(1);
    ok = ok && f2.x == 0.5;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
