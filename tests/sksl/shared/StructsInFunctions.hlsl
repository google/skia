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
    S _68 = returns_a_struct_S();
    S s = _68;
    S _70 = _68;
    float x = accepts_a_struct_fS(_70);
    S _72 = _68;
    modifies_a_struct_vS(_72);
    s = _72;
    S expected = constructs_a_struct_S();
    Nested n1 = { { 0.0f, 0 }, { 0.0f, 0 } };
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    Nested n2 = n1;
    Nested n3 = n1;
    S _90 = n3.b;
    modifies_a_struct_vS(_90);
    n3.b = _90;
    Compound _104 = { float4(1.0f, 2.0f, 3.0f, 4.0f), int3(5, 6, 7) };
    Compound c1 = _104;
    float4 _110 = float4(_14_colorGreen.y, 2.0f, 3.0f, 4.0f);
    Compound _111 = { _110, int3(5, 6, 7) };
    Compound c2 = _111;
    float4 _116 = float4(_14_colorGreen.x, 2.0f, 3.0f, 4.0f);
    Compound _117 = { _116, int3(5, 6, 7) };
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
    bool _156 = false;
    if (_146)
    {
        S _150 = { 2.0f, 3 };
        _156 = (s.y == 3) && (s.x == 2.0f);
    }
    else
    {
        _156 = false;
    }
    bool _168 = false;
    if (_156)
    {
        S _160 = returns_a_struct_S();
        _168 = (s.y != _160.y) || (s.x != _160.x);
    }
    else
    {
        _168 = false;
    }
    bool _192 = false;
    if (_168)
    {
        _192 = ((n1.b.y == n2.b.y) && (n1.b.x == n2.b.x)) && ((n1.a.y == n2.a.y) && (n1.a.x == n2.a.x));
    }
    else
    {
        _192 = false;
    }
    bool _216 = false;
    if (_192)
    {
        _216 = ((n1.b.y != n3.b.y) || (n1.b.x != n3.b.x)) || ((n1.a.y != n3.a.y) || (n1.a.x != n3.a.x));
    }
    else
    {
        _216 = false;
    }
    bool _236 = false;
    if (_216)
    {
        S _220 = { 1.0f, 2 };
        S _221 = { 2.0f, 3 };
        Nested _222 = { _220, _221 };
        _236 = ((n3.b.y == 3) && (n3.b.x == 2.0f)) && ((n3.a.y == 2) && (n3.a.x == 1.0f));
    }
    else
    {
        _236 = false;
    }
    bool _244 = false;
    if (_236)
    {
        _244 = true && all(bool4(float4(1.0f, 2.0f, 3.0f, 4.0f).x == _110.x, float4(1.0f, 2.0f, 3.0f, 4.0f).y == _110.y, float4(1.0f, 2.0f, 3.0f, 4.0f).z == _110.z, float4(1.0f, 2.0f, 3.0f, 4.0f).w == _110.w));
    }
    else
    {
        _244 = false;
    }
    bool _250 = false;
    if (_244)
    {
        _250 = false || any(bool4(_110.x != _116.x, _110.y != _116.y, _110.z != _116.z, _110.w != _116.w));
    }
    else
    {
        _250 = false;
    }
    bool valid = _250;
    float4 _251 = 0.0f.xxxx;
    if (_250)
    {
        _251 = _14_colorGreen;
    }
    else
    {
        _251 = _14_colorRed;
    }
    return _251;
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
