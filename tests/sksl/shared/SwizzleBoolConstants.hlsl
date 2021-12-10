cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool4 _36 = (_10_colorGreen.y != 0.0f).xxxx;
    bool4 v = _36;
    bool _38 = _36.x;
    bool4 result = bool4(_38, true, true, true);
    result = bool4(_36.xy, false, true);
    result = bool4(_38, true, true, false);
    bool _48 = _36.y;
    result = bool4(false, _48, true, true);
    result = bool4(_36.xyz, true);
    result = bool4(_36.xy, true, true);
    bool _60 = _36.z;
    bool4 _61 = bool4(_38, false, _60, true);
    result = _61;
    result = bool4(_38, true, false, false);
    result = bool4(true, _36.yz, false);
    result = bool4(false, _48, true, false);
    result = bool4(true, true, _60, false);
    result = _36;
    result = bool4(_36.xyz, true);
    bool _77 = _36.w;
    result = bool4(_36.xy, false, _77);
    result = bool4(_36.xy, true, false);
    result = bool4(_38, true, _36.zw);
    result = _61;
    result = bool4(_38, true, true, _77);
    result = bool4(_38, true, false, true);
    result = bool4(true, _36.yzw);
    result = bool4(false, _36.yz, true);
    result = bool4(false, _48, true, _77);
    result = bool4(true, _48, true, true);
    result = bool4(false, false, _36.zw);
    result = bool4(false, false, _60, true);
    bool4 _105 = bool4(false, true, true, _77);
    result = _105;
    float4 _107 = 0.0f.xxxx;
    if (any(_105))
    {
        _107 = _10_colorGreen;
    }
    else
    {
        _107 = _10_colorRed;
    }
    return _107;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
