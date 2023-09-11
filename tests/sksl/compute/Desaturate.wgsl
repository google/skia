diagnostic(off, derivative_uniformity);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
@group(0) @binding(0) var src: texture_2d<f32>;
@group(0) @binding(1) var dest: texture_storage_2d<rgba8unorm, write>;
fn _skslMain(_stageIn: CSIn) {
  {
    let _skTemp0 = textureDimensions(src);
    let _skTemp1 = textureDimensions(src);
    if (_stageIn.sk_GlobalInvocationID.x < _skTemp0.x) && (_stageIn.sk_GlobalInvocationID.y < _skTemp1.y) {
      {
        let _skTemp2 = _stageIn.sk_GlobalInvocationID.xy;
        var _0_color: vec4<f32> = textureLoad(src, _skTemp2, 0);
        let _skTemp3 = dot(_0_color.xyz, vec3<f32>(0.22, 0.67, 0.11));
        _0_color = vec4<f32>((vec3<f32>(_skTemp3)), _0_color.w).xyzw;
        textureStore(dest, _stageIn.sk_GlobalInvocationID.xy, _0_color);
      }
    }
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
