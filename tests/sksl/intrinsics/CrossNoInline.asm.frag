               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "ah"
               OpMemberName %_UniformBuffer 1 "bh"
               OpMemberName %_UniformBuffer 2 "af"
               OpMemberName %_UniformBuffer 3 "bf"
               OpName %cross_length_2d_ff2f2 "cross_length_2d_ff2f2"
               OpName %cross_length_2d_hh2h2 "cross_length_2d_hh2h2"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 8
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 16
               OpMemberDecorate %_UniformBuffer 3 Offset 24
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %14 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
       %void = OpTypeVoid
         %31 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%cross_length_2d_ff2f2 = OpFunction %float None %14
         %15 = OpFunctionParameter %_ptr_Function_v2float
         %16 = OpFunctionParameter %_ptr_Function_v2float
         %17 = OpLabel
         %19 = OpLoad %v2float %15
         %20 = OpLoad %v2float %16
         %22 = OpCompositeConstruct %mat2v2float %19 %20
         %18 = OpExtInst %float %1 Determinant %22
               OpReturnValue %18
               OpFunctionEnd
%cross_length_2d_hh2h2 = OpFunction %float None %14
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
         %27 = OpLoad %v2float %23
         %28 = OpLoad %v2float %24
         %29 = OpCompositeConstruct %mat2v2float %27 %28
         %26 = OpExtInst %float %1 Determinant %29
               OpReturnValue %26
               OpFunctionEnd
       %main = OpFunction %void None %31
         %32 = OpLabel
         %38 = OpVariable %_ptr_Function_v2float Function
         %42 = OpVariable %_ptr_Function_v2float Function
         %49 = OpVariable %_ptr_Function_v2float Function
         %53 = OpVariable %_ptr_Function_v2float Function
         %33 = OpAccessChain %_ptr_Uniform_v2float %9 %int_0
         %37 = OpLoad %v2float %33
               OpStore %38 %37
         %39 = OpAccessChain %_ptr_Uniform_v2float %9 %int_1
         %41 = OpLoad %v2float %39
               OpStore %42 %41
         %43 = OpFunctionCall %float %cross_length_2d_hh2h2 %38 %42
         %44 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %44 %43
         %46 = OpAccessChain %_ptr_Uniform_v2float %9 %int_2
         %48 = OpLoad %v2float %46
               OpStore %49 %48
         %50 = OpAccessChain %_ptr_Uniform_v2float %9 %int_3
         %52 = OpLoad %v2float %50
               OpStore %53 %52
         %54 = OpFunctionCall %float %cross_length_2d_ff2f2 %49 %53
         %55 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
               OpStore %55 %54
               OpReturn
               OpFunctionEnd
