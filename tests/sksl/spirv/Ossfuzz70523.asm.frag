               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %aSampler_texture "aSampler_texture"
               OpName %aSampler_sampler "aSampler_sampler"
               OpName %anotherSampler_texture "anotherSampler_texture"
               OpName %anotherSampler_sampler "anotherSampler_sampler"
               OpName %helper_h4Z_aSampler "helper_h4Z_aSampler"
               OpName %helper_h4Z_anotherSampler "helper_h4Z_anotherSampler"
               OpName %main "main"
               OpDecorate %aSampler_texture Binding 2
               OpDecorate %aSampler_texture DescriptorSet 0
               OpDecorate %aSampler_sampler Binding 3
               OpDecorate %aSampler_sampler DescriptorSet 0
               OpDecorate %anotherSampler_texture Binding 3
               OpDecorate %anotherSampler_texture DescriptorSet 0
               OpDecorate %anotherSampler_sampler Binding 5
               OpDecorate %anotherSampler_sampler DescriptorSet 0
               OpDecorate %37 RelaxedPrecision
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
%aSampler_texture = OpVariable %_ptr_UniformConstant_7 UniformConstant
         %10 = OpTypeSampler
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
%aSampler_sampler = OpVariable %_ptr_UniformConstant_10 UniformConstant
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_7 UniformConstant
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_10 UniformConstant
    %v4float = OpTypeVector %float 4
         %15 = OpTypeFunction %v4float %_ptr_UniformConstant_7
         %22 = OpTypeSampledImage %7
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %25 = OpConstantComposite %v2float %float_1 %float_1
       %void = OpTypeVoid
         %33 = OpTypeFunction %void
%helper_h4Z_aSampler = OpFunction %v4float None %15
         %16 = OpFunctionParameter %_ptr_UniformConstant_7
         %17 = OpLabel
         %19 = OpLoad %7 %16
         %20 = OpLoad %10 %aSampler_sampler
         %21 = OpSampledImage %22 %19 %20
         %18 = OpImageSampleImplicitLod %v4float %21 %25
               OpReturnValue %18
               OpFunctionEnd
%helper_h4Z_anotherSampler = OpFunction %v4float None %15
         %26 = OpFunctionParameter %_ptr_UniformConstant_7
         %27 = OpLabel
         %29 = OpLoad %7 %26
         %30 = OpLoad %10 %anotherSampler_sampler
         %31 = OpSampledImage %22 %29 %30
         %28 = OpImageSampleImplicitLod %v4float %31 %25
               OpReturnValue %28
               OpFunctionEnd
       %main = OpFunction %void None %33
         %34 = OpLabel
         %35 = OpFunctionCall %v4float %helper_h4Z_aSampler %aSampler_texture
         %36 = OpFunctionCall %v4float %helper_h4Z_anotherSampler %anotherSampler_texture
         %37 = OpFAdd %v4float %35 %36
               OpReturn
               OpFunctionEnd
