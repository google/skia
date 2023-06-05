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
fn mat2x3f32_from_vec4f32_vec2f32(x0: vec4<f32>, x1: vec2<f32>) -> mat2x3<f32> {
    return mat2x3<f32>(vec3<f32>(x0.xyz), vec3<f32>(x0.w, x1.xy));
}
fn mat2x4f32_from_vec3f32_vec4f32_f32(x0: vec3<f32>, x1: vec4<f32>, x2: f32) -> mat2x4<f32> {
    return mat2x4<f32>(vec4<f32>(x0.xyz, x1.x), vec4<f32>(x1.yzw, x2));
}
fn mat3x3f32_from_vec2f32_vec2f32_vec4f32_f32(x0: vec2<f32>, x1: vec2<f32>, x2: vec4<f32>, x3: f32) -> mat3x3<f32> {
    return mat3x3<f32>(vec3<f32>(x0.xy, x1.x), vec3<f32>(x1.y, x2.xy), vec3<f32>(x2.zw, x3));
}
fn mat4x2f32_from_vec3f32_vec4f32_f32(x0: vec3<f32>, x1: vec4<f32>, x2: f32) -> mat4x2<f32> {
    return mat4x2<f32>(vec2<f32>(x0.xy), vec2<f32>(x0.z, x1.x), vec2<f32>(x1.yz), vec2<f32>(x1.w, x2));
}
fn mat4x3f32_from_f32_vec4f32_vec4f32_vec3f32(x0: f32, x1: vec4<f32>, x2: vec4<f32>, x3: vec3<f32>) -> mat4x3<f32> {
    return mat4x3<f32>(vec3<f32>(x0, x1.xy), vec3<f32>(x1.zw, x2.x), vec3<f32>(x2.yzw), vec3<f32>(x3.xyz));
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var f4: vec4<f32> = vec4<f32>(_globalUniforms.testMatrix2x2[0], _globalUniforms.testMatrix2x2[1]);
    let _skTemp0 = mat2x3f32_from_vec4f32_vec2f32(f4, f4.xy);
    let _skTemp1 = mat2x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 1.0, 2.0));
    var ok: bool = (all(_skTemp0[0] == _skTemp1[0]) && all(_skTemp0[1] == _skTemp1[1]));
    let _skTemp2 = mat2x4f32_from_vec3f32_vec4f32_f32(f4.xyz, f4.wxyz, f4.w);
    let _skTemp3 = mat2x4<f32>(vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(1.0, 2.0, 3.0, 4.0));
    ok = ok && (all(_skTemp2[0] == _skTemp3[0]) && all(_skTemp2[1] == _skTemp3[1]));
    let _skTemp4 = mat3x3f32_from_vec2f32_vec2f32_vec4f32_f32(f4.xy, f4.zw, f4, f4.x);
    let _skTemp5 = mat3x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 1.0, 2.0), vec3<f32>(3.0, 4.0, 1.0));
    ok = ok && (all(_skTemp4[0] == _skTemp5[0]) && all(_skTemp4[1] == _skTemp5[1]) && all(_skTemp4[2] == _skTemp5[2]));
    let _skTemp6 = mat4x2f32_from_vec3f32_vec4f32_f32(f4.xyz, f4.wxyz, f4.w);
    let _skTemp7 = mat4x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0), vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    ok = ok && (all(_skTemp6[0] == _skTemp7[0]) && all(_skTemp6[1] == _skTemp7[1]) && all(_skTemp6[2] == _skTemp7[2]) && all(_skTemp6[3] == _skTemp7[3]));
    let _skTemp8 = mat4x3f32_from_f32_vec4f32_vec4f32_vec3f32(f4.x, f4.yzwx, f4.yzwx, f4.yzw);
    let _skTemp9 = mat4x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 1.0, 2.0), vec3<f32>(3.0, 4.0, 1.0), vec3<f32>(2.0, 3.0, 4.0));
    ok = ok && (all(_skTemp8[0] == _skTemp9[0]) && all(_skTemp8[1] == _skTemp9[1]) && all(_skTemp8[2] == _skTemp9[2]) && all(_skTemp8[3] == _skTemp9[3]));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
