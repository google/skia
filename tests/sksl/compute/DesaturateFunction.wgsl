diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
@group(0) @binding(0) var src: texture_2d<f32>;
@group(0) @binding(1) var dest: texture_storage_2d<rgba32float, write>;
fn desaturate_vTT(_stageIn: CSIn, src: texture_2d<f32>, dest: texture_storage_2d<rgba32float, write>) {
  {
    var color: vec4<f16> = vec4<f16>(textureLoad(src, _stageIn.sk_GlobalInvocationID.xy, 0));
    color = vec4<f16>((vec3<f16>(dot(color.xyz, vec3<f16>(0.22h, 0.67h, 0.11h)))), color.w);
    textureStore(dest, _stageIn.sk_GlobalInvocationID.xy, vec4<f32>(color));
  }
}
fn _skslMain(_stageIn: CSIn) {
  {
    if (_stageIn.sk_GlobalInvocationID.x < textureDimensions(src).x) && (_stageIn.sk_GlobalInvocationID.y < textureDimensions(src).y) {
      {
        desaturate_vTT(_stageIn, src, dest);
      }
    }
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
