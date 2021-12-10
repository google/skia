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
    bool4 v = (_10_colorGreen.y != 0.0f).xxxx;
    bool4 result = bool4(v.x, true, true, true);
    result = bool4(v.xy, false, true);
    result = bool4(v.x, true, true, false);
    result = bool4(false, v.y, true, true);
    result = bool4(v.xyz, true);
    result = bool4(v.xy, true, true);
    result = bool4(v.x, false, v.z, true);
    result = bool4(v.x, true, false, false);
    result = bool4(true, v.yz, false);
    result = bool4(false, v.y, true, false);
    result = bool4(true, true, v.z, false);
    result = v;
    result = bool4(v.xyz, true);
    result = bool4(v.xy, false, v.w);
    result = bool4(v.xy, true, false);
    result = bool4(v.x, true, v.zw);
    result = bool4(v.x, false, v.z, true);
    result = bool4(v.x, true, true, v.w);
    result = bool4(v.x, true, false, true);
    result = bool4(true, v.yzw);
    result = bool4(false, v.yz, true);
    result = bool4(false, v.y, true, v.w);
    result = bool4(true, v.y, true, true);
    result = bool4(false, false, v.zw);
    result = bool4(false, false, v.z, true);
    result = bool4(false, true, true, v.w);
    float4 _157 = 0.0f.xxxx;
    if (any(result))
    {
        _157 = _10_colorGreen;
    }
    else
    {
        _157 = _10_colorRed;
    }
    return _157;
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
