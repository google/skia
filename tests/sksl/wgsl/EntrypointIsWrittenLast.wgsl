diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct Uniforms {
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _uniform0 : Uniforms;
fn this_function_is_defined_before_use_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    return -this_function_is_defined_near_the_end_h4h4(x);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    return this_function_is_defined_after_use_h4h4(_uniform0.colorGreen);
  }
}
fn this_function_is_defined_after_use_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    return this_function_is_defined_before_use_h4h4(-x);
  }
}
fn this_function_is_defined_near_the_end_h4h4(x: vec4<f32>) -> vec4<f32> {
  {
    return x;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
