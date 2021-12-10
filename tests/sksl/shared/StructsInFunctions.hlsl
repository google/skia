struct S
{
    float x;
    int y;
};

struct Nested
{
    S a;
    S b;
};

struct Compound
{
    float4 f4;
    int3 i3;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _14_colorRed : packoffset(c0);
    float4 _14_colorGreen : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

S returns_a_struct_S()
{
    S s = { 0.0f, 0 };
    s.x = 1.0f;
    s.y = 2;
    return s;
}

float accepts_a_struct_fS(S _47)
{
    return _47.x + float(_47.y);
}

void modifies_a_struct_vS(inout S _56)
{
    _56.x += 1.0f;
    _56.y++;
}

S constructs_a_struct_S()
{
    S _45 = { 2.0f, 3 };
    return _45;
}

float4 main(float2 _65)
{
    S s = returns_a_struct_S();
    S _71 = s;
    float x = accepts_a_struct_fS(_71);
    modifies_a_struct_vS(s);
    S expected = constructs_a_struct_S();
    Nested n1 = { { 0.0f, 0 }, { 0.0f, 0 } };
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    Nested n2 = n1;
    Nested n3 = n2;
    S _90 = n3.b;
    modifies_a_struct_vS(_90);
    n3.b = _90;
    Compound _104 = { float4(1.0f, 2.0f, 3.0f, 4.0f), int3(5, 6, 7) };
    Compound c1 = _104;
    Compound _111 = { float4(_14_colorGreen.y, 2.0f, 3.0f, 4.0f), int3(5, 6, 7) };
    Compound c2 = _111;
    Compound _117 = { float4(_14_colorGreen.x, 2.0f, 3.0f, 4.0f), int3(5, 6, 7) };
    Compound c3 = _117;
    bool _128 = false;
    if (x == 3.0f)
    {
        _128 = s.x == 2.0f;
    }
    else
    {
        _128 = false;
    }
    bool _134 = false;
    if (_128)
    {
        _134 = s.y == 3;
    }
    else
    {
        _134 = false;
    }
    bool _146 = false;
    if (_134)
    {
        _146 = (s.y == expected.y) && (s.x == expected.x);
    }
    else
    {
        _146 = false;
    }
    bool _158 = false;
    if (_146)
    {
        S _150 = { 2.0f, 3 };
        _158 = (s.y == _150.y) && (s.x == _150.x);
    }
    else
    {
        _158 = false;
    }
    bool _170 = false;
    if (_158)
    {
        S _162 = returns_a_struct_S();
        _170 = (s.y != _162.y) || (s.x != _162.x);
    }
    else
    {
        _170 = false;
    }
    bool _194 = false;
    if (_170)
    {
        _194 = ((n1.b.y == n2.b.y) && (n1.b.x == n2.b.x)) && ((n1.a.y == n2.a.y) && (n1.a.x == n2.a.x));
    }
    else
    {
        _194 = false;
    }
    bool _218 = false;
    if (_194)
    {
        _218 = ((n1.b.y != n3.b.y) || (n1.b.x != n3.b.x)) || ((n1.a.y != n3.a.y) || (n1.a.x != n3.a.x));
    }
    else
    {
        _218 = false;
    }
    bool _244 = false;
    if (_218)
    {
        S _222 = { 1.0f, 2 };
        S _223 = { 2.0f, 3 };
        Nested _224 = { _222, _223 };
        _244 = ((n3.b.y == _224.b.y) && (n3.b.x == _224.b.x)) && ((n3.a.y == _224.a.y) && (n3.a.x == _224.a.x));
    }
    else
    {
        _244 = false;
    }
    bool _260 = false;
    if (_244)
    {
        _260 = all(bool3(c1.i3.x == c2.i3.x, c1.i3.y == c2.i3.y, c1.i3.z == c2.i3.z)) && all(bool4(c1.f4.x == c2.f4.x, c1.f4.y == c2.f4.y, c1.f4.z == c2.f4.z, c1.f4.w == c2.f4.w));
    }
    else
    {
        _260 = false;
    }
    bool _274 = false;
    if (_260)
    {
        _274 = any(bool3(c2.i3.x != c3.i3.x, c2.i3.y != c3.i3.y, c2.i3.z != c3.i3.z)) || any(bool4(c2.f4.x != c3.f4.x, c2.f4.y != c3.f4.y, c2.f4.z != c3.f4.z, c2.f4.w != c3.f4.w));
    }
    else
    {
        _274 = false;
    }
    bool valid = _274;
    float4 _276 = 0.0f.xxxx;
    if (valid)
    {
        _276 = _14_colorGreen;
    }
    else
    {
        _276 = _14_colorRed;
    }
    return _276;
}

void frag_main()
{
    float2 _24 = 0.0f.xx;
    sk_FragColor = main(_24);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
