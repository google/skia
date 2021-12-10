cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 f4 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
    float2x2 _50 = float2x2(float2(f4.xy), float2(f4.xyz.z, 4.0f));
    float2x2 _56 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    float2 _58 = _50[0];
    float2 _59 = _56[0];
    float2 _62 = _50[1];
    float2 _63 = _56[1];
    bool ok = all(bool2(_58.x == _59.x, _58.y == _59.y)) && all(bool2(_62.x == _63.x, _62.y == _63.y));
    bool _110 = false;
    if (ok)
    {
        float3x3 _89 = float3x3(float3(f4.xy, f4.zw.x), float3(f4.zw.y, f4.xy), float3(f4.zw, f4.x));
        float3x3 _94 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 1.0f));
        float3 _96 = _89[0];
        float3 _97 = _94[0];
        float3 _100 = _89[1];
        float3 _101 = _94[1];
        float3 _105 = _89[2];
        float3 _106 = _94[2];
        _110 = (all(bool3(_96.x == _97.x, _96.y == _97.y, _96.z == _97.z)) && all(bool3(_100.x == _101.x, _100.y == _101.y, _100.z == _101.z))) && all(bool3(_105.x == _106.x, _105.y == _106.y, _105.z == _106.z));
    }
    else
    {
        _110 = false;
    }
    ok = _110;
    bool _165 = false;
    if (ok)
    {
        float4x4 _138 = float4x4(float4(f4.xyz, f4.wxy.x), float4(f4.wxy.yz, f4.zw), float4(f4.zwxy.zw, f4.zw), f4);
        float4x4 _144 = float4x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(1.0f, 2.0f, 3.0f, 4.0f), float4(1.0f, 2.0f, 3.0f, 4.0f), float4(1.0f, 2.0f, 3.0f, 4.0f));
        float4 _146 = _138[0];
        float4 _147 = _144[0];
        float4 _150 = _138[1];
        float4 _151 = _144[1];
        float4 _155 = _138[2];
        float4 _156 = _144[2];
        float4 _160 = _138[3];
        float4 _161 = _144[3];
        _165 = ((all(bool4(_146.x == _147.x, _146.y == _147.y, _146.z == _147.z, _146.w == _147.w)) && all(bool4(_150.x == _151.x, _150.y == _151.y, _150.z == _151.z, _150.w == _151.w))) && all(bool4(_155.x == _156.x, _155.y == _156.y, _155.z == _156.z, _155.w == _156.w))) && all(bool4(_160.x == _161.x, _160.y == _161.y, _160.z == _161.z, _160.w == _161.w));
    }
    else
    {
        _165 = false;
    }
    ok = _165;
    float4 _167 = 0.0f.xxxx;
    if (ok)
    {
        _167 = _10_colorGreen;
    }
    else
    {
        _167 = _10_colorRed;
    }
    return _167;
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
