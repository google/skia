diagnostic(off, derivative_uniformity);
struct VSIn {
  @builtin(instance_index) sk_InstanceID: u32,
  @builtin(vertex_index) sk_VertexID: u32,
};
struct VSOut {
  @builtin(position) sk_Position: vec4<f32>,
  @location(1) vcoord_Stage0: vec2<f32>,
};
/* unsupported */ var<private> sk_PointSize: f32;
fn _skslMain(_stageIn: VSIn, _stageOut: ptr<function, VSOut>) {
  {
    var x: i32 = i32(_stageIn.sk_InstanceID) % 200;
    var y: i32 = i32(_stageIn.sk_InstanceID) / 200;
    var ileft: i32 = (i32(_stageIn.sk_InstanceID) * 929) % 17;
    var iright: i32 = (ileft + 1) + (i32(_stageIn.sk_InstanceID) * 1637) % (17 - ileft);
    var itop: i32 = (i32(_stageIn.sk_InstanceID) * 313) % 17;
    var ibot: i32 = (itop + 1) + (i32(_stageIn.sk_InstanceID) * 1901) % (17 - itop);
    var outset: f32 = 0.03125;
    outset = select(outset, -outset, 0 == ((x + y) % 2));
    var l: f32 = f32(ileft) * 0.0625 - outset;
    var r: f32 = f32(iright) * 0.0625 + outset;
    var t: f32 = f32(itop) * 0.0625 - outset;
    var b: f32 = f32(ibot) * 0.0625 + outset;
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
