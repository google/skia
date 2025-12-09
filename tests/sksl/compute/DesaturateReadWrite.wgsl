diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
@group(0) @binding(0) var texIn: texture_2d<f32>;
@group(0) @binding(1) var texOut: texture_storage_2d<rgba32float, write>;
fn _skslMain(_stageIn: CSIn) {
  {
    if (_stageIn.sk_GlobalInvocationID.x < textureDimensions(texIn).x) && (_stageIn.sk_GlobalInvocationID.y < textureDimensions(texIn).y) {
      {
        var _0_color: vec4<f32> = textureLoad(texIn, _stageIn.sk_GlobalInvocationID.xy, 0);
        _0_color = vec4<f32>((vec3<f32>(dot(_0_color.xyz, vec3<f32>(0.22, 0.67, 0.11)))), _0_color.w);
        let gray: vec4<f32> = _0_color;
        textureStore(texOut, _stageIn.sk_GlobalInvocationID.xy, gray);
      }
    }
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
