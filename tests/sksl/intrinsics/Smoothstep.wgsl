diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  testInputs: vec4<f16>,
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    const constVal: vec4<f16> = vec4<f16>(-1.25h, 0.0h, 0.75h, 2.25h);
    const expectedA: vec4<f16> = vec4<f16>(0.0h, 0.0h, 0.84375h, 1.0h);
    const expectedB: vec4<f16> = vec4<f16>(1.0h, 0.0h, 1.0h, 1.0h);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((((((((((((((((((((0.0h == expectedA.x) && all(vec2<f16>(0.0h) == expectedA.xy)) && all(vec3<f16>(0.0h, 0.0h, 0.84375h) == expectedA.xyz)) && all(vec4<f16>(0.0h, 0.0h, 0.84375h, 1.0h) == expectedA)) && (0.0h == expectedA.x)) && all(vec2<f16>(0.0h) == expectedA.xy)) && all(vec3<f16>(0.0h, 0.0h, 0.84375h) == expectedA.xyz)) && all(vec4<f16>(0.0h, 0.0h, 0.84375h, 1.0h) == expectedA)) && (smoothstep(_globalUniforms.colorRed.y, _globalUniforms.colorGreen.y, -1.25h) == expectedA.x)) && all(smoothstep(vec2<f16>(_globalUniforms.colorRed.y), vec2<f16>(_globalUniforms.colorGreen.y), vec2<f16>(-1.25h, 0.0h)) == expectedA.xy)) && all(smoothstep(vec3<f16>(_globalUniforms.colorRed.y), vec3<f16>(_globalUniforms.colorGreen.y), vec3<f16>(-1.25h, 0.0h, 0.75h)) == expectedA.xyz)) && all(smoothstep(vec4<f16>(_globalUniforms.colorRed.y), vec4<f16>(_globalUniforms.colorGreen.y), constVal) == expectedA)) && (1.0h == expectedB.x)) && all(vec2<f16>(1.0h, 0.0h) == expectedB.xy)) && all(vec3<f16>(1.0h, 0.0h, 1.0h) == expectedB.xyz)) && all(vec4<f16>(1.0h, 0.0h, 1.0h, 1.0h) == expectedB)) && (smoothstep(_globalUniforms.colorRed.x, _globalUniforms.colorGreen.x, -1.25h) == expectedB.x)) && all(smoothstep(_globalUniforms.colorRed.xy, _globalUniforms.colorGreen.xy, vec2<f16>(-1.25h, 0.0h)) == expectedB.xy)) && all(smoothstep(_globalUniforms.colorRed.xyz, _globalUniforms.colorGreen.xyz, vec3<f16>(-1.25h, 0.0h, 0.75h)) == expectedB.xyz)) && all(smoothstep(_globalUniforms.colorRed, _globalUniforms.colorGreen, constVal) == expectedB)));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
