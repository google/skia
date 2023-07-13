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
    float4 scalar = _10_colorGreen * 0.5f;
    scalar.w = 2.0f;
    scalar.y *= 4.0f;
    float3 _55 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), scalar.yzw);
    scalar = float4(scalar.x, _55.x, _55.y, _55.z);
    float4 _58 = scalar;
    float4 _63 = _58.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _64 = scalar;
    float4 _65 = float4(_63.w, _63.y, _63.x, _63.z);
    scalar = _65;
    float _71 = 0.0f;
    if (_65.w <= 1.0f)
    {
        _71 = _65.z;
    }
    else
    {
        _71 = 0.0f;
    }
    scalar.x += _71;
    float4 array[1] = { 0.0f.xxxx };
    array[0] = _10_colorGreen * 0.5f;
    array[0].w = 2.0f;
    array[0].y *= 4.0f;
    float3 _91 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), array[0].yzw);
    array[0] = float4(array[0].x, _91.x, _91.y, _91.z);
    float4 _97 = array[0].zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    array[0] = float4(_97.w, _97.y, _97.x, _97.z);
    float _107 = 0.0f;
    if (array[0].w <= 1.0f)
    {
        _107 = array[0].z;
    }
    else
    {
        _107 = 0.0f;
    }
    array[0].x += _107;
    bool _128 = false;
    if (all(bool4(scalar.x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, scalar.y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, scalar.z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, scalar.w == float4(1.0f, 1.0f, 0.25f, 1.0f).w)))
    {
        _128 = all(bool4(array[0].x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, array[0].y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, array[0].z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, array[0].w == float4(1.0f, 1.0f, 0.25f, 1.0f).w));
    }
    else
    {
        _128 = false;
    }
    float4 _129 = 0.0f.xxxx;
    if (_128)
    {
        _129 = _10_colorGreen;
    }
    else
    {
        _129 = _10_colorRed;
    }
    return _129;
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
