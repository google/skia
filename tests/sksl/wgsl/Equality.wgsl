struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    f1: f32,
    f2: f32,
    f3: f32,
    h1: f32,
    h2: f32,
    h3: f32,
    v1: vec2<f32>,
    v2: vec2<f32>,
    v3: vec2<f32>,
    hv1: vec2<f32>,
    hv2: vec2<f32>,
    hv3: vec2<f32>,
    m1: mat2x2<f32>,
    m2: mat2x2<f32>,
    m3: mat2x2<f32>,
    hm1: mat2x2<f32>,
    hm2: mat2x2<f32>,
    hm3: mat2x2<f32>,
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main() -> vec4<f32> {
    var ok: bool = true;
    ok = ok && _globalUniforms.f1 == _globalUniforms.f2;
    ok = ok && _globalUniforms.h1 == _globalUniforms.h2;
    ok = ok && _globalUniforms.f1 == f32(_globalUniforms.h2);
    ok = ok && f32(_globalUniforms.h1) == _globalUniforms.f2;
    ok = ok && _globalUniforms.f1 != _globalUniforms.f3;
    ok = ok && _globalUniforms.h1 != _globalUniforms.h3;
    ok = ok && _globalUniforms.f1 != f32(_globalUniforms.h3);
    ok = ok && f32(_globalUniforms.h1) != _globalUniforms.f3;
    ok = ok && all(_globalUniforms.v1 == _globalUniforms.v2);
    ok = ok && all(_globalUniforms.hv1 == _globalUniforms.hv2);
    ok = ok && all(_globalUniforms.v1 == vec2<f32>(_globalUniforms.hv2));
    ok = ok && all(vec2<f32>(_globalUniforms.hv1) == _globalUniforms.v2);
    ok = ok && any(_globalUniforms.v1 != _globalUniforms.v3);
    ok = ok && any(_globalUniforms.hv1 != _globalUniforms.hv3);
    ok = ok && any(_globalUniforms.v1 != vec2<f32>(_globalUniforms.hv3));
    ok = ok && any(vec2<f32>(_globalUniforms.hv1) != _globalUniforms.v3);
    let _skTemp0 = _globalUniforms.m1;
    let _skTemp1 = _globalUniforms.m2;
    ok = ok && (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]));
    let _skTemp2 = _globalUniforms.hm1;
    let _skTemp3 = _globalUniforms.hm2;
    ok = ok && (all(_skTemp2[0] == _skTemp3[0]) && all(_skTemp2[1] == _skTemp3[1]));
    let _skTemp4 = _globalUniforms.m1;
    let _skTemp5 = mat2x2<f32>(_globalUniforms.hm2);
    ok = ok && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]));
    let _skTemp6 = mat2x2<f32>(_globalUniforms.hm1);
    let _skTemp7 = _globalUniforms.m2;
    ok = ok && (all(_skTemp6[0] == _skTemp7[0]) && all(_skTemp6[1] == _skTemp7[1]));
    let _skTemp8 = _globalUniforms.m1;
    let _skTemp9 = _globalUniforms.m3;
    ok = ok && !(all(_skTemp8[0] == _skTemp9[0]) && all(_skTemp8[1] == _skTemp9[1]));
    let _skTemp10 = _globalUniforms.hm1;
    let _skTemp11 = _globalUniforms.hm3;
    ok = ok && !(all(_skTemp10[0] == _skTemp11[0]) && all(_skTemp10[1] == _skTemp11[1]));
    let _skTemp12 = _globalUniforms.m1;
    let _skTemp13 = mat2x2<f32>(_globalUniforms.hm3);
    ok = ok && !(all(_skTemp12[0] == _skTemp13[0]) && all(_skTemp12[1] == _skTemp13[1]));
    let _skTemp14 = mat2x2<f32>(_globalUniforms.hm1);
    let _skTemp15 = _globalUniforms.m3;
    ok = ok && !(all(_skTemp14[0] == _skTemp15[0]) && all(_skTemp14[1] == _skTemp15[1]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main();
    return _stageOut;
}
