struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _outParamHelper_0_outParameterWrite_vh4(c: ptr<function, vec4<f32>>) {
    var _var0: vec4<f32>;
    outParameterWrite_vh4(&_var0);
    (*c) = _var0;
}
fn _outParamHelper_1_inoutParameterWrite_vh4(x: ptr<function, vec4<f32>>) {
    var _var0: vec4<f32> = (*x);
    inoutParameterWrite_vh4(&_var0);
    (*x) = _var0;
}
fn _outParamHelper_2_outParameterWrite_vh4(c: ptr<function, vec4<f32>>) {
    var _var0: vec4<f32>;
    outParameterWrite_vh4(&_var0);
    (*c) = _var0;
}
fn _outParamHelper_3_outParameterWriteIndirect_vh4(c: ptr<function, vec4<f32>>) {
    var _var0: vec4<f32>;
    outParameterWriteIndirect_vh4(&_var0);
    (*c) = _var0;
}
fn _outParamHelper_4_inoutParameterWrite_vh4(c: ptr<function, vec4<f32>>) {
    var _var0: vec4<f32> = (*c);
    inoutParameterWrite_vh4(&_var0);
    (*c) = _var0;
}
fn _outParamHelper_5_inoutParameterWriteIndirect_vh4(c: ptr<function, vec4<f32>>) {
    var _var0: vec4<f32> = (*c);
    inoutParameterWriteIndirect_vh4(&_var0);
    (*c) = _var0;
}
fn outParameterWrite_vh4(x: ptr<function, vec4<f32>>) {
    (*x) = _globalUniforms.colorGreen;
}
fn outParameterWriteIndirect_vh4(c: ptr<function, vec4<f32>>) {
    _outParamHelper_0_outParameterWrite_vh4(&(*c));
}
fn inoutParameterWrite_vh4(x: ptr<function, vec4<f32>>) {
    (*x) *= (*x);
}
fn inoutParameterWriteIndirect_vh4(x: ptr<function, vec4<f32>>) {
    _outParamHelper_1_inoutParameterWrite_vh4(&(*x));
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var c: vec4<f32>;
    _outParamHelper_2_outParameterWrite_vh4(&c);
    _outParamHelper_3_outParameterWriteIndirect_vh4(&c);
    _outParamHelper_4_inoutParameterWrite_vh4(&c);
    _outParamHelper_5_inoutParameterWriteIndirect_vh4(&c);
    return c;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
