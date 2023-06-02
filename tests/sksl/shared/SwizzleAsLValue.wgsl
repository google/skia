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
fn main(_skAnonymous0: vec2<f32>) -> vec4<f32> {
    var color: vec4<f32> = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    color.w = 2.0;
    color.y = color.y * 4.0;
    color = vec4<f32>((color.yzw * vec3<f32>(0.5)), color.x).wxyz;
    color = (color.zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    color.x = color.x + (select(0.0, color.z, color.w <= 1.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(color == vec4<f32>(1.0, 1.0, 0.25, 1.0))));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
