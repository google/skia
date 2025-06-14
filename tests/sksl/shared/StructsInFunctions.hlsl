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
    float4 _15_colorRed : packoffset(c0);
    float4 _15_colorGreen : packoffset(c1);
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
    float4 _110 = float4(_15_colorGreen.y, 2.0f, 3.0f, 4.0f);
    Compound _111 = { _110, int3(5, 6, 7) };
    Compound c2 = _111;
    float4 _116 = float4(_15_colorGreen.x, 2.0f, 3.0f, 4.0f);
    Compound _117 = { _116, int3(5, 6, 7) };
    Compound c3 = _117;
    bool _129 = false;
    if (x == 3.0f)
    {
        _129 = s.x == 2.0f;
    }
    else
    {
        _129 = false;
    }
    bool _135 = false;
    if (_129)
    {
        _135 = s.y == 3;
    }
    else
    {
        _135 = false;
    }
    bool _147 = false;
    if (_135)
    {
        _147 = (s.y == expected.y) && (s.x == expected.x);
    }
    else
    {
        _147 = false;
    }
    bool _157 = false;
    if (_147)
    {
        S _151 = { 2.0f, 3 };
        _157 = (s.y == 3) && (s.x == 2.0f);
    }
    else
    {
        _157 = false;
    }
    bool _169 = false;
    if (_157)
    {
        S _161 = returns_a_struct_S();
        _169 = (s.y != _161.y) || (s.x != _161.x);
    }
    else
    {
        _169 = false;
    }
    bool _193 = false;
    if (_169)
    {
        _193 = ((n1.b.y == n2.b.y) && (n1.b.x == n2.b.x)) && ((n1.a.y == n2.a.y) && (n1.a.x == n2.a.x));
    }
    else
    {
        _193 = false;
    }
    bool _217 = false;
    if (_193)
    {
        _217 = ((n1.b.y != n3.b.y) || (n1.b.x != n3.b.x)) || ((n1.a.y != n3.a.y) || (n1.a.x != n3.a.x));
    }
    else
    {
        _217 = false;
    }
    bool _237 = false;
    if (_217)
    {
        S _221 = { 1.0f, 2 };
        S _222 = { 2.0f, 3 };
        Nested _223 = { _221, _222 };
        _237 = ((n3.b.y == 3) && (n3.b.x == 2.0f)) && ((n3.a.y == 2) && (n3.a.x == 1.0f));
    }
    else
    {
        _237 = false;
    }
    bool _245 = false;
    if (_237)
    {
        _245 = true && all(bool4(float4(1.0f, 2.0f, 3.0f, 4.0f).x == _110.x, float4(1.0f, 2.0f, 3.0f, 4.0f).y == _110.y, float4(1.0f, 2.0f, 3.0f, 4.0f).z == _110.z, float4(1.0f, 2.0f, 3.0f, 4.0f).w == _110.w));
    }
    else
    {
        _245 = false;
    }
    bool _251 = false;
    if (_245)
    {
        _251 = false || any(bool4(_110.x != _116.x, _110.y != _116.y, _110.z != _116.z, _110.w != _116.w));
    }
    else
    {
        _251 = false;
    }
    bool valid = _251;
    float4 _252 = 0.0f.xxxx;
    if (_251)
    {
        _252 = _15_colorGreen;
    }
    else
    {
        _252 = _15_colorRed;
    }
    return _252;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    sk_FragColor = main(_25);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
