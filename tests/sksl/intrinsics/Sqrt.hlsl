cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _7_testMatrix2x2 : packoffset(c0);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(out float2 _22)
{
    _22 = sqrt(float4(-1.0f, -4.0f, -16.0f, -64.0f)).xy;
    float4 _47 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y) + float4(0.0f, 2.0f, 6.0f, 12.0f);
    float4 inputVal = _47;
    bool _68 = false;
    if (abs(sqrt(_47.x) - 1.0f) < 0.0500000007450580596923828125f)
    {
        float2 _61 = abs(sqrt(_47.xy) - float2(1.0f, 2.0f));
        _68 = all(bool2(_61.x < 0.0500000007450580596923828125f.xx.x, _61.y < 0.0500000007450580596923828125f.xx.y));
    }
    else
    {
        _68 = false;
    }
    bool _82 = false;
    if (_68)
    {
        float3 _73 = abs(sqrt(_47.xyz) - float3(1.0f, 2.0f, 3.0f));
        _82 = all(bool3(_73.x < 0.0500000007450580596923828125f.xxx.x, _73.y < 0.0500000007450580596923828125f.xxx.y, _73.z < 0.0500000007450580596923828125f.xxx.z));
    }
    else
    {
        _82 = false;
    }
    bool _94 = false;
    if (_82)
    {
        float4 _87 = abs(sqrt(_47) - float4(1.0f, 2.0f, 3.0f, 4.0f));
        _94 = all(bool4(_87.x < 0.0500000007450580596923828125f.xxxx.x, _87.y < 0.0500000007450580596923828125f.xxxx.y, _87.z < 0.0500000007450580596923828125f.xxxx.z, _87.w < 0.0500000007450580596923828125f.xxxx.w));
    }
    else
    {
        _94 = false;
    }
    float4 _95 = 0.0f.xxxx;
    if (_94)
    {
        _95 = _7_colorGreen;
    }
    else
    {
        _95 = _7_colorRed;
    }
    return _95;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    float4 _20 = main(_18);
    sk_FragColor = _20;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
