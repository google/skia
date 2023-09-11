cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    row_major float2x2 _7_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    float4 _35 = float4(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y);
    float4 f4 = _35;
    float3 _41 = float3(_7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y, _7_testMatrix2x2[1].x);
    float3 _44 = float3(_7_testMatrix2x2[1].y, _35.xy);
    bool _59 = all(bool3(_41.x == float3(1.0f, 2.0f, 3.0f).x, _41.y == float3(1.0f, 2.0f, 3.0f).y, _41.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_44.x == float3(4.0f, 1.0f, 2.0f).x, _44.y == float3(4.0f, 1.0f, 2.0f).y, _44.z == float3(4.0f, 1.0f, 2.0f).z));
    bool ok = _59;
    bool _84 = false;
    if (_59)
    {
        float4 _64 = _35.wxyz;
        float4 _69 = float4(_35.xyz, _64.x);
        float4 _73 = float4(_64.yzw, _7_testMatrix2x2[1].y);
        _84 = all(bool4(_69.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _69.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _69.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _69.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w)) && all(bool4(_73.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _73.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _73.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _73.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _84 = false;
    }
    ok = _84;
    bool _108 = false;
    if (_84)
    {
        float2 _88 = _35.zw;
        float3 _92 = float3(_35.xy, _88.x);
        float3 _94 = float3(_88.y, _7_testMatrix2x2[0].x, _7_testMatrix2x2[0].y);
        float3 _95 = float3(_7_testMatrix2x2[1].x, _7_testMatrix2x2[1].y, _7_testMatrix2x2[0].x);
        _108 = (all(bool3(_92.x == float3(1.0f, 2.0f, 3.0f).x, _92.y == float3(1.0f, 2.0f, 3.0f).y, _92.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_94.x == float3(4.0f, 1.0f, 2.0f).x, _94.y == float3(4.0f, 1.0f, 2.0f).y, _94.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_95.x == float3(3.0f, 4.0f, 1.0f).x, _95.y == float3(3.0f, 4.0f, 1.0f).y, _95.z == float3(3.0f, 4.0f, 1.0f).z));
    }
    else
    {
        _108 = false;
    }
    ok = _108;
    bool _141 = false;
    if (_108)
    {
        float3 _111 = _35.xyz;
        float4 _112 = _35.wxyz;
        float2 _115 = float2(_111.xy);
        float2 _118 = float2(_111.z, _112.x);
        float2 _121 = float2(_112.yz);
        float2 _123 = float2(_112.w, _7_testMatrix2x2[1].y);
        _141 = ((all(bool2(_115.x == float2(1.0f, 2.0f).x, _115.y == float2(1.0f, 2.0f).y)) && all(bool2(_118.x == float2(3.0f, 4.0f).x, _118.y == float2(3.0f, 4.0f).y))) && all(bool2(_121.x == float2(1.0f, 2.0f).x, _121.y == float2(1.0f, 2.0f).y))) && all(bool2(_123.x == float2(3.0f, 4.0f).x, _123.y == float2(3.0f, 4.0f).y));
    }
    else
    {
        _141 = false;
    }
    ok = _141;
    bool _173 = false;
    if (_141)
    {
        float4 _144 = _35.yzwx;
        float4 _145 = _35.yzwx;
        float3 _146 = _35.yzw;
        float3 _149 = float3(_7_testMatrix2x2[0].x, _144.xy);
        float3 _153 = float3(_144.zw, _145.x);
        float3 _157 = float3(_145.yzw);
        _173 = ((all(bool3(_149.x == float3(1.0f, 2.0f, 3.0f).x, _149.y == float3(1.0f, 2.0f, 3.0f).y, _149.z == float3(1.0f, 2.0f, 3.0f).z)) && all(bool3(_153.x == float3(4.0f, 1.0f, 2.0f).x, _153.y == float3(4.0f, 1.0f, 2.0f).y, _153.z == float3(4.0f, 1.0f, 2.0f).z))) && all(bool3(_157.x == float3(3.0f, 4.0f, 1.0f).x, _157.y == float3(3.0f, 4.0f, 1.0f).y, _157.z == float3(3.0f, 4.0f, 1.0f).z))) && all(bool3(_146.x == float3(2.0f, 3.0f, 4.0f).x, _146.y == float3(2.0f, 3.0f, 4.0f).y, _146.z == float3(2.0f, 3.0f, 4.0f).z));
    }
    else
    {
        _173 = false;
    }
    ok = _173;
    float4 _174 = 0.0f.xxxx;
    if (_173)
    {
        _174 = _7_colorGreen;
    }
    else
    {
        _174 = _7_colorRed;
    }
    return _174;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
