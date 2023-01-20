cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _10_testMatrix2x2 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(out float2 _25)
{
    _25 = sqrt(float4(-1.0f, -4.0f, -16.0f, -64.0f)).xy;
    float4 _50 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) + float4(0.0f, 2.0f, 6.0f, 12.0f);
    float4 inputVal = _50;
    bool _64 = false;
    if (sqrt(_50.x) == 1.0f)
    {
        float2 _58 = sqrt(_50.xy);
        _64 = all(bool2(_58.x == float2(1.0f, 2.0f).x, _58.y == float2(1.0f, 2.0f).y));
    }
    else
    {
        _64 = false;
    }
    bool _75 = false;
    if (_64)
    {
        float3 _67 = sqrt(_50.xyz);
        _75 = all(bool3(_67.x == float3(1.0f, 2.0f, 3.0f).x, _67.y == float3(1.0f, 2.0f, 3.0f).y, _67.z == float3(1.0f, 2.0f, 3.0f).z));
    }
    else
    {
        _75 = false;
    }
    bool _84 = false;
    if (_75)
    {
        float4 _78 = sqrt(_50);
        _84 = all(bool4(_78.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _78.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _78.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _78.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _84 = false;
    }
    float4 _85 = 0.0f.xxxx;
    if (_84)
    {
        _85 = _10_colorGreen;
    }
    else
    {
        _85 = _10_colorRed;
    }
    return _85;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    float4 _23 = main(_21);
    sk_FragColor = _23;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
