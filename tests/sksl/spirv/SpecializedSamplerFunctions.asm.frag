               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %aSampler "aSampler"
               OpName %aSecondSampler "aSecondSampler"
               OpName %aThirdSampler "aThirdSampler"
               OpName %baz_h4Z_aSampler "baz_h4Z_aSampler"
               OpName %baz_h4Z_aSecondSampler "baz_h4Z_aSecondSampler"
               OpName %baz_h4Z_aThirdSampler "baz_h4Z_aThirdSampler"
               OpName %bar_h4Z_aSampler "bar_h4Z_aSampler"
               OpName %bar_h4Z_aThirdSampler "bar_h4Z_aThirdSampler"
               OpName %bar_h4Z_aSecondSampler "bar_h4Z_aSecondSampler"
               OpName %foo_h4ZZ_aSampler_aSecondSampler "foo_h4ZZ_aSampler_aSecondSampler"
               OpName %a "a"
               OpName %b "b"
               OpName %foo_h4ZZ_aSecondSampler_aThirdSampler "foo_h4ZZ_aSecondSampler_aThirdSampler"
               OpName %a_0 "a"
               OpName %b_0 "b"
               OpName %main "main"
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
               OpDecorate %24 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %a_0 RelaxedPrecision
               OpDecorate %b_0 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %16 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %17 = OpTypeSampledImage %16
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
   %aSampler = OpVariable %_ptr_UniformConstant_17 UniformConstant
%aSecondSampler = OpVariable %_ptr_UniformConstant_17 UniformConstant
%aThirdSampler = OpVariable %_ptr_UniformConstant_17 UniformConstant
         %21 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %void = OpTypeVoid
         %54 = OpTypeFunction %void
%baz_h4Z_aSampler = OpFunction %v4float None %21
         %22 = OpLabel
         %24 = OpLoad %17 %aSampler
         %23 = OpImageSampleImplicitLod %v4float %24 %27
               OpReturnValue %23
               OpFunctionEnd
%baz_h4Z_aSecondSampler = OpFunction %v4float None %21
         %28 = OpLabel
         %30 = OpLoad %17 %aSecondSampler
         %29 = OpImageSampleImplicitLod %v4float %30 %27
               OpReturnValue %29
               OpFunctionEnd
%baz_h4Z_aThirdSampler = OpFunction %v4float None %21
         %31 = OpLabel
         %33 = OpLoad %17 %aThirdSampler
         %32 = OpImageSampleImplicitLod %v4float %33 %27
               OpReturnValue %32
               OpFunctionEnd
%bar_h4Z_aSampler = OpFunction %v4float None %21
         %34 = OpLabel
         %35 = OpFunctionCall %v4float %baz_h4Z_aSampler
               OpReturnValue %35
               OpFunctionEnd
%bar_h4Z_aThirdSampler = OpFunction %v4float None %21
         %36 = OpLabel
         %37 = OpFunctionCall %v4float %baz_h4Z_aThirdSampler
               OpReturnValue %37
               OpFunctionEnd
%bar_h4Z_aSecondSampler = OpFunction %v4float None %21
         %38 = OpLabel
         %39 = OpFunctionCall %v4float %baz_h4Z_aSecondSampler
               OpReturnValue %39
               OpFunctionEnd
%foo_h4ZZ_aSampler_aSecondSampler = OpFunction %v4float None %21
         %40 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %43 = OpFunctionCall %v4float %bar_h4Z_aSampler
               OpStore %a %43
         %45 = OpFunctionCall %v4float %baz_h4Z_aSecondSampler
               OpStore %b %45
         %46 = OpFAdd %v4float %43 %45
               OpReturnValue %46
               OpFunctionEnd
%foo_h4ZZ_aSecondSampler_aThirdSampler = OpFunction %v4float None %21
         %47 = OpLabel
        %a_0 = OpVariable %_ptr_Function_v4float Function
        %b_0 = OpVariable %_ptr_Function_v4float Function
         %49 = OpFunctionCall %v4float %bar_h4Z_aSecondSampler
               OpStore %a_0 %49
         %51 = OpFunctionCall %v4float %baz_h4Z_aThirdSampler
               OpStore %b_0 %51
         %52 = OpFAdd %v4float %49 %51
               OpReturnValue %52
               OpFunctionEnd
       %main = OpFunction %void None %54
         %55 = OpLabel
         %56 = OpFunctionCall %v4float %foo_h4ZZ_aSampler_aSecondSampler
               OpStore %sk_FragColor %56
         %57 = OpFunctionCall %v4float %bar_h4Z_aThirdSampler
               OpStore %sk_FragColor %57
         %58 = OpFunctionCall %v4float %foo_h4ZZ_aSecondSampler_aThirdSampler
               OpStore %sk_FragColor %58
               OpReturn
               OpFunctionEnd
