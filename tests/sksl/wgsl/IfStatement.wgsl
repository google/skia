struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_stageOut: ptr<function, FSOut>) {
    if (_globalUniforms.colorGreen.y == 1.0) {
        (*_stageOut).sk_FragColor = _globalUniforms.colorGreen;
    }
    else if (_globalUniforms.colorRed.x == 1.0) {
        if (_globalUniforms.colorRed.y == 0.0) {
            (*_stageOut).sk_FragColor = _globalUniforms.colorGreen;
        }
        else {
            (*_stageOut).sk_FragColor = _globalUniforms.colorRed;
        }
    }
    else {
        (*_stageOut).sk_FragColor = _globalUniforms.colorRed;
    }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    main(&_stageOut);
    return _stageOut;
}
