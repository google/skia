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
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%17 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%21 = OpLoad %v4float %17
%22 = OpCompositeExtract %float %21 3
%23 = OpFSub %float %float_1 %22
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%26 = OpLoad %v4float %24
%27 = OpVectorShuffle %v3float %26 %26 0 1 2
%29 = OpVectorTimesScalar %v3float %27 %23
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%31 = OpLoad %v4float %30
%32 = OpCompositeExtract %float %31 3
%33 = OpFSub %float %float_1 %32
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %34
%36 = OpVectorShuffle %v3float %35 %35 0 1 2
%37 = OpVectorTimesScalar %v3float %36 %33
%38 = OpFAdd %v3float %29 %37
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %39
%41 = OpVectorShuffle %v3float %40 %40 0 1 2
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %42
%44 = OpVectorShuffle %v3float %43 %43 0 1 2
%45 = OpFMul %v3float %41 %44
%46 = OpFAdd %v3float %38 %45
%47 = OpCompositeExtract %float %46 0
%48 = OpCompositeExtract %float %46 1
%49 = OpCompositeExtract %float %46 2
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpCompositeExtract %float %51 3
%53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%54 = OpLoad %v4float %53
%55 = OpCompositeExtract %float %54 3
%56 = OpFSub %float %float_1 %55
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%58 = OpLoad %v4float %57
%59 = OpCompositeExtract %float %58 3
%60 = OpFMul %float %56 %59
%61 = OpFAdd %float %52 %60
%62 = OpCompositeConstruct %v4float %47 %48 %49 %61
OpStore %sk_FragColor %62
OpReturn
OpFunctionEnd
