diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct CSIn {
  @builtin(global_invocation_id) sk_GlobalInvocationID: vec3<u32>,
};
@group(0) @binding(0) var dest: texture_storage_2d<rgba32float, write>;
fn _skslMain(_stageIn: CSIn) {
  {
    var pixel: vec4<f16> = vec4<f16>(0.0h, 0.0h, 0.0h, 1.0h);
    const max_x: f32 = 5.0;
    const max_y: f32 = 5.0;
    let x: f32 = f32(_stageIn.sk_GlobalInvocationID.x * 2u - textureDimensions(dest).x) / f32(textureDimensions(dest).x);
    let y: f32 = f32(_stageIn.sk_GlobalInvocationID.y * 2u - textureDimensions(dest).y) / f32(textureDimensions(dest).y);
    const ray_origin: vec3<f32> = vec3<f32>(0.0, 0.0, -1.0);
    let ray_target: vec3<f32> = vec3<f32>(x * max_x, y * max_y, 0.0);
    const sphere_center: vec3<f32> = vec3<f32>(0.0, 0.0, -10.0);
    const sphere_radius: f32 = 1.0;
    let t_minus_c: vec3<f32> = ray_target - sphere_center;
    let b: f32 = dot(ray_origin, t_minus_c);
    let c: f32 = dot(t_minus_c, t_minus_c) - sphere_radius * sphere_radius;
    let bsqmc: f32 = b * b - c;
    if bsqmc >= 0.0 {
      {
        pixel = vec4<f16>(0.4h, 0.4h, 1.0h, 1.0h);
      }
    }
    textureStore(dest, _stageIn.sk_GlobalInvocationID.xy, vec4<f32>(pixel));
  }
}
@compute @workgroup_size(16, 16, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
