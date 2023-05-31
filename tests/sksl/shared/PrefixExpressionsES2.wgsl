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
    testMatrix2x2: mat2x2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_eq_mat2x2f32(left: mat2x2<f32>, right: mat2x2<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
fn main(_skAnonymous0: vec2<f32>) -> vec4<f32> {
    var ok: bool = true;
    var i: i32 = 5;
    let _skTemp1 = &(i);
    (*_skTemp1) += i32(1);
    ok = ok && i == 6;
    var _skTemp2: bool;
    if ok {
        let _skTemp3 = &(i);
        (*_skTemp3) += i32(1);
        _skTemp2 = (*_skTemp3) == 7;
    } else {
        _skTemp2 = false;
    }
    ok = _skTemp2;
    var _skTemp4: bool;
    if ok {
        let _skTemp5 = &(i);
        (*_skTemp5) -= i32(1);
        _skTemp4 = (*_skTemp5) == 6;
    } else {
        _skTemp4 = false;
    }
    ok = _skTemp4;
    let _skTemp6 = &(i);
    (*_skTemp6) -= i32(1);
    ok = ok && i == 5;
    var f: f32 = 0.5;
    let _skTemp7 = &(f);
    (*_skTemp7) += f32(1);
    ok = ok && f == 1.5;
    var _skTemp8: bool;
    if ok {
        let _skTemp9 = &(f);
        (*_skTemp9) += f32(1);
        _skTemp8 = (*_skTemp9) == 2.5;
    } else {
        _skTemp8 = false;
    }
    ok = _skTemp8;
    var _skTemp10: bool;
    if ok {
        let _skTemp11 = &(f);
        (*_skTemp11) -= f32(1);
        _skTemp10 = (*_skTemp11) == 1.5;
    } else {
        _skTemp10 = false;
    }
    ok = _skTemp10;
    let _skTemp12 = &(f);
    (*_skTemp12) -= f32(1);
    ok = ok && f == 0.5;
    var f2: vec2<f32> = vec2<f32>(0.5);
    let _skTemp13 = &(f2);
    (*_skTemp13).x += f32(1);
    ok = ok && f2.x == 1.5;
    var _skTemp14: bool;
    if ok {
        let _skTemp15 = &(f2);
        (*_skTemp15).x += f32(1);
        _skTemp14 = (*_skTemp15).x == 2.5;
    } else {
        _skTemp14 = false;
    }
    ok = _skTemp14;
    var _skTemp16: bool;
    if ok {
        let _skTemp17 = &(f2);
        (*_skTemp17).x -= f32(1);
        _skTemp16 = (*_skTemp17).x == 1.5;
    } else {
        _skTemp16 = false;
    }
    ok = _skTemp16;
    let _skTemp18 = &(f2);
    (*_skTemp18).x -= f32(1);
    ok = ok && f2.x == 0.5;
    ok = ok && !(_globalUniforms.colorGreen.x == 1.0);
    ok = ok && -1.0 == -_globalUniforms.colorGreen.y;
    ok = ok && all(vec4<f32>(0.0, -1.0, 0.0, -1.0) == -_globalUniforms.colorGreen);
    ok = ok && mat2x2f32_eq_mat2x2f32(mat2x2<f32>(vec2<f32>(-1.0, -2.0), vec2<f32>(-3.0, -4.0)), (-1.0 * _globalUniforms.testMatrix2x2));
    var iv: vec2<i32> = vec2<i32>(i, -i);
    ok = ok && -i == -5;
    ok = ok && all(-iv == vec2<i32>(-5, 5));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
