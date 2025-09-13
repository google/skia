               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %aSampler_texture "aSampler_texture"  ; id %9
               OpName %aSampler_sampler "aSampler_sampler"  ; id %13
               OpName %anotherSampler_texture "anotherSampler_texture"  ; id %16
               OpName %anotherSampler_sampler "anotherSampler_sampler"  ; id %17
               OpName %helper_h4Z_aSampler "helper_h4Z_aSampler"        ; id %6
               OpName %helper_h4Z_anotherSampler "helper_h4Z_anotherSampler"    ; id %7
               OpName %main "main"                                              ; id %8

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
               OpDecorate %20 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
%aSampler_texture = OpVariable %_ptr_UniformConstant_11 UniformConstant     ; Binding 2, DescriptorSet 0
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%aSampler_sampler = OpVariable %_ptr_UniformConstant_14 UniformConstant     ; Binding 3, DescriptorSet 0
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_11 UniformConstant   ; Binding 3, DescriptorSet 0
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_14 UniformConstant   ; Binding 5, DescriptorSet 0
    %v4float = OpTypeVector %float 4
         %19 = OpTypeFunction %v4float %_ptr_UniformConstant_11
         %26 = OpTypeSampledImage %11
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %29 = OpConstantComposite %v2float %float_1 %float_1
       %void = OpTypeVoid
         %37 = OpTypeFunction %void


               ; Function helper_h4Z_aSampler
%helper_h4Z_aSampler = OpFunction %v4float None %19     ; RelaxedPrecision
         %20 = OpFunctionParameter %_ptr_UniformConstant_11     ; RelaxedPrecision

         %21 = OpLabel
         %23 =   OpLoad %11 %20
         %24 =   OpLoad %14 %aSampler_sampler
         %25 =   OpSampledImage %26 %23 %24
         %22 =   OpImageSampleImplicitLod %v4float %25 %29  ; RelaxedPrecision
                 OpReturnValue %22
               OpFunctionEnd


               ; Function helper_h4Z_anotherSampler
%helper_h4Z_anotherSampler = OpFunction %v4float None %19   ; RelaxedPrecision
         %30 = OpFunctionParameter %_ptr_UniformConstant_11     ; RelaxedPrecision

         %31 = OpLabel
         %33 =   OpLoad %11 %30
         %34 =   OpLoad %14 %anotherSampler_sampler
         %35 =   OpSampledImage %26 %33 %34
         %32 =   OpImageSampleImplicitLod %v4float %35 %29  ; RelaxedPrecision
                 OpReturnValue %32
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %37

         %38 = OpLabel
         %39 =   OpFunctionCall %v4float %helper_h4Z_aSampler %aSampler_texture
         %40 =   OpFunctionCall %v4float %helper_h4Z_anotherSampler %anotherSampler_texture
         %41 =   OpFAdd %v4float %39 %40            ; RelaxedPrecision
                 OpReturn
               OpFunctionEnd
