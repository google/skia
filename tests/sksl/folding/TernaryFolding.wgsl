diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn do_side_effect_bb(x: ptr<function, bool>) -> bool {
  {
    (*x) = true;
    return false;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var ok: bool = true;
    var green: vec4<f32> = _globalUniforms.colorGreen;
    var red: vec4<f32> = _globalUniforms.colorRed;
    var param: bool = false;
    var _skTemp0: bool;
    let _skTemp1 = do_side_effect_bb(&_skTemp0);
    param = _skTemp0;
    var call: bool = true;
    return select(red, green, vec4<bool>((ok && param) && call));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
