diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  testInputs: vec4<f32>,
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    const constVal: vec4<f32> = vec4<f32>(-1.25, 0.0, 0.75, 2.25);
    const expectedA: vec4<f32> = vec4<f32>(0.0, 0.0, 0.84375, 1.0);
    const expectedB: vec4<f32> = vec4<f32>(1.0, 0.0, 1.0, 1.0);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((0.0 == expectedA.x) && all(vec2<f32>(0.0) == expectedA.xy)) && all(vec3<f32>(0.0, 0.0, 0.84375) == expectedA.xyz)) && all(vec4<f32>(0.0, 0.0, 0.84375, 1.0) == expectedA)) && (0.0 == expectedA.x)) && all(vec2<f32>(0.0) == expectedA.xy)) && all(vec3<f32>(0.0, 0.0, 0.84375) == expectedA.xyz)) && all(vec4<f32>(0.0, 0.0, 0.84375, 1.0) == expectedA)) && (smoothstep(_globalUniforms.colorRed.y, _globalUniforms.colorGreen.y, -1.25) == expectedA.x)) && all(smoothstep(vec2<f32>(_globalUniforms.colorRed.y), vec2<f32>(_globalUniforms.colorGreen.y), vec2<f32>(-1.25, 0.0)) == expectedA.xy)) && all(smoothstep(vec3<f32>(_globalUniforms.colorRed.y), vec3<f32>(_globalUniforms.colorGreen.y), vec3<f32>(-1.25, 0.0, 0.75)) == expectedA.xyz)) && all(smoothstep(vec4<f32>(_globalUniforms.colorRed.y), vec4<f32>(_globalUniforms.colorGreen.y), constVal) == expectedA)) && (1.0 == expectedB.x)) && all(vec2<f32>(1.0, 0.0) == expectedB.xy)) && all(vec3<f32>(1.0, 0.0, 1.0) == expectedB.xyz)) && all(vec4<f32>(1.0, 0.0, 1.0, 1.0) == expectedB)) && (smoothstep(_globalUniforms.colorRed.x, _globalUniforms.colorGreen.x, -1.25) == expectedB.x)) && all(smoothstep(_globalUniforms.colorRed.xy, _globalUniforms.colorGreen.xy, vec2<f32>(-1.25, 0.0)) == expectedB.xy)) && all(smoothstep(_globalUniforms.colorRed.xyz, _globalUniforms.colorGreen.xyz, vec3<f32>(-1.25, 0.0, 0.75)) == expectedB.xyz)) && all(smoothstep(_globalUniforms.colorRed, _globalUniforms.colorGreen, constVal) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
