OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %tex "tex"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %tex RelaxedPrecision
OpDecorate %tex Binding 0
OpDecorate %tex DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%tex = OpVariable %_ptr_UniformConstant_12 UniformConstant
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%v3float = OpTypeVector %float 3
%33 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%main = OpFunction %void None %15
%16 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%20 = OpLoad %12 %tex
%19 = OpImageSampleImplicitLod %v4float %20 %23
%24 = OpCompositeExtract %float %19 0
%25 = OpCompositeExtract %float %19 1
%26 = OpCompositeExtract %float %19 2
%27 = OpCompositeExtract %float %19 3
%28 = OpCompositeConstruct %v4float %24 %25 %26 %27
OpStore %a %28
%31 = OpLoad %12 %tex
%30 = OpImageSampleProjImplicitLod %v4float %31 %33
%34 = OpCompositeExtract %float %30 0
%35 = OpCompositeExtract %float %30 1
%36 = OpCompositeExtract %float %30 2
%37 = OpCompositeExtract %float %30 3
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %b %38
%39 = OpLoad %v4float %a
%40 = OpVectorShuffle %v2float %39 %39 0 1
%41 = OpCompositeExtract %float %40 0
%42 = OpCompositeExtract %float %40 1
%43 = OpCompositeConstruct %v2float %41 %42
%44 = OpCompositeExtract %float %43 0
%45 = OpCompositeExtract %float %43 1
%46 = OpLoad %v4float %b
%47 = OpVectorShuffle %v2float %46 %46 2 3
%48 = OpCompositeExtract %float %47 0
%49 = OpCompositeExtract %float %47 1
%50 = OpCompositeConstruct %v2float %48 %49
%51 = OpCompositeExtract %float %50 0
%52 = OpCompositeExtract %float %50 1
%53 = OpCompositeConstruct %v4float %44 %45 %51 %52
OpStore %sk_FragColor %53
OpReturn
OpFunctionEnd
