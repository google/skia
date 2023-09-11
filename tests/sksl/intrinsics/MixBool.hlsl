cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float4 _7_colorBlack : packoffset(c2);
    float4 _7_colorWhite : packoffset(c3);
    float4 _7_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _32 = _7_colorGreen * 100.0f;
    int4 _41 = int4(int(_32.x), int(_32.y), int(_32.z), int(_32.w));
    int4 intGreen = _41;
    float4 _46 = _7_colorRed * 100.0f;
    int4 _55 = int4(int(_46.x), int(_46.y), int(_46.z), int(_46.w));
    int4 intRed = _55;
    int _59 = _41.x;
    int _60 = _55.x;
    bool _75 = false;
    if ((false ? _60 : _59) == _59)
    {
        int2 _70 = _41.xy;
        int2 _71 = _55.xy;
        int2 _64 = int2(bool2(false, false).x ? _71.x : _70.x, bool2(false, false).y ? _71.y : _70.y);
        int2 _72 = _41.xy;
        _75 = all(bool2(_64.x == _72.x, _64.y == _72.y));
    }
    else
    {
        _75 = false;
    }
    bool _89 = false;
    if (_75)
    {
        int3 _84 = _41.xyz;
        int3 _85 = _55.xyz;
        int3 _78 = int3(bool3(false, false, false).x ? _85.x : _84.x, bool3(false, false, false).y ? _85.y : _84.y, bool3(false, false, false).z ? _85.z : _84.z);
        int3 _86 = _41.xyz;
        _89 = all(bool3(_78.x == _86.x, _78.y == _86.y, _78.z == _86.z));
    }
    else
    {
        _89 = false;
    }
    bool _97 = false;
    if (_89)
    {
        int4 _92 = int4(bool4(false, false, false, false).x ? _55.x : _41.x, bool4(false, false, false, false).y ? _55.y : _41.y, bool4(false, false, false, false).z ? _55.z : _41.z, bool4(false, false, false, false).w ? _55.w : _41.w);
        _97 = all(bool4(_92.x == _41.x, _92.y == _41.y, _92.z == _41.z, _92.w == _41.w));
    }
    else
    {
        _97 = false;
    }
    bool _103 = false;
    if (_97)
    {
        _103 = (true ? _60 : _59) == _60;
    }
    else
    {
        _103 = false;
    }
    bool _115 = false;
    if (_103)
    {
        int2 _110 = _41.xy;
        int2 _111 = _55.xy;
        int2 _106 = int2(bool2(true, true).x ? _111.x : _110.x, bool2(true, true).y ? _111.y : _110.y);
        int2 _112 = _55.xy;
        _115 = all(bool2(_106.x == _112.x, _106.y == _112.y));
    }
    else
    {
        _115 = false;
    }
    bool _127 = false;
    if (_115)
    {
        int3 _122 = _41.xyz;
        int3 _123 = _55.xyz;
        int3 _118 = int3(bool3(true, true, true).x ? _123.x : _122.x, bool3(true, true, true).y ? _123.y : _122.y, bool3(true, true, true).z ? _123.z : _122.z);
        int3 _124 = _55.xyz;
        _127 = all(bool3(_118.x == _124.x, _118.y == _124.y, _118.z == _124.z));
    }
    else
    {
        _127 = false;
    }
    bool _134 = false;
    if (_127)
    {
        int4 _130 = int4(bool4(true, true, true, true).x ? _55.x : _41.x, bool4(true, true, true, true).y ? _55.y : _41.y, bool4(true, true, true, true).z ? _55.z : _41.z, bool4(true, true, true, true).w ? _55.w : _41.w);
        _134 = all(bool4(_130.x == _55.x, _130.y == _55.y, _130.z == _55.z, _130.w == _55.w));
    }
    else
    {
        _134 = false;
    }
    bool _138 = false;
    if (_134)
    {
        _138 = 0 == _59;
    }
    else
    {
        _138 = false;
    }
    bool _146 = false;
    if (_138)
    {
        int2 _143 = _41.xy;
        _146 = all(bool2(int2(0, 100).x == _143.x, int2(0, 100).y == _143.y));
    }
    else
    {
        _146 = false;
    }
    bool _153 = false;
    if (_146)
    {
        int3 _150 = _41.xyz;
        _153 = all(bool3(int3(0, 100, 0).x == _150.x, int3(0, 100, 0).y == _150.y, int3(0, 100, 0).z == _150.z));
    }
    else
    {
        _153 = false;
    }
    bool _159 = false;
    if (_153)
    {
        _159 = all(bool4(int4(0, 100, 0, 100).x == _41.x, int4(0, 100, 0, 100).y == _41.y, int4(0, 100, 0, 100).z == _41.z, int4(0, 100, 0, 100).w == _41.w));
    }
    else
    {
        _159 = false;
    }
    bool _163 = false;
    if (_159)
    {
        _163 = 100 == _60;
    }
    else
    {
        _163 = false;
    }
    bool _170 = false;
    if (_163)
    {
        int2 _167 = _55.xy;
        _170 = all(bool2(int2(100, 0).x == _167.x, int2(100, 0).y == _167.y));
    }
    else
    {
        _170 = false;
    }
    bool _177 = false;
    if (_170)
    {
        int3 _174 = _55.xyz;
        _177 = all(bool3(int3(100, 0, 0).x == _174.x, int3(100, 0, 0).y == _174.y, int3(100, 0, 0).z == _174.z));
    }
    else
    {
        _177 = false;
    }
    bool _183 = false;
    if (_177)
    {
        _183 = all(bool4(int4(100, 0, 0, 100).x == _55.x, int4(100, 0, 0, 100).y == _55.y, int4(100, 0, 0, 100).z == _55.z, int4(100, 0, 0, 100).w == _55.w));
    }
    else
    {
        _183 = false;
    }
    bool _203 = false;
    if (_183)
    {
        _203 = (false ? _7_colorRed.x : _7_colorGreen.x) == _7_colorGreen.x;
    }
    else
    {
        _203 = false;
    }
    bool _224 = false;
    if (_203)
    {
        float2 _206 = float2(bool2(false, false).x ? _7_colorRed.xy.x : _7_colorGreen.xy.x, bool2(false, false).y ? _7_colorRed.xy.y : _7_colorGreen.xy.y);
        _224 = all(bool2(_206.x == _7_colorGreen.xy.x, _206.y == _7_colorGreen.xy.y));
    }
    else
    {
        _224 = false;
    }
    bool _246 = false;
    if (_224)
    {
        float3 _227 = float3(bool3(false, false, false).x ? _7_colorRed.xyz.x : _7_colorGreen.xyz.x, bool3(false, false, false).y ? _7_colorRed.xyz.y : _7_colorGreen.xyz.y, bool3(false, false, false).z ? _7_colorRed.xyz.z : _7_colorGreen.xyz.z);
        _246 = all(bool3(_227.x == _7_colorGreen.xyz.x, _227.y == _7_colorGreen.xyz.y, _227.z == _7_colorGreen.xyz.z));
    }
    else
    {
        _246 = false;
    }
    bool _262 = false;
    if (_246)
    {
        float4 _249 = float4(bool4(false, false, false, false).x ? _7_colorRed.x : _7_colorGreen.x, bool4(false, false, false, false).y ? _7_colorRed.y : _7_colorGreen.y, bool4(false, false, false, false).z ? _7_colorRed.z : _7_colorGreen.z, bool4(false, false, false, false).w ? _7_colorRed.w : _7_colorGreen.w);
        _262 = all(bool4(_249.x == _7_colorGreen.x, _249.y == _7_colorGreen.y, _249.z == _7_colorGreen.z, _249.w == _7_colorGreen.w));
    }
    else
    {
        _262 = false;
    }
    bool _282 = false;
    if (_262)
    {
        _282 = (true ? _7_colorRed.x : _7_colorGreen.x) == _7_colorRed.x;
    }
    else
    {
        _282 = false;
    }
    bool _303 = false;
    if (_282)
    {
        float2 _285 = float2(bool2(true, true).x ? _7_colorRed.xy.x : _7_colorGreen.xy.x, bool2(true, true).y ? _7_colorRed.xy.y : _7_colorGreen.xy.y);
        _303 = all(bool2(_285.x == _7_colorRed.xy.x, _285.y == _7_colorRed.xy.y));
    }
    else
    {
        _303 = false;
    }
    bool _324 = false;
    if (_303)
    {
        float3 _306 = float3(bool3(true, true, true).x ? _7_colorRed.xyz.x : _7_colorGreen.xyz.x, bool3(true, true, true).y ? _7_colorRed.xyz.y : _7_colorGreen.xyz.y, bool3(true, true, true).z ? _7_colorRed.xyz.z : _7_colorGreen.xyz.z);
        _324 = all(bool3(_306.x == _7_colorRed.xyz.x, _306.y == _7_colorRed.xyz.y, _306.z == _7_colorRed.xyz.z));
    }
    else
    {
        _324 = false;
    }
    bool _340 = false;
    if (_324)
    {
        float4 _327 = float4(bool4(true, true, true, true).x ? _7_colorRed.x : _7_colorGreen.x, bool4(true, true, true, true).y ? _7_colorRed.y : _7_colorGreen.y, bool4(true, true, true, true).z ? _7_colorRed.z : _7_colorGreen.z, bool4(true, true, true, true).w ? _7_colorRed.w : _7_colorGreen.w);
        _340 = all(bool4(_327.x == _7_colorRed.x, _327.y == _7_colorRed.y, _327.z == _7_colorRed.z, _327.w == _7_colorRed.w));
    }
    else
    {
        _340 = false;
    }
    bool _347 = false;
    if (_340)
    {
        _347 = 0.0f == _7_colorGreen.x;
    }
    else
    {
        _347 = false;
    }
    bool _357 = false;
    if (_347)
    {
        _357 = all(bool2(float2(0.0f, 1.0f).x == _7_colorGreen.xy.x, float2(0.0f, 1.0f).y == _7_colorGreen.xy.y));
    }
    else
    {
        _357 = false;
    }
    bool _366 = false;
    if (_357)
    {
        _366 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == _7_colorGreen.xyz.x, float3(0.0f, 1.0f, 0.0f).y == _7_colorGreen.xyz.y, float3(0.0f, 1.0f, 0.0f).z == _7_colorGreen.xyz.z));
    }
    else
    {
        _366 = false;
    }
    bool _374 = false;
    if (_366)
    {
        _374 = all(bool4(float4(0.0f, 1.0f, 0.0f, 1.0f).x == _7_colorGreen.x, float4(0.0f, 1.0f, 0.0f, 1.0f).y == _7_colorGreen.y, float4(0.0f, 1.0f, 0.0f, 1.0f).z == _7_colorGreen.z, float4(0.0f, 1.0f, 0.0f, 1.0f).w == _7_colorGreen.w));
    }
    else
    {
        _374 = false;
    }
    bool _381 = false;
    if (_374)
    {
        _381 = 1.0f == _7_colorRed.x;
    }
    else
    {
        _381 = false;
    }
    bool _390 = false;
    if (_381)
    {
        _390 = all(bool2(float2(1.0f, 0.0f).x == _7_colorRed.xy.x, float2(1.0f, 0.0f).y == _7_colorRed.xy.y));
    }
    else
    {
        _390 = false;
    }
    bool _399 = false;
    if (_390)
    {
        _399 = all(bool3(float3(1.0f, 0.0f, 0.0f).x == _7_colorRed.xyz.x, float3(1.0f, 0.0f, 0.0f).y == _7_colorRed.xyz.y, float3(1.0f, 0.0f, 0.0f).z == _7_colorRed.xyz.z));
    }
    else
    {
        _399 = false;
    }
    bool _407 = false;
    if (_399)
    {
        _407 = all(bool4(float4(1.0f, 0.0f, 0.0f, 1.0f).x == _7_colorRed.x, float4(1.0f, 0.0f, 0.0f, 1.0f).y == _7_colorRed.y, float4(1.0f, 0.0f, 0.0f, 1.0f).z == _7_colorRed.z, float4(1.0f, 0.0f, 0.0f, 1.0f).w == _7_colorRed.w));
    }
    else
    {
        _407 = false;
    }
    float4 _408 = 0.0f.xxxx;
    if (_407)
    {
        _408 = _7_colorGreen;
    }
    else
    {
        _408 = _7_colorRed;
    }
    return _408;
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
