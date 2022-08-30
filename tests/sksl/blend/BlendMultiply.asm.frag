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
OpName %blend_multiply_h4h4h4 "blend_multiply_h4h4h4"
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
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
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
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%54 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_multiply_h4h4h4 = OpFunction %v4float None %15
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%20 = OpLoad %v4float %16
%21 = OpCompositeExtract %float %20 3
%22 = OpFSub %float %float_1 %21
%23 = OpLoad %v4float %17
%24 = OpVectorShuffle %v3float %23 %23 0 1 2
%26 = OpVectorTimesScalar %v3float %24 %22
%27 = OpLoad %v4float %17
%28 = OpCompositeExtract %float %27 3
%29 = OpFSub %float %float_1 %28
%30 = OpLoad %v4float %16
%31 = OpVectorShuffle %v3float %30 %30 0 1 2
%32 = OpVectorTimesScalar %v3float %31 %29
%33 = OpFAdd %v3float %26 %32
%34 = OpLoad %v4float %16
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%36 = OpLoad %v4float %17
%37 = OpVectorShuffle %v3float %36 %36 0 1 2
%38 = OpFMul %v3float %35 %37
%39 = OpFAdd %v3float %33 %38
%40 = OpCompositeExtract %float %39 0
%41 = OpCompositeExtract %float %39 1
%42 = OpCompositeExtract %float %39 2
%43 = OpLoad %v4float %16
%44 = OpCompositeExtract %float %43 3
%45 = OpLoad %v4float %16
%46 = OpCompositeExtract %float %45 3
%47 = OpFSub %float %float_1 %46
%48 = OpLoad %v4float %17
%49 = OpCompositeExtract %float %48 3
%50 = OpFMul %float %47 %49
%51 = OpFAdd %float %44 %50
%52 = OpCompositeConstruct %v4float %40 %41 %42 %51
OpReturnValue %52
OpFunctionEnd
%main = OpFunction %void None %54
%55 = OpLabel
%61 = OpVariable %_ptr_Function_v4float Function
%65 = OpVariable %_ptr_Function_v4float Function
%56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%60 = OpLoad %v4float %56
OpStore %61 %60
%62 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%64 = OpLoad %v4float %62
OpStore %65 %64
%66 = OpFunctionCall %v4float %blend_multiply_h4h4h4 %61 %65
OpStore %sk_FragColor %66
OpReturn
OpFunctionEnd
