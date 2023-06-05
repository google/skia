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
fn mat4x4f32_eq_mat4x4f32(left: mat4x4<f32>, right: mat4x4<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
const SEVEN: i32 = 7;
const TEN: i32 = 10;
const MATRIXFIVE: mat4x4<f32> = mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0);
fn verify_const_globals_biih44(seven: i32, ten: i32, matrixFive: mat4x4<f32>) -> bool {
    return (seven == 7 && ten == 10) && mat4x4f32_eq_mat4x4f32(matrixFive, mat4x4<f32>(5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 5.0));
}
fn main(xy: vec2<f32>) -> vec4<f32> {
    var _skTemp0: vec4<f32>;
    let _skTemp1 = verify_const_globals_biih44(SEVEN, TEN, MATRIXFIVE);
    if _skTemp1 {
        _skTemp0 = _globalUniforms.colorGreen;
    } else {
        _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
