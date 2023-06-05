struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    testMatrix3x3: mat3x3<f32>,
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_from_mat3x3f32(x0: mat3x3<f32>) -> mat2x2<f32> {
    return mat2x2<f32>(vec2<f32>(x0[0].xy), vec2<f32>(x0[1].xy));
}
fn mat3x3f32_from_mat2x2f32(x0: mat2x2<f32>) -> mat3x3<f32> {
    return mat3x3<f32>(vec3<f32>(x0[0].xy, 0.0), vec3<f32>(x0[1].xy, 0.0), vec3<f32>(0.0, 0.0, 1.0));
}
fn resizeMatrix_f22() -> mat2x2<f32> {
    return mat2x2f32_from_mat3x3f32(_globalUniforms.testMatrix3x3);
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    let _skTemp2 = resizeMatrix_f22();
    let _skTemp3 = _skTemp2;
    let _skTemp4 = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(4.0, 5.0));
    if (all(_skTemp3[0] == _skTemp4[0]) && all(_skTemp3[1] == _skTemp4[1])) {
        let _skTemp5 = resizeMatrix_f22();
        let _skTemp6 = mat3x3f32_from_mat2x2f32(_skTemp5);
        let _skTemp7 = mat3x3<f32>(vec3<f32>(1.0, 2.0, 0.0), vec3<f32>(4.0, 5.0, 0.0), vec3<f32>(0.0, 0.0, 1.0));
        _skTemp1 = (all(_skTemp6[0] == _skTemp7[0]) && all(_skTemp6[1] == _skTemp7[1]) && all(_skTemp6[2] == _skTemp7[2]));
    } else {
        _skTemp1 = false;
    }
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
