diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
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
    var S: bool = true;
    return S;
  }
}
fn local_struct_variable_hides_struct_type_b() -> bool {
  {
    var S: S = S(1);
    return S.i == 1;
  }
}
fn local_variable_hides_global_variable_b() -> bool {
  {
    var glob: i32 = 1;
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
      let _skTemp5 = block_variable_hides_global_variable_b();
      _skTemp4 = _skTemp5;
    } else {
      _skTemp4 = false;
    }
    if _skTemp4 {
      let _skTemp6 = local_variable_hides_struct_b();
      _skTemp3 = _skTemp6;
    } else {
      _skTemp3 = false;
    }
    if _skTemp3 {
      let _skTemp7 = local_struct_variable_hides_struct_type_b();
      _skTemp2 = _skTemp7;
    } else {
      _skTemp2 = false;
    }
    if _skTemp2 {
      let _skTemp8 = local_variable_hides_global_variable_b();
      _skTemp1 = _skTemp8;
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
