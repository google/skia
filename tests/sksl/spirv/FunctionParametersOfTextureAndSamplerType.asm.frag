OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %c
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %s "s"
OpName %t "t"
OpName %cs "cs"
OpName %c "c"
OpName %combined_sampler_only_helper_h4Z "combined_sampler_only_helper_h4Z"
OpName %bottom_helper_h4Tss "bottom_helper_h4Tss"
OpName %helpers_helper_h4Tss "helpers_helper_h4Tss"
OpName %color "color"
OpName %helper_h4Tss "helper_h4Tss"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %s Binding 0
OpDecorate %s DescriptorSet 0
OpDecorate %t Binding 1
OpDecorate %t DescriptorSet 0
OpDecorate %cs RelaxedPrecision
OpDecorate %cs Binding 2
OpDecorate %cs DescriptorSet 0
OpDecorate %c Location 1
OpDecorate %30 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %color RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%15 = OpTypeSampler
%_ptr_UniformConstant_15 = OpTypePointer UniformConstant %15
%s = OpVariable %_ptr_UniformConstant_15 UniformConstant
%18 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%t = OpVariable %_ptr_UniformConstant_18 UniformConstant
%21 = OpTypeSampledImage %18
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
%cs = OpVariable %_ptr_UniformConstant_21 UniformConstant
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%c = OpVariable %_ptr_Input_v2float Input
%26 = OpTypeFunction %v4float %_ptr_UniformConstant_21
%32 = OpTypeFunction %v4float %_ptr_UniformConstant_18 %_ptr_UniformConstant_15
%_ptr_Function_v4float = OpTypePointer Function %v4float
%void = OpTypeVoid
%59 = OpTypeFunction %void
%combined_sampler_only_helper_h4Z = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_UniformConstant_21
%28 = OpLabel
%30 = OpLoad %21 %27
%31 = OpLoad %v2float %c
%29 = OpImageSampleImplicitLod %v4float %30 %31
OpReturnValue %29
OpFunctionEnd
%bottom_helper_h4Tss = OpFunction %v4float None %32
%33 = OpFunctionParameter %_ptr_UniformConstant_18
%34 = OpFunctionParameter %_ptr_UniformConstant_15
%35 = OpLabel
%38 = OpLoad %18 %33
%39 = OpLoad %15 %34
%37 = OpSampledImage %21 %38 %39
%40 = OpLoad %v2float %c
%36 = OpImageSampleImplicitLod %v4float %37 %40
OpReturnValue %36
OpFunctionEnd
%helpers_helper_h4Tss = OpFunction %v4float None %32
%41 = OpFunctionParameter %_ptr_UniformConstant_18
%42 = OpFunctionParameter %_ptr_UniformConstant_15
%43 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%48 = OpLoad %18 %41
%49 = OpLoad %15 %42
%47 = OpSampledImage %21 %48 %49
%50 = OpLoad %v2float %c
%46 = OpImageSampleImplicitLod %v4float %47 %50
OpStore %color %46
%51 = OpVectorShuffle %v4float %46 %46 2 1 0 3
%52 = OpFunctionCall %v4float %bottom_helper_h4Tss %41 %42
%53 = OpFMul %v4float %51 %52
OpReturnValue %53
OpFunctionEnd
%helper_h4Tss = OpFunction %v4float None %32
%54 = OpFunctionParameter %_ptr_UniformConstant_18
%55 = OpFunctionParameter %_ptr_UniformConstant_15
%56 = OpLabel
%57 = OpFunctionCall %v4float %helpers_helper_h4Tss %54 %55
OpReturnValue %57
OpFunctionEnd
%main = OpFunction %void None %59
%60 = OpLabel
%61 = OpFunctionCall %v4float %helper_h4Tss %t %s
%62 = OpFunctionCall %v4float %combined_sampler_only_helper_h4Z %cs
%63 = OpFAdd %v4float %61 %62
OpStore %sk_FragColor %63
OpReturn
OpFunctionEnd
