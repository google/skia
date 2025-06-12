               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %aSampler "aSampler"          ; id %15
               OpName %aSecondSampler "aSecondSampler"  ; id %19
               OpName %aThirdSampler "aThirdSampler"    ; id %20
               OpName %baz_h4Z_aSampler "baz_h4Z_aSampler"  ; id %2
               OpName %baz_h4Z_aSecondSampler "baz_h4Z_aSecondSampler"  ; id %3
               OpName %baz_h4Z_aThirdSampler "baz_h4Z_aThirdSampler"    ; id %4
               OpName %bar_h4Z_aSampler "bar_h4Z_aSampler"              ; id %5
               OpName %bar_h4Z_aThirdSampler "bar_h4Z_aThirdSampler"    ; id %6
               OpName %bar_h4Z_aSecondSampler "bar_h4Z_aSecondSampler"  ; id %7
               OpName %foo_h4ZZ_aSampler_aSecondSampler "foo_h4ZZ_aSampler_aSecondSampler"  ; id %8
               OpName %a "a"                                                                ; id %41
               OpName %b "b"                                                                ; id %44
               OpName %foo_h4ZZ_aSecondSampler_aThirdSampler "foo_h4ZZ_aSecondSampler_aThirdSampler"    ; id %9
               OpName %a_0 "a"                                                                          ; id %48
               OpName %b_0 "b"                                                                          ; id %50
               OpName %main "main"                                                                      ; id %10

               ; Annotations
               OpDecorate %baz_h4Z_aSampler RelaxedPrecision
               OpDecorate %baz_h4Z_aSecondSampler RelaxedPrecision
               OpDecorate %baz_h4Z_aThirdSampler RelaxedPrecision
               OpDecorate %bar_h4Z_aSampler RelaxedPrecision
               OpDecorate %bar_h4Z_aThirdSampler RelaxedPrecision
               OpDecorate %bar_h4Z_aSecondSampler RelaxedPrecision
               OpDecorate %foo_h4ZZ_aSampler_aSecondSampler RelaxedPrecision
               OpDecorate %foo_h4ZZ_aSecondSampler_aThirdSampler RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aSampler RelaxedPrecision
               OpDecorate %aSampler Binding 0
               OpDecorate %aSampler DescriptorSet 0
               OpDecorate %aSecondSampler RelaxedPrecision
               OpDecorate %aSecondSampler Binding 1
               OpDecorate %aSecondSampler DescriptorSet 0
               OpDecorate %aThirdSampler RelaxedPrecision
               OpDecorate %aThirdSampler Binding 2
               OpDecorate %aThirdSampler DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %a_0 RelaxedPrecision
               OpDecorate %b_0 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %16 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %17 = OpTypeSampledImage %16
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
   %aSampler = OpVariable %_ptr_UniformConstant_17 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
%aSecondSampler = OpVariable %_ptr_UniformConstant_17 UniformConstant   ; RelaxedPrecision, Binding 1, DescriptorSet 0
%aThirdSampler = OpVariable %_ptr_UniformConstant_17 UniformConstant    ; RelaxedPrecision, Binding 2, DescriptorSet 0
         %21 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %54 = OpTypeFunction %void


               ; Function baz_h4Z_aSampler
%baz_h4Z_aSampler = OpFunction %v4float None %21    ; RelaxedPrecision

         %22 = OpLabel
         %24 =   OpLoad %17 %aSampler               ; RelaxedPrecision
         %23 =   OpImageSampleImplicitLod %v4float %24 %27  ; RelaxedPrecision
                 OpReturnValue %23
               OpFunctionEnd


               ; Function baz_h4Z_aSecondSampler
%baz_h4Z_aSecondSampler = OpFunction %v4float None %21  ; RelaxedPrecision

         %28 = OpLabel
         %30 =   OpLoad %17 %aSecondSampler         ; RelaxedPrecision
         %29 =   OpImageSampleImplicitLod %v4float %30 %27  ; RelaxedPrecision
                 OpReturnValue %29
               OpFunctionEnd


               ; Function baz_h4Z_aThirdSampler
%baz_h4Z_aThirdSampler = OpFunction %v4float None %21   ; RelaxedPrecision

         %31 = OpLabel
         %33 =   OpLoad %17 %aThirdSampler          ; RelaxedPrecision
         %32 =   OpImageSampleImplicitLod %v4float %33 %27  ; RelaxedPrecision
                 OpReturnValue %32
               OpFunctionEnd


               ; Function bar_h4Z_aSampler
%bar_h4Z_aSampler = OpFunction %v4float None %21    ; RelaxedPrecision

         %34 = OpLabel
         %35 =   OpFunctionCall %v4float %baz_h4Z_aSampler
                 OpReturnValue %35
               OpFunctionEnd


               ; Function bar_h4Z_aThirdSampler
%bar_h4Z_aThirdSampler = OpFunction %v4float None %21   ; RelaxedPrecision

         %36 = OpLabel
         %37 =   OpFunctionCall %v4float %baz_h4Z_aThirdSampler
                 OpReturnValue %37
               OpFunctionEnd


               ; Function bar_h4Z_aSecondSampler
%bar_h4Z_aSecondSampler = OpFunction %v4float None %21  ; RelaxedPrecision

         %38 = OpLabel
         %39 =   OpFunctionCall %v4float %baz_h4Z_aSecondSampler
                 OpReturnValue %39
               OpFunctionEnd


               ; Function foo_h4ZZ_aSampler_aSecondSampler
%foo_h4ZZ_aSampler_aSecondSampler = OpFunction %v4float None %21    ; RelaxedPrecision

         %40 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %43 =   OpFunctionCall %v4float %bar_h4Z_aSampler
                 OpStore %a %43
         %45 =   OpFunctionCall %v4float %baz_h4Z_aSecondSampler
                 OpStore %b %45
         %46 =   OpFAdd %v4float %43 %45            ; RelaxedPrecision
                 OpReturnValue %46
               OpFunctionEnd


               ; Function foo_h4ZZ_aSecondSampler_aThirdSampler
%foo_h4ZZ_aSecondSampler_aThirdSampler = OpFunction %v4float None %21   ; RelaxedPrecision

         %47 = OpLabel
        %a_0 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %b_0 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %49 =   OpFunctionCall %v4float %bar_h4Z_aSecondSampler
                 OpStore %a_0 %49
         %51 =   OpFunctionCall %v4float %baz_h4Z_aThirdSampler
                 OpStore %b_0 %51
         %52 =   OpFAdd %v4float %49 %51            ; RelaxedPrecision
                 OpReturnValue %52
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %54

         %55 = OpLabel
         %56 =   OpFunctionCall %v4float %foo_h4ZZ_aSampler_aSecondSampler
                 OpStore %sk_FragColor %56
         %57 =   OpFunctionCall %v4float %bar_h4Z_aThirdSampler
                 OpStore %sk_FragColor %57
         %58 =   OpFunctionCall %v4float %foo_h4ZZ_aSecondSampler_aThirdSampler
                 OpStore %sk_FragColor %58
                 OpReturn
               OpFunctionEnd
