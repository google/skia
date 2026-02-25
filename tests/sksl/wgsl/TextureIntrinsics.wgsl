diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
@group(1) @binding(0) var texRGBA: texture_storage_2d<rgba8unorm, write>;
@group(1) @binding(1) var texRed: texture_2d<f32>;
fn fill_texture_vTT(passedInTexRGBA: texture_storage_2d<rgba8unorm, write>, passedInTexRed: texture_2d<f32>) {
  {
    let red: vec4<f16> = vec4<f16>(textureLoad(passedInTexRed, vec2<u32>(0u), 0));
    let sizeX: u32 = textureDimensions(passedInTexRGBA).x;
    let sizeY: u32 = textureDimensions(passedInTexRGBA).y;
    var coords: vec2<u32>;
    {
      coords.y = 0u;
      for (; coords.y < sizeY; coords.y = coords.y + u32(1)) {
        {
          {
            coords.x = 0u;
            for (; coords.x < sizeX; coords.x = coords.x + u32(1)) {
              {
                textureStore(passedInTexRGBA, coords, vec4<f32>(red));
              }
            }
          }
        }
      }
    }
  }
}
fn _skslMain() {
  {
    fill_texture_vTT(texRGBA, texRed);
  }
}
@compute @workgroup_size(1, 1, 1) fn main() {
  _skslMain();
}
