               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "a"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %uint
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %v2float = OpTypeVector %float 2
       %main = OpFunction %void None %12
         %13 = OpLabel
         %15 = OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %19 = OpLoad %uint %15
         %14 = OpExtInst %v2float %1 UnpackHalf2x16 %19
         %21 = OpLoad %v4float %sk_FragColor
         %22 = OpVectorShuffle %v4float %21 %14 4 5 2 3
               OpStore %sk_FragColor %22
         %24 = OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %25 = OpLoad %uint %24
         %23 = OpExtInst %v2float %1 UnpackUnorm2x16 %25
         %26 = OpLoad %v4float %sk_FragColor
         %27 = OpVectorShuffle %v4float %26 %23 4 5 2 3
               OpStore %sk_FragColor %27
         %29 = OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %30 = OpLoad %uint %29
         %28 = OpExtInst %v2float %1 UnpackSnorm2x16 %30
         %31 = OpLoad %v4float %sk_FragColor
         %32 = OpVectorShuffle %v4float %31 %28 4 5 2 3
               OpStore %sk_FragColor %32
         %34 = OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %35 = OpLoad %uint %34
         %33 = OpExtInst %v4float %1 UnpackUnorm4x8 %35
               OpStore %sk_FragColor %33
         %37 = OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %38 = OpLoad %uint %37
         %36 = OpExtInst %v4float %1 UnpackSnorm4x8 %38
               OpStore %sk_FragColor %36
               OpReturn
               OpFunctionEnd
