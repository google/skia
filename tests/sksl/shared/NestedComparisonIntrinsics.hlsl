cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorRed : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    bool4 _30 = bool4(_10_colorRed.x < 2.0f.xxxx.x, _10_colorRed.y < 2.0f.xxxx.y, _10_colorRed.z < 2.0f.xxxx.z, _10_colorRed.w < 2.0f.xxxx.w);
    bool4 _38 = bool4(3.0f.xxxx.x > _10_colorGreen.x, 3.0f.xxxx.y > _10_colorGreen.y, 3.0f.xxxx.z > _10_colorGreen.z, 3.0f.xxxx.w > _10_colorGreen.w);
    bool4 _29 = bool4(_30.x == _38.x, _30.y == _38.y, _30.z == _38.z, _30.w == _38.w);
    bool4 result = _29;
    float4 _45 = 0.0f.xxxx;
    if (all(_29))
    {
        _45 = _10_colorGreen;
    }
    else
    {
        _45 = _10_colorRed;
    }
    return _45;
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
