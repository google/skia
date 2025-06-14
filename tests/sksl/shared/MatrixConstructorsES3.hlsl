cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    row_major float2x2 _11_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 _38 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y);
    float4 f4 = _38;
    float3 _44 = float3(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x);
    float3 _47 = float3(_11_testMatrix2x2[1].y, _38.xy);
    bool _62 = all(bool3(_44.x == float3(1.0f, 2.0f, 3.0f).x, _44.y == float3(1.0f, 2.0f, 3.0f).y, _44.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_47.x == float3(4.0f, 1.0f, 2.0f).x, _47.y == float3(4.0f, 1.0f, 2.0f).y, _47.z == float3(4.0f, 1.0f, 2.0f).z));
    bool ok = _62;
    bool _87 = false;
    if (_62)
    {
        float4 _67 = _38.wxyz;
        float4 _72 = float4(_38.xyz, _67.x);
        float4 _76 = float4(_67.yzw, _11_testMatrix2x2[1].y);
        _87 = all(bool4(_72.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _72.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _72.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _72.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_76.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _76.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _76.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _76.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _87 = false;
    }
    ok = _87;
    bool _111 = false;
    if (_87)
    {
        float2 _91 = _38.zw;
        float3 _95 = float3(_38.xy, _91.x);
        float3 _97 = float3(_91.y, _11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y);
        float3 _98 = float3(_11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y, _11_testMatrix2x2[0].x);
        _111 = (all(bool3(_95.x == float3(1.0f, 2.0f, 3.0f).x, _95.y == float3(1.0f, 2.0f, 3.0f).y, _95.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_97.x == float3(4.0f, 1.0f, 2.0f).x, _97.y == float3(4.0f, 1.0f, 2.0f).y, _97.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_98.x == float3(3.0f, 4.0f, 1.0f).x, _98.y == float3(3.0f, 4.0f, 1.0f).y, _98.z == float3(3.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _111 = false;
    }
    ok = _111;
    bool _144 = false;
    if (_111)
    {
        float3 _114 = _38.xyz;
        float4 _115 = _38.wxyz;
        float2 _118 = float2(_114.xy);
        float2 _121 = float2(_114.z, _115.x);
        float2 _124 = float2(_115.yz);
        float2 _126 = float2(_115.w, _11_testMatrix2x2[1].y);
        _144 = ((all(bool2(_118.x == float2(1.0f, 2.0f).x, _118.y == float2(1.0f, 2.0f).y)) && all(bool2(_121.x == float2(3.0f, 4.0f).x, _121.y == float2(3.0f, 4.0f).y))) && all(bool2(_124.x == float2(1.0f, 2.0f).x, _124.y == float2(1.0f, 2.0f).y))) && all(bool2(_126.x == float2(3.0f, 4.0f).x, _126.y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _144 = false;
    }
    ok = _144;
    bool _176 = false;
    if (_144)
    {
        float4 _147 = _38.yzwx;
        float4 _148 = _38.yzwx;
        float3 _149 = _38.yzw;
        float3 _152 = float3(_11_testMatrix2x2[0].x, _147.xy);
        float3 _156 = float3(_147.zw, _148.x);
        float3 _160 = float3(_148.yzw);
        _176 = ((all(bool3(_152.x == float3(1.0f, 2.0f, 3.0f).x, _152.y == float3(1.0f, 2.0f, 3.0f).y, _152.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_156.x == float3(4.0f, 1.0f, 2.0f).x, _156.y == float3(4.0f, 1.0f, 2.0f).y, _156.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_160.x == float3(3.0f, 4.0f, 1.0f).x, _160.y == float3(3.0f, 4.0f, 1.0f).y, _160.z == float3(3.0f, 4.0f, 1.0f).z))) && all(bool3(_149.x == float3(2.0f, 3.0f, 4.0f).x, _149.y == float3(2.0f, 3.0f, 4.0f).y, _149.z == float3(2.0f, 3.0f, 4.0f).z));
    }
    else
    {
        _176 = false;
    }
    ok = _176;
    float4 _177 = 0.0f.xxxx;
    if (_176)
    {
        _177 = _11_colorGreen;
    }
    else
    {
        _177 = _11_colorRed;
    }
    return _177;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
