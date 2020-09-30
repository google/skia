
out vec4 sk_FragColor;
in float val;
uniform int ui;
uniform vec4 uh4;
void main() {
    struct S {
        vec4[1] ah4;
        float[1] ah;
        vec4 h4;
        float h;
    } s;
    s.ah4[0] = vec4(val);
    s.ah[0] = val;
    s.h4 = vec4(val);
    s.h = val;
    S as[1];
    as[0].ah4[0] = vec4(val);
    {
        sk_FragColor = sk_FragColor.xxxx;
    }


    {
        sk_FragColor = vec4(s.h);
    }


    {
        sk_FragColor = s.ah4[0].ywyw;
    }


    {
        sk_FragColor = as[0].ah4[0].xyxy;
    }


    {
        sk_FragColor = s.h4.zzzz;
    }


    {
        sk_FragColor = uh4.xyzx;
    }


    {
        sk_FragColor = vec3(s.h).xyzx;
    }


    {
        sk_FragColor = vec4(s.h);
    }


    {
        sk_FragColor = s.ah4[0].xxxy;
    }


    {
        sk_FragColor = uh4;
    }


    {
        sk_FragColor = !true ? sk_FragColor.xxxx : sk_FragColor.yyyy;
    }


    {
        sk_FragColor = vec4(-s.h);
    }


    {
        sk_FragColor = s.ah4[ui].ywyw;
    }


    {
        sk_FragColor = (s.h4.yyy + s.h4.zzz).xyzx;
    }


    {
        sk_FragColor = vec3(s.h4.y, 0.0, 1.0).xyyz;
    }


}
