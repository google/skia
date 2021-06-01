OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "ah"
OpMemberName %_UniformBuffer 1 "bh"
OpMemberName %_UniformBuffer 2 "af"
OpMemberName %_UniformBuffer 3 "bf"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 8
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 16
OpMemberDecorate %_UniformBuffer 3 Offset 24
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%float_12 = OpConstant %float 12
%v3float = OpTypeVector %float 3
%float_n8 = OpConstant %float -8
%60 = OpConstantComposite %v3float %float_n8 %float_n8 %float_12
%float_9 = OpConstant %float 9
%float_n18 = OpConstant %float -18
%float_n9 = OpConstant %float -9
%66 = OpConstantComposite %v3float %float_9 %float_n18 %float_n9
%main = OpFunction %void None %15
%16 = OpLabel
%17 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%21 = OpLoad %v2float %17
%22 = OpCompositeExtract %float %21 0
%23 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%25 = OpLoad %v2float %23
%26 = OpCompositeExtract %float %25 1
%27 = OpFMul %float %22 %26
%28 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%29 = OpLoad %v2float %28
%30 = OpCompositeExtract %float %29 1
%31 = OpAccessChain %_ptr_Uniform_v2float %10 %int_1
%32 = OpLoad %v2float %31
%33 = OpCompositeExtract %float %32 0
%34 = OpFMul %float %30 %33
%35 = OpFSub %float %27 %34
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %35
%38 = OpAccessChain %_ptr_Uniform_v2float %10 %int_2
%40 = OpLoad %v2float %38
%41 = OpCompositeExtract %float %40 0
%42 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%44 = OpLoad %v2float %42
%45 = OpCompositeExtract %float %44 1
%46 = OpFMul %float %41 %45
%47 = OpAccessChain %_ptr_Uniform_v2float %10 %int_2
%48 = OpLoad %v2float %47
%49 = OpCompositeExtract %float %48 1
%50 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%51 = OpLoad %v2float %50
%52 = OpCompositeExtract %float %51 0
%53 = OpFMul %float %49 %52
%54 = OpFSub %float %46 %53
%55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %55 %54
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %57 %float_12
%61 = OpLoad %v4float %sk_FragColor
%62 = OpVectorShuffle %v4float %61 %60 4 5 6 3
OpStore %sk_FragColor %62
%67 = OpLoad %v4float %sk_FragColor
%68 = OpVectorShuffle %v4float %67 %66 0 4 5 6
OpStore %sk_FragColor %68
OpReturn
OpFunctionEnd
