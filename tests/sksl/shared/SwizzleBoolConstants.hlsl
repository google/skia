cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool4 _37 = (_11_colorGreen.y != 0.0f).xxxx;
    bool4 v = _37;
    bool _39 = _37.x;
    bool4 result = bool4(_39, true, true, true);
    result = bool4(_37.xy, false, true);
    result = bool4(_39, true, true, false);
    bool _49 = _37.y;
    result = bool4(false, _49, true, true);
    result = bool4(_37.xyz, true);
    result = bool4(_37.xy, true, true);
    bool _61 = _37.z;
    bool4 _62 = bool4(_39, false, _61, true);
    result = _62;
    result = bool4(_39, true, false, false);
    result = bool4(true, _37.yz, false);
    result = bool4(false, _49, true, false);
    result = bool4(true, true, _61, false);
    result = _37;
    result = bool4(_37.xyz, true);
    bool _78 = _37.w;
    result = bool4(_37.xy, false, _78);
    result = bool4(_37.xy, true, false);
    result = bool4(_39, true, _37.zw);
    result = _62;
    result = bool4(_39, true, true, _78);
    result = bool4(_39, true, false, true);
    result = bool4(true, _37.yzw);
    result = bool4(false, _37.yz, true);
    result = bool4(false, _49, true, _78);
    result = bool4(true, _49, true, true);
    result = bool4(false, false, _37.zw);
    result = bool4(false, false, _61, true);
    bool4 _106 = bool4(false, true, true, _78);
    result = _106;
    float4 _108 = 0.0f.xxxx;
    if (any(_106))
    {
        _108 = _11_colorGreen;
    }
    else
    {
        _108 = _11_colorRed;
    }
    return _108;
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
