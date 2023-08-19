diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn unpremul_h4h4(_skParam0: vec4<f32>) -> vec4<f32> {
  let color = _skParam0;
  {
    let _skTemp0 = max(color.w, 0.0001);
    return vec4<f32>(color.xyz / _skTemp0, color.w);
  }
}
fn live_fn_h4h4h4(_skParam0: vec4<f32>, _skParam1: vec4<f32>) -> vec4<f32> {
  let a = _skParam0;
  let b = _skParam1;
  {
    return a + b;
  }
}
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  let coords = _skParam0;
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
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
