               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %aSampler_texture "aSampler_texture"
               OpName %aSampler_sampler "aSampler_sampler"
               OpName %anotherSampler_texture "anotherSampler_texture"
               OpName %anotherSampler_sampler "anotherSampler_sampler"
               OpName %helpers_helper_h4Z "helpers_helper_h4Z"
               OpName %helper_h4Z "helper_h4Z"
               OpName %helper2_h4ZZ "helper2_h4ZZ"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aSampler_texture Binding 2
               OpDecorate %aSampler_texture DescriptorSet 1
               OpDecorate %aSampler_sampler Binding 3
               OpDecorate %aSampler_sampler DescriptorSet 1
               OpDecorate %anotherSampler_texture Binding 4
               OpDecorate %anotherSampler_texture DescriptorSet 1
               OpDecorate %anotherSampler_sampler Binding 5
               OpDecorate %anotherSampler_sampler DescriptorSet 1
               OpDecorate %48 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %14 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%aSampler_texture = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %17 = OpTypeSampler
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%aSampler_sampler = OpVariable %_ptr_UniformConstant_17 UniformConstant
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_14 UniformConstant
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_17 UniformConstant
         %21 = OpTypeFunction %v4float %_ptr_UniformConstant_14 %_ptr_UniformConstant_17
         %29 = OpTypeSampledImage %14
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %32 = OpConstantComposite %v2float %float_1 %float_1
         %37 = OpTypeFunction %v4float %_ptr_UniformConstant_14 %_ptr_UniformConstant_17 %_ptr_UniformConstant_14 %_ptr_UniformConstant_17
       %void = OpTypeVoid
         %50 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %57 = OpConstantComposite %v2float %float_0 %float_0
%helpers_helper_h4Z = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_UniformConstant_14
         %23 = OpFunctionParameter %_ptr_UniformConstant_17
         %24 = OpLabel
         %26 = OpLoad %14 %22
         %27 = OpLoad %17 %23
         %28 = OpSampledImage %29 %26 %27
         %25 = OpImageSampleImplicitLod %v4float %28 %32
               OpReturnValue %25
               OpFunctionEnd
 %helper_h4Z = OpFunction %v4float None %21
         %33 = OpFunctionParameter %_ptr_UniformConstant_14
         %34 = OpFunctionParameter %_ptr_UniformConstant_17
         %35 = OpLabel
         %36 = OpFunctionCall %v4float %helpers_helper_h4Z %33 %34
               OpReturnValue %36
               OpFunctionEnd
%helper2_h4ZZ = OpFunction %v4float None %37
         %38 = OpFunctionParameter %_ptr_UniformConstant_14
         %39 = OpFunctionParameter %_ptr_UniformConstant_17
         %40 = OpFunctionParameter %_ptr_UniformConstant_14
         %41 = OpFunctionParameter %_ptr_UniformConstant_17
         %42 = OpLabel
         %44 = OpLoad %14 %38
         %45 = OpLoad %17 %39
         %46 = OpSampledImage %29 %44 %45
         %43 = OpImageSampleImplicitLod %v4float %46 %32
         %47 = OpFunctionCall %v4float %helper_h4Z %40 %41
         %48 = OpFAdd %v4float %43 %47
               OpReturnValue %48
               OpFunctionEnd
       %main = OpFunction %void None %50
         %51 = OpLabel
         %53 = OpLoad %14 %aSampler_texture
         %54 = OpLoad %17 %aSampler_sampler
         %55 = OpSampledImage %29 %53 %54
         %52 = OpImageSampleImplicitLod %v4float %55 %57
         %58 = OpFunctionCall %v4float %helper_h4Z %aSampler_texture %aSampler_sampler
         %59 = OpFAdd %v4float %52 %58
         %60 = OpFunctionCall %v4float %helper2_h4ZZ %aSampler_texture %aSampler_sampler %anotherSampler_texture %anotherSampler_sampler
         %61 = OpFAdd %v4float %59 %60
               OpStore %sk_FragColor %61
               OpReturn
               OpFunctionEnd
