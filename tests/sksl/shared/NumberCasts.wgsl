diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var B: vec3<bool>;
    B.x = true;
    B.y = true;
    B.z = true;
    var F: vec3<f32>;
    F.x = 1.23;
    F.y = 0.0;
    F.z = 1.0;
    var I: vec3<i32>;
    I.x = 1;
    I.y = 1;
    I.z = 1;
    return vec4<f32>(f32((F.x * F.y) * F.z), f32((B.x && B.y) && B.z), 0.0, f32((I.x * I.y) * I.z));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
