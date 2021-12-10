cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_a : packoffset(c0);
    float4 _10_b : packoffset(c1);
    uint2 _10_c : packoffset(c2);
    uint2 _10_d : packoffset(c2.z);
    int3 _10_e : packoffset(c3);
    int3 _10_f : packoffset(c4);
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
    sk_FragColor.x = float(int(bool4(_10_a.x > _10_b.x, _10_a.y > _10_b.y, _10_a.z > _10_b.z, _10_a.w > _10_b.w).x));
    sk_FragColor.y = float(int(bool2(_10_c.x > _10_d.x, _10_c.y > _10_d.y).y));
    sk_FragColor.z = float(int(bool3(_10_e.x > _10_f.x, _10_e.y > _10_f.y, _10_e.z > _10_f.z).z));
    bool _73 = false;
    if (any(expectTTFF))
    {
        _73 = true;
    }
    else
    {
        _73 = any(expectFFTT);
    }
    sk_FragColor.w = float(int(_73));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
