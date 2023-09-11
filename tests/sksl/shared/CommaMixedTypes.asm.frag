               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %result "result"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %float
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
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_2 = OpConstant %float 2
         %38 = OpConstantComposite %v2float %float_2 %float_2
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %45 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_2 = OpConstant %int 2
    %float_4 = OpConstant %float 4
         %52 = OpConstantComposite %v2float %float_4 %float_0
         %53 = OpConstantComposite %v2float %float_0 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
         %55 = OpConstantComposite %mat2v2float %52 %53
      %int_3 = OpConstant %int 3
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
         %25 = OpAccessChain %_ptr_Uniform_float %7 %int_1
         %29 = OpLoad %float %25
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %33 = OpLoad %v4float %30
         %34 = OpCompositeExtract %float %33 0
         %35 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %35 %34
         %39 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %40 = OpLoad %v4float %39
         %41 = OpCompositeExtract %float %40 1
         %42 = OpAccessChain %_ptr_Function_float %result %int_1
               OpStore %42 %41
         %46 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpCompositeExtract %float %47 2
         %49 = OpAccessChain %_ptr_Function_float %result %int_2
               OpStore %49 %48
         %56 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %57 = OpLoad %v4float %56
         %58 = OpCompositeExtract %float %57 3
         %59 = OpAccessChain %_ptr_Function_float %result %int_3
               OpStore %59 %58
         %61 = OpLoad %v4float %result
               OpReturnValue %61
               OpFunctionEnd
