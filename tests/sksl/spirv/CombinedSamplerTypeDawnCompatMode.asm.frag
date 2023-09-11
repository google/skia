               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %aSampler_texture "aSampler_texture"
               OpName %aSampler_sampler "aSampler_sampler"
               OpName %anotherSampler_texture "anotherSampler_texture"
               OpName %anotherSampler_sampler "anotherSampler_sampler"
               OpName %helpers_helper_h4Z "helpers_helper_h4Z"
               OpName %helper_h4Z "helper_h4Z"
               OpName %helper2_h4ZZ "helper2_h4ZZ"
               OpName %main "main"
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
               OpDecorate %45 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_11 = OpTypePointer UniformConstant %11
%aSampler_texture = OpVariable %_ptr_UniformConstant_11 UniformConstant
         %14 = OpTypeSampler
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%aSampler_sampler = OpVariable %_ptr_UniformConstant_14 UniformConstant
%anotherSampler_texture = OpVariable %_ptr_UniformConstant_11 UniformConstant
%anotherSampler_sampler = OpVariable %_ptr_UniformConstant_14 UniformConstant
         %18 = OpTypeFunction %v4float %_ptr_UniformConstant_11 %_ptr_UniformConstant_14
         %26 = OpTypeSampledImage %11
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
         %29 = OpConstantComposite %v2float %float_1 %float_1
         %34 = OpTypeFunction %v4float %_ptr_UniformConstant_11 %_ptr_UniformConstant_14 %_ptr_UniformConstant_11 %_ptr_UniformConstant_14
       %void = OpTypeVoid
         %47 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %54 = OpConstantComposite %v2float %float_0 %float_0
%helpers_helper_h4Z = OpFunction %v4float None %18
         %19 = OpFunctionParameter %_ptr_UniformConstant_11
         %20 = OpFunctionParameter %_ptr_UniformConstant_14
         %21 = OpLabel
         %23 = OpLoad %11 %19
         %24 = OpLoad %14 %20
         %25 = OpSampledImage %26 %23 %24
         %22 = OpImageSampleImplicitLod %v4float %25 %29
               OpReturnValue %22
               OpFunctionEnd
 %helper_h4Z = OpFunction %v4float None %18
         %30 = OpFunctionParameter %_ptr_UniformConstant_11
         %31 = OpFunctionParameter %_ptr_UniformConstant_14
         %32 = OpLabel
         %33 = OpFunctionCall %v4float %helpers_helper_h4Z %30 %31
               OpReturnValue %33
               OpFunctionEnd
%helper2_h4ZZ = OpFunction %v4float None %34
         %35 = OpFunctionParameter %_ptr_UniformConstant_11
         %36 = OpFunctionParameter %_ptr_UniformConstant_14
         %37 = OpFunctionParameter %_ptr_UniformConstant_11
         %38 = OpFunctionParameter %_ptr_UniformConstant_14
         %39 = OpLabel
         %41 = OpLoad %11 %35
         %42 = OpLoad %14 %36
         %43 = OpSampledImage %26 %41 %42
         %40 = OpImageSampleImplicitLod %v4float %43 %29
         %44 = OpFunctionCall %v4float %helper_h4Z %37 %38
         %45 = OpFAdd %v4float %40 %44
               OpReturnValue %45
               OpFunctionEnd
       %main = OpFunction %void None %47
         %48 = OpLabel
         %50 = OpLoad %11 %aSampler_texture
         %51 = OpLoad %14 %aSampler_sampler
         %52 = OpSampledImage %26 %50 %51
         %49 = OpImageSampleImplicitLod %v4float %52 %54
         %55 = OpFunctionCall %v4float %helper_h4Z %aSampler_texture %aSampler_sampler
         %56 = OpFAdd %v4float %49 %55
         %57 = OpFunctionCall %v4float %helper2_h4ZZ %aSampler_texture %aSampler_sampler %anotherSampler_texture %anotherSampler_sampler
         %58 = OpFAdd %v4float %56 %57
               OpStore %sk_FragColor %58
               OpReturn
               OpFunctionEnd
