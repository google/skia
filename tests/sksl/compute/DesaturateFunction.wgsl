diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
@group(0) @binding(0) var src: texture_2d<f32>;
@group(0) @binding(1) var dest: texture_storage_2d<rgba32float, write>;
fn desaturate_vTT(_stageIn: CSIn, src: texture_2d<f32>, dest: texture_storage_2d<rgba32float, write>) {
  {
    let _skTemp0 = _stageIn.sk_GlobalInvocationID.xy;
    var color: vec4<f32> = textureLoad(src, _skTemp0, 0);
    let _skTemp1 = dot(color.xyz, vec3<f32>(0.22, 0.67, 0.11));
    color = vec4<f32>((vec3<f32>(_skTemp1)), color.w).xyzw;
    textureStore(dest, _stageIn.sk_GlobalInvocationID.xy, color);
  }
}
fn _skslMain(_stageIn: CSIn) {
  {
    let _skTemp2 = textureDimensions(src);
    let _skTemp3 = textureDimensions(src);
    if (_stageIn.sk_GlobalInvocationID.x < _skTemp2.x) && (_stageIn.sk_GlobalInvocationID.y < _skTemp3.y) {
      {
        desaturate_vTT(_stageIn, src, dest);
      }
    }
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
