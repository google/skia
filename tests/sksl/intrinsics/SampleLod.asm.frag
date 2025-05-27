               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %t "t"
               OpName %main "main"
               OpName %c "c"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t RelaxedPrecision
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %c RelaxedPrecision
               OpDecorate %17 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
          %t = OpVariable %_ptr_UniformConstant_9 UniformConstant
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
         %25 = OpConstantComposite %v3float %float_1 %float_1 %float_1
       %main = OpFunction %void None %12
         %13 = OpLabel
          %c = OpVariable %_ptr_Function_v4float Function
         %17 = OpLoad %9 %t
         %16 = OpImageSampleExplicitLod %v4float %17 %20 Lod %float_0
               OpStore %c %16
         %22 = OpLoad %9 %t
         %21 = OpImageSampleProjExplicitLod %v4float %22 %25 Lod %float_0
         %26 = OpFMul %v4float %16 %21
               OpStore %sk_FragColor %26
               OpReturn
               OpFunctionEnd
