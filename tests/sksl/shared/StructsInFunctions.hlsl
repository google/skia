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
    float4 _11_colorRed : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
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

float accepts_a_struct_fS(S _44)
{
    return _44.x + float(_44.y);
}

void modifies_a_struct_vS(inout S _53)
{
    _53.x += 1.0f;
    _53.y++;
}

S constructs_a_struct_S()
{
    S _42 = { 2.0f, 3 };
    return _42;
}

float4 main(float2 _62)
{
    S _65 = returns_a_struct_S();
    S s = _65;
    S _67 = _65;
    float x = accepts_a_struct_fS(_67);
    S _69 = _65;
    modifies_a_struct_vS(_69);
    s = _69;
    S expected = constructs_a_struct_S();
    Nested n1 = { { 0.0f, 0 }, { 0.0f, 0 } };
    n1.a = returns_a_struct_S();
    n1.b = n1.a;
    Nested n2 = n1;
    Nested n3 = n1;
    S _87 = n3.b;
    modifies_a_struct_vS(_87);
    n3.b = _87;
    Compound _101 = { float4(1.0f, 2.0f, 3.0f, 4.0f), int3(5, 6, 7) };
    Compound c1 = _101;
    float4 _107 = float4(_11_colorGreen.y, 2.0f, 3.0f, 4.0f);
    Compound _108 = { _107, int3(5, 6, 7) };
    Compound c2 = _108;
    float4 _113 = float4(_11_colorGreen.x, 2.0f, 3.0f, 4.0f);
    Compound _114 = { _113, int3(5, 6, 7) };
    Compound c3 = _114;
    bool _126 = false;
    if (x == 3.0f)
    {
        _126 = s.x == 2.0f;
    }
    else
    {
        _126 = false;
    }
    bool _132 = false;
    if (_126)
    {
        _132 = s.y == 3;
    }
    else
    {
        _132 = false;
    }
    bool _144 = false;
    if (_132)
    {
        _144 = (s.y == expected.y) && (s.x == expected.x);
    }
    else
    {
        _144 = false;
    }
    bool _154 = false;
    if (_144)
    {
        S _148 = { 2.0f, 3 };
        _154 = (s.y == 3) && (s.x == 2.0f);
    }
    else
    {
        _154 = false;
    }
    bool _166 = false;
    if (_154)
    {
        S _158 = returns_a_struct_S();
        _166 = (s.y != _158.y) || (s.x != _158.x);
    }
    else
    {
        _166 = false;
    }
    bool _190 = false;
    if (_166)
    {
        _190 = ((n1.b.y == n2.b.y) && (n1.b.x == n2.b.x)) && ((n1.a.y == n2.a.y) && (n1.a.x == n2.a.x));
    }
    else
    {
        _190 = false;
    }
    bool _214 = false;
    if (_190)
    {
        _214 = ((n1.b.y != n3.b.y) || (n1.b.x != n3.b.x)) || ((n1.a.y != n3.a.y) || (n1.a.x != n3.a.x));
    }
    else
    {
        _214 = false;
    }
    bool _234 = false;
    if (_214)
    {
        S _218 = { 1.0f, 2 };
        S _219 = { 2.0f, 3 };
        Nested _220 = { _218, _219 };
        _234 = ((n3.b.y == 3) && (n3.b.x == 2.0f)) && ((n3.a.y == 2) && (n3.a.x == 1.0f));
    }
    else
    {
        _234 = false;
    }
    bool _242 = false;
    if (_234)
    {
        _242 = true && all(bool4(float4(1.0f, 2.0f, 3.0f, 4.0f).x == _107.x, float4(1.0f, 2.0f, 3.0f, 4.0f).y == _107.y, float4(1.0f, 2.0f, 3.0f, 4.0f).z == _107.z, float4(1.0f, 2.0f, 3.0f, 4.0f).w == _107.w));
    }
    else
    {
        _242 = false;
    }
    bool _248 = false;
    if (_242)
    {
        _248 = false || any(bool4(_107.x != _113.x, _107.y != _113.y, _107.z != _113.z, _107.w != _113.w));
    }
    else
    {
        _248 = false;
    }
    bool valid = _248;
    float4 _249 = 0.0f.xxxx;
    if (_248)
    {
        _249 = _11_colorGreen;
    }
    else
    {
        _249 = _11_colorRed;
    }
    return _249;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
