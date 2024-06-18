diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
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
    let f: f32 = f32(_globalUniforms.colorGreen.y);
    let i: i32 = i32(_globalUniforms.colorGreen.y);
    let b: bool = bool(_globalUniforms.colorGreen.y);
    let f1: f32 = f;
    let f2: f32 = f32(i);
    let f3: f32 = f32(b);
    let i1: i32 = i32(f);
    let i2: i32 = i;
    let i3: i32 = i32(b);
    let b1: bool = bool(f);
    let b2: bool = bool(i);
    let b3: bool = b;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((f32(f1) + f32(f2)) + f32(f3)) + f32(i1)) + f32(i2)) + f32(i3)) + f32(b1)) + f32(b2)) + f32(b3)) == 9.0));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
