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
const SEVEN: i32 = 7;
const TEN: i32 = 10;
const MATRIXFIVE: mat4x4<f32> = mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0);
fn verify_const_globals_biih44(seven: i32, ten: i32, matrixFive: mat4x4<f32>) -> bool {
    let _skTemp0 = matrixFive;
    let _skTemp1 = mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0);
    return (seven == 7 && ten == 10) && (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]) && all(_skTemp0[2] == _skTemp1[2]) && all(_skTemp0[3] == _skTemp1[3]));
}
fn main(xy: vec2<f32>) -> vec4<f32> {
    var _skTemp2: vec4<f32>;
    let _skTemp3 = verify_const_globals_biih44(SEVEN, TEN, MATRIXFIVE);
    if _skTemp3 {
        _skTemp2 = _globalUniforms.colorGreen;
    } else {
        _skTemp2 = _globalUniforms.colorRed;
    }
    return _skTemp2;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
