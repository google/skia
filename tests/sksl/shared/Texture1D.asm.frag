### Compilation failed:

error: SPIR-V validation error: Operand 3 of TypeImage requires one of these capabilities: Sampled1D Image1D 
  %13 = OpTypeImage %float 1D 0 0 0 1 Unknown

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
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 1D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%tex = OpVariable %_ptr_UniformConstant_12 UniformConstant
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%31 = OpConstantComposite %v2float %float_0 %float_0
%main = OpFunction %void None %15
%16 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%20 = OpLoad %12 %tex
%19 = OpImageSampleImplicitLod %v4float %20 %float_0
%22 = OpCompositeExtract %float %19 0
%23 = OpCompositeExtract %float %19 1
%24 = OpCompositeExtract %float %19 2
%25 = OpCompositeExtract %float %19 3
%26 = OpCompositeConstruct %v4float %22 %23 %24 %25
OpStore %a %26
%29 = OpLoad %12 %tex
%28 = OpImageSampleProjImplicitLod %v4float %29 %31
%32 = OpCompositeExtract %float %28 0
%33 = OpCompositeExtract %float %28 1
%34 = OpCompositeExtract %float %28 2
%35 = OpCompositeExtract %float %28 3
%36 = OpCompositeConstruct %v4float %32 %33 %34 %35
OpStore %b %36
%37 = OpLoad %v4float %a
%38 = OpVectorShuffle %v2float %37 %37 0 1
%39 = OpCompositeExtract %float %38 0
%40 = OpCompositeExtract %float %38 1
%41 = OpCompositeConstruct %v2float %39 %40
%42 = OpCompositeExtract %float %41 0
%43 = OpCompositeExtract %float %41 1
%44 = OpLoad %v4float %b
%45 = OpVectorShuffle %v2float %44 %44 2 3
%46 = OpCompositeExtract %float %45 0
%47 = OpCompositeExtract %float %45 1
%48 = OpCompositeConstruct %v2float %46 %47
%49 = OpCompositeExtract %float %48 0
%50 = OpCompositeExtract %float %48 1
%51 = OpCompositeConstruct %v4float %42 %43 %49 %50
OpStore %sk_FragColor %51
OpReturn
OpFunctionEnd

1 error
