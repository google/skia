               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
       %main = OpFunction %void None %11
         %12 = OpLabel
         %14 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %18 = OpLoad %v4float %14
         %19 = OpCompositeExtract %float %18 3
         %20 = OpFSub %float %float_1 %19
         %21 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %23 = OpLoad %v4float %21
         %24 = OpVectorTimesScalar %v4float %23 %20
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %26 = OpLoad %v4float %25
         %27 = OpCompositeExtract %float %26 3
         %28 = OpFSub %float %float_1 %27
         %29 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %30 = OpLoad %v4float %29
         %31 = OpVectorTimesScalar %v4float %30 %28
         %32 = OpFAdd %v4float %24 %31
               OpStore %sk_FragColor %32
               OpReturn
               OpFunctionEnd
