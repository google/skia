diagnostic(off, derivative_uniformity);
struct VSIn {
  @builtin(instance_index) sk_InstanceID: u32,
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
};
/* unsupported */ var<private> sk_PointSize: f32;
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    var x: f32 = f32(i32(_stageIn.sk_VertexID));
    var y: f32 = f32(i32(_stageIn.sk_InstanceID));
    sk_PointSize = x;
    (*_stageOut).sk_Position = vec4<f32>(x, y, 1.0, 1.0);
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
