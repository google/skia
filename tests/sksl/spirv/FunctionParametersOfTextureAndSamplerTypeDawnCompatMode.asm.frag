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
         %13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
   %aTexture = OpVariable %_ptr_UniformConstant_13 UniformConstant
%aSampledTexture_texture = OpVariable %_ptr_UniformConstant_13 UniformConstant
         %17 = OpTypeSampler
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%aSampledTexture_sampler = OpVariable %_ptr_UniformConstant_17 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
         %22 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_17 %_ptr_UniformConstant_13
         %31 = OpTypeSampledImage %13
         %33 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_13 %_ptr_UniformConstant_17
       %void = OpTypeVoid
         %40 = OpTypeFunction %void
%helpers_helper_h4ZT = OpFunction %v4float None %22
         %23 = OpFunctionParameter %_ptr_UniformConstant_13
         %24 = OpFunctionParameter %_ptr_UniformConstant_17
         %25 = OpFunctionParameter %_ptr_UniformConstant_13
         %26 = OpLabel
         %28 = OpLoad %13 %23
         %29 = OpLoad %17 %24
         %30 = OpSampledImage %31 %28 %29
         %32 = OpLoad %v2float %c
         %27 = OpImageSampleImplicitLod %v4float %30 %32
               OpReturnValue %27
               OpFunctionEnd
%helper_h4TZ = OpFunction %v4float None %33
         %34 = OpFunctionParameter %_ptr_UniformConstant_13
         %35 = OpFunctionParameter %_ptr_UniformConstant_13
         %36 = OpFunctionParameter %_ptr_UniformConstant_17
         %37 = OpLabel
         %38 = OpFunctionCall %v4float %helpers_helper_h4ZT %35 %36 %34
               OpReturnValue %38
               OpFunctionEnd
       %main = OpFunction %void None %40
         %41 = OpLabel
         %42 = OpFunctionCall %v4float %helper_h4TZ %aTexture %aSampledTexture_texture %aSampledTexture_sampler
               OpStore %sk_FragColor %42
               OpReturn
               OpFunctionEnd
