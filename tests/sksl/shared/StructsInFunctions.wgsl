diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
};
@group(0) @binding(0) var<uniform> _globalUniforms : _GlobalUniforms;
struct S {
  x: f32,
  y: i32,
};
struct Nested {
  a: S,
  b: S,
};
struct Compound {
  f4: vec4<f32>,
  i3: vec3<i32>,
};
fn returns_a_struct_S() -> S {
  {
    var s: S;
    s.x = 1.0;
    s.y = 2;
    return s;
  }
}
fn constructs_a_struct_S() -> S {
  {
    return S(2.0, 3);
  }
}
fn accepts_a_struct_fS(s: S) -> f32 {
  {
    return s.x + f32(s.y);
  }
}
fn modifies_a_struct_vS(s: ptr<function, S>) {
  {
    (*s).x = (*s).x + f32(1);
    (*s).y = (*s).y + i32(1);
  }
}
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var s: S = returns_a_struct_S();
    let x: f32 = accepts_a_struct_fS(s);
    var _skTemp0: S = s;
    modifies_a_struct_vS(&_skTemp0);
    s = _skTemp0;
    let expected: S = constructs_a_struct_S();
    var n1: Nested;
    var n2: Nested;
    var n3: Nested;
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    n2 = n1;
    n3 = n2;
    var _skTemp1: S = n3.b;
    modifies_a_struct_vS(&_skTemp1);
    n3.b = _skTemp1;
    const c1: Compound = Compound(vec4<f32>(1.0, 2.0, 3.0, 4.0), vec3<i32>(5, 6, 7));
    let c2: Compound = Compound(vec4<f32>(f32(_globalUniforms.colorGreen.y), 2.0, 3.0, 4.0), vec3<i32>(5, 6, 7));
    let c3: Compound = Compound(vec4<f32>(f32(_globalUniforms.colorGreen.x), 2.0, 3.0, 4.0), vec3<i32>(5, 6, 7));
    var _skTemp2: bool;
    const _skTemp3 = S(2.0, 3);
    if ((((x == 3.0) && (s.x == 2.0)) && (s.y == 3)) && ((s.x == expected.x) && (s.y == expected.y))) && ((s.x == _skTemp3.x) && (s.y == _skTemp3.y)) {
      let _skTemp4 = returns_a_struct_S();
      _skTemp2 = ((s.x != _skTemp4.x) || (s.y != _skTemp4.y));
    } else {
      _skTemp2 = false;
    }
    const _skTemp5 = Nested(S(1.0, 2), S(2.0, 3));
    let valid: bool = ((((_skTemp2 && (((n1.a.x == n2.a.x) && (n1.a.y == n2.a.y)) && ((n1.b.x == n2.b.x) && (n1.b.y == n2.b.y)))) && (((n1.a.x != n3.a.x) || (n1.a.y != n3.a.y)) || ((n1.b.x != n3.b.x) || (n1.b.y != n3.b.y)))) && (((n3.a.x == _skTemp5.a.x) && (n3.a.y == _skTemp5.a.y)) && ((n3.b.x == _skTemp5.b.x) && (n3.b.y == _skTemp5.b.y)))) && (all(c1.f4 == c2.f4) && all(c1.i3 == c2.i3))) && (any(c2.f4 != c3.f4) || any(c2.i3 != c3.i3));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(valid));
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = _skslMain(/*fragcoord*/ vec2<f32>());
  return _stageOut;
}
