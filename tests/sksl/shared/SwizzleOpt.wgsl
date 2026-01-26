diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct FSOut {
  @location(0) sk_FragColor: vec4<f16>,
};
struct _GlobalUniforms {
  colorRed: vec4<f16>,
  colorGreen: vec4<f16>,
  testInputs: vec4<f16>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn fn_hh4(v: vec4<f16>) -> f16 {
  {
    {
      var x: i32 = 1;
      loop {
        {
          return v.x;
        }
        continuing {
          x = x + i32(1);
          break if x > 2;
        }
      }
    }
  }
  return f16();
}
fn _skslMain(coords: vec2<f32>) -> vec4<f16> {
  {
    var v: vec4<f16> = _globalUniforms.testInputs;
    v = vec4<f16>(0.0h, v.zyx);
    v = vec4<f16>(0.0h, 0.0h, v.xw);
    v = vec4<f16>(1.0h, 1.0h, v.wx);
    v = vec4<f16>(v.zy, 1.0h, 1.0h);
    v = vec4<f16>(v.xx, 1.0h, 1.0h);
    v = v.wzwz;
    v = vec3<f16>(fn_hh4(v), 123.0h, 456.0h).yyzz;
    v = vec3<f16>(fn_hh4(v), 123.0h, 456.0h).yyzz;
    v = vec4<f16>(123.0h, 456.0h, 456.0h, fn_hh4(v));
    v = vec4<f16>(123.0h, 456.0h, 456.0h, fn_hh4(v));
    v = vec3<f16>(fn_hh4(v), 123.0h, 456.0h).yxxz;
    v = vec3<f16>(fn_hh4(v), 123.0h, 456.0h).yxxz;
    v = vec4<f16>(1.0h, 1.0h, 2.0h, 3.0h);
    v = vec4<f16>(_globalUniforms.colorRed.xyz, 1.0h);
    v = vec4<f16>(_globalUniforms.colorRed.x, 1.0h, _globalUniforms.colorRed.yz);
    v = (v).wzyx;
    v = vec4<f16>((v.yz), v.yz).xzwy;
    v = vec4<f16>((vec3<f16>(v.ww, 1.0h)), v.w).zyxw;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(v == vec4<f16>(1.0h))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
