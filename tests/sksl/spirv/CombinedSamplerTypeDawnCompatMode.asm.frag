               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %aSampler_texture "aSampler_texture"
               OpName %aSampler_sampler "aSampler_sampler"
               OpName %anotherSampler_texture "anotherSampler_texture"
               OpName %anotherSampler_sampler "anotherSampler_sampler"
               OpName %helpers_helper_h4Z_aSampler "helpers_helper_h4Z_aSampler"
               OpName %helpers_helper_h4Z_anotherSampler "helpers_helper_h4Z_anotherSampler"
               OpName %helper_h4Z_aSampler "helper_h4Z_aSampler"
               OpName %helper_h4Z_anotherSampler "helper_h4Z_anotherSampler"
               OpName %helper2_h4ZZ_aSampler_anotherSampler "helper2_h4ZZ_aSampler_anotherSampler"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aSampler_texture Binding 2
               OpDecorate %aSampler_texture DescriptorSet 1
               OpDecorate %aSampler_sampler Binding 3
               OpDecorate %aSampler_sampler DescriptorSet 1
               OpDecorate %anotherSampler_texture Binding 4
               OpDecorate %anotherSampler_texture DescriptorSet 1
               OpDecorate %anotherSampler_sampler Binding 5
               OpDecorate %anotherSampler_sampler DescriptorSet 1
               OpDecorate %52 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
%aSampler_texture = OpVariable %_ptr_UniformConstant_13 UniformConstant
         %16 = OpTypeSampler
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%aSampler_sampler = OpVariable %_ptr_UniformConstant_16 UniformConstant
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_13 UniformConstant
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_16 UniformConstant
         %20 = OpTypeFunction %v4float %_ptr_UniformConstant_13
         %27 = OpTypeSampledImage %13
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %30 = OpConstantComposite %v2float %float_1 %float_1
         %43 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_13
       %void = OpTypeVoid
         %54 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %61 = OpConstantComposite %v2float %float_0 %float_0
%helpers_helper_h4Z_aSampler = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_UniformConstant_13
         %22 = OpLabel
         %24 = OpLoad %13 %21
         %25 = OpLoad %16 %aSampler_sampler
         %26 = OpSampledImage %27 %24 %25
         %23 = OpImageSampleImplicitLod %v4float %26 %30
               OpReturnValue %23
               OpFunctionEnd
%helpers_helper_h4Z_anotherSampler = OpFunction %v4float None %20
         %31 = OpFunctionParameter %_ptr_UniformConstant_13
         %32 = OpLabel
         %34 = OpLoad %13 %31
         %35 = OpLoad %16 %anotherSampler_sampler
         %36 = OpSampledImage %27 %34 %35
         %33 = OpImageSampleImplicitLod %v4float %36 %30
               OpReturnValue %33
               OpFunctionEnd
%helper_h4Z_aSampler = OpFunction %v4float None %20
         %37 = OpFunctionParameter %_ptr_UniformConstant_13
         %38 = OpLabel
         %39 = OpFunctionCall %v4float %helpers_helper_h4Z_aSampler %37
               OpReturnValue %39
               OpFunctionEnd
%helper_h4Z_anotherSampler = OpFunction %v4float None %20
         %40 = OpFunctionParameter %_ptr_UniformConstant_13
         %41 = OpLabel
         %42 = OpFunctionCall %v4float %helpers_helper_h4Z_anotherSampler %40
               OpReturnValue %42
               OpFunctionEnd
%helper2_h4ZZ_aSampler_anotherSampler = OpFunction %v4float None %43
         %44 = OpFunctionParameter %_ptr_UniformConstant_13
         %45 = OpFunctionParameter %_ptr_UniformConstant_13
         %46 = OpLabel
         %48 = OpLoad %13 %44
         %49 = OpLoad %16 %aSampler_sampler
         %50 = OpSampledImage %27 %48 %49
         %47 = OpImageSampleImplicitLod %v4float %50 %30
         %51 = OpFunctionCall %v4float %helper_h4Z_anotherSampler %45
         %52 = OpFAdd %v4float %47 %51
               OpReturnValue %52
               OpFunctionEnd
       %main = OpFunction %void None %54
         %55 = OpLabel
         %57 = OpLoad %13 %aSampler_texture
         %58 = OpLoad %16 %aSampler_sampler
         %59 = OpSampledImage %27 %57 %58
         %56 = OpImageSampleImplicitLod %v4float %59 %61
         %62 = OpFunctionCall %v4float %helper_h4Z_aSampler %aSampler_texture
         %63 = OpFAdd %v4float %56 %62
         %64 = OpFunctionCall %v4float %helper2_h4ZZ_aSampler_anotherSampler %aSampler_texture %anotherSampler_texture
         %65 = OpFAdd %v4float %63 %64
               OpStore %sk_FragColor %65
               OpReturn
               OpFunctionEnd
