diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct VSIn {
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @location(1) @interpolate(flat) id: i32,
  @builtin(position) sk_Position: vec4<f32>,
};
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).id = i32(_stageIn.sk_VertexID);
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
