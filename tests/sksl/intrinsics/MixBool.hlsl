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
    float4 _35 = _11_colorGreen * 100.0f;
    int4 _44 = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 intGreen = _44;
    float4 _49 = _11_colorRed * 100.0f;
    int4 _58 = int4(int(_49.x), int(_49.y), int(_49.z), int(_49.w));
    int4 intRed = _58;
    int _62 = _44.x;
    int _63 = _58.x;
    bool _78 = false;
    if ((false ? _63 : _62) == _62)
    {
        int2 _73 = _44.xy;
        int2 _74 = _58.xy;
        int2 _67 = int2(bool2(false, false).x ? _74.x : _73.x, bool2(false, false).y ? _74.y : _73.y);
        int2 _75 = _44.xy;
        _78 = all(bool2(_67.x == _75.x, _67.y == _75.y));
    }
    else
    {
        _78 = false;
    }
    bool _92 = false;
    if (_78)
    {
        int3 _87 = _44.xyz;
        int3 _88 = _58.xyz;
        int3 _81 = int3(bool3(false, false, false).x ? _88.x : _87.x, bool3(false, false, false).y ? _88.y : _87.y, bool3(false, false, false).z ? _88.z : _87.z);
        int3 _89 = _44.xyz;
        _92 = all(bool3(_81.x == _89.x, _81.y == _89.y, _81.z == _89.z));
    }
    else
    {
        _92 = false;
    }
    bool _100 = false;
    if (_92)
    {
        int4 _95 = int4(bool4(false, false, false, false).x ? _58.x : _44.x, bool4(false, false, false, false).y ? _58.y : _44.y, bool4(false, false, false, false).z ? _58.z : _44.z, bool4(false, false, false, false).w ? _58.w : _44.w);
        _100 = all(bool4(_95.x == _44.x, _95.y == _44.y, _95.z == _44.z, _95.w == _44.w));
    }
    else
    {
        _100 = false;
    }
    bool _106 = false;
    if (_100)
    {
        _106 = (true ? _63 : _62) == _63;
    }
    else
    {
        _106 = false;
    }
    bool _118 = false;
    if (_106)
    {
        int2 _113 = _44.xy;
        int2 _114 = _58.xy;
        int2 _109 = int2(bool2(true, true).x ? _114.x : _113.x, bool2(true, true).y ? _114.y : _113.y);
        int2 _115 = _58.xy;
        _118 = all(bool2(_109.x == _115.x, _109.y == _115.y));
    }
    else
    {
        _118 = false;
    }
    bool _130 = false;
    if (_118)
    {
        int3 _125 = _44.xyz;
        int3 _126 = _58.xyz;
        int3 _121 = int3(bool3(true, true, true).x ? _126.x : _125.x, bool3(true, true, true).y ? _126.y : _125.y, bool3(true, true, true).z ? _126.z : _125.z);
        int3 _127 = _58.xyz;
        _130 = all(bool3(_121.x == _127.x, _121.y == _127.y, _121.z == _127.z));
    }
    else
    {
        _130 = false;
    }
    bool _137 = false;
    if (_130)
    {
        int4 _133 = int4(bool4(true, true, true, true).x ? _58.x : _44.x, bool4(true, true, true, true).y ? _58.y : _44.y, bool4(true, true, true, true).z ? _58.z : _44.z, bool4(true, true, true, true).w ? _58.w : _44.w);
        _137 = all(bool4(_133.x == _58.x, _133.y == _58.y, _133.z == _58.z, _133.w == _58.w));
    }
    else
    {
        _137 = false;
    }
    bool _141 = false;
    if (_137)
    {
        _141 = 0 == _62;
    }
    else
    {
        _141 = false;
    }
    bool _149 = false;
    if (_141)
    {
        int2 _146 = _44.xy;
        _149 = all(bool2(int2(0, 100).x == _146.x, int2(0, 100).y == _146.y));
    }
    else
    {
        _149 = false;
    }
    bool _156 = false;
    if (_149)
    {
        int3 _153 = _44.xyz;
        _156 = all(bool3(int3(0, 100, 0).x == _153.x, int3(0, 100, 0).y == _153.y, int3(0, 100, 0).z == _153.z));
    }
    else
    {
        _156 = false;
    }
    bool _162 = false;
    if (_156)
    {
        _162 = all(bool4(int4(0, 100, 0, 100).x == _44.x, int4(0, 100, 0, 100).y == _44.y, int4(0, 100, 0, 100).z == _44.z, int4(0, 100, 0, 100).w == _44.w));
    }
    else
    {
        _162 = false;
    }
    bool _166 = false;
    if (_162)
    {
        _166 = 100 == _63;
    }
    else
    {
        _166 = false;
    }
    bool _173 = false;
    if (_166)
    {
        int2 _170 = _58.xy;
        _173 = all(bool2(int2(100, 0).x == _170.x, int2(100, 0).y == _170.y));
    }
    else
    {
        _173 = false;
    }
    bool _180 = false;
    if (_173)
    {
        int3 _177 = _58.xyz;
        _180 = all(bool3(int3(100, 0, 0).x == _177.x, int3(100, 0, 0).y == _177.y, int3(100, 0, 0).z == _177.z));
    }
    else
    {
        _180 = false;
    }
    bool _186 = false;
    if (_180)
    {
        _186 = all(bool4(int4(100, 0, 0, 100).x == _58.x, int4(100, 0, 0, 100).y == _58.y, int4(100, 0, 0, 100).z == _58.z, int4(100, 0, 0, 100).w == _58.w));
    }
    else
    {
        _186 = false;
    }
    bool _206 = false;
    if (_186)
    {
        _206 = (false ? _11_colorRed.x : _11_colorGreen.x) == _11_colorGreen.x;
    }
    else
    {
        _206 = false;
    }
    bool _227 = false;
    if (_206)
    {
        float2 _209 = float2(bool2(false, false).x ? _11_colorRed.xy.x : _11_colorGreen.xy.x, bool2(false, false).y ? _11_colorRed.xy.y : _11_colorGreen.xy.y);
        _227 = all(bool2(_209.x == _11_colorGreen.xy.x, _209.y == _11_colorGreen.xy.y));
    }
    else
    {
        _227 = false;
    }
    bool _249 = false;
    if (_227)
    {
        float3 _230 = float3(bool3(false, false, false).x ? _11_colorRed.xyz.x : _11_colorGreen.xyz.x, bool3(false, false, false).y ? _11_colorRed.xyz.y : _11_colorGreen.xyz.y, bool3(false, false, false).z ? _11_colorRed.xyz.z : _11_colorGreen.xyz.z);
        _249 = all(bool3(_230.x == _11_colorGreen.xyz.x, _230.y == _11_colorGreen.xyz.y, _230.z == _11_colorGreen.xyz.z));
    }
    else
    {
        _249 = false;
    }
    bool _265 = false;
    if (_249)
    {
        float4 _252 = float4(bool4(false, false, false, false).x ? _11_colorRed.x : _11_colorGreen.x, bool4(false, false, false, false).y ? _11_colorRed.y : _11_colorGreen.y, bool4(false, false, false, false).z ? _11_colorRed.z : _11_colorGreen.z, bool4(false, false, false, false).w ? _11_colorRed.w : _11_colorGreen.w);
        _265 = all(bool4(_252.x == _11_colorGreen.x, _252.y == _11_colorGreen.y, _252.z == _11_colorGreen.z, _252.w == _11_colorGreen.w));
    }
    else
    {
        _265 = false;
    }
    bool _285 = false;
    if (_265)
    {
        _285 = (true ? _11_colorRed.x : _11_colorGreen.x) == _11_colorRed.x;
    }
    else
    {
        _285 = false;
    }
    bool _306 = false;
    if (_285)
    {
        float2 _288 = float2(bool2(true, true).x ? _11_colorRed.xy.x : _11_colorGreen.xy.x, bool2(true, true).y ? _11_colorRed.xy.y : _11_colorGreen.xy.y);
        _306 = all(bool2(_288.x == _11_colorRed.xy.x, _288.y == _11_colorRed.xy.y));
    }
    else
    {
        _306 = false;
    }
    bool _327 = false;
    if (_306)
    {
        float3 _309 = float3(bool3(true, true, true).x ? _11_colorRed.xyz.x : _11_colorGreen.xyz.x, bool3(true, true, true).y ? _11_colorRed.xyz.y : _11_colorGreen.xyz.y, bool3(true, true, true).z ? _11_colorRed.xyz.z : _11_colorGreen.xyz.z);
        _327 = all(bool3(_309.x == _11_colorRed.xyz.x, _309.y == _11_colorRed.xyz.y, _309.z == _11_colorRed.xyz.z));
    }
    else
    {
        _327 = false;
    }
    bool _343 = false;
    if (_327)
    {
        float4 _330 = float4(bool4(true, true, true, true).x ? _11_colorRed.x : _11_colorGreen.x, bool4(true, true, true, true).y ? _11_colorRed.y : _11_colorGreen.y, bool4(true, true, true, true).z ? _11_colorRed.z : _11_colorGreen.z, bool4(true, true, true, true).w ? _11_colorRed.w : _11_colorGreen.w);
        _343 = all(bool4(_330.x == _11_colorRed.x, _330.y == _11_colorRed.y, _330.z == _11_colorRed.z, _330.w == _11_colorRed.w));
    }
    else
    {
        _343 = false;
    }
    bool _350 = false;
    if (_343)
    {
        _350 = 0.0f == _11_colorGreen.x;
    }
    else
    {
        _350 = false;
    }
    bool _360 = false;
    if (_350)
    {
        _360 = all(bool2(float2(0.0f, 1.0f).x == _11_colorGreen.xy.x, float2(0.0f, 1.0f).y == _11_colorGreen.xy.y));
    }
    else
    {
        _360 = false;
    }
    bool _369 = false;
    if (_360)
    {
        _369 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == _11_colorGreen.xyz.x, float3(0.0f, 1.0f, 0.0f).y == _11_colorGreen.xyz.y, float3(0.0f, 1.0f, 0.0f).z == _11_colorGreen.xyz.z));
    }
    else
    {
        _369 = false;
    }
    bool _377 = false;
    if (_369)
    {
        _377 = all(bool4(float4(0.0f, 1.0f, 0.0f, 1.0f).x == _11_colorGreen.x, float4(0.0f, 1.0f, 0.0f, 1.0f).y == _11_colorGreen.y, float4(0.0f, 1.0f, 0.0f, 1.0f).z == _11_colorGreen.z, float4(0.0f, 1.0f, 0.0f, 1.0f).w == _11_colorGreen.w));
    }
    else
    {
        _377 = false;
    }
    bool _384 = false;
    if (_377)
    {
        _384 = 1.0f == _11_colorRed.x;
    }
    else
    {
        _384 = false;
    }
    bool _393 = false;
    if (_384)
    {
        _393 = all(bool2(float2(1.0f, 0.0f).x == _11_colorRed.xy.x, float2(1.0f, 0.0f).y == _11_colorRed.xy.y));
    }
    else
    {
        _393 = false;
    }
    bool _402 = false;
    if (_393)
    {
        _402 = all(bool3(float3(1.0f, 0.0f, 0.0f).x == _11_colorRed.xyz.x, float3(1.0f, 0.0f, 0.0f).y == _11_colorRed.xyz.y, float3(1.0f, 0.0f, 0.0f).z == _11_colorRed.xyz.z));
    }
    else
    {
        _402 = false;
    }
    bool _410 = false;
    if (_402)
    {
        _410 = all(bool4(float4(1.0f, 0.0f, 0.0f, 1.0f).x == _11_colorRed.x, float4(1.0f, 0.0f, 0.0f, 1.0f).y == _11_colorRed.y, float4(1.0f, 0.0f, 0.0f, 1.0f).z == _11_colorRed.z, float4(1.0f, 0.0f, 0.0f, 1.0f).w == _11_colorRed.w));
    }
    else
    {
        _410 = false;
    }
    float4 _411 = 0.0f.xxxx;
    if (_410)
    {
        _411 = _11_colorGreen;
    }
    else
    {
        _411 = _11_colorRed;
    }
    return _411;
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
