struct S
{
    float f;
    float af[5];
    float4 h4;
    float4 ah4[5];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _20_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4 globalVar = 0.0f.xxxx;
static S globalStruct = { 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f.xxxx, { 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx } };

void assignToFunctionParameter_vif(out int _44, out float _45)
{
    _44 = 1;
    _45 = 1.0f;
}

void keepAlive_vf(float _37)
{
}

void keepAlive_vh(float _35)
{
}

void keepAlive_vi(int _41)
{
}

float4 main(float2 _50)
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
    int _118 = 0;
    float _123 = f3x3[0].x;
    assignToFunctionParameter_vif(_118, _123);
    f3x3[0].x = _123;
    float l = 0.0f;
    ai[0] += ai4[0].x;
    s.f = 1.0f;
    s.af[0] = 2.0f;
    s.h4 = 1.0f.xxxx;
    s.ah4[0] = 2.0f.xxxx;
    float repeat = 1.0f;
    repeat = 1.0f;
    float _142 = af4[0].x;
    keepAlive_vf(_142);
    af4[0].x = _142;
    float _148 = ah3x3[0][0].x;
    keepAlive_vh(_148);
    ah3x3[0][0].x = _148;
    int _152 = i;
    keepAlive_vi(_152);
    i = _152;
    int _157 = i4.y;
    keepAlive_vi(_157);
    i4.y = _157;
    int _162 = ai[0];
    keepAlive_vi(_162);
    ai[0] = _162;
    int _168 = ai4[0].x;
    keepAlive_vi(_168);
    ai4[0].x = _168;
    float _173 = x.y;
    keepAlive_vh(_173);
    x.y = _173;
    float _178 = s.f;
    keepAlive_vf(_178);
    s.f = _178;
    float _182 = l;
    keepAlive_vh(_182);
    l = _182;
    float _188 = f3x3[0].x;
    keepAlive_vf(_188);
    f3x3[0].x = _188;
    float _192 = repeat;
    keepAlive_vf(_192);
    repeat = _192;
    return _20_colorGreen;
}

void frag_main()
{
    float2 _30 = 0.0f.xx;
    float4 _32 = main(_30);
    sk_FragColor = _32;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
