### Compilation failed:

error: :10:42 error: unresolved identifier 'vertices'
    (*_stageOut).sk_Position = vec4<f32>(vertices[i32(_stageIn.sk_VertexID)], 1.0, 1.0);
                                         ^^^^^^^^


struct VSIn {
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
};
/* unsupported */ var<private> sk_PointSize: f32;
fn main(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    (*_stageOut).sk_Position = vec4<f32>(vertices[i32(_stageIn.sk_VertexID)], 1.0, 1.0);
  }
}
@vertex fn vertexMain(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  main(_stageIn, &_stageOut);
  return _stageOut;
}

1 error
