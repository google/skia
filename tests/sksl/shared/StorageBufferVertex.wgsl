diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct VSIn {
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
};
/* unsupported */ var<private> sk_PointSize: f32;
struct storageBuffer {
  vertices: array<vec2<f32>>,
};
@group(0) @binding(0) var<storage, read> _storage0 : storageBuffer;
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).sk_Position = vec4<f32>(_storage0.vertices[i32(_stageIn.sk_VertexID)], 1.0, 1.0);
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
