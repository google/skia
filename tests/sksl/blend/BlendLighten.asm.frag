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
OpName %_0_result "_0_result"
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
OpDecorate %22 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %14
%15 = OpLabel
%_0_result = OpVariable %_ptr_Function_v4float Function
%18 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%22 = OpLoad %v4float %18
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%25 = OpLoad %v4float %24
%26 = OpCompositeExtract %float %25 3
%27 = OpFSub %float %float_1 %26
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%30 = OpLoad %v4float %28
%31 = OpVectorTimesScalar %v4float %30 %27
%32 = OpFAdd %v4float %22 %31
OpStore %_0_result %32
%34 = OpLoad %v4float %_0_result
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%38 = OpLoad %v4float %37
%39 = OpCompositeExtract %float %38 3
%40 = OpFSub %float %float_1 %39
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%42 = OpLoad %v4float %41
%43 = OpVectorShuffle %v3float %42 %42 0 1 2
%44 = OpVectorTimesScalar %v3float %43 %40
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%46 = OpLoad %v4float %45
%47 = OpVectorShuffle %v3float %46 %46 0 1 2
%48 = OpFAdd %v3float %44 %47
%33 = OpExtInst %v3float %1 FMax %35 %48
%49 = OpLoad %v4float %_0_result
%50 = OpVectorShuffle %v4float %49 %33 4 5 6 3
OpStore %_0_result %50
%51 = OpLoad %v4float %_0_result
OpStore %sk_FragColor %51
OpReturn
OpFunctionEnd
