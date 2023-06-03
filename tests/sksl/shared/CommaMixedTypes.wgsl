struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn mat2x2f32_diagonal(x: f32) -> mat2x2<f32> {
    return mat2x2<f32>(x, 0.0, 0.0, x);
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var result: vec4<f32>;
    result.x = _globalUniforms.colorGreen.x;
    result.y = _globalUniforms.colorGreen.y;
    result.z = _globalUniforms.colorGreen.z;
    result.w = _globalUniforms.colorGreen.w;
    return result;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
