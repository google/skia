OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_src_atop "blend_src_atop"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %19 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
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
%31 = OpTypeFunction %void
%blend_src_atop = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %17
%20 = OpCompositeExtract %float %19 3
%21 = OpLoad %v4float %16
%22 = OpVectorTimesScalar %v4float %21 %20
%24 = OpLoad %v4float %16
%25 = OpCompositeExtract %float %24 3
%26 = OpFSub %float %float_1 %25
%27 = OpLoad %v4float %17
%28 = OpVectorTimesScalar %v4float %27 %26
%29 = OpFAdd %v4float %22 %28
OpReturnValue %29
OpFunctionEnd
%main = OpFunction %void None %31
%32 = OpLabel
%34 = OpVariable %_ptr_Function_v4float Function
%36 = OpVariable %_ptr_Function_v4float Function
%33 = OpLoad %v4float %src
OpStore %34 %33
%35 = OpLoad %v4float %dst
OpStore %36 %35
%37 = OpFunctionCall %v4float %blend_src_atop %34 %36
OpStore %sk_FragColor %37
OpReturn
OpFunctionEnd
