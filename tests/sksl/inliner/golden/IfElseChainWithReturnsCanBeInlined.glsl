
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
void main() {
    mediump vec4 _0_branchy;
    mediump vec4 _1_c = color;
    {
        _1_c *= 0.5;
        if (_1_c.x > 0.0) _0_branchy = _1_c.xxxx; else if (_1_c.y > 0.0) _0_branchy = _1_c.yyyy; else if (_1_c.z > 0.0) _0_branchy = _1_c.zzzz; else _0_branchy = _1_c.wwww;
    }

    mediump vec4 _2_branchyAndBlocky;
    {
        {
            {
                if (color.x > 0.0) {
                    mediump vec4 _3_d = color * 0.5;
                    _2_branchyAndBlocky = _3_d.xxxx;
                } else {
                    {
                        {
                            if (color.x < 0.0) {
                                _2_branchyAndBlocky = color.wwww;
                            } else {
                                _2_branchyAndBlocky = color.yyyy;
                            }
                        }
                    }
                }
            }
        }
    }

    sk_FragColor = _0_branchy * _2_branchyAndBlocky;

}
