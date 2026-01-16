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
    var ok: bool = true;
    ok = ok && (_globalUniforms.colorGreen.y == 1.0);
    ok = ok && (_globalUniforms.colorGreen.x != 1.0);
    ok = ok && all(_globalUniforms.colorGreen.yx == _globalUniforms.colorRed.xy);
    ok = ok && all(_globalUniforms.colorGreen.yx == _globalUniforms.colorRed.xy);
    ok = ok && (all(_globalUniforms.colorGreen.yx == _globalUniforms.colorRed.xy) || (_globalUniforms.colorGreen.w != _globalUniforms.colorRed.w));
    ok = ok && (any(_globalUniforms.colorGreen.yx != _globalUniforms.colorRed.xy) && (_globalUniforms.colorGreen.w == _globalUniforms.colorRed.w));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
