cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_a : packoffset(c0);
    float4 _7_b : packoffset(c1);
    uint2 _7_c : packoffset(c2);
    uint2 _7_d : packoffset(c2.z);
    int3 _7_e : packoffset(c3);
    int3 _7_f : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    bool4 expectFFTT = bool4(false, false, true, true);
    bool4 expectTTFF = bool4(true, true, false, false);
    sk_FragColor.x = float(bool4(_7_a.x > _7_b.x, _7_a.y > _7_b.y, _7_a.z > _7_b.z, _7_a.w > _7_b.w).x);
    sk_FragColor.y = float(bool2(_7_c.x > _7_d.x, _7_c.y > _7_d.y).y);
    sk_FragColor.z = float(bool3(_7_e.x > _7_f.x, _7_e.y > _7_f.y, _7_e.z > _7_f.z).z);
    bool _70 = false;
    if (any(expectTTFF))
    {
        _70 = true;
    }
    else
    {
        _70 = any(expectFFTT);
    }
    sk_FragColor.w = float(_70);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
