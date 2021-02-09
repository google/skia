OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %blend_src_in "blend_src_in"
OpName %blend_dst_in "blend_dst_in"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
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
%15 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%32 = OpTypeFunction %void
%blend_src_in = OpFunction %v4float None %15
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpLabel
%20 = OpLoad %v4float %17
%21 = OpLoad %v4float %18
%22 = OpCompositeExtract %float %21 3
%23 = OpVectorTimesScalar %v4float %20 %22
OpReturnValue %23
OpFunctionEnd
%blend_dst_in = OpFunction %v4float None %15
%24 = OpFunctionParameter %_ptr_Function_v4float
%25 = OpFunctionParameter %_ptr_Function_v4float
%26 = OpLabel
%27 = OpLoad %v4float %25
%28 = OpLoad %v4float %24
%29 = OpCompositeExtract %float %28 3
%30 = OpVectorTimesScalar %v4float %27 %29
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
%38 = OpFunctionCall %v4float %blend_dst_in %35 %37
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
