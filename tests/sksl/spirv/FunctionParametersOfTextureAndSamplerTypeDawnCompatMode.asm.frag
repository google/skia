               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %aTexture "aTexture"          ; id %13
               OpName %aSampledTexture_texture "aSampledTexture_texture"    ; id %16
               OpName %aSampledTexture_sampler "aSampledTexture_sampler"    ; id %19
               OpName %c "c"                                                ; id %22
               OpName %helpers_helper_h4ZT_aSampledTexture "helpers_helper_h4ZT_aSampledTexture"    ; id %6
               OpName %helper_h4TZ_aSampledTexture "helper_h4TZ_aSampledTexture"                    ; id %7
               OpName %main "main"                                                                  ; id %8

               ; Annotations
               OpDecorate %helpers_helper_h4ZT_aSampledTexture RelaxedPrecision
               OpDecorate %helper_h4TZ_aSampledTexture RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aTexture Binding 1
               OpDecorate %aTexture DescriptorSet 0
               OpDecorate %aSampledTexture_texture Binding 2
               OpDecorate %aSampledTexture_texture DescriptorSet 0
               OpDecorate %aSampledTexture_sampler Binding 3
               OpDecorate %aSampledTexture_sampler DescriptorSet 0
               OpDecorate %c Location 1
               OpDecorate %26 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %14 = OpTypeImage %float 2D 0 0 0 2 R32f
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
   %aTexture = OpVariable %_ptr_UniformConstant_14 UniformConstant  ; Binding 1, DescriptorSet 0
         %17 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%aSampledTexture_texture = OpVariable %_ptr_UniformConstant_17 UniformConstant  ; Binding 2, DescriptorSet 0
         %20 = OpTypeSampler
%_ptr_UniformConstant_20 = OpTypePointer UniformConstant %20
%aSampledTexture_sampler = OpVariable %_ptr_UniformConstant_20 UniformConstant  ; Binding 3, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input     ; Location 1
         %25 = OpTypeFunction %v4float %_ptr_UniformConstant_17 %_ptr_UniformConstant_14
         %33 = OpTypeSampledImage %17
         %35 = OpTypeFunction %v4float %_ptr_UniformConstant_14 %_ptr_UniformConstant_17
       %void = OpTypeVoid
         %41 = OpTypeFunction %void


               ; Function helpers_helper_h4ZT_aSampledTexture
%helpers_helper_h4ZT_aSampledTexture = OpFunction %v4float None %25     ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_UniformConstant_17             ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_UniformConstant_14

         %28 = OpLabel
         %30 =   OpLoad %17 %26
         %31 =   OpLoad %20 %aSampledTexture_sampler
         %32 =   OpSampledImage %33 %30 %31
         %34 =   OpLoad %v2float %c
         %29 =   OpImageSampleImplicitLod %v4float %32 %34  ; RelaxedPrecision
                 OpReturnValue %29
               OpFunctionEnd


               ; Function helper_h4TZ_aSampledTexture
%helper_h4TZ_aSampledTexture = OpFunction %v4float None %35     ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_UniformConstant_14
         %37 = OpFunctionParameter %_ptr_UniformConstant_17     ; RelaxedPrecision

         %38 = OpLabel
         %39 =   OpFunctionCall %v4float %helpers_helper_h4ZT_aSampledTexture %37 %36
                 OpReturnValue %39
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %41

         %42 = OpLabel
         %43 =   OpFunctionCall %v4float %helper_h4TZ_aSampledTexture %aTexture %aSampledTexture_texture
                 OpStore %sk_FragColor %43
                 OpReturn
               OpFunctionEnd
