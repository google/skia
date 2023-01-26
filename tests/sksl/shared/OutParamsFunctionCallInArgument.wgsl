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
fn _outParamHelper_1_out_param_func2_ih(testArray: ptr<function, array<f32, 2>>) -> i32 {
    var _var0: f32;
    var _return: i32 = out_param_func2_ih(&_var0);
    (*testArray)[0] = _var0;
    return _return;
}
fn _outParamHelper_2_out_param_func2_ih(testArray: ptr<function, array<f32, 2>>) -> i32 {
    var _var0: f32;
    var _return: i32 = out_param_func2_ih(&_var0);
    (*testArray)[0] = _var0;
    return _return;
}
fn _outParamHelper_0_out_param_func1_vh(testArray: ptr<function, array<f32, 2>>) {
    var _var0: f32 = (*testArray)[_outParamHelper_1_out_param_func2_ih(&(*testArray))];
    out_param_func1_vh(&_var0);
    (*testArray)[_outParamHelper_2_out_param_func2_ih(&(*testArray))] = _var0;
}
fn out_param_func1_vh(v: ptr<function, f32>) {
    (*v) = _globalUniforms.colorGreen.y;
}
fn out_param_func2_ih(v: ptr<function, f32>) -> i32 {
    (*v) = _globalUniforms.colorRed.x;
    return i32((*v));
}
fn main(c: vec2<f32>) -> vec4<f32> {
    var testArray: array<f32, 2>;
    _outParamHelper_0_out_param_func1_vh(&testArray);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(testArray[0] == 1.0 && testArray[1] == 1.0));
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
