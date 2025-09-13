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

float4 main(float2 _25)
{
    bool4 _32 = bool4(_11_colorRed.x < 2.0f.xxxx.x, _11_colorRed.y < 2.0f.xxxx.y, _11_colorRed.z < 2.0f.xxxx.z, _11_colorRed.w < 2.0f.xxxx.w);
    bool4 _39 = bool4(3.0f.xxxx.x > _11_colorGreen.x, 3.0f.xxxx.y > _11_colorGreen.y, 3.0f.xxxx.z > _11_colorGreen.z, 3.0f.xxxx.w > _11_colorGreen.w);
    bool4 _31 = bool4(_32.x == _39.x, _32.y == _39.y, _32.z == _39.z, _32.w == _39.w);
    bool4 result = _31;
    float4 _46 = 0.0f.xxxx;
    if (all(_31))
    {
        _46 = _11_colorGreen;
    }
    else
    {
        _46 = _11_colorRed;
    }
    return _46;
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
