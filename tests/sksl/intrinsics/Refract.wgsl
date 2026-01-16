diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: f32,
  b: f32,
  c: f32,
  d: vec4<f32>,
  e: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    let _skTemp0 = 65504.0;
    var result: vec4<f32> = vec4<f32>(refract(vec2<f32>(_skTemp0 * 2.0, 0), vec2<f32>(2.0, 0), 2.0).x);
    result.x = refract(vec2<f32>(_globalUniforms.a, 0), vec2<f32>(_globalUniforms.b, 0), _globalUniforms.c).x;
    result = refract(_globalUniforms.d, _globalUniforms.e, _globalUniforms.c);
    result = vec4<f32>((vec2<f32>(0.5, -0.8660254)), result.zw);
    result = vec4<f32>((vec3<f32>(0.5, 0.0, -0.8660254)), result.w);
    result = vec4<f32>(0.5, 0.0, 0.0, -0.8660254);
    return result;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
