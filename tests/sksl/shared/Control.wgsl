diagnostic(off, derivative_uniformity);
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  unknownInput: f32,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn _skslMain(_stageOut: ptr<function, FSOut>) {
  {
    if _globalUniforms.unknownInput > 5.0 {
      {
        (*_stageOut).sk_FragColor = vec4<f32>(0.75);
      }
    } else {
      {
        discard;
      }
    }
    var i: i32 = 0;
    loop {
      if i < 10 {
        {
          (*_stageOut).sk_FragColor = (*_stageOut).sk_FragColor * 0.5;
          i = i + i32(1);
        }
      } else {
        break;
      }
    }
    loop {
      {
        (*_stageOut).sk_FragColor = (*_stageOut).sk_FragColor + 0.25;
      }
      continuing {
        break if (*_stageOut).sk_FragColor.x >= 0.75;
      }
    }
    {
      var i: i32 = 0;
      loop {
        {
          if (i % 2) == 1 {
            break;
          } else {
            if i > 100 {
              return ;
            } else {
              continue;
            }
          }
        }
        continuing {
          i = i + i32(1);
          break if i >= 10;
        }
      }
    }
    return ;
  }
}
@fragment fn main() -> FSOut {
  var _stageOut: FSOut;
  _skslMain(&_stageOut);
  return _stageOut;
}
