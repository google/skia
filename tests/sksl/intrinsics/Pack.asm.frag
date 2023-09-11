               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
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
               OpDecorate %19 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %12
         %13 = OpLabel
         %15 = OpAccessChain %_ptr_Uniform_v2float %7 %int_0
         %19 = OpLoad %v2float %15
         %14 = OpExtInst %uint %1 PackHalf2x16 %19
         %21 = OpConvertUToF %float %14
         %22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %22 %21
         %25 = OpAccessChain %_ptr_Uniform_v2float %7 %int_0
         %26 = OpLoad %v2float %25
         %24 = OpExtInst %uint %1 PackUnorm2x16 %26
         %27 = OpConvertUToF %float %24
         %28 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %28 %27
         %30 = OpAccessChain %_ptr_Uniform_v2float %7 %int_0
         %31 = OpLoad %v2float %30
         %29 = OpExtInst %uint %1 PackSnorm2x16 %31
         %32 = OpConvertUToF %float %29
         %33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %33 %32
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %38 = OpLoad %v4float %35
         %34 = OpExtInst %uint %1 PackUnorm4x8 %38
         %39 = OpConvertUToF %float %34
         %40 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %40 %39
         %42 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %43 = OpLoad %v4float %42
         %41 = OpExtInst %uint %1 PackSnorm4x8 %43
         %44 = OpConvertUToF %float %41
         %45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %45 %44
               OpReturn
               OpFunctionEnd
