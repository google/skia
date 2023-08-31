               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %aTexture "aTexture"
               OpName %aSampledTexture_texture "aSampledTexture_texture"
               OpName %aSampledTexture_sampler "aSampledTexture_sampler"
               OpName %c "c"
               OpName %helpers_helper_h4ZT "helpers_helper_h4ZT"
               OpName %helper_h4TZ "helper_h4TZ"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aTexture Binding 1
               OpDecorate %aTexture DescriptorSet 0
               OpDecorate %aSampledTexture_texture Binding 2
               OpDecorate %aSampledTexture_texture DescriptorSet 0
               OpDecorate %aSampledTexture_sampler Binding 3
               OpDecorate %aSampledTexture_sampler DescriptorSet 0
               OpDecorate %c Location 1
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %13 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
   %aTexture = OpVariable %_ptr_UniformConstant_13 UniformConstant
         %16 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%aSampledTexture_texture = OpVariable %_ptr_UniformConstant_16 UniformConstant
         %19 = OpTypeSampler
%_ptr_UniformConstant_19 = OpTypePointer UniformConstant %19
%aSampledTexture_sampler = OpVariable %_ptr_UniformConstant_19 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
         %24 = OpTypeFunction %v4float %_ptr_UniformConstant_16 %_ptr_UniformConstant_19 %_ptr_UniformConstant_13
         %33 = OpTypeSampledImage %16
         %35 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_16 %_ptr_UniformConstant_19
       %void = OpTypeVoid
         %42 = OpTypeFunction %void
%helpers_helper_h4ZT = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_UniformConstant_16
         %26 = OpFunctionParameter %_ptr_UniformConstant_19
         %27 = OpFunctionParameter %_ptr_UniformConstant_13
         %28 = OpLabel
         %30 = OpLoad %16 %25
         %31 = OpLoad %19 %26
         %32 = OpSampledImage %33 %30 %31
         %34 = OpLoad %v2float %c
         %29 = OpImageSampleImplicitLod %v4float %32 %34
               OpReturnValue %29
               OpFunctionEnd
%helper_h4TZ = OpFunction %v4float None %35
         %36 = OpFunctionParameter %_ptr_UniformConstant_13
         %37 = OpFunctionParameter %_ptr_UniformConstant_16
         %38 = OpFunctionParameter %_ptr_UniformConstant_19
         %39 = OpLabel
         %40 = OpFunctionCall %v4float %helpers_helper_h4ZT %37 %38 %36
               OpReturnValue %40
               OpFunctionEnd
       %main = OpFunction %void None %42
         %43 = OpLabel
         %44 = OpFunctionCall %v4float %helper_h4TZ %aTexture %aSampledTexture_texture %aSampledTexture_sampler
               OpStore %sk_FragColor %44
               OpReturn
               OpFunctionEnd
