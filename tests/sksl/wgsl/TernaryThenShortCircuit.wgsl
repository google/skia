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
fn _outParamHelper_2_Increment_ii(y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*y);
    var _return: i32 = Increment_ii(&_var0);
    (*y) = _var0;
    return _return;
}
fn _outParamHelper_5_Increment_ii(y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*y);
    var _return: i32 = Increment_ii(&_var0);
    (*y) = _var0;
    return _return;
}
fn _outParamHelper_8_Increment_ii(y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*y);
    var _return: i32 = Increment_ii(&_var0);
    (*y) = _var0;
    return _return;
}
fn _outParamHelper_10_Increment_ii(_2_y: ptr<function, i32>) -> i32 {
    var _var0: i32 = (*_2_y);
    var _return: i32 = Increment_ii(&_var0);
    (*_2_y) = _var0;
    return _return;
}
fn Increment_ii(y: ptr<function, i32>) -> i32 {
    (*y) += 1;
    return (*y);
}
fn True_b() -> bool {
    return true;
}
fn False_b() -> bool {
    return false;
}
fn TrueFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp0: bool;
    var _skTemp1: bool;
    if x == 1 {
        _skTemp1 = True_b();
    } else {
        _skTemp1 = False_b();
    }
    if _skTemp1 {
        _skTemp0 = true;
    } else {
        _skTemp0 = _outParamHelper_2_Increment_ii(&y) == 3;
    }
    if (_skTemp0) {
        {
            return x == 1 && y == 1;
        }
    } else {
        {
            return false;
        }
    }
}
fn FalseTrue_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp3: bool;
    var _skTemp4: bool;
    if x == 2 {
        _skTemp4 = True_b();
    } else {
        _skTemp4 = False_b();
    }
    if _skTemp4 {
        _skTemp3 = true;
    } else {
        _skTemp3 = _outParamHelper_5_Increment_ii(&y) == 2;
    }
    if (_skTemp3) {
        {
            return x == 1 && y == 2;
        }
    } else {
        {
            return false;
        }
    }
}
fn FalseFalse_b() -> bool {
    var x: i32 = 1;
    var y: i32 = 1;
    var _skTemp6: bool;
    var _skTemp7: bool;
    if x == 2 {
        _skTemp7 = True_b();
    } else {
        _skTemp7 = False_b();
    }
    if _skTemp7 {
        _skTemp6 = true;
    } else {
        _skTemp6 = _outParamHelper_8_Increment_ii(&y) == 3;
    }
    if (_skTemp6) {
        {
            return false;
        }
    } else {
        {
            return x == 1 && y == 2;
        }
    }
}
fn main(coords: vec2<f32>) -> vec4<f32> {
    var _0_TrueTrue: bool;
    var _2_y: i32 = 1;
    var _skTemp9: bool;
    if True_b() {
        _skTemp9 = true;
    } else {
        _skTemp9 = _outParamHelper_10_Increment_ii(&_2_y) == 2;
    }
    if (_skTemp9) {
        {
            _0_TrueTrue = _2_y == 1;
        }
    } else {
        {
            _0_TrueTrue = false;
        }
    }
    var _skTemp11: vec4<f32>;
    var _skTemp12: bool;
    var _skTemp13: bool;
    var _skTemp14: bool;
    if _0_TrueTrue {
        _skTemp14 = TrueFalse_b();
    } else {
        _skTemp14 = false;
    }
    if _skTemp14 {
        _skTemp13 = FalseTrue_b();
    } else {
        _skTemp13 = false;
    }
    if _skTemp13 {
        _skTemp12 = FalseFalse_b();
    } else {
        _skTemp12 = false;
    }
    if _skTemp12 {
        _skTemp11 = _globalUniforms.colorGreen;
    } else {
        _skTemp11 = _globalUniforms.colorRed;
    }
    return _skTemp11;
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
    var _stageOut: FSOut;
    _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
    return _stageOut;
}
