OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_exclusion_h4h4h4 "blend_exclusion_h4h4h4"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %19 RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
%15 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%48 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_exclusion_h4h4h4 = OpFunction %v4float None %15
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %17
%20 = OpVectorShuffle %v3float %19 %19 0 1 2
%22 = OpLoad %v4float %16
%23 = OpVectorShuffle %v3float %22 %22 0 1 2
%24 = OpFAdd %v3float %20 %23
%26 = OpLoad %v4float %17
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%28 = OpVectorTimesScalar %v3float %27 %float_2
%29 = OpLoad %v4float %16
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%31 = OpFMul %v3float %28 %30
%32 = OpFSub %v3float %24 %31
%33 = OpCompositeExtract %float %32 0
%34 = OpCompositeExtract %float %32 1
%35 = OpCompositeExtract %float %32 2
%36 = OpLoad %v4float %16
%37 = OpCompositeExtract %float %36 3
%39 = OpLoad %v4float %16
%40 = OpCompositeExtract %float %39 3
%41 = OpFSub %float %float_1 %40
%42 = OpLoad %v4float %17
%43 = OpCompositeExtract %float %42 3
%44 = OpFMul %float %41 %43
%45 = OpFAdd %float %37 %44
%46 = OpCompositeConstruct %v4float %33 %34 %35 %45
OpReturnValue %46
OpFunctionEnd
%main = OpFunction %void None %48
%49 = OpLabel
%55 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%50 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%54 = OpLoad %v4float %50
OpStore %55 %54
%56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%58 = OpLoad %v4float %56
OpStore %59 %58
%60 = OpFunctionCall %v4float %blend_exclusion_h4h4h4 %55 %59
OpStore %sk_FragColor %60
OpReturn
OpFunctionEnd
