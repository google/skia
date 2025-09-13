struct S
{
    float f;
    float af[5];
    float4 h4;
    float4 ah4[5];
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _23_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4 globalVar = 0.0f.xxxx;
static S globalStruct = { 0.0f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f.xxxx, { 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx } };

void assignToFunctionParameter_vif(out int _47, out float _48)
{
    _47 = 1;
    _48 = 1.0f;
}

void keepAlive_vf(float _40)
{
}

void keepAlive_vh(float _38)
{
}

void keepAlive_vi(int _44)
{
}

float4 main(float2 _53)
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
    int _121 = 0;
    float _126 = f3x3[0].x;
    assignToFunctionParameter_vif(_121, _126);
    f3x3[0].x = _126;
    float l = 0.0f;
    ai[0] += ai4[0].x;
    s.f = 1.0f;
    s.af[0] = 2.0f;
    s.h4 = 1.0f.xxxx;
    s.ah4[0] = 2.0f.xxxx;
    float repeat = 1.0f;
    repeat = 1.0f;
    float _145 = af4[0].x;
    keepAlive_vf(_145);
    af4[0].x = _145;
    float _151 = ah3x3[0][0].x;
    keepAlive_vh(_151);
    ah3x3[0][0].x = _151;
    int _155 = i;
    keepAlive_vi(_155);
    i = _155;
    int _160 = i4.y;
    keepAlive_vi(_160);
    i4.y = _160;
    int _165 = ai[0];
    keepAlive_vi(_165);
    ai[0] = _165;
    int _171 = ai4[0].x;
    keepAlive_vi(_171);
    ai4[0].x = _171;
    float _176 = x.y;
    keepAlive_vh(_176);
    x.y = _176;
    float _181 = s.f;
    keepAlive_vf(_181);
    s.f = _181;
    float _185 = l;
    keepAlive_vh(_185);
    l = _185;
    float _191 = f3x3[0].x;
    keepAlive_vf(_191);
    f3x3[0].x = _191;
    float _195 = repeat;
    keepAlive_vf(_195);
    repeat = _195;
    return _23_colorGreen;
}

void frag_main()
{
    float2 _33 = 0.0f.xx;
    float4 _35 = main(_33);
    sk_FragColor = _35;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
