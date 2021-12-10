struct S
{
    float f;
    float af[5];
    float4 h4;
    float4 ah4[5];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _19_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4 globalVar = 0.0f.xxxx;
static S globalStruct = { 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f.xxxx, { 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx } };

float4 main(float2 _33)
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
    float3x3 ah2x4[1] = { float3x3(0.0f.xxx, 0.0f.xxx, 0.0f.xxx) };
    ah2x4[0] = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f), float3(7.0f, 8.0f, 9.0f));
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
    af4[0] *= ah2x4[0][0].x;
    i4.y *= i;
    x.y *= l;
    s.f *= l;
    return _19_colorGreen;
}

void frag_main()
{
    float2 _29 = 0.0f.xx;
    float4 _31 = main(_29);
    sk_FragColor = _31;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
