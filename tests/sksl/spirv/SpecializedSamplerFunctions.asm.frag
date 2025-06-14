               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %15
               OpName %aSampler "aSampler"          ; id %19
               OpName %aSecondSampler "aSecondSampler"  ; id %23
               OpName %aThirdSampler "aThirdSampler"    ; id %24
               OpName %baz_h4Z_aSampler "baz_h4Z_aSampler"  ; id %6
               OpName %baz_h4Z_aSecondSampler "baz_h4Z_aSecondSampler"  ; id %7
               OpName %baz_h4Z_aThirdSampler "baz_h4Z_aThirdSampler"    ; id %8
               OpName %bar_h4Z_aSampler "bar_h4Z_aSampler"              ; id %9
               OpName %bar_h4Z_aThirdSampler "bar_h4Z_aThirdSampler"    ; id %10
               OpName %bar_h4Z_aSecondSampler "bar_h4Z_aSecondSampler"  ; id %11
               OpName %foo_h4ZZ_aSampler_aSecondSampler "foo_h4ZZ_aSampler_aSecondSampler"  ; id %12
               OpName %a "a"                                                                ; id %45
               OpName %b "b"                                                                ; id %48
               OpName %foo_h4ZZ_aSecondSampler_aThirdSampler "foo_h4ZZ_aSecondSampler_aThirdSampler"    ; id %13
               OpName %a_0 "a"                                                                          ; id %52
               OpName %b_0 "b"                                                                          ; id %54
               OpName %main "main"                                                                      ; id %14

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
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %a_0 RelaxedPrecision
               OpDecorate %b_0 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %20 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %21 = OpTypeSampledImage %20
%_ptr_UniformConstant_21 = OpTypePointer UniformConstant %21
   %aSampler = OpVariable %_ptr_UniformConstant_21 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
%aSecondSampler = OpVariable %_ptr_UniformConstant_21 UniformConstant   ; RelaxedPrecision, Binding 1, DescriptorSet 0
%aThirdSampler = OpVariable %_ptr_UniformConstant_21 UniformConstant    ; RelaxedPrecision, Binding 2, DescriptorSet 0
         %25 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %58 = OpTypeFunction %void


               ; Function baz_h4Z_aSampler
%baz_h4Z_aSampler = OpFunction %v4float None %25    ; RelaxedPrecision

         %26 = OpLabel
         %28 =   OpLoad %21 %aSampler               ; RelaxedPrecision
         %27 =   OpImageSampleImplicitLod %v4float %28 %31  ; RelaxedPrecision
                 OpReturnValue %27
               OpFunctionEnd


               ; Function baz_h4Z_aSecondSampler
%baz_h4Z_aSecondSampler = OpFunction %v4float None %25  ; RelaxedPrecision

         %32 = OpLabel
         %34 =   OpLoad %21 %aSecondSampler         ; RelaxedPrecision
         %33 =   OpImageSampleImplicitLod %v4float %34 %31  ; RelaxedPrecision
                 OpReturnValue %33
               OpFunctionEnd


               ; Function baz_h4Z_aThirdSampler
%baz_h4Z_aThirdSampler = OpFunction %v4float None %25   ; RelaxedPrecision

         %35 = OpLabel
         %37 =   OpLoad %21 %aThirdSampler          ; RelaxedPrecision
         %36 =   OpImageSampleImplicitLod %v4float %37 %31  ; RelaxedPrecision
                 OpReturnValue %36
               OpFunctionEnd


               ; Function bar_h4Z_aSampler
%bar_h4Z_aSampler = OpFunction %v4float None %25    ; RelaxedPrecision

         %38 = OpLabel
         %39 =   OpFunctionCall %v4float %baz_h4Z_aSampler
                 OpReturnValue %39
               OpFunctionEnd


               ; Function bar_h4Z_aThirdSampler
%bar_h4Z_aThirdSampler = OpFunction %v4float None %25   ; RelaxedPrecision

         %40 = OpLabel
         %41 =   OpFunctionCall %v4float %baz_h4Z_aThirdSampler
                 OpReturnValue %41
               OpFunctionEnd


               ; Function bar_h4Z_aSecondSampler
%bar_h4Z_aSecondSampler = OpFunction %v4float None %25  ; RelaxedPrecision

         %42 = OpLabel
         %43 =   OpFunctionCall %v4float %baz_h4Z_aSecondSampler
                 OpReturnValue %43
               OpFunctionEnd


               ; Function foo_h4ZZ_aSampler_aSecondSampler
%foo_h4ZZ_aSampler_aSecondSampler = OpFunction %v4float None %25    ; RelaxedPrecision

         %44 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %47 =   OpFunctionCall %v4float %bar_h4Z_aSampler
                 OpStore %a %47
         %49 =   OpFunctionCall %v4float %baz_h4Z_aSecondSampler
                 OpStore %b %49
         %50 =   OpFAdd %v4float %47 %49            ; RelaxedPrecision
                 OpReturnValue %50
               OpFunctionEnd


               ; Function foo_h4ZZ_aSecondSampler_aThirdSampler
%foo_h4ZZ_aSecondSampler_aThirdSampler = OpFunction %v4float None %25   ; RelaxedPrecision

         %51 = OpLabel
        %a_0 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %b_0 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %53 =   OpFunctionCall %v4float %bar_h4Z_aSecondSampler
                 OpStore %a_0 %53
         %55 =   OpFunctionCall %v4float %baz_h4Z_aThirdSampler
                 OpStore %b_0 %55
         %56 =   OpFAdd %v4float %53 %55            ; RelaxedPrecision
                 OpReturnValue %56
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %58

         %59 = OpLabel
         %60 =   OpFunctionCall %v4float %foo_h4ZZ_aSampler_aSecondSampler
                 OpStore %sk_FragColor %60
         %61 =   OpFunctionCall %v4float %bar_h4Z_aThirdSampler
                 OpStore %sk_FragColor %61
         %62 =   OpFunctionCall %v4float %foo_h4ZZ_aSecondSampler_aThirdSampler
                 OpStore %sk_FragColor %62
                 OpReturn
               OpFunctionEnd
