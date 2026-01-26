diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct VSIn {
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @location(1) @interpolate(flat, either) id: i32,
  @builtin(position) sk_Position: vec4<f32>,
};
fn fn_i(_stageIn: VSIn) -> i32 {
  {
    return i32(_stageIn.sk_VertexID);
  }
}
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).id = fn_i(_stageIn);
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
