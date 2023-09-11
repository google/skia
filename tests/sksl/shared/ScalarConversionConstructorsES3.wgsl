diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var f: f32 = f32(_globalUniforms.colorGreen.y);
    var i: i32 = i32(_globalUniforms.colorGreen.y);
    var u: u32 = u32(_globalUniforms.colorGreen.y);
    var b: bool = bool(_globalUniforms.colorGreen.y);
    var f1: f32 = f;
    var f2: f32 = f32(i);
    var f3: f32 = f32(u);
    var f4: f32 = f32(b);
    var i1: i32 = i32(f);
    var i2: i32 = i;
    var i3: i32 = i32(u);
    var i4: i32 = i32(b);
    var u1: u32 = u32(f);
    var u2: u32 = u32(i);
    var u3: u32 = u;
    var u4: u32 = u32(b);
    var b1: bool = bool(f);
    var b2: bool = bool(i);
    var b3: bool = bool(u);
    var b4: bool = b;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((f32(f1) + f32(f2)) + f32(f3)) + f32(f4)) + f32(i1)) + f32(i2)) + f32(i3)) + f32(i4)) + f32(u1)) + f32(u2)) + f32(u3)) + f32(u4)) + f32(b1)) + f32(b2)) + f32(b3)) + f32(b4)) == 16.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
