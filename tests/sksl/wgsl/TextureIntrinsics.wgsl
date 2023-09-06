diagnostic(off, derivative_uniformity);
@group(1) @binding(0) var texRGBA: texture_storage_2d<rgba8unorm, write>;
@group(1) @binding(1) var texRed: texture_storage_2d<r32float, write>;
fn use_texture_vTT(passedInTexRGBA: texture_storage_2d<rgba8unorm, write>, passedInTexRed: texture_storage_2d<r32float, write>) {
  {
  }
}
fn _skslMain() {
  {
    use_texture_vTT(texRGBA, texRed);
  }
}
@compute @workgroup_size(1, 1, 1) fn main() {
  _skslMain();
}
