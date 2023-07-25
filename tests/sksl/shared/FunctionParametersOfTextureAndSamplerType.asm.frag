               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %c
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %aTexture "aTexture"
               OpName %aSampledTexture "aSampledTexture"
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
               OpDecorate %aSampledTexture RelaxedPrecision
               OpDecorate %aSampledTexture Binding 2
               OpDecorate %aSampledTexture DescriptorSet 0
               OpDecorate %c Location 1
               OpDecorate %26 RelaxedPrecision
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
         %16 = OpTypeSampledImage %13
%_ptr_UniformConstant_16 = OpTypePointer UniformConstant %16
%aSampledTexture = OpVariable %_ptr_UniformConstant_16 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
         %21 = OpTypeFunction %v4float %_ptr_UniformConstant_16 %_ptr_UniformConstant_13
         %28 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_16
       %void = OpTypeVoid
         %34 = OpTypeFunction %void
%helpers_helper_h4ZT = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_UniformConstant_16
         %23 = OpFunctionParameter %_ptr_UniformConstant_13
         %24 = OpLabel
         %26 = OpLoad %16 %22
         %27 = OpLoad %v2float %c
         %25 = OpImageSampleImplicitLod %v4float %26 %27
               OpReturnValue %25
               OpFunctionEnd
%helper_h4TZ = OpFunction %v4float None %28
         %29 = OpFunctionParameter %_ptr_UniformConstant_13
         %30 = OpFunctionParameter %_ptr_UniformConstant_16
         %31 = OpLabel
         %32 = OpFunctionCall %v4float %helpers_helper_h4ZT %30 %29
               OpReturnValue %32
               OpFunctionEnd
       %main = OpFunction %void None %34
         %35 = OpLabel
         %36 = OpFunctionCall %v4float %helper_h4TZ %aTexture %aSampledTexture
               OpStore %sk_FragColor %36
               OpReturn
               OpFunctionEnd
