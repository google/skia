               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %12
               OpName %aSampler_texture "aSampler_texture"  ; id %16
               OpName %aSampler_sampler "aSampler_sampler"  ; id %19
               OpName %anotherSampler_texture "anotherSampler_texture"  ; id %22
               OpName %anotherSampler_sampler "anotherSampler_sampler"  ; id %23
               OpName %helpers_helper_h4Z_aSampler "helpers_helper_h4Z_aSampler"    ; id %6
               OpName %helpers_helper_h4Z_anotherSampler "helpers_helper_h4Z_anotherSampler"    ; id %7
               OpName %helper_h4Z_aSampler "helper_h4Z_aSampler"                                ; id %8
               OpName %helper_h4Z_anotherSampler "helper_h4Z_anotherSampler"                    ; id %9
               OpName %helper2_h4ZZ_aSampler_anotherSampler "helper2_h4ZZ_aSampler_anotherSampler"  ; id %10
               OpName %main "main"                                                                  ; id %11

               ; Annotations
               OpDecorate %helpers_helper_h4Z_aSampler RelaxedPrecision
               OpDecorate %helpers_helper_h4Z_anotherSampler RelaxedPrecision
               OpDecorate %helper_h4Z_aSampler RelaxedPrecision
               OpDecorate %helper_h4Z_anotherSampler RelaxedPrecision
               OpDecorate %helper2_h4ZZ_aSampler_anotherSampler RelaxedPrecision
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
               OpDecorate %25 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %17 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%aSampler_texture = OpVariable %_ptr_UniformConstant_17 UniformConstant     ; Binding 2, DescriptorSet 1
         %20 = OpTypeSampler
%_ptr_UniformConstant_20 = OpTypePointer UniformConstant %20
%aSampler_sampler = OpVariable %_ptr_UniformConstant_20 UniformConstant     ; Binding 3, DescriptorSet 1
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_17 UniformConstant   ; Binding 4, DescriptorSet 1
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_20 UniformConstant   ; Binding 5, DescriptorSet 1
         %24 = OpTypeFunction %v4float %_ptr_UniformConstant_17
         %31 = OpTypeSampledImage %17
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %34 = OpConstantComposite %v2float %float_1 %float_1
         %47 = OpTypeFunction %v4float %_ptr_UniformConstant_17 %_ptr_UniformConstant_17
       %void = OpTypeVoid
         %58 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %65 = OpConstantComposite %v2float %float_0 %float_0


               ; Function helpers_helper_h4Z_aSampler
%helpers_helper_h4Z_aSampler = OpFunction %v4float None %24     ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_UniformConstant_17     ; RelaxedPrecision

         %26 = OpLabel
         %28 =   OpLoad %17 %25
         %29 =   OpLoad %20 %aSampler_sampler
         %30 =   OpSampledImage %31 %28 %29
         %27 =   OpImageSampleImplicitLod %v4float %30 %34  ; RelaxedPrecision
                 OpReturnValue %27
               OpFunctionEnd


               ; Function helpers_helper_h4Z_anotherSampler
%helpers_helper_h4Z_anotherSampler = OpFunction %v4float None %24   ; RelaxedPrecision
         %35 = OpFunctionParameter %_ptr_UniformConstant_17         ; RelaxedPrecision

         %36 = OpLabel
         %38 =   OpLoad %17 %35
         %39 =   OpLoad %20 %anotherSampler_sampler
         %40 =   OpSampledImage %31 %38 %39
         %37 =   OpImageSampleImplicitLod %v4float %40 %34  ; RelaxedPrecision
                 OpReturnValue %37
               OpFunctionEnd


               ; Function helper_h4Z_aSampler
%helper_h4Z_aSampler = OpFunction %v4float None %24     ; RelaxedPrecision
         %41 = OpFunctionParameter %_ptr_UniformConstant_17     ; RelaxedPrecision

         %42 = OpLabel
         %43 =   OpFunctionCall %v4float %helpers_helper_h4Z_aSampler %41
                 OpReturnValue %43
               OpFunctionEnd


               ; Function helper_h4Z_anotherSampler
%helper_h4Z_anotherSampler = OpFunction %v4float None %24   ; RelaxedPrecision
         %44 = OpFunctionParameter %_ptr_UniformConstant_17     ; RelaxedPrecision

         %45 = OpLabel
         %46 =   OpFunctionCall %v4float %helpers_helper_h4Z_anotherSampler %44
                 OpReturnValue %46
               OpFunctionEnd


               ; Function helper2_h4ZZ_aSampler_anotherSampler
%helper2_h4ZZ_aSampler_anotherSampler = OpFunction %v4float None %47    ; RelaxedPrecision
         %48 = OpFunctionParameter %_ptr_UniformConstant_17             ; RelaxedPrecision
         %49 = OpFunctionParameter %_ptr_UniformConstant_17             ; RelaxedPrecision

         %50 = OpLabel
         %52 =   OpLoad %17 %48
         %53 =   OpLoad %20 %aSampler_sampler
         %54 =   OpSampledImage %31 %52 %53
         %51 =   OpImageSampleImplicitLod %v4float %54 %34  ; RelaxedPrecision
         %55 =   OpFunctionCall %v4float %helper_h4Z_anotherSampler %49
         %56 =   OpFAdd %v4float %51 %55            ; RelaxedPrecision
                 OpReturnValue %56
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %58

         %59 = OpLabel
         %61 =   OpLoad %17 %aSampler_texture
         %62 =   OpLoad %20 %aSampler_sampler
         %63 =   OpSampledImage %31 %61 %62
         %60 =   OpImageSampleImplicitLod %v4float %63 %65  ; RelaxedPrecision
         %66 =   OpFunctionCall %v4float %helper_h4Z_aSampler %aSampler_texture
         %67 =   OpFAdd %v4float %60 %66            ; RelaxedPrecision
         %68 =   OpFunctionCall %v4float %helper2_h4ZZ_aSampler_anotherSampler %aSampler_texture %anotherSampler_texture
         %69 =   OpFAdd %v4float %67 %68            ; RelaxedPrecision
                 OpStore %sk_FragColor %69
                 OpReturn
               OpFunctionEnd
