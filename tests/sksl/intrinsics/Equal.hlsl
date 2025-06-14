cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_a : packoffset(c0);
    float4 _11_b : packoffset(c1);
    uint2 _11_c : packoffset(c2);
    uint2 _11_d : packoffset(c2.z);
    int3 _11_e : packoffset(c3);
    int3 _11_f : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    bool4 expectTTFF = bool4(true, true, false, false);
    bool4 expectFFTT = bool4(false, false, true, true);
    bool4 expectTTTT = bool4(true, true, true, true);
    sk_FragColor.x = float(bool4(_11_a.x == _11_b.x, _11_a.y == _11_b.y, _11_a.z == _11_b.z, _11_a.w == _11_b.w).x);
    sk_FragColor.y = float(bool2(_11_c.x == _11_d.x, _11_c.y == _11_d.y).y);
    sk_FragColor.z = float(bool3(_11_e.x == _11_f.x, _11_e.y == _11_f.y, _11_e.z == _11_f.z).z);
    bool _75 = false;
    if (any(expectTTFF))
    {
        _75 = true;
    }
    else
    {
        _75 = any(expectFFTT);
    }
    bool _80 = false;
    if (_75)
    {
        _80 = true;
    }
    else
    {
        _80 = any(expectTTTT);
    }
    sk_FragColor.w = float(_80);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
