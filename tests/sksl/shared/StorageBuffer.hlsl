struct SomeData
{
    float4 a;
    float2 b;
};

RWByteAddressBuffer _3 : register(u0, space0);
RWByteAddressBuffer _12 : register(u1, space0);

static float4 sk_FragColor;
static int bufferIndex;

struct SPIRV_Cross_Input
{
    nointerpolation int bufferIndex : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _30)
{
    SomeData _39 = { 0.0f.xxxx, 0.0f.xx };
    _39.a = asfloat(_3.Load4(_3.Load(0) * 32 + 16));
    _39.b = asfloat(_3.Load2(_3.Load(0) * 32 + 32));
    _12.Store4(_3.Load(0) * 32 + 0, asuint(_39.a));
    _12.Store2(_3.Load(0) * 32 + 16, asuint(_39.b));
    return asfloat(_3.Load4(bufferIndex * 32 + 16)) * asfloat(_3.Load2(bufferIndex * 32 + 32)).x;
}

void frag_main()
{
    float2 _26 = 0.0f.xx;
    float4 _28 = main(_26);
    sk_FragColor = _28;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    bufferIndex = stage_input.bufferIndex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
