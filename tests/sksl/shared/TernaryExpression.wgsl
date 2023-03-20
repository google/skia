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
fn main(coords: vec2<f32>) -> vec4<f32> {
    var check: i32 = 0;
    check += i32(select(1, 0, _globalUniforms.colorGreen.y == 1.0));
    check += i32(select(0, 1, _globalUniforms.colorGreen.x == 1.0));
    check += i32(select(1, 0, all(_globalUniforms.colorGreen.yx == _globalUniforms.colorRed.xy)));
    check += i32(select(0, 1, any(_globalUniforms.colorGreen.yx != _globalUniforms.colorRed.xy)));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(check == 0));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
