diagnostic(off, derivative_uniformity);
diagnostic(off, chromium.unreachable_code);
enable f16;
struct CSIn {
  @builtin(local_invocation_id) sk_LocalInvocationID: vec3<u32>,
};
struct outputBlock {
  values: array<u32>,
};
@group(0) @binding(0) var<storage, read_write> _storage0 : outputBlock;
fn clampValue_II(x: u32) -> u32 {
  {
    if x > 8u {
      {
        return x - 8u;
      }
    }
    return x;
  }
}
fn _skslMain(_stageIn: CSIn) {
  {
    let id: u32 = _stageIn.sk_LocalInvocationID.x;
    var value: u32 = _storage0.values[id];
    if (id % 2u) == 0u {
      {
        value = value + 1u;
      }
    } else {
      {
        value = value * 3u;
      }
    }
    {
      var i: u32 = 0u;
      for (; i < id; i = i + u32(1)) {
        {
          if value > 100u {
            {
              break;
            }
          }
          if (i & 1u) == 1u {
            {
              continue;
            }
          }
          value = value + i;
        }
      }
    }
    let _skTemp1 = i32(value % 4u);
    switch _skTemp1 {
      case 0, 1 {
        var _skTemp2: bool = false;
        if _skTemp1 == 0 {
          value = value + 4u;
        }
        value = value + 1u;
        break;
      }
      case default {
        value = clampValue_II(value);
        break;
      }
    }
    let small: bool = (value < 16u) && (id != 0u);
    let large: bool = (value > 1000u) || (id == 63u);
    var _skTemp3: u32;
    if small {
      _skTemp3 = value;
    } else {
      var _skTemp4: u32;
      if large {
        _skTemp4 = 0u;
      } else {
        _skTemp4 = value + 1u;
      }
      _skTemp3 = _skTemp4;
    }
    _storage0.values[id] = _skTemp3;
  }
}
@compute @workgroup_size(64, 1, 1) fn main(_stageIn: CSIn) {
  _skslMain(_stageIn);
}
