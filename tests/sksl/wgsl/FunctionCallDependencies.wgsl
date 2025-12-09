diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSIn {
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
    return vec4<f32>(_stageIn.sk_FragCoord.xyxy * vec4<f32>(getColor_h4h(c)));
  }
}
fn writeColorToOutput_vh(_stageOut: ptr<function, FSOut>, c: f32) {
  {
    (*_stageOut).sk_FragColor = getColor_h4h(c);
  }
}
fn writeToOutput_v(_stageOut: ptr<function, FSOut>) {
  {
    writeColorToOutput_vh(_stageOut, 1.0);
  }
}
fn modifyOutputColor_v(_stageIn: FSIn, _stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = (*_stageOut).sk_FragColor + getFragCoordAugmentedColor_h4h(_stageIn, 2.0);
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
