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
fn main(c: vec2<f32>) -> vec4<f32> {
    var ok: bool = true;
    var i: i32 = 5;
    let _skTemp0 = &(i);
    (*_skTemp0) += i32(1);
    ok = ok && i == 6;
    var _skTemp1: bool;
    if ok {
        let _skTemp2 = &(i);
        (*_skTemp2) += i32(1);
        _skTemp1 = (*_skTemp2) == 7;
    } else {
        _skTemp1 = false;
    }
    ok = _skTemp1;
    var _skTemp3: bool;
    if ok {
        let _skTemp4 = &(i);
        (*_skTemp4) -= i32(1);
        _skTemp3 = (*_skTemp4) == 6;
    } else {
        _skTemp3 = false;
    }
    ok = _skTemp3;
    let _skTemp5 = &(i);
    (*_skTemp5) -= i32(1);
    ok = ok && i == 5;
    var f: f32 = 0.5;
    let _skTemp6 = &(f);
    (*_skTemp6) += f32(1);
    ok = ok && f == 1.5;
    var _skTemp7: bool;
    if ok {
        let _skTemp8 = &(f);
        (*_skTemp8) += f32(1);
        _skTemp7 = (*_skTemp8) == 2.5;
    } else {
        _skTemp7 = false;
    }
    ok = _skTemp7;
    var _skTemp9: bool;
    if ok {
        let _skTemp10 = &(f);
        (*_skTemp10) -= f32(1);
        _skTemp9 = (*_skTemp10) == 1.5;
    } else {
        _skTemp9 = false;
    }
    ok = _skTemp9;
    let _skTemp11 = &(f);
    (*_skTemp11) -= f32(1);
    ok = ok && f == 0.5;
    ok = ok && !(_globalUniforms.colorGreen.x == 1.0);
    var val: u32 = u32(_globalUniforms.colorGreen.x);
    var mask: vec2<u32> = vec2<u32>(val, ~val);
    var imask: vec2<i32> = vec2<i32>(~mask);
    mask = ~mask & vec2<u32>(~imask);
    ok = ok && all(mask == vec2<u32>(0u));
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
