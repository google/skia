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
               OpName %_0_a "_0_a"
               OpName %_1_b "_1_b"
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
               OpDecorate %_0_a RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %_1_b RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
       %main = OpFunction %void None %11
         %12 = OpLabel
       %_0_a = OpVariable %_ptr_Function_v4float Function
       %_1_b = OpVariable %_ptr_Function_v3float Function
         %15 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %19 = OpLoad %v4float %15
         %21 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %22 = OpLoad %v4float %21
         %23 = OpCompositeExtract %float %22 3
         %24 = OpFSub %float %float_1 %23
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %27 = OpLoad %v4float %25
         %28 = OpVectorTimesScalar %v4float %27 %24
         %29 = OpFAdd %v4float %19 %28
               OpStore %_0_a %29
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %34 = OpLoad %v4float %33
         %35 = OpCompositeExtract %float %34 3
         %36 = OpFSub %float %float_1 %35
         %37 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %37
         %39 = OpVectorShuffle %v3float %38 %38 0 1 2
         %40 = OpVectorTimesScalar %v3float %39 %36
         %41 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %42 = OpLoad %v4float %41
         %43 = OpVectorShuffle %v3float %42 %42 0 1 2
         %44 = OpFAdd %v3float %40 %43
               OpStore %_1_b %44
         %46 = OpVectorShuffle %v3float %29 %29 0 1 2
         %45 = OpExtInst %v3float %1 FMin %46 %44
         %47 = OpLoad %v4float %_0_a
         %48 = OpVectorShuffle %v4float %47 %45 4 5 6 3
               OpStore %_0_a %48
               OpStore %sk_FragColor %48
               OpReturn
               OpFunctionEnd
