               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %aSampler "aSampler"
               OpName %aSecondSampler "aSecondSampler"
               OpName %bar_h4Z_aSampler "bar_h4Z_aSampler"
               OpName %bar_h4Z_aSecondSampler "bar_h4Z_aSecondSampler"
               OpName %foo_h4 "foo_h4"
               OpName %a "a"
               OpName %b "b"
               OpName %main "main"
               OpDecorate %aSampler RelaxedPrecision
               OpDecorate %aSampler Binding 0
               OpDecorate %aSampler DescriptorSet 0
               OpDecorate %aSecondSampler RelaxedPrecision
               OpDecorate %aSecondSampler Binding 1
               OpDecorate %aSecondSampler DescriptorSet 0
               OpDecorate %16 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
      %float = OpTypeFloat 32
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
   %aSampler = OpVariable %_ptr_UniformConstant_9 UniformConstant
%aSecondSampler = OpVariable %_ptr_UniformConstant_9 UniformConstant
    %v4float = OpTypeVector %float 4
         %13 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %31 = OpTypeFunction %void
%bar_h4Z_aSampler = OpFunction %v4float None %13
         %14 = OpLabel
         %16 = OpLoad %9 %aSampler
         %15 = OpImageSampleImplicitLod %v4float %16 %19
               OpReturnValue %15
               OpFunctionEnd
%bar_h4Z_aSecondSampler = OpFunction %v4float None %13
         %20 = OpLabel
         %22 = OpLoad %9 %aSecondSampler
         %21 = OpImageSampleImplicitLod %v4float %22 %19
               OpReturnValue %21
               OpFunctionEnd
     %foo_h4 = OpFunction %v4float None %13
         %23 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %26 = OpFunctionCall %v4float %bar_h4Z_aSampler
               OpStore %a %26
         %28 = OpFunctionCall %v4float %bar_h4Z_aSecondSampler
               OpStore %b %28
         %29 = OpFAdd %v4float %26 %28
               OpReturnValue %29
               OpFunctionEnd
       %main = OpFunction %void None %31
         %32 = OpLabel
         %33 = OpFunctionCall %v4float %foo_h4
               OpReturn
               OpFunctionEnd
