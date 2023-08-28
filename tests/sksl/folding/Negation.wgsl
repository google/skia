diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn test_ivec_b() -> bool {
  {
    var one: i32 = 1;
    const two: i32 = 2;
    var ok: bool = true;
    ok = ok && all((-vec2<i32>(-one, one + one)) == (-vec2<i32>(one - two, 2)));
    return ok;
  }
}
fn test_mat_b() -> bool {
  {
    var ok: bool = true;
    return ok;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _4_ok: bool = true;
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    if _4_ok {
      let _skTemp3 = test_ivec_b();
      _skTemp2 = _skTemp3;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp4 = test_mat_b();
      _skTemp1 = _skTemp4;
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
