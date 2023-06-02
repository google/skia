struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
fn mat4x3f32_diagonal(x: f32) -> mat4x3<f32> {
    return mat4x3<f32>(x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0);
}
fn mat4x3f32_from_mat4x4f32(x0: mat4x4<f32>) -> mat4x3<f32> {
    return mat4x3<f32>(vec3<f32>(x0[0].xyz), vec3<f32>(x0[1].xyz), vec3<f32>(x0[2].xyz), vec3<f32>(x0[3].xyz));
}
fn mat4x4f32_diagonal(x: f32) -> mat4x4<f32> {
    return mat4x4<f32>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn main(_stageOut: ptr<function, FSOut>) {
    var a: mat4x3<f32> = mat4x3f32_diagonal(1.0);
    var b: mat4x3<f32> = mat4x3f32_from_mat4x4f32(mat4x4f32_diagonal(1.0));
    (*_stageOut).sk_FragColor.x = f32(select(1, 0, all(a[0] == b[0])));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
