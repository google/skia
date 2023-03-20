cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float4 _10_colorBlack : packoffset(c2);
    float4 _10_colorWhite : packoffset(c3);
    float4 _10_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _35 = _10_colorGreen * 100.0f;
    int4 _44 = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 intGreen = _44;
    float4 _49 = _10_colorRed * 100.0f;
    int4 _58 = int4(int(_49.x), int(_49.y), int(_49.z), int(_49.w));
    int4 intRed = _58;
    int _61 = _44.x;
    int _62 = _58.x;
    bool _77 = false;
    if ((false ? _62 : _61) == _61)
    {
        int2 _72 = _44.xy;
        int2 _73 = _58.xy;
        int2 _66 = int2(bool2(false, false).x ? _73.x : _72.x, bool2(false, false).y ? _73.y : _72.y);
        int2 _74 = _44.xy;
        _77 = all(bool2(_66.x == _74.x, _66.y == _74.y));
    }
    else
    {
        _77 = false;
    }
    bool _91 = false;
    if (_77)
    {
        int3 _86 = _44.xyz;
        int3 _87 = _58.xyz;
        int3 _80 = int3(bool3(false, false, false).x ? _87.x : _86.x, bool3(false, false, false).y ? _87.y : _86.y, bool3(false, false, false).z ? _87.z : _86.z);
        int3 _88 = _44.xyz;
        _91 = all(bool3(_80.x == _88.x, _80.y == _88.y, _80.z == _88.z));
    }
    else
    {
        _91 = false;
    }
    bool _99 = false;
    if (_91)
    {
        int4 _94 = int4(bool4(false, false, false, false).x ? _58.x : _44.x, bool4(false, false, false, false).y ? _58.y : _44.y, bool4(false, false, false, false).z ? _58.z : _44.z, bool4(false, false, false, false).w ? _58.w : _44.w);
        _99 = all(bool4(_94.x == _44.x, _94.y == _44.y, _94.z == _44.z, _94.w == _44.w));
    }
    else
    {
        _99 = false;
    }
    bool _105 = false;
    if (_99)
    {
        _105 = (true ? _62 : _61) == _62;
    }
    else
    {
        _105 = false;
    }
    bool _117 = false;
    if (_105)
    {
        int2 _112 = _44.xy;
        int2 _113 = _58.xy;
        int2 _108 = int2(bool2(true, true).x ? _113.x : _112.x, bool2(true, true).y ? _113.y : _112.y);
        int2 _114 = _58.xy;
        _117 = all(bool2(_108.x == _114.x, _108.y == _114.y));
    }
    else
    {
        _117 = false;
    }
    bool _129 = false;
    if (_117)
    {
        int3 _124 = _44.xyz;
        int3 _125 = _58.xyz;
        int3 _120 = int3(bool3(true, true, true).x ? _125.x : _124.x, bool3(true, true, true).y ? _125.y : _124.y, bool3(true, true, true).z ? _125.z : _124.z);
        int3 _126 = _58.xyz;
        _129 = all(bool3(_120.x == _126.x, _120.y == _126.y, _120.z == _126.z));
    }
    else
    {
        _129 = false;
    }
    bool _136 = false;
    if (_129)
    {
        int4 _132 = int4(bool4(true, true, true, true).x ? _58.x : _44.x, bool4(true, true, true, true).y ? _58.y : _44.y, bool4(true, true, true, true).z ? _58.z : _44.z, bool4(true, true, true, true).w ? _58.w : _44.w);
        _136 = all(bool4(_132.x == _58.x, _132.y == _58.y, _132.z == _58.z, _132.w == _58.w));
    }
    else
    {
        _136 = false;
    }
    bool _140 = false;
    if (_136)
    {
        _140 = 0 == _61;
    }
    else
    {
        _140 = false;
    }
    bool _148 = false;
    if (_140)
    {
        int2 _145 = _44.xy;
        _148 = all(bool2(int2(0, 100).x == _145.x, int2(0, 100).y == _145.y));
    }
    else
    {
        _148 = false;
    }
    bool _155 = false;
    if (_148)
    {
        int3 _152 = _44.xyz;
        _155 = all(bool3(int3(0, 100, 0).x == _152.x, int3(0, 100, 0).y == _152.y, int3(0, 100, 0).z == _152.z));
    }
    else
    {
        _155 = false;
    }
    bool _161 = false;
    if (_155)
    {
        _161 = all(bool4(int4(0, 100, 0, 100).x == _44.x, int4(0, 100, 0, 100).y == _44.y, int4(0, 100, 0, 100).z == _44.z, int4(0, 100, 0, 100).w == _44.w));
    }
    else
    {
        _161 = false;
    }
    bool _165 = false;
    if (_161)
    {
        _165 = 100 == _62;
    }
    else
    {
        _165 = false;
    }
    bool _172 = false;
    if (_165)
    {
        int2 _169 = _58.xy;
        _172 = all(bool2(int2(100, 0).x == _169.x, int2(100, 0).y == _169.y));
    }
    else
    {
        _172 = false;
    }
    bool _179 = false;
    if (_172)
    {
        int3 _176 = _58.xyz;
        _179 = all(bool3(int3(100, 0, 0).x == _176.x, int3(100, 0, 0).y == _176.y, int3(100, 0, 0).z == _176.z));
    }
    else
    {
        _179 = false;
    }
    bool _185 = false;
    if (_179)
    {
        _185 = all(bool4(int4(100, 0, 0, 100).x == _58.x, int4(100, 0, 0, 100).y == _58.y, int4(100, 0, 0, 100).z == _58.z, int4(100, 0, 0, 100).w == _58.w));
    }
    else
    {
        _185 = false;
    }
    bool _205 = false;
    if (_185)
    {
        _205 = (false ? _10_colorRed.x : _10_colorGreen.x) == _10_colorGreen.x;
    }
    else
    {
        _205 = false;
    }
    bool _226 = false;
    if (_205)
    {
        float2 _208 = float2(bool2(false, false).x ? _10_colorRed.xy.x : _10_colorGreen.xy.x, bool2(false, false).y ? _10_colorRed.xy.y : _10_colorGreen.xy.y);
        _226 = all(bool2(_208.x == _10_colorGreen.xy.x, _208.y == _10_colorGreen.xy.y));
    }
    else
    {
        _226 = false;
    }
    bool _248 = false;
    if (_226)
    {
        float3 _229 = float3(bool3(false, false, false).x ? _10_colorRed.xyz.x : _10_colorGreen.xyz.x, bool3(false, false, false).y ? _10_colorRed.xyz.y : _10_colorGreen.xyz.y, bool3(false, false, false).z ? _10_colorRed.xyz.z : _10_colorGreen.xyz.z);
        _248 = all(bool3(_229.x == _10_colorGreen.xyz.x, _229.y == _10_colorGreen.xyz.y, _229.z == _10_colorGreen.xyz.z));
    }
    else
    {
        _248 = false;
    }
    bool _264 = false;
    if (_248)
    {
        float4 _251 = float4(bool4(false, false, false, false).x ? _10_colorRed.x : _10_colorGreen.x, bool4(false, false, false, false).y ? _10_colorRed.y : _10_colorGreen.y, bool4(false, false, false, false).z ? _10_colorRed.z : _10_colorGreen.z, bool4(false, false, false, false).w ? _10_colorRed.w : _10_colorGreen.w);
        _264 = all(bool4(_251.x == _10_colorGreen.x, _251.y == _10_colorGreen.y, _251.z == _10_colorGreen.z, _251.w == _10_colorGreen.w));
    }
    else
    {
        _264 = false;
    }
    bool _284 = false;
    if (_264)
    {
        _284 = (true ? _10_colorRed.x : _10_colorGreen.x) == _10_colorRed.x;
    }
    else
    {
        _284 = false;
    }
    bool _305 = false;
    if (_284)
    {
        float2 _287 = float2(bool2(true, true).x ? _10_colorRed.xy.x : _10_colorGreen.xy.x, bool2(true, true).y ? _10_colorRed.xy.y : _10_colorGreen.xy.y);
        _305 = all(bool2(_287.x == _10_colorRed.xy.x, _287.y == _10_colorRed.xy.y));
    }
    else
    {
        _305 = false;
    }
    bool _326 = false;
    if (_305)
    {
        float3 _308 = float3(bool3(true, true, true).x ? _10_colorRed.xyz.x : _10_colorGreen.xyz.x, bool3(true, true, true).y ? _10_colorRed.xyz.y : _10_colorGreen.xyz.y, bool3(true, true, true).z ? _10_colorRed.xyz.z : _10_colorGreen.xyz.z);
        _326 = all(bool3(_308.x == _10_colorRed.xyz.x, _308.y == _10_colorRed.xyz.y, _308.z == _10_colorRed.xyz.z));
    }
    else
    {
        _326 = false;
    }
    bool _342 = false;
    if (_326)
    {
        float4 _329 = float4(bool4(true, true, true, true).x ? _10_colorRed.x : _10_colorGreen.x, bool4(true, true, true, true).y ? _10_colorRed.y : _10_colorGreen.y, bool4(true, true, true, true).z ? _10_colorRed.z : _10_colorGreen.z, bool4(true, true, true, true).w ? _10_colorRed.w : _10_colorGreen.w);
        _342 = all(bool4(_329.x == _10_colorRed.x, _329.y == _10_colorRed.y, _329.z == _10_colorRed.z, _329.w == _10_colorRed.w));
    }
    else
    {
        _342 = false;
    }
    bool _349 = false;
    if (_342)
    {
        _349 = 0.0f == _10_colorGreen.x;
    }
    else
    {
        _349 = false;
    }
    bool _359 = false;
    if (_349)
    {
        _359 = all(bool2(float2(0.0f, 1.0f).x == _10_colorGreen.xy.x, float2(0.0f, 1.0f).y == _10_colorGreen.xy.y));
    }
    else
    {
        _359 = false;
    }
    bool _368 = false;
    if (_359)
    {
        _368 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == _10_colorGreen.xyz.x, float3(0.0f, 1.0f, 0.0f).y == _10_colorGreen.xyz.y, float3(0.0f, 1.0f, 0.0f).z == _10_colorGreen.xyz.z));
    }
    else
    {
        _368 = false;
    }
    bool _376 = false;
    if (_368)
    {
        _376 = all(bool4(float4(0.0f, 1.0f, 0.0f, 1.0f).x == _10_colorGreen.x, float4(0.0f, 1.0f, 0.0f, 1.0f).y == _10_colorGreen.y, float4(0.0f, 1.0f, 0.0f, 1.0f).z == _10_colorGreen.z, float4(0.0f, 1.0f, 0.0f, 1.0f).w == _10_colorGreen.w));
    }
    else
    {
        _376 = false;
    }
    bool _383 = false;
    if (_376)
    {
        _383 = 1.0f == _10_colorRed.x;
    }
    else
    {
        _383 = false;
    }
    bool _392 = false;
    if (_383)
    {
        _392 = all(bool2(float2(1.0f, 0.0f).x == _10_colorRed.xy.x, float2(1.0f, 0.0f).y == _10_colorRed.xy.y));
    }
    else
    {
        _392 = false;
    }
    bool _401 = false;
    if (_392)
    {
        _401 = all(bool3(float3(1.0f, 0.0f, 0.0f).x == _10_colorRed.xyz.x, float3(1.0f, 0.0f, 0.0f).y == _10_colorRed.xyz.y, float3(1.0f, 0.0f, 0.0f).z == _10_colorRed.xyz.z));
    }
    else
    {
        _401 = false;
    }
    bool _409 = false;
    if (_401)
    {
        _409 = all(bool4(float4(1.0f, 0.0f, 0.0f, 1.0f).x == _10_colorRed.x, float4(1.0f, 0.0f, 0.0f, 1.0f).y == _10_colorRed.y, float4(1.0f, 0.0f, 0.0f, 1.0f).z == _10_colorRed.z, float4(1.0f, 0.0f, 0.0f, 1.0f).w == _10_colorRed.w));
    }
    else
    {
        _409 = false;
    }
    float4 _410 = 0.0f.xxxx;
    if (_409)
    {
        _410 = _10_colorGreen;
    }
    else
    {
        _410 = _10_colorRed;
    }
    return _410;
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
