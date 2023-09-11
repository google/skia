cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 scalar = _7_colorGreen * 0.5f;
    scalar.w = 2.0f;
    scalar.y *= 4.0f;
    float3 _52 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), scalar.yzw);
    scalar = float4(scalar.x, _52.x, _52.y, _52.z);
    float4 _55 = scalar;
    float4 _60 = _55.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _61 = scalar;
    float4 _62 = float4(_60.w, _60.y, _60.x, _60.z);
    scalar = _62;
    float _69 = 0.0f;
    if (_62.w <= 1.0f)
    {
        _69 = _62.z;
    }
    else
    {
        _69 = 0.0f;
    }
    scalar.x += _69;
    float4 array[1] = { 0.0f.xxxx };
    array[0] = _7_colorGreen * 0.5f;
    array[0].w = 2.0f;
    array[0].y *= 4.0f;
    float3 _89 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), array[0].yzw);
    array[0] = float4(array[0].x, _89.x, _89.y, _89.z);
    float4 _95 = array[0].zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    array[0] = float4(_95.w, _95.y, _95.x, _95.z);
    float _105 = 0.0f;
    if (array[0].w <= 1.0f)
    {
        _105 = array[0].z;
    }
    else
    {
        _105 = 0.0f;
    }
    array[0].x += _105;
    bool _126 = false;
    if (all(bool4(scalar.x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, scalar.y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, scalar.z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, scalar.w == float4(1.0f, 1.0f, 0.25f, 1.0f).w)))
    {
        _126 = all(bool4(array[0].x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, array[0].y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, array[0].z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, array[0].w == float4(1.0f, 1.0f, 0.25f, 1.0f).w));
    }
    else
    {
        _126 = false;
    }
    float4 _127 = 0.0f.xxxx;
    if (_126)
    {
        _127 = _7_colorGreen;
    }
    else
    {
        _127 = _7_colorRed;
    }
    return _127;
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
