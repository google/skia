### Compilation failed:

error: :2:1 error: chromium_experimental_pixel_local requires TINT_ENABLE_PIXEL_LOCAL_EXTENSION
enable chromium_experimental_pixel_local;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


diagnostic(off, derivative_uniformity);
enable chromium_experimental_pixel_local;
struct PixelLocalData {
  i: i32,
  f: f32,
};
var<pixel_local> pls: PixelLocalData;
fn _skslMain() {
  {
    pls.i = pls.i + i32(1);
    pls.f = pls.f * 2.0;
  }
}
@fragment fn main() {
  _skslMain();
}

1 error
