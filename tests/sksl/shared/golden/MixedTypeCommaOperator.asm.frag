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
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_1 %float_1
%float_1_0 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%25 = OpConstantComposite %v3float %float_1_0 %float_1_0 %float_1_0
%int_2 = OpConstant %int 2
%float_0 = OpConstant %float 0
%mat2v2float = OpTypeMatrix %v2float 2
%int_3 = OpConstant %int 3
%main = OpFunction %void None %11
%12 = OpLabel
%13 = OpConvertSToF %float %int_1
%17 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %17 %13
%20 = OpConvertSToF %float %int_1
%23 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %23 %20
%24 = OpConvertSToF %float %int_1
%28 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %28 %24
%33 = OpCompositeConstruct %v2float %float_1 %float_0
%34 = OpCompositeConstruct %v2float %float_0 %float_1
%31 = OpCompositeConstruct %mat2v2float %33 %34
%30 = OpConvertSToF %float %int_1
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_3
OpStore %36 %30
OpReturn
OpFunctionEnd
