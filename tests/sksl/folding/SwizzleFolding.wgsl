diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _2_ok: bool = true;
    _2_ok = _2_ok && any(_globalUniforms.colorGreen != _globalUniforms.colorRed);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(_2_ok));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
