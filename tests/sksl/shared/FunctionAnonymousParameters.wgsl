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
struct S {
    i: i32,
};
fn fnGreen_h4bf2(b: bool, _skAnonymous0: vec2<f32>) -> vec4<f32> {
    return _globalUniforms.colorGreen;
}
fn fnRed_h4ifS(_skAnonymous1: i32, f: f32, _skAnonymous2: S) -> vec4<f32> {
    return _globalUniforms.colorRed;
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _skTemp3: vec4<f32>;
    if bool(_globalUniforms.colorGreen.y) {
        let _skTemp4 = fnGreen_h4bf2(true, coords);
        _skTemp3 = _skTemp4;
    } else {
        let _skTemp5 = fnRed_h4ifS(123, 3.14, S(0));
        _skTemp3 = _skTemp5;
    }
    return _skTemp3;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
