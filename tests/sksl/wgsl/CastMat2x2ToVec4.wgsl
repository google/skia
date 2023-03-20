struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    f: mat2x2<f32>,
    h: mat2x2<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn vec4f32_from_mat2x2f32(x: mat2x2<f32>) -> vec4<f32> {
    return vec4<f32>(x[0].xy, x[1].xy);
}
fn main(_stageOut: ptr<function, FSOut>) {
    (*_stageOut).sk_FragColor = (vec4f32_from_mat2x2f32(_globalUniforms.h) + vec4<f32>(vec4f32_from_mat2x2f32(_globalUniforms.f))) + vec4<f32>(vec4<f32>(vec4f32_from_mat2x2f32(_globalUniforms.h)) + vec4f32_from_mat2x2f32(_globalUniforms.f));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
