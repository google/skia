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
    var v: vec4<f16> = _globalUniforms.testInputs;
    v = vec4<f16>(v.x, 1.0h, 1.0h, 1.0h);
    v = vec4<f16>(v.xy, 1.0h, 1.0h);
    v = vec4<f16>(v.x, 1.0h, 1.0h, 1.0h);
    v = vec4<f16>(0.0h, v.y, 1.0h, 1.0h);
    v = vec4<f16>(v.xyz, 1.0h);
    v = vec4<f16>(v.xy, 1.0h, 1.0h);
    v = vec4<f16>(v.x, 0.0h, v.z, 1.0h);
    v = vec4<f16>(v.x, 1.0h, 0.0h, 1.0h);
    v = vec4<f16>(1.0h, v.yz, 1.0h);
    v = vec4<f16>(0.0h, v.y, 1.0h, 1.0h);
    v = vec4<f16>(1.0h, 1.0h, v.z, 1.0h);
    v = vec4<f16>(v.xyz, 1.0h);
    v = vec4<f16>(v.xy, 0.0h, v.w);
    v = vec4<f16>(v.xy, 1.0h, 0.0h);
    v = vec4<f16>(v.x, 1.0h, v.zw);
    v = vec4<f16>(v.x, 0.0h, v.z, 1.0h);
    v = vec4<f16>(v.x, 1.0h, 1.0h, v.w);
    v = vec4<f16>(v.x, 1.0h, 0.0h, 1.0h);
    v = vec4<f16>(1.0h, v.yzw);
    v = vec4<f16>(0.0h, v.yz, 1.0h);
    v = vec4<f16>(0.0h, v.y, 1.0h, v.w);
    v = vec4<f16>(1.0h, v.y, 1.0h, 1.0h);
    v = vec4<f16>(0.0h, 0.0h, v.zw);
    v = vec4<f16>(0.0h, 0.0h, v.z, 1.0h);
    v = vec4<f16>(0.0h, 1.0h, 1.0h, v.w);
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(v == vec4<f16>(0.0h, 1.0h, 1.0h, 1.0h))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
