diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  testInputs: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
fn fn_hh4(v: vec4<f32>) -> f32 {
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
  return f32();
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var v: vec4<f32> = _globalUniforms.testInputs;
    v = vec4<f32>(0.0, v.zyx);
    v = vec4<f32>(0.0, 0.0, v.xw);
    v = vec4<f32>(1.0, 1.0, v.wx);
    v = vec4<f32>(v.zy, 1.0, 1.0);
    v = vec4<f32>(v.xx, 1.0, 1.0);
    v = v.wzwz;
    v = vec3<f32>(fn_hh4(v), 123.0, 456.0).yyzz;
    v = vec3<f32>(fn_hh4(v), 123.0, 456.0).yyzz;
    v = vec4<f32>(123.0, 456.0, 456.0, fn_hh4(v));
    v = vec4<f32>(123.0, 456.0, 456.0, fn_hh4(v));
    v = vec3<f32>(fn_hh4(v), 123.0, 456.0).yxxz;
    v = vec3<f32>(fn_hh4(v), 123.0, 456.0).yxxz;
    v = vec4<f32>(1.0, 1.0, 2.0, 3.0);
    v = vec4<f32>(_globalUniforms.colorRed.xyz, 1.0);
    v = vec4<f32>(_globalUniforms.colorRed.x, 1.0, _globalUniforms.colorRed.yz);
    v = (v).wzyx;
    v = vec4<f32>((v.yz), v.yz).xzwy;
    v = vec4<f32>((vec3<f32>(v.ww, 1.0)), v.w).zyxw;
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(v == vec4<f32>(1.0))));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
