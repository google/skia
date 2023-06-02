struct FSIn {
    @builtin(front_facing) sk_Clockwise: bool,
    @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
    @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
    colorGreen: vec4<f32>,
    colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn TrueFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp0: bool;
    if x == 1 {
        y = y + 1;
        _skTemp0 = y == 3;
    } else {
        _skTemp0 = false;
    }
    if (_skTemp0) {
        {
            return false;
        }
    } else {
        {
            return x == 1 && y == 2;
        }
    }
}
fn FalseTrue_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp1: bool;
    if x == 2 {
        y = y + 1;
        _skTemp1 = y == 2;
    } else {
        _skTemp1 = false;
    }
    if (_skTemp1) {
        {
            return false;
        }
    } else {
        {
            return x == 1 && y == 1;
        }
    }
}
fn FalseFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp2: bool;
    if x == 2 {
        y = y + 1;
        _skTemp2 = y == 3;
    } else {
        _skTemp2 = false;
    }
    if (_skTemp2) {
        {
            return false;
        }
    } else {
        {
            return x == 1 && y == 1;
        }
    }
}
fn main(_skAnonymous3: vec2<f32>) -> vec4<f32> {
    var _0_TrueTrue: bool;
    var _2_y: i32 = 1;
    _2_y = _2_y + 1;
    if (_2_y == 2) {
        {
            _0_TrueTrue = _2_y == 2;
        }
    } else {
        {
            _0_TrueTrue = false;
        }
    }
    var _skTemp4: vec4<f32>;
    var _skTemp5: bool;
    var _skTemp6: bool;
    var _skTemp7: bool;
    if _0_TrueTrue {
        let _skTemp8 = TrueFalse_b();
        _skTemp7 = _skTemp8;
    } else {
        _skTemp7 = false;
    }
    if _skTemp7 {
        let _skTemp9 = FalseTrue_b();
        _skTemp6 = _skTemp9;
    } else {
        _skTemp6 = false;
    }
    if _skTemp6 {
        let _skTemp10 = FalseFalse_b();
        _skTemp5 = _skTemp10;
    } else {
        _skTemp5 = false;
    }
    if _skTemp5 {
        _skTemp4 = _globalUniforms.colorGreen;
    } else {
        _skTemp4 = _globalUniforms.colorRed;
    }
    return _skTemp4;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
