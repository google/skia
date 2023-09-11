static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int zero = 0;
static float globalArray[2] = { 0.0f, 0.0f };
static float2x2 globalMatrix = float2x2(0.0f.xx, 0.0f.xx);

float4 main(float2 _33)
{
    zero = 0;
    float _16[2] = { 1.0f, 1.0f };
    globalArray = _16;
    globalMatrix = float2x2(1.0f.xx, 1.0f.xx);
    float _37[2] = { 0.0f, 1.0f };
    float localArray[2] = _37;
    float2x2 localMatrix = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
    return float4(globalArray[zero] * localArray[zero], 1.0f.xx[zero] * 1.0f.xx[zero], globalMatrix[zero] * localMatrix[zero]);
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
