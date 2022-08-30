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
OpName %blend_difference_h4h4h4 "blend_difference_h4h4h4"
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
%v3float = OpTypeVector %float 3
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%54 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_difference_h4h4h4 = OpFunction %v4float None %15
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%19 = OpLoad %v4float %16
%20 = OpVectorShuffle %v3float %19 %19 0 1 2
%22 = OpLoad %v4float %17
%23 = OpVectorShuffle %v3float %22 %22 0 1 2
%24 = OpFAdd %v3float %20 %23
%27 = OpLoad %v4float %16
%28 = OpVectorShuffle %v3float %27 %27 0 1 2
%29 = OpLoad %v4float %17
%30 = OpCompositeExtract %float %29 3
%31 = OpVectorTimesScalar %v3float %28 %30
%32 = OpLoad %v4float %17
%33 = OpVectorShuffle %v3float %32 %32 0 1 2
%34 = OpLoad %v4float %16
%35 = OpCompositeExtract %float %34 3
%36 = OpVectorTimesScalar %v3float %33 %35
%26 = OpExtInst %v3float %1 FMin %31 %36
%37 = OpVectorTimesScalar %v3float %26 %float_2
%38 = OpFSub %v3float %24 %37
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeExtract %float %38 2
%42 = OpLoad %v4float %16
%43 = OpCompositeExtract %float %42 3
%45 = OpLoad %v4float %16
%46 = OpCompositeExtract %float %45 3
%47 = OpFSub %float %float_1 %46
%48 = OpLoad %v4float %17
%49 = OpCompositeExtract %float %48 3
%50 = OpFMul %float %47 %49
%51 = OpFAdd %float %43 %50
%52 = OpCompositeConstruct %v4float %39 %40 %41 %51
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
%66 = OpFunctionCall %v4float %blend_difference_h4h4h4 %61 %65
OpStore %sk_FragColor %66
OpReturn
OpFunctionEnd
