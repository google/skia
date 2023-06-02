struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
fn mat3x3f32_diagonal(x: f32) -> mat3x3<f32> {
    return mat3x3<f32>(x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, x);
}
fn mat3x3f32_from_mat2x2f32(x0: mat2x2<f32>) -> mat3x3<f32> {
    return mat3x3<f32>(vec3<f32>(x0[0].xy, 0.0), vec3<f32>(x0[1].xy, 0.0), vec3<f32>(0.0, 0.0, 1.0));
}
fn mat2x2f32_diagonal(x: f32) -> mat2x2<f32> {
    return mat2x2<f32>(x, 0.0, 0.0, x);
}
fn main(_stageOut: ptr<function, FSOut>) {
    var a: mat3x3<f32> = mat3x3f32_diagonal(1.0);
    var b: mat3x3<f32> = mat3x3f32_from_mat2x2f32(mat2x2f32_diagonal(1.0));
    (*_stageOut).sk_FragColor.x = f32(select(1, 0, all(a[0] == b[0])));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
