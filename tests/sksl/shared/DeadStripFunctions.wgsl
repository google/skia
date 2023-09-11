diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn unpremul_h4h4(color: vec4<f32>) -> vec4<f32> {
  {
    let _skTemp0 = max(color.w, 0.0001);
    return vec4<f32>(color.xyz / _skTemp0, color.w);
  }
}
fn live_fn_h4h4h4(a: vec4<f32>, b: vec4<f32>) -> vec4<f32> {
  {
    return a + b;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var a: vec4<f32>;
    var b: vec4<f32>;
    {
      let _skTemp1 = live_fn_h4h4h4(vec4<f32>(3.0), vec4<f32>(-5.0));
      a = _skTemp1;
    }
    {
      let _skTemp2 = unpremul_h4h4(vec4<f32>(1.0));
      b = _skTemp2;
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(any(a != vec4<f32>(0.0)) && any(b != vec4<f32>(0.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
