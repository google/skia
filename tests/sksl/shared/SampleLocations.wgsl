diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct VSIn {
  @builtin(instance_index) sk_InstanceID: u32,
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
  @location(1) @interpolate(linear) vcoord_Stage0: vec2<f32>,
};
/* unsupported */ var<private> sk_PointSize: f32;
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    let x: i32 = i32(_stageIn.sk_InstanceID) % 200;
    let y: i32 = i32(_stageIn.sk_InstanceID) / 200;
    let ileft: i32 = (i32(_stageIn.sk_InstanceID) * 929) % 17;
    let iright: i32 = (ileft + 1) + (i32(_stageIn.sk_InstanceID) * 1637) % (17 - ileft);
    let itop: i32 = (i32(_stageIn.sk_InstanceID) * 313) % 17;
    let ibot: i32 = (itop + 1) + (i32(_stageIn.sk_InstanceID) * 1901) % (17 - itop);
    var outset: f32 = 0.03125;
    outset = select(outset, -outset, 0 == ((x + y) % 2));
    let l: f32 = f32(ileft) * 0.0625 - outset;
    let r: f32 = f32(iright) * 0.0625 + outset;
    let t: f32 = f32(itop) * 0.0625 - outset;
    let b: f32 = f32(ibot) * 0.0625 + outset;
    var vertexpos: vec2<f32>;
    vertexpos.x = f32(x) + (select(r, l, 0 == (i32(_stageIn.sk_VertexID) % 2)));
    vertexpos.y = f32(y) + (select(b, t, 0 == (i32(_stageIn.sk_VertexID) / 2)));
    (*_stageOut).vcoord_Stage0.x = f32(select(1, -1, 0 == (i32(_stageIn.sk_VertexID) % 2)));
    (*_stageOut).vcoord_Stage0.y = f32(select(1, -1, 0 == (i32(_stageIn.sk_VertexID) / 2)));
    (*_stageOut).sk_Position = vec4<f32>(vertexpos.x, vertexpos.y, 0.0, 1.0);
  }
}
@vertex fn main(_stageIn: VSIn) -> VSOut {
  var _stageOut: VSOut;
  _skslMain(_stageIn, &_stageOut);
  return _stageOut;
}
