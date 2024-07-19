diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    let R_active: vec4<f32> = _globalUniforms.colorGreen;
    let R_centroid: vec4<f32> = _globalUniforms.colorGreen;
    let R_coherent: vec4<f32> = _globalUniforms.colorGreen;
    let R_common: vec4<f32> = _globalUniforms.colorGreen;
    let R_filter: vec4<f32> = _globalUniforms.colorGreen;
    let R_partition: vec4<f32> = _globalUniforms.colorGreen;
    let R_patch: vec4<f32> = _globalUniforms.colorGreen;
    let R_precise: vec4<f32> = _globalUniforms.colorGreen;
    let R_resource: vec4<f32> = _globalUniforms.colorGreen;
    let R_restrict: vec4<f32> = _globalUniforms.colorGreen;
    let R_shared: vec4<f32> = _globalUniforms.colorGreen;
    let R_smooth: vec4<f32> = _globalUniforms.colorGreen;
    let R_subroutine: vec4<f32> = _globalUniforms.colorGreen;
    return (((((((((((R_active * R_centroid) * R_coherent) * R_common) * R_filter) * R_partition) * R_patch) * R_precise) * R_resource) * R_restrict) * R_shared) * R_smooth) * R_subroutine;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
