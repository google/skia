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
    float4 scalar = _11_colorGreen * 0.5f;
    scalar.w = 2.0f;
    scalar.y *= 4.0f;
    float3 _55 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), scalar.yzw);
    scalar = float4(scalar.x, _55.x, _55.y, _55.z);
    float4 _58 = scalar;
    float4 _63 = _58.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _64 = scalar;
    float4 _65 = float4(_63.w, _63.y, _63.x, _63.z);
    scalar = _65;
    float _72 = 0.0f;
    if (_65.w <= 1.0f)
    {
        _72 = _65.z;
    }
    else
    {
        _72 = 0.0f;
    }
    scalar.x += _72;
    float4 array[1] = { 0.0f.xxxx };
    array[0] = _11_colorGreen * 0.5f;
    array[0].w = 2.0f;
    array[0].y *= 4.0f;
    float3 _92 = mul(float3x3(float3(0.5f, 0.0f, 0.0f), float3(0.0f, 0.5f, 0.0f), float3(0.0f, 0.0f, 0.5f)), array[0].yzw);
    array[0] = float4(array[0].x, _92.x, _92.y, _92.z);
    float4 _98 = array[0].zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    array[0] = float4(_98.w, _98.y, _98.x, _98.z);
    float _108 = 0.0f;
    if (array[0].w <= 1.0f)
    {
        _108 = array[0].z;
    }
    else
    {
        _108 = 0.0f;
    }
    array[0].x += _108;
    bool _129 = false;
    if (all(bool4(scalar.x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, scalar.y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, scalar.z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, scalar.w == float4(1.0f, 1.0f, 0.25f, 1.0f).w)))
    {
        _129 = all(bool4(array[0].x == float4(1.0f, 1.0f, 0.25f, 1.0f).x, array[0].y == float4(1.0f, 1.0f, 0.25f, 1.0f).y, array[0].z == float4(1.0f, 1.0f, 0.25f, 1.0f).z, array[0].w == float4(1.0f, 1.0f, 0.25f, 1.0f).w));
    }
    else
    {
        _129 = false;
    }
    float4 _130 = 0.0f.xxxx;
    if (_129)
    {
        _130 = _11_colorGreen;
    }
    else
    {
        _130 = _11_colorRed;
    }
    return _130;
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
