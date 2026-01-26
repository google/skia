diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f16>,
  colorRed: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn _skslMain(_skParam0: vec2<f32>) -> vec4<f16> {
  {
    var ok: bool = true;
    let val: u32 = u32(_globalUniforms.colorGreen.x);
    var mask: vec2<u32> = vec2<u32>(val, ~val);
    let imask: vec2<i32> = vec2<i32>(~mask);
    mask = (~mask) & vec2<u32>(~imask);
    ok = ok && all(mask == vec2<u32>(0u));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(ok));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
