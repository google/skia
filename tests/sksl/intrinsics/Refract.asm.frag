               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpMemberName %_UniformBuffer 2 "c"
               OpMemberName %_UniformBuffer 3 "d"
               OpMemberName %_UniformBuffer 4 "e"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %result "result"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 4
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 8
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 16
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 32
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float %float %float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6_00000015e_26 = OpConstant %float 6.00000015e+26
    %float_2 = OpConstant %float 2
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
  %float_0_5 = OpConstant %float 0.5
%float_n0_866025388 = OpConstant %float -0.866025388
         %58 = OpConstantComposite %v2float %float_0_5 %float_n0_866025388
    %v3float = OpTypeVector %float 3
         %62 = OpConstantComposite %v3float %float_0_5 %float_0 %float_n0_866025388
         %65 = OpConstantComposite %v4float %float_0_5 %float_0 %float_0 %float_n0_866025388
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %28 = OpExtInst %float %1 Refract %float_6_00000015e_26 %float_2 %float_2
         %31 = OpCompositeConstruct %v4float %28 %28 %28 %28
               OpStore %result %31
         %33 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %37 = OpLoad %float %33
         %38 = OpAccessChain %_ptr_Uniform_float %10 %int_1
         %40 = OpLoad %float %38
         %41 = OpAccessChain %_ptr_Uniform_float %10 %int_2
         %43 = OpLoad %float %41
         %32 = OpExtInst %float %1 Refract %37 %40 %43
         %44 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %44 %32
         %47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %50 = OpLoad %v4float %47
         %51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
         %53 = OpLoad %v4float %51
         %54 = OpAccessChain %_ptr_Uniform_float %10 %int_2
         %55 = OpLoad %float %54
         %46 = OpExtInst %v4float %1 Refract %50 %53 %55
               OpStore %result %46
         %59 = OpLoad %v4float %result
         %60 = OpVectorShuffle %v4float %59 %58 4 5 2 3
               OpStore %result %60
         %63 = OpLoad %v4float %result
         %64 = OpVectorShuffle %v4float %63 %62 4 5 6 3
               OpStore %result %64
               OpStore %result %65
               OpReturnValue %65
               OpFunctionEnd
