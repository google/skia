               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %t "t"
               OpName %main "main"
               OpName %c "c"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t RelaxedPrecision
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %c RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
          %t = OpVariable %_ptr_UniformConstant_12 UniformConstant
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
         %28 = OpConstantComposite %v3float %float_1 %float_1 %float_1
       %main = OpFunction %void None %15
         %16 = OpLabel
          %c = OpVariable %_ptr_Function_v4float Function
         %20 = OpLoad %12 %t
         %19 = OpImageSampleImplicitLod %v4float %20 %23
               OpStore %c %19
         %25 = OpLoad %12 %t
         %24 = OpImageSampleProjImplicitLod %v4float %25 %28
         %29 = OpFMul %v4float %19 %24
               OpStore %sk_FragColor %29
               OpReturn
               OpFunctionEnd
