diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
  colorBlack: vec4<f16>,
  colorWhite: vec4<f16>,
  testInputs: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    let intGreen: vec4<i32> = vec4<i32>(_globalUniforms.colorGreen * 100.0h);
    let intRed: vec4<i32> = vec4<i32>(_globalUniforms.colorRed * 100.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((((((((((((((select(intGreen.x, intRed.x, false) == intGreen.x) && all(select(intGreen.xy, intRed.xy, vec2<bool>(false)) == intGreen.xy)) && all(select(intGreen.xyz, intRed.xyz, vec3<bool>(false)) == intGreen.xyz)) && all(select(intGreen, intRed, vec4<bool>(false)) == intGreen)) && (select(intGreen.x, intRed.x, true) == intRed.x)) && all(select(intGreen.xy, intRed.xy, vec2<bool>(true)) == intRed.xy)) && all(select(intGreen.xyz, intRed.xyz, vec3<bool>(true)) == intRed.xyz)) && all(select(intGreen, intRed, vec4<bool>(true)) == intRed)) && (0 == intGreen.x)) && all(vec2<i32>(0, 100) == intGreen.xy)) && all(vec3<i32>(0, 100, 0) == intGreen.xyz)) && all(vec4<i32>(0, 100, 0, 100) == intGreen)) && (100 == intRed.x)) && all(vec2<i32>(100, 0) == intRed.xy)) && all(vec3<i32>(100, 0, 0) == intRed.xyz)) && all(vec4<i32>(100, 0, 0, 100) == intRed)) && (select(_globalUniforms.colorGreen.x, _globalUniforms.colorRed.x, false) == _globalUniforms.colorGreen.x)) && all(select(_globalUniforms.colorGreen.xy, _globalUniforms.colorRed.xy, vec2<bool>(false)) == _globalUniforms.colorGreen.xy)) && all(select(_globalUniforms.colorGreen.xyz, _globalUniforms.colorRed.xyz, vec3<bool>(false)) == _globalUniforms.colorGreen.xyz)) && all(select(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<bool>(false)) == _globalUniforms.colorGreen)) && (select(_globalUniforms.colorGreen.x, _globalUniforms.colorRed.x, true) == _globalUniforms.colorRed.x)) && all(select(_globalUniforms.colorGreen.xy, _globalUniforms.colorRed.xy, vec2<bool>(true)) == _globalUniforms.colorRed.xy)) && all(select(_globalUniforms.colorGreen.xyz, _globalUniforms.colorRed.xyz, vec3<bool>(true)) == _globalUniforms.colorRed.xyz)) && all(select(_globalUniforms.colorGreen, _globalUniforms.colorRed, vec4<bool>(true)) == _globalUniforms.colorRed)) && (0.0h == _globalUniforms.colorGreen.x)) && all(vec2<f16>(0.0h, 1.0h) == _globalUniforms.colorGreen.xy)) && all(vec3<f16>(0.0h, 1.0h, 0.0h) == _globalUniforms.colorGreen.xyz)) && all(vec4<f16>(0.0h, 1.0h, 0.0h, 1.0h) == _globalUniforms.colorGreen)) && (1.0h == _globalUniforms.colorRed.x)) && all(vec2<f16>(1.0h, 0.0h) == _globalUniforms.colorRed.xy)) && all(vec3<f16>(1.0h, 0.0h, 0.0h) == _globalUniforms.colorRed.xyz)) && all(vec4<f16>(1.0h, 0.0h, 0.0h, 1.0h) == _globalUniforms.colorRed)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
