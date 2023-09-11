diagnostic(off, derivative_uniformity);
@group(1) @binding(0) var texRGBA: texture_storage_2d<rgba8unorm, write>;
@group(1) @binding(1) var texRed: texture_2d<f32>;
fn fill_texture_vTT(passedInTexRGBA: texture_storage_2d<rgba8unorm, write>, passedInTexRed: texture_2d<f32>) {
  {
    let _skTemp0 = vec2<u32>(0u);
    var red: vec4<f32> = textureLoad(passedInTexRed, _skTemp0, 0);
    let _skTemp1 = textureDimensions(passedInTexRGBA);
    var sizeX: u32 = _skTemp1.x;
    let _skTemp2 = textureDimensions(passedInTexRGBA);
    var sizeY: u32 = _skTemp2.y;
    var coords: vec2<u32>;
    {
      coords.y = 0u;

      loop {
        if coords.y < sizeY {
          {
            {
              coords.x = 0u;

              loop {
                if coords.x < sizeX {
                  {
                    textureStore(passedInTexRGBA, coords, red);
                  }
                } else {
                  break;
                }
                continuing {
                  coords.x = coords.x + u32(1);
                }
              }
            }
          }
        } else {
          break;
        }
        continuing {
          coords.y = coords.y + u32(1);
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
