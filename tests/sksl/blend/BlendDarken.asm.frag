               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"
               OpName %_0_a "_0_a"
               OpName %_1_b "_1_b"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %_0_a RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %_1_b RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
       %main = OpFunction %void None %14
         %15 = OpLabel
       %_0_a = OpVariable %_ptr_Function_v4float Function
       %_1_b = OpVariable %_ptr_Function_v3float Function
         %18 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %22 = OpLoad %v4float %18
         %24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %25 = OpLoad %v4float %24
         %26 = OpCompositeExtract %float %25 3
         %27 = OpFSub %float %float_1 %26
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %30 = OpLoad %v4float %28
         %31 = OpVectorTimesScalar %v4float %30 %27
         %32 = OpFAdd %v4float %22 %31
               OpStore %_0_a %32
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %37 = OpLoad %v4float %36
         %38 = OpCompositeExtract %float %37 3
         %39 = OpFSub %float %float_1 %38
         %40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpVectorShuffle %v3float %41 %41 0 1 2
         %43 = OpVectorTimesScalar %v3float %42 %39
         %44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %45 = OpLoad %v4float %44
         %46 = OpVectorShuffle %v3float %45 %45 0 1 2
         %47 = OpFAdd %v3float %43 %46
               OpStore %_1_b %47
         %49 = OpVectorShuffle %v3float %32 %32 0 1 2
         %48 = OpExtInst %v3float %1 FMin %49 %47
         %50 = OpLoad %v4float %_0_a
         %51 = OpVectorShuffle %v4float %50 %48 4 5 6 3
               OpStore %_0_a %51
               OpStore %sk_FragColor %51
               OpReturn
               OpFunctionEnd
