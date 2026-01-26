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
var<private> x: f16 = 1.0h;
fn out_params_are_distinct_from_global_bh(y: ptr<function, f16>) -> bool {
  {
    (*y) = 2.0h;
    return (x == 1.0h) && ((*y) == 2.0h);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var _skTemp0: vec4<f16>;
    var _skTemp1: f16;
    let _skTemp2 = out_params_are_distinct_from_global_bh(&_skTemp1);
    x = _skTemp1;
    if _skTemp2 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
