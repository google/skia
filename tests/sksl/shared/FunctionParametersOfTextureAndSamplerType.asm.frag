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
               OpDecorate %27 RelaxedPrecision
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
         %17 = OpTypeSampledImage %16
%_ptr_UniformConstant_17 = OpTypePointer UniformConstant %17
%aSampledTexture = OpVariable %_ptr_UniformConstant_17 UniformConstant
    %v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
          %c = OpVariable %_ptr_Input_v2float Input
         %22 = OpTypeFunction %v4float %_ptr_UniformConstant_17 %_ptr_UniformConstant_13
         %29 = OpTypeFunction %v4float %_ptr_UniformConstant_13 %_ptr_UniformConstant_17
       %void = OpTypeVoid
         %35 = OpTypeFunction %void
%helpers_helper_h4ZT = OpFunction %v4float None %22
         %23 = OpFunctionParameter %_ptr_UniformConstant_17
         %24 = OpFunctionParameter %_ptr_UniformConstant_13
         %25 = OpLabel
         %27 = OpLoad %17 %23
         %28 = OpLoad %v2float %c
         %26 = OpImageSampleImplicitLod %v4float %27 %28
               OpReturnValue %26
               OpFunctionEnd
%helper_h4TZ = OpFunction %v4float None %29
         %30 = OpFunctionParameter %_ptr_UniformConstant_13
         %31 = OpFunctionParameter %_ptr_UniformConstant_17
         %32 = OpLabel
         %33 = OpFunctionCall %v4float %helpers_helper_h4ZT %31 %30
               OpReturnValue %33
               OpFunctionEnd
       %main = OpFunction %void None %35
         %36 = OpLabel
         %37 = OpFunctionCall %v4float %helper_h4TZ %aTexture %aSampledTexture
               OpStore %sk_FragColor %37
               OpReturn
               OpFunctionEnd
