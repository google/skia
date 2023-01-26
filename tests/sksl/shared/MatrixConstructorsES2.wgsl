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
fn vec4f32_from_mat2x2f32(x: mat2x2<f32>) -> vec4<f32> {
    return vec4<f32>(x[0].xy, x[1].xy);
}
fn mat2x2f32_eq_mat2x2f32(left: mat2x2<f32>, right: mat2x2<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
fn mat2x2f32_from_vec3f32_f32(x0: vec3<f32>, x1: f32) -> mat2x2<f32> {
    return mat2x2<f32>(vec2<f32>(x0.xy), vec2<f32>(x0.z, x1));
}
fn mat3x3f32_eq_mat3x3f32(left: mat3x3<f32>, right: mat3x3<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
fn mat3x3f32_from_vec2f32_vec2f32_vec4f32_f32(x0: vec2<f32>, x1: vec2<f32>, x2: vec4<f32>, x3: f32) -> mat3x3<f32> {
    return mat3x3<f32>(vec3<f32>(x0.xy, x1.x), vec3<f32>(x1.y, x2.xy), vec3<f32>(x2.zw, x3));
}
fn mat4x4f32_eq_mat4x4f32(left: mat4x4<f32>, right: mat4x4<f32>) -> bool {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
fn mat4x4f32_from_vec3f32_vec3f32_vec4f32_vec2f32_vec4f32(x0: vec3<f32>, x1: vec3<f32>, x2: vec4<f32>, x3: vec2<f32>, x4: vec4<f32>) -> mat4x4<f32> {
    return mat4x4<f32>(vec4<f32>(x0.xyz, x1.x), vec4<f32>(x1.yz, x2.xy), vec4<f32>(x2.zw, x3.xy), vec4<f32>(x4.xyzw));
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var f4: vec4<f32> = vec4f32_from_mat2x2f32(_globalUniforms.testMatrix2x2);
    var ok: bool = mat2x2f32_eq_mat2x2f32(mat2x2f32_from_vec3f32_f32(f4.xyz, 4.0), mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0)));
    ok = ok && mat3x3f32_eq_mat3x3f32(mat3x3f32_from_vec2f32_vec2f32_vec4f32_f32(f4.xy, f4.zw, f4, f4.x), mat3x3<f32>(vec3<f32>(1.0, 2.0, 3.0), vec3<f32>(4.0, 1.0, 2.0), vec3<f32>(3.0, 4.0, 1.0)));
    ok = ok && mat4x4f32_eq_mat4x4f32(mat4x4f32_from_vec3f32_vec3f32_vec4f32_vec2f32_vec4f32(f4.xyz, f4.wxy, f4.zwxy, f4.zw, f4), mat4x4<f32>(vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(1.0, 2.0, 3.0, 4.0)));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
