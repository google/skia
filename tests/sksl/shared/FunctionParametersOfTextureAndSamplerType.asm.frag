               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %aTexture "aTexture"
               OpName %aSampledTexture "aSampledTexture"
               OpName %c "c"
               OpName %helpers_helper_h4ZT "helpers_helper_h4ZT"
               OpName %helper_h4TZ "helper_h4TZ"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %aTexture Binding 1
               OpDecorate %aTexture DescriptorSet 0
               OpDecorate %aSampledTexture RelaxedPrecision
               OpDecorate %aSampledTexture Binding 2
               OpDecorate %aSampledTexture DescriptorSet 0
               OpDecorate %c Location 1
               OpDecorate %24 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %10 = OpTypeImage %float 2D 0 0 0 2 Rgba8
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
   %aTexture = OpVariable %_ptr_UniformConstant_10 UniformConstant
         %13 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %14 = OpTypeSampledImage %13
%_ptr_UniformConstant_14 = OpTypePointer UniformConstant %14
%aSampledTexture = OpVariable %_ptr_UniformConstant_14 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
         %19 = OpTypeFunction %v4float %_ptr_UniformConstant_14 %_ptr_UniformConstant_10
         %26 = OpTypeFunction %v4float %_ptr_UniformConstant_10 %_ptr_UniformConstant_14
       %void = OpTypeVoid
         %32 = OpTypeFunction %void
%helpers_helper_h4ZT = OpFunction %v4float None %19
         %20 = OpFunctionParameter %_ptr_UniformConstant_14
         %21 = OpFunctionParameter %_ptr_UniformConstant_10
         %22 = OpLabel
         %24 = OpLoad %14 %20
         %25 = OpLoad %v2float %c
         %23 = OpImageSampleImplicitLod %v4float %24 %25
               OpReturnValue %23
               OpFunctionEnd
%helper_h4TZ = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_UniformConstant_10
         %28 = OpFunctionParameter %_ptr_UniformConstant_14
         %29 = OpLabel
         %30 = OpFunctionCall %v4float %helpers_helper_h4ZT %28 %27
               OpReturnValue %30
               OpFunctionEnd
       %main = OpFunction %void None %32
         %33 = OpLabel
         %34 = OpFunctionCall %v4float %helper_h4TZ %aTexture %aSampledTexture
               OpStore %sk_FragColor %34
               OpReturn
               OpFunctionEnd
