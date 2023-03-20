struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
    h: f32,
    f: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_diagonal(x: f32) -> mat2x2<f32> {
    return mat2x2<f32>(x, 0.0, 0.0, x);
}
fn mat4x4f32_diagonal(x: f32) -> mat4x4<f32> {
    return mat4x4<f32>(x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, 0.0, x);
}
fn main() -> vec4<f32> {
    var ok: bool = true;
    ok = ok && all(mat2x2f32_diagonal(2.0) * vec2<f32>(_globalUniforms.f) == vec2<f32>(2.0 * _globalUniforms.f));
    ok = ok && all(mat4x4f32_diagonal(2.0) * vec4<f32>(_globalUniforms.h) == vec4<f32>(2.0 * _globalUniforms.h));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main();
    return _stageOut;
}
