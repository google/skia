diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
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
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(((((((((f16(f1) + f16(f2)) + f16(f3)) + f16(i1)) + f16(i2)) + f16(i3)) + f16(b1)) + f16(b2)) + f16(b3)) == 9.0h));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
