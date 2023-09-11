cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorRed : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool4 _28 = bool4(_7_colorRed.x < 2.0f.xxxx.x, _7_colorRed.y < 2.0f.xxxx.y, _7_colorRed.z < 2.0f.xxxx.z, _7_colorRed.w < 2.0f.xxxx.w);
    bool4 _36 = bool4(3.0f.xxxx.x > _7_colorGreen.x, 3.0f.xxxx.y > _7_colorGreen.y, 3.0f.xxxx.z > _7_colorGreen.z, 3.0f.xxxx.w > _7_colorGreen.w);
    bool4 _27 = bool4(_28.x == _36.x, _28.y == _36.y, _28.z == _36.z, _28.w == _36.w);
    bool4 result = _27;
    float4 _43 = 0.0f.xxxx;
    if (all(_27))
    {
        _43 = _7_colorGreen;
    }
    else
    {
        _43 = _7_colorRed;
    }
    return _43;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
