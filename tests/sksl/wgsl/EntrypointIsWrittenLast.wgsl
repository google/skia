diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct Uniforms {
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _uniform0 : Uniforms;
fn this_function_is_defined_before_use_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    let _skTemp1 = this_function_is_defined_near_the_end_h4h4(x);
    return -_skTemp1;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp2 = this_function_is_defined_after_use_h4h4(_uniform0.colorGreen);
    return _skTemp2;
  }
}
fn this_function_is_defined_after_use_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    let _skTemp3 = this_function_is_defined_before_use_h4h4(-x);
    return _skTemp3;
  }
}
fn this_function_is_defined_near_the_end_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    return x;
  }
}
@fragment fn main(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
