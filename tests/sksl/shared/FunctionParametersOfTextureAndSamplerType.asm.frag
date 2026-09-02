               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %aTexture "aTexture"          ; id %15
               OpName %aSampledTexture "aSampledTexture"    ; id %18
               OpName %aSecondSampledTexture "aSecondSampledTexture"    ; id %21
               OpName %c "c"                                            ; id %22
               OpName %helpers_helper_h4ZT_aSampledTexture "helpers_helper_h4ZT_aSampledTexture"    ; id %6
               OpName %helpers_helper_h4ZT_aSecondSampledTexture "helpers_helper_h4ZT_aSecondSampledTexture"    ; id %7
               OpName %helper_h4TZ_aSampledTexture "helper_h4TZ_aSampledTexture"                                ; id %8
               OpName %helper_h4TZ_aSecondSampledTexture "helper_h4TZ_aSecondSampledTexture"                    ; id %9
               OpName %main "main"                                                                              ; id %10

               ; Annotations
               OpDecorate %helpers_helper_h4ZT_aSampledTexture RelaxedPrecision
               OpDecorate %helpers_helper_h4ZT_aSecondSampledTexture RelaxedPrecision
               OpDecorate %helper_h4TZ_aSampledTexture RelaxedPrecision
               OpDecorate %helper_h4TZ_aSecondSampledTexture RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aTexture Binding 1
               OpDecorate %aTexture DescriptorSet 0
               OpDecorate %aSampledTexture RelaxedPrecision
               OpDecorate %aSampledTexture Binding 2
               OpDecorate %aSampledTexture DescriptorSet 0
               OpDecorate %aSecondSampledTexture RelaxedPrecision
               OpDecorate %aSecondSampledTexture Binding 3
               OpDecorate %aSecondSampledTexture DescriptorSet 0
               OpDecorate %c Location 1
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
   %aTexture = OpVariable %_ptr_UniformConstant_16 UniformConstant  ; Binding 1, DescriptorSet 0
         %19 = OpTypeSampledImage %16
%_ptr_UniformConstant_19 = OpTypePointer UniformConstant %19
%aSampledTexture = OpVariable %_ptr_UniformConstant_19 UniformConstant  ; RelaxedPrecision, Binding 2, DescriptorSet 0
%aSecondSampledTexture = OpVariable %_ptr_UniformConstant_19 UniformConstant    ; RelaxedPrecision, Binding 3, DescriptorSet 0
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input     ; Location 1
         %25 = OpTypeFunction %v4float %_ptr_UniformConstant_16
       %void = OpTypeVoid
         %43 = OpTypeFunction %void


               ; Function helpers_helper_h4ZT_aSampledTexture
%helpers_helper_h4ZT_aSampledTexture = OpFunction %v4float None %25     ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_UniformConstant_16

         %27 = OpLabel
         %29 =   OpLoad %19 %aSampledTexture        ; RelaxedPrecision
         %30 =   OpLoad %v2float %c
         %28 =   OpImageSampleImplicitLod %v4float %29 %30  ; RelaxedPrecision
                 OpReturnValue %28
               OpFunctionEnd


               ; Function helpers_helper_h4ZT_aSecondSampledTexture
%helpers_helper_h4ZT_aSecondSampledTexture = OpFunction %v4float None %25   ; RelaxedPrecision
         %31 = OpFunctionParameter %_ptr_UniformConstant_16

         %32 = OpLabel
         %34 =   OpLoad %19 %aSecondSampledTexture  ; RelaxedPrecision
         %35 =   OpLoad %v2float %c
         %33 =   OpImageSampleImplicitLod %v4float %34 %35  ; RelaxedPrecision
                 OpReturnValue %33
               OpFunctionEnd


               ; Function helper_h4TZ_aSampledTexture
%helper_h4TZ_aSampledTexture = OpFunction %v4float None %25     ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_UniformConstant_16

         %37 = OpLabel
         %38 =   OpFunctionCall %v4float %helpers_helper_h4ZT_aSampledTexture %36
                 OpReturnValue %38
               OpFunctionEnd


               ; Function helper_h4TZ_aSecondSampledTexture
%helper_h4TZ_aSecondSampledTexture = OpFunction %v4float None %25   ; RelaxedPrecision
         %39 = OpFunctionParameter %_ptr_UniformConstant_16

         %40 = OpLabel
         %41 =   OpFunctionCall %v4float %helpers_helper_h4ZT_aSecondSampledTexture %39
                 OpReturnValue %41
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %43

         %44 = OpLabel
         %45 =   OpFunctionCall %v4float %helper_h4TZ_aSampledTexture %aTexture
                 OpStore %sk_FragColor %45
         %46 =   OpFunctionCall %v4float %helper_h4TZ_aSecondSampledTexture %aTexture
                 OpStore %sk_FragColor %46
         %47 =   OpFunctionCall %v4float %helper_h4TZ_aSampledTexture %aTexture
                 OpStore %sk_FragColor %47
                 OpReturn
               OpFunctionEnd
