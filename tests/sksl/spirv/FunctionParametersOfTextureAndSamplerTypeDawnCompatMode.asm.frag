               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %aTexture "aTexture"          ; id %13
               OpName %aSampledTexture_texture "aSampledTexture_texture"    ; id %16
               OpName %aSampledTexture_sampler "aSampledTexture_sampler"    ; id %17
               OpName %c "c"                                                ; id %20
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
               OpDecorate %24 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %14 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
   %aTexture = OpVariable %_ptr_UniformConstant_14 UniformConstant  ; Binding 1, DescriptorSet 0
%aSampledTexture_texture = OpVariable %_ptr_UniformConstant_14 UniformConstant  ; Binding 2, DescriptorSet 0
         %18 = OpTypeSampler
%_ptr_UniformConstant_18 = OpTypePointer UniformConstant %18
%aSampledTexture_sampler = OpVariable %_ptr_UniformConstant_18 UniformConstant  ; Binding 3, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input     ; Location 1
         %23 = OpTypeFunction %v4float %_ptr_UniformConstant_14 %_ptr_UniformConstant_14
         %31 = OpTypeSampledImage %14
       %void = OpTypeVoid
         %38 = OpTypeFunction %void


               ; Function helpers_helper_h4ZT_aSampledTexture
%helpers_helper_h4ZT_aSampledTexture = OpFunction %v4float None %23     ; RelaxedPrecision
         %24 = OpFunctionParameter %_ptr_UniformConstant_14             ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_UniformConstant_14

         %26 = OpLabel
         %28 =   OpLoad %14 %24
         %29 =   OpLoad %18 %aSampledTexture_sampler
         %30 =   OpSampledImage %31 %28 %29
         %32 =   OpLoad %v2float %c
         %27 =   OpImageSampleImplicitLod %v4float %30 %32  ; RelaxedPrecision
                 OpReturnValue %27
               OpFunctionEnd


               ; Function helper_h4TZ_aSampledTexture
%helper_h4TZ_aSampledTexture = OpFunction %v4float None %23     ; RelaxedPrecision
         %33 = OpFunctionParameter %_ptr_UniformConstant_14
         %34 = OpFunctionParameter %_ptr_UniformConstant_14     ; RelaxedPrecision

         %35 = OpLabel
         %36 =   OpFunctionCall %v4float %helpers_helper_h4ZT_aSampledTexture %34 %33
                 OpReturnValue %36
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %38

         %39 = OpLabel
         %40 =   OpFunctionCall %v4float %helper_h4TZ_aSampledTexture %aTexture %aSampledTexture_texture
                 OpStore %sk_FragColor %40
                 OpReturn
               OpFunctionEnd
