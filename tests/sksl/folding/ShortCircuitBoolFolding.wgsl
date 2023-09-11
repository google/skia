diagnostic(off, derivative_uniformity);
struct _GlobalUniforms {
  colorRed: vec4<f32>,
  colorGreen: vec4<f32>,
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(coords: vec2<f32>) -> vec4<f32> {
  {
    var _0_expr: bool = _globalUniforms.unknownInput > 0.0;
    var _1_ok: i32 = 0;
    var _2_bad: i32 = 0;
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    {
      _1_ok = _1_ok + i32(1);
    }
    if true != _0_expr {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    {
      _1_ok = _1_ok + i32(1);
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    if false == _0_expr {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    if true != _0_expr {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    {
      _1_ok = _1_ok + i32(1);
    }
    if _0_expr != true {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    {
      _1_ok = _1_ok + i32(1);
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    if _0_expr == false {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    if _0_expr != true {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    if _0_expr {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    var _3_a: f32 = f32(_globalUniforms.unknownInput + 2.0);
    var _4_b: f32 = f32(_globalUniforms.unknownInput * 2.0);
    if _3_a == _4_b {
      {
        _2_bad = _2_bad + i32(1);
      }
    } else {
      {
        _1_ok = _1_ok + i32(1);
      }
    }
    _3_a = _4_b;
    if _3_a == _4_b {
      {
        _1_ok = _1_ok + i32(1);
      }
    } else {
      {
        _2_bad = _2_bad + i32(1);
      }
    }
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>((_1_ok == 22) && (_2_bad == 0)));
  }
}
@fragment fn main(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
  return _skslMain(_coords);
}
