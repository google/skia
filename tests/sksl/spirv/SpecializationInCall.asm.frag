               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %aSampler "aSampler"          ; id %6
               OpName %aSecondSampler "aSecondSampler"  ; id %11
               OpName %bar_h4Z_aSampler "bar_h4Z_aSampler"  ; id %2
               OpName %bar_h4Z_aSecondSampler "bar_h4Z_aSecondSampler"  ; id %3
               OpName %foo_h4 "foo_h4"                                  ; id %4
               OpName %a "a"                                            ; id %24
               OpName %b "b"                                            ; id %27
               OpName %main "main"                                      ; id %5

               ; Annotations
               OpDecorate %bar_h4Z_aSampler RelaxedPrecision
               OpDecorate %bar_h4Z_aSecondSampler RelaxedPrecision
               OpDecorate %foo_h4 RelaxedPrecision
               OpDecorate %aSampler RelaxedPrecision
               OpDecorate %aSampler Binding 0
               OpDecorate %aSampler DescriptorSet 0
               OpDecorate %aSecondSampler RelaxedPrecision
               OpDecorate %aSecondSampler Binding 1
               OpDecorate %aSecondSampler DescriptorSet 0
               OpDecorate %15 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %29 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
   %aSampler = OpVariable %_ptr_UniformConstant_9 UniformConstant   ; RelaxedPrecision, Binding 0, DescriptorSet 0
%aSecondSampler = OpVariable %_ptr_UniformConstant_9 UniformConstant    ; RelaxedPrecision, Binding 1, DescriptorSet 0
    %v4float = OpTypeVector %float 4
         %13 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %31 = OpTypeFunction %void


               ; Function bar_h4Z_aSampler
%bar_h4Z_aSampler = OpFunction %v4float None %13    ; RelaxedPrecision

         %14 = OpLabel
         %16 =   OpLoad %9 %aSampler                ; RelaxedPrecision
         %15 =   OpImageSampleImplicitLod %v4float %16 %19  ; RelaxedPrecision
                 OpReturnValue %15
               OpFunctionEnd


               ; Function bar_h4Z_aSecondSampler
%bar_h4Z_aSecondSampler = OpFunction %v4float None %13  ; RelaxedPrecision

         %20 = OpLabel
         %22 =   OpLoad %9 %aSecondSampler          ; RelaxedPrecision
         %21 =   OpImageSampleImplicitLod %v4float %22 %19  ; RelaxedPrecision
                 OpReturnValue %21
               OpFunctionEnd


               ; Function foo_h4
     %foo_h4 = OpFunction %v4float None %13         ; RelaxedPrecision

         %23 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %26 =   OpFunctionCall %v4float %bar_h4Z_aSampler
                 OpStore %a %26
         %28 =   OpFunctionCall %v4float %bar_h4Z_aSecondSampler
                 OpStore %b %28
         %29 =   OpFAdd %v4float %26 %28            ; RelaxedPrecision
                 OpReturnValue %29
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %31

         %32 = OpLabel
         %33 =   OpFunctionCall %v4float %foo_h4
                 OpReturn
               OpFunctionEnd
