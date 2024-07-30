               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %aSampler "aSampler"
               OpName %aSecondSampler "aSecondSampler"
               OpName %bar_h4Z "bar_h4Z"
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
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
%_ptr_UniformConstant_8 = OpTypePointer UniformConstant %8
   %aSampler = OpVariable %_ptr_UniformConstant_8 UniformConstant
%aSecondSampler = OpVariable %_ptr_UniformConstant_8 UniformConstant
    %v4float = OpTypeVector %float 4
         %12 = OpTypeFunction %v4float %_ptr_UniformConstant_8
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
         %20 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %29 = OpTypeFunction %void
    %bar_h4Z = OpFunction %v4float None %12
         %13 = OpFunctionParameter %_ptr_UniformConstant_8
         %14 = OpLabel
         %16 = OpLoad %8 %13
         %15 = OpImageSampleImplicitLod %v4float %16 %19
               OpReturnValue %15
               OpFunctionEnd
     %foo_h4 = OpFunction %v4float None %20
         %21 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %24 = OpFunctionCall %v4float %bar_h4Z %aSampler
               OpStore %a %24
         %26 = OpFunctionCall %v4float %bar_h4Z %aSecondSampler
               OpStore %b %26
         %27 = OpFAdd %v4float %24 %26
               OpReturnValue %27
               OpFunctionEnd
       %main = OpFunction %void None %29
         %30 = OpLabel
         %31 = OpFunctionCall %v4float %foo_h4
               OpReturn
               OpFunctionEnd
