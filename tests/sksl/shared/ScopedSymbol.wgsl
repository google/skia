diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
var<private> glob: i32;
fn block_variable_hides_global_variable_b() -> bool {
  {
    return glob == 2;
  }
}
struct S {
  i: i32,
};
fn local_variable_hides_struct_b() -> bool {
  {
    const S: bool = true;
    return S;
  }
}
fn local_struct_variable_hides_struct_type_b() -> bool {
  {
    const S: S = S(1);
    return S.i == 1;
  }
}
fn local_variable_hides_global_variable_b() -> bool {
  {
    const glob: i32 = 1;
    return glob == 1;
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    glob = 2;
    const _0_var: bool = true;
    var _skTemp0: vec4<f32>;
    var _skTemp1: bool;
    var _skTemp2: bool;
    var _skTemp3: bool;
    var _skTemp4: bool;
    if _0_var {
      _skTemp4 = block_variable_hides_global_variable_b();
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      _skTemp3 = local_variable_hides_struct_b();
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      _skTemp2 = local_struct_variable_hides_struct_type_b();
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      _skTemp1 = local_variable_hides_global_variable_b();
    } else {
      _skTemp1 = false;
    }
    if _skTemp1 {
      _skTemp0 = _globalUniforms.colorGreen;
    } else {
      _skTemp0 = _globalUniforms.colorRed;
    }
    return _skTemp0;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
