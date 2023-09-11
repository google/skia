diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn getColor_h4h(c: f32) -> vec4<f32> {
  {
    return vec4<f32>(c);
  }
}
fn getFragCoordAugmentedColor_h4h(_stageIn: FSIn, c: f32) -> vec4<f32> {
  {
    let _skTemp0 = getColor_h4h(c);
    return vec4<f32>(_stageIn.sk_FragCoord.xyxy * vec4<f32>(_skTemp0));
  }
}
fn writeColorToOutput_vh(_stageOut: ptr<function, FSOut>, c: f32) {
  {
    let _skTemp1 = getColor_h4h(c);
    (*_stageOut).sk_FragColor = _skTemp1;
  }
}
fn writeToOutput_v(_stageOut: ptr<function, FSOut>) {
  {
    writeColorToOutput_vh(_stageOut, 1.0);
  }
}
fn modifyOutputColor_v(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    let _skTemp2 = getFragCoordAugmentedColor_h4h(_stageIn, 2.0);
    (*_stageOut).sk_FragColor = (*_stageOut).sk_FragColor + _skTemp2;
  }
}
fn _skslMain(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    writeToOutput_v(_stageOut);
    modifyOutputColor_v(_stageIn, _stageOut);
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
