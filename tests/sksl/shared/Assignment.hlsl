struct S
{
    float f;
    float af[5];
    float4 h4;
    float4 ah4[5];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _22_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4 globalVar = 0.0f.xxxx;
static S globalStruct = { 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f.xxxx, { 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx } };

void keepAlive_vf(float _39)
{
}

void keepAlive_vh(float _37)
{
}

void keepAlive_vi(int _43)
{
}

float4 main(float2 _46)
{
    int i = 0;
    int4 i4 = int4(1, 2, 3, 4);
    float3x3 f3x3 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
    float4 x = 0.0f.xxxx;
    x.w = 0.0f;
    x = float4(0.0f.xx.y, 0.0f.xx.x, x.z, x.w);
    int ai[1] = { 0 };
    ai[0] = 0;
    int4 ai4[1] = { int4(0, 0, 0, 0) };
    ai4[0] = int4(1, 2, 3, 4);
    float3x3 ah3x3[1] = { float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx) };
    ah3x3[0] = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
    float4 af4[1] = { 0.0f.xxxx };
    af4[0].x = 0.0f;
    af4[0] = float4(1.0f.xxxx.z, 1.0f.xxxx.x, 1.0f.xxxx.w, 1.0f.xxxx.y);
    S s = { 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f.xxxx, { 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx } };
    s.f = 0.0f;
    s.af[1] = 0.0f;
    s.h4 = float4(9.0f.xxx.y, 9.0f.xxx.z, 9.0f.xxx.x, s.h4.w);
    s.ah4[2] = float4(s.ah4[2].x, 5.0f.xx.x, s.ah4[2].z, 5.0f.xx.y);
    globalVar = 0.0f.xxxx;
    globalStruct.f = 0.0f;
    float l = 0.0f;
    ai[0] += ai4[0].x;
    s.f = 1.0f;
    s.af[0] = 2.0f;
    s.h4 = 1.0f.xxxx;
    s.ah4[0] = 2.0f.xxxx;
    float repeat = 1.0f;
    repeat = 1.0f;
    float _132 = af4[0].x;
    keepAlive_vf(_132);
    af4[0].x = _132;
    float _139 = ah3x3[0][0].x;
    keepAlive_vh(_139);
    ah3x3[0][0].x = _139;
    int _143 = i;
    keepAlive_vi(_143);
    i = _143;
    int _148 = i4.y;
    keepAlive_vi(_148);
    i4.y = _148;
    int _153 = ai[0];
    keepAlive_vi(_153);
    ai[0] = _153;
    int _159 = ai4[0].x;
    keepAlive_vi(_159);
    ai4[0].x = _159;
    float _164 = x.y;
    keepAlive_vh(_164);
    x.y = _164;
    float _169 = s.f;
    keepAlive_vf(_169);
    s.f = _169;
    float _173 = l;
    keepAlive_vh(_173);
    l = _173;
    float _179 = f3x3[0].x;
    keepAlive_vf(_179);
    f3x3[0].x = _179;
    float _183 = repeat;
    keepAlive_vf(_183);
    repeat = _183;
    return _22_colorGreen;
}

void frag_main()
{
    float2 _32 = 0.0f.xx;
    float4 _34 = main(_32);
    sk_FragColor = _34;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
