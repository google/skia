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
OpDecorate %31 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
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
%int_0 = OpConstant %int 0
%v3float = OpTypeVector %float 3
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%main = OpFunction %void None %14
%15 = OpLabel
%16 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%20 = OpLoad %v4float %16
%21 = OpVectorShuffle %v3float %20 %20 0 1 2
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%25 = OpLoad %v4float %23
%26 = OpVectorShuffle %v3float %25 %25 0 1 2
%27 = OpFAdd %v3float %21 %26
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%31 = OpLoad %v4float %30
%32 = OpVectorShuffle %v3float %31 %31 0 1 2
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%34 = OpLoad %v4float %33
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %32 %35
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%38 = OpLoad %v4float %37
%39 = OpVectorShuffle %v3float %38 %38 0 1 2
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %40
%42 = OpCompositeExtract %float %41 3
%43 = OpVectorTimesScalar %v3float %39 %42
%29 = OpExtInst %v3float %1 FMin %36 %43
%44 = OpVectorTimesScalar %v3float %29 %float_2
%45 = OpFSub %v3float %27 %44
%46 = OpCompositeExtract %float %45 0
%47 = OpCompositeExtract %float %45 1
%48 = OpCompositeExtract %float %45 2
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 3
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpCompositeExtract %float %54 3
%56 = OpFSub %float %float_1 %55
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%58 = OpLoad %v4float %57
%59 = OpCompositeExtract %float %58 3
%60 = OpFMul %float %56 %59
%61 = OpFAdd %float %51 %60
%62 = OpCompositeConstruct %v4float %46 %47 %48 %61
OpStore %sk_FragColor %62
OpReturn
OpFunctionEnd
