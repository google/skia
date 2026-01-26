diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn do_side_effect_bb(x: ptr<function, bool>) -> bool {
  {
    (*x) = true;
    return false;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const ok: bool = true;
    let green: vec4<f16> = _globalUniforms.colorGreen;
    let red: vec4<f16> = _globalUniforms.colorRed;
    var param: bool = false;
    var _skTemp0: bool;
    do_side_effect_bb(&_skTemp0);
    param = _skTemp0;
    let call: bool = true;
    return select(red, green, vec4<bool>((ok && param) && call));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f16> {
  return _skslMain(_coords);
}
