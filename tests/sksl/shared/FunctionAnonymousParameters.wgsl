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
struct S {
  i: i32,
};
fn fnGreen_h4bf2(b: bool, _skParam1: vec2<f32>) -> vec4<f16> {
  {
    return _globalUniforms.colorGreen;
  }
}
fn fnRed_h4ifS(_skParam0: i32, f: f32, _skParam2: S) -> vec4<f16> {
  {
    return _globalUniforms.colorRed;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var _skTemp0: vec4<f16>;
    if bool(_globalUniforms.colorGreen.y) {
      _skTemp0 = fnGreen_h4bf2(true, coords);
    } else {
      _skTemp0 = fnRed_h4ifS(123, 3.14, S(0));
    }
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
