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
        let _skTemp1 = &(y);
        (*_skTemp1) += 1;
        _skTemp0 = (*_skTemp1) == 3;
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
    var _skTemp2: bool;
    if x == 2 {
        let _skTemp3 = &(y);
        (*_skTemp3) += 1;
        _skTemp2 = (*_skTemp3) == 2;
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
fn FalseFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp4: bool;
    if x == 2 {
        let _skTemp5 = &(y);
        (*_skTemp5) += 1;
        _skTemp4 = (*_skTemp5) == 3;
    } else {
        _skTemp4 = false;
    }
    if (_skTemp4) {
        {
            return false;
        }
    } else {
        {
            return x == 1 && y == 1;
        }
    }
}
fn main(_skAnonymous6: vec2<f32>) -> vec4<f32> {
    var _0_TrueTrue: bool;
    var _2_y: i32 = 1;
    let _skTemp7 = &(_2_y);
    (*_skTemp7) += 1;
    if ((*_skTemp7) == 2) {
        {
            _0_TrueTrue = _2_y == 2;
        }
    } else {
        {
            _0_TrueTrue = false;
        }
    }
    var _skTemp8: vec4<f32>;
    var _skTemp9: bool;
    var _skTemp10: bool;
    var _skTemp11: bool;
    if _0_TrueTrue {
        _skTemp11 = TrueFalse_b();
    } else {
        _skTemp11 = false;
    }
    if _skTemp11 {
        _skTemp10 = FalseTrue_b();
    } else {
        _skTemp10 = false;
    }
    if _skTemp10 {
        _skTemp9 = FalseFalse_b();
    } else {
        _skTemp9 = false;
    }
    if _skTemp9 {
        _skTemp8 = _globalUniforms.colorGreen;
    } else {
        _skTemp8 = _globalUniforms.colorRed;
    }
    return _skTemp8;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
