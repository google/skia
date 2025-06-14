cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float4 _11_colorBlack : packoffset(c2);
    float4 _11_colorWhite : packoffset(c3);
    float4 _11_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool4 _43 = bool4(_11_colorGreen.x != 0.0f, _11_colorGreen.y != 0.0f, _11_colorGreen.z != 0.0f, _11_colorGreen.w != 0.0f);
    bool4 FTFT = _43;
    bool4 _45 = _43.wzyx;
    bool4 TFTF = _45;
    bool _92 = false;
    if ((_43.x ? _11_colorWhite.x : _11_colorBlack.x) == _11_colorBlack.x)
    {
        bool2 _84 = _43.xy;
        float2 _69 = float2(_84.x ? _11_colorWhite.xy.x : _11_colorBlack.xy.x, _84.y ? _11_colorWhite.xy.y : _11_colorBlack.xy.y);
        float2 _89 = float2(_11_colorBlack.x, 1.0f);
        _92 = all(bool2(_69.x == _89.x, _69.y == _89.y));
    }
    else
    {
        _92 = false;
    }
    bool _121 = false;
    if (_92)
    {
        bool3 _111 = _43.xyz;
        float3 _95 = float3(_111.x ? _11_colorWhite.xyz.x : _11_colorBlack.xyz.x, _111.y ? _11_colorWhite.xyz.y : _11_colorBlack.xyz.y, _111.z ? _11_colorWhite.xyz.z : _11_colorBlack.xyz.z);
        float3 _118 = float3(_11_colorBlack.x, 1.0f, _11_colorBlack.z);
        _121 = all(bool3(_95.x == _118.x, _95.y == _118.y, _95.z == _118.z));
    }
    else
    {
        _121 = false;
    }
    bool _142 = false;
    if (_121)
    {
        float4 _124 = float4(_43.x ? _11_colorWhite.x : _11_colorBlack.x, _43.y ? _11_colorWhite.y : _11_colorBlack.y, _43.z ? _11_colorWhite.z : _11_colorBlack.z, _43.w ? _11_colorWhite.w : _11_colorBlack.w);
        float4 _139 = float4(_11_colorBlack.x, 1.0f, _11_colorBlack.z, 1.0f);
        _142 = all(bool4(_124.x == _139.x, _124.y == _139.y, _124.z == _139.z, _124.w == _139.w));
    }
    else
    {
        _142 = false;
    }
    bool _164 = false;
    if (_142)
    {
        _164 = (_45.x ? _11_testInputs.x : _11_colorWhite.x) == _11_testInputs.x;
    }
    else
    {
        _164 = false;
    }
    bool _188 = false;
    if (_164)
    {
        bool2 _181 = _45.xy;
        float2 _167 = float2(_181.x ? _11_testInputs.xy.x : _11_colorWhite.xy.x, _181.y ? _11_testInputs.xy.y : _11_colorWhite.xy.y);
        float2 _185 = float2(_11_testInputs.x, 1.0f);
        _188 = all(bool2(_167.x == _185.x, _167.y == _185.y));
    }
    else
    {
        _188 = false;
    }
    bool _215 = false;
    if (_188)
    {
        bool3 _205 = _45.xyz;
        float3 _191 = float3(_205.x ? _11_testInputs.xyz.x : _11_colorWhite.xyz.x, _205.y ? _11_testInputs.xyz.y : _11_colorWhite.xyz.y, _205.z ? _11_testInputs.xyz.z : _11_colorWhite.xyz.z);
        float3 _212 = float3(_11_testInputs.x, 1.0f, _11_testInputs.z);
        _215 = all(bool3(_191.x == _212.x, _191.y == _212.y, _191.z == _212.z));
    }
    else
    {
        _215 = false;
    }
    bool _236 = false;
    if (_215)
    {
        float4 _218 = float4(_45.x ? _11_testInputs.x : _11_colorWhite.x, _45.y ? _11_testInputs.y : _11_colorWhite.y, _45.z ? _11_testInputs.z : _11_colorWhite.z, _45.w ? _11_testInputs.w : _11_colorWhite.w);
        float4 _233 = float4(_11_testInputs.x, 1.0f, _11_testInputs.z, 1.0f);
        _236 = all(bool4(_218.x == _233.x, _218.y == _233.y, _218.z == _233.z, _218.w == _233.w));
    }
    else
    {
        _236 = false;
    }
    float4 _237 = 0.0f.xxxx;
    if (_236)
    {
        _237 = _11_colorGreen;
    }
    else
    {
        _237 = _11_colorRed;
    }
    return _237;
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
