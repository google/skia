OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_0 = OpConstant %int 0
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%20 = OpLoad %v4float %16
%21 = OpVectorShuffle %v3float %20 %20 0 1 2
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%25 = OpLoad %v4float %23
%26 = OpVectorShuffle %v3float %25 %25 0 1 2
%27 = OpFAdd %v3float %21 %26
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%30 = OpLoad %v4float %29
%31 = OpVectorShuffle %v3float %30 %30 0 1 2
%32 = OpVectorTimesScalar %v3float %31 %float_2
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %33
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%36 = OpFMul %v3float %32 %35
%37 = OpFSub %v3float %27 %36
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeExtract %float %37 2
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%42 = OpLoad %v4float %41
%43 = OpCompositeExtract %float %42 3
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 3
%48 = OpFSub %float %float_1 %47
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 3
%52 = OpFMul %float %48 %51
%53 = OpFAdd %float %43 %52
%54 = OpCompositeConstruct %v4float %38 %39 %40 %53
OpStore %sk_FragColor %54
OpReturn
OpFunctionEnd
