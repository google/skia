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
    var ok: bool = true;
    var TRUE: bool = bool(_globalUniforms.colorGreen.y);
    ok = ok && (1 == i32(TRUE));
    ok = ok && (1.0 == f32(TRUE));
    ok = ok && TRUE;
    ok = ok && (1 == i32(TRUE));
    ok = ok && (1.0 == f32(TRUE));
    ok = ok && all(vec2<bool>(true) == vec2<bool>(TRUE));
    ok = ok && all(vec2<i32>(1) == vec2<i32>(i32(TRUE)));
    ok = ok && all(vec2<f32>(1.0) == vec2<f32>(f32(TRUE)));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
