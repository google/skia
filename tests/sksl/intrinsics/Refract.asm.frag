               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float %float %float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %55 = OpConstantComposite %v2float %float_0_5 %float_n0_866025388
    %v3float = OpTypeVector %float 3
         %59 = OpConstantComposite %v3float %float_0_5 %float_0 %float_n0_866025388
         %62 = OpConstantComposite %v4float %float_0_5 %float_0 %float_0 %float_n0_866025388
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
     %result = OpVariable %_ptr_Function_v4float Function
         %25 = OpExtInst %float %1 Refract %float_6_00000015e_26 %float_2 %float_2
         %28 = OpCompositeConstruct %v4float %25 %25 %25 %25
               OpStore %result %28
         %30 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %34 = OpLoad %float %30
         %35 = OpAccessChain %_ptr_Uniform_float %7 %int_1
         %37 = OpLoad %float %35
         %38 = OpAccessChain %_ptr_Uniform_float %7 %int_2
         %40 = OpLoad %float %38
         %29 = OpExtInst %float %1 Refract %34 %37 %40
         %41 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %41 %29
         %44 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %47 = OpLoad %v4float %44
         %48 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
         %50 = OpLoad %v4float %48
         %51 = OpAccessChain %_ptr_Uniform_float %7 %int_2
         %52 = OpLoad %float %51
         %43 = OpExtInst %v4float %1 Refract %47 %50 %52
               OpStore %result %43
         %56 = OpLoad %v4float %result
         %57 = OpVectorShuffle %v4float %56 %55 4 5 2 3
               OpStore %result %57
         %60 = OpLoad %v4float %result
         %61 = OpVectorShuffle %v4float %60 %59 4 5 6 3
               OpStore %result %61
               OpStore %result %62
               OpReturnValue %62
               OpFunctionEnd
