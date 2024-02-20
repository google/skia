diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool = true;
    ok = ok && (_globalUniforms.colorGreen.y >= _globalUniforms.colorGreen.x);
    ok = ok && (_globalUniforms.colorGreen.y > _globalUniforms.colorGreen.x);
    ok = ok && (_globalUniforms.colorGreen.z <= _globalUniforms.colorGreen.y);
    ok = ok && (_globalUniforms.colorGreen.z < _globalUniforms.colorGreen.y);
    ok = ok && (_globalUniforms.colorGreen.y >= _globalUniforms.colorGreen.w);
    ok = ok && (_globalUniforms.colorGreen.x <= _globalUniforms.colorGreen.z);
    ok = ok && (_globalUniforms.colorGreen.y != _globalUniforms.colorGreen.x);
    ok = ok && (_globalUniforms.colorGreen.y == _globalUniforms.colorGreen.w);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
