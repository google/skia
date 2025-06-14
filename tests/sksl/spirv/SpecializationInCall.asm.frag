               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %aSampler "aSampler"          ; id %10
               OpName %aSecondSampler "aSecondSampler"  ; id %15
               OpName %bar_h4Z_aSampler "bar_h4Z_aSampler"  ; id %6
               OpName %bar_h4Z_aSecondSampler "bar_h4Z_aSecondSampler"  ; id %7
               OpName %foo_h4 "foo_h4"                                  ; id %8
               OpName %a "a"                                            ; id %28
               OpName %b "b"                                            ; id %31
               OpName %main "main"                                      ; id %9

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
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %33 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
   %aSampler = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
%aSecondSampler = OpVariable %_ptr_UniformConstant_13 UniformConstant   ; RelaxedPrecision, Binding 1, DescriptorSet 0
    %v4float = OpTypeVector %float 4
         %17 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %35 = OpTypeFunction %void


               ; Function bar_h4Z_aSampler
%bar_h4Z_aSampler = OpFunction %v4float None %17    ; RelaxedPrecision

         %18 = OpLabel
         %20 =   OpLoad %13 %aSampler               ; RelaxedPrecision
         %19 =   OpImageSampleImplicitLod %v4float %20 %23  ; RelaxedPrecision
                 OpReturnValue %19
               OpFunctionEnd


               ; Function bar_h4Z_aSecondSampler
%bar_h4Z_aSecondSampler = OpFunction %v4float None %17  ; RelaxedPrecision

         %24 = OpLabel
         %26 =   OpLoad %13 %aSecondSampler         ; RelaxedPrecision
         %25 =   OpImageSampleImplicitLod %v4float %26 %23  ; RelaxedPrecision
                 OpReturnValue %25
               OpFunctionEnd


               ; Function foo_h4
     %foo_h4 = OpFunction %v4float None %17         ; RelaxedPrecision

         %27 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %30 =   OpFunctionCall %v4float %bar_h4Z_aSampler
                 OpStore %a %30
         %32 =   OpFunctionCall %v4float %bar_h4Z_aSecondSampler
                 OpStore %b %32
         %33 =   OpFAdd %v4float %30 %32            ; RelaxedPrecision
                 OpReturnValue %33
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %35

         %36 = OpLabel
         %37 =   OpFunctionCall %v4float %foo_h4
                 OpReturn
               OpFunctionEnd
