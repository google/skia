               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "ah"
               OpMemberName %_UniformBuffer 1 "bh"
               OpMemberName %_UniformBuffer 2 "af"
               OpMemberName %_UniformBuffer 3 "bf"
               OpName %cross_length_2d_ff2f2 "cross_length_2d_ff2f2"
               OpName %cross_length_2d_hh2h2 "cross_length_2d_hh2h2"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
       %void = OpTypeVoid
         %34 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%cross_length_2d_ff2f2 = OpFunction %float None %17
         %18 = OpFunctionParameter %_ptr_Function_v2float
         %19 = OpFunctionParameter %_ptr_Function_v2float
         %20 = OpLabel
         %22 = OpLoad %v2float %18
         %23 = OpLoad %v2float %19
         %25 = OpCompositeConstruct %mat2v2float %22 %23
         %21 = OpExtInst %float %1 Determinant %25
               OpReturnValue %21
               OpFunctionEnd
%cross_length_2d_hh2h2 = OpFunction %float None %17
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpFunctionParameter %_ptr_Function_v2float
         %28 = OpLabel
         %30 = OpLoad %v2float %26
         %31 = OpLoad %v2float %27
         %32 = OpCompositeConstruct %mat2v2float %30 %31
         %29 = OpExtInst %float %1 Determinant %32
               OpReturnValue %29
               OpFunctionEnd
       %main = OpFunction %void None %34
         %35 = OpLabel
         %41 = OpVariable %_ptr_Function_v2float Function
         %45 = OpVariable %_ptr_Function_v2float Function
         %52 = OpVariable %_ptr_Function_v2float Function
         %56 = OpVariable %_ptr_Function_v2float Function
         %36 = OpAccessChain %_ptr_Uniform_v2float %12 %int_0
         %40 = OpLoad %v2float %36
               OpStore %41 %40
         %42 = OpAccessChain %_ptr_Uniform_v2float %12 %int_1
         %44 = OpLoad %v2float %42
               OpStore %45 %44
         %46 = OpFunctionCall %float %cross_length_2d_hh2h2 %41 %45
         %47 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %47 %46
         %49 = OpAccessChain %_ptr_Uniform_v2float %12 %int_2
         %51 = OpLoad %v2float %49
               OpStore %52 %51
         %53 = OpAccessChain %_ptr_Uniform_v2float %12 %int_3
         %55 = OpLoad %v2float %53
               OpStore %56 %55
         %57 = OpFunctionCall %float %cross_length_2d_ff2f2 %52 %56
         %58 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
               OpStore %58 %57
               OpReturn
               OpFunctionEnd
