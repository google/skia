               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %tex "tex"                    ; id %11
               OpName %main "main"                  ; id %6
               OpName %a "a"                        ; id %18
               OpName %b "b"                        ; id %25

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %tex RelaxedPrecision
               OpDecorate %tex Binding 0
               OpDecorate %tex DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
        %tex = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
    %v3float = OpTypeVector %float 3
         %29 = OpConstantComposite %v3float %float_0 %float_0 %float_0


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function
          %b =   OpVariable %_ptr_Function_v4float Function
         %21 =   OpLoad %13 %tex                    ; RelaxedPrecision
         %20 =   OpImageSampleImplicitLod %v4float %21 %24  ; RelaxedPrecision
                 OpStore %a %20
         %27 =   OpLoad %13 %tex                    ; RelaxedPrecision
         %26 =   OpImageSampleProjImplicitLod %v4float %27 %29  ; RelaxedPrecision
                 OpStore %b %26
         %30 =   OpVectorShuffle %v2float %20 %20 0 1
         %31 =   OpCompositeExtract %float %30 0    ; RelaxedPrecision
         %32 =   OpCompositeExtract %float %30 1    ; RelaxedPrecision
         %33 =   OpVectorShuffle %v2float %26 %26 2 3
         %34 =   OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %33 1    ; RelaxedPrecision
         %36 =   OpCompositeConstruct %v4float %31 %32 %34 %35  ; RelaxedPrecision
                 OpStore %sk_FragColor %36
                 OpReturn
               OpFunctionEnd
