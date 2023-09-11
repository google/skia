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
               OpMemberDecorate %_UniformBuffer 1 Offset 4
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %int %uint
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
%_ptr_Uniform_int = OpTypePointer Uniform %int
      %int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %13
         %14 = OpLabel
         %16 = OpAccessChain %_ptr_Uniform_int %7 %int_0
         %19 = OpLoad %int %16
         %15 = OpBitCount %int %19
         %20 = OpConvertSToF %float %15
         %21 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %21 %20
         %24 = OpAccessChain %_ptr_Uniform_uint %7 %int_1
         %27 = OpLoad %uint %24
         %23 = OpBitCount %int %27
         %28 = OpConvertSToF %float %23
         %29 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
               OpStore %29 %28
               OpReturn
               OpFunctionEnd
