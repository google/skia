diagnostic(off, derivative_uniformity);
struct VSIn {
  @builtin(instance_index) sk_InstanceID: u32,
};
struct VSOut {
  @location(1) @interpolate(flat) id: i32,
  @builtin(position) sk_Position: vec4<f32>,
};
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).id = i32(_stageIn.sk_InstanceID);
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
