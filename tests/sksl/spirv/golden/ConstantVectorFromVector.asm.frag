OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%15 = OpConstantComposite %v2float %float_0 %float_0
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %11
%12 = OpLabel
%18 = OpCompositeExtract %float %15 0
%19 = OpConvertFToS %int %18
%21 = OpCompositeExtract %float %15 1
%22 = OpConvertFToS %int %21
%23 = OpCompositeConstruct %v2int %19 %22
%14 = OpExtInst %v2int %1 SAbs %23
%25 = OpCompositeExtract %int %14 0
%13 = OpConvertSToF %float %25
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %26 %13
OpReturn
OpFunctionEnd
