               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %aSampler_texture "aSampler_texture"  ; id %5
               OpName %aSampler_sampler "aSampler_sampler"  ; id %9
               OpName %anotherSampler_texture "anotherSampler_texture"  ; id %12
               OpName %anotherSampler_sampler "anotherSampler_sampler"  ; id %13
               OpName %helper_h4Z_aSampler "helper_h4Z_aSampler"        ; id %2
               OpName %helper_h4Z_anotherSampler "helper_h4Z_anotherSampler"    ; id %3
               OpName %main "main"                                              ; id %4

               ; Annotations
               OpDecorate %helper_h4Z_aSampler RelaxedPrecision
               OpDecorate %helper_h4Z_anotherSampler RelaxedPrecision
               OpDecorate %aSampler_texture Binding 2
               OpDecorate %aSampler_texture DescriptorSet 0
               OpDecorate %aSampler_sampler Binding 3
               OpDecorate %aSampler_sampler DescriptorSet 0
               OpDecorate %anotherSampler_texture Binding 3
               OpDecorate %anotherSampler_texture DescriptorSet 0
               OpDecorate %anotherSampler_sampler Binding 5
               OpDecorate %anotherSampler_sampler DescriptorSet 0
               OpDecorate %16 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
%aSampler_texture = OpVariable %_ptr_UniformConstant_7 UniformConstant  ; Binding 2, DescriptorSet 0
         %10 = OpTypeSampler
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
%aSampler_sampler = OpVariable %_ptr_UniformConstant_10 UniformConstant     ; Binding 3, DescriptorSet 0
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_7 UniformConstant    ; Binding 3, DescriptorSet 0
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_10 UniformConstant   ; Binding 5, DescriptorSet 0
    %v4float = OpTypeVector %float 4
         %15 = OpTypeFunction %v4float %_ptr_UniformConstant_7
         %22 = OpTypeSampledImage %7
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %25 = OpConstantComposite %v2float %float_1 %float_1
       %void = OpTypeVoid
         %33 = OpTypeFunction %void


               ; Function helper_h4Z_aSampler
%helper_h4Z_aSampler = OpFunction %v4float None %15     ; RelaxedPrecision
         %16 = OpFunctionParameter %_ptr_UniformConstant_7  ; RelaxedPrecision

         %17 = OpLabel
         %19 =   OpLoad %7 %16
         %20 =   OpLoad %10 %aSampler_sampler
         %21 =   OpSampledImage %22 %19 %20
         %18 =   OpImageSampleImplicitLod %v4float %21 %25  ; RelaxedPrecision
                 OpReturnValue %18
               OpFunctionEnd


               ; Function helper_h4Z_anotherSampler
%helper_h4Z_anotherSampler = OpFunction %v4float None %15   ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_UniformConstant_7  ; RelaxedPrecision

         %27 = OpLabel
         %29 =   OpLoad %7 %26
         %30 =   OpLoad %10 %anotherSampler_sampler
         %31 =   OpSampledImage %22 %29 %30
         %28 =   OpImageSampleImplicitLod %v4float %31 %25  ; RelaxedPrecision
                 OpReturnValue %28
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %33

         %34 = OpLabel
         %35 =   OpFunctionCall %v4float %helper_h4Z_aSampler %aSampler_texture
         %36 =   OpFunctionCall %v4float %helper_h4Z_anotherSampler %anotherSampler_texture
         %37 =   OpFAdd %v4float %35 %36            ; RelaxedPrecision
                 OpReturn
               OpFunctionEnd
