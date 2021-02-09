OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_xor "blend_xor"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%32 = OpTypeFunction %void
%blend_xor = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%20 = OpLoad %v4float %17
%21 = OpCompositeExtract %float %20 3
%22 = OpFSub %float %float_1 %21
%23 = OpLoad %v4float %16
%24 = OpVectorTimesScalar %v4float %23 %22
%25 = OpLoad %v4float %16
%26 = OpCompositeExtract %float %25 3
%27 = OpFSub %float %float_1 %26
%28 = OpLoad %v4float %17
%29 = OpVectorTimesScalar %v4float %28 %27
%30 = OpFAdd %v4float %24 %29
OpReturnValue %30
OpFunctionEnd
%main = OpFunction %void None %32
%33 = OpLabel
%35 = OpVariable %_ptr_Function_v4float Function
%37 = OpVariable %_ptr_Function_v4float Function
%34 = OpLoad %v4float %src
OpStore %35 %34
%36 = OpLoad %v4float %dst
OpStore %37 %36
%38 = OpFunctionCall %v4float %blend_xor %35 %37
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
