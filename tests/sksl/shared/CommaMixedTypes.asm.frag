               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %result "result"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %result RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %float
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
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
    %float_2 = OpConstant %float 2
         %41 = OpConstantComposite %v2float %float_2 %float_2
    %float_3 = OpConstant %float 3
    %v3float = OpTypeVector %float 3
         %48 = OpConstantComposite %v3float %float_3 %float_3 %float_3
      %int_2 = OpConstant %int 2
    %float_4 = OpConstant %float 4
         %55 = OpConstantComposite %v2float %float_4 %float_0
         %56 = OpConstantComposite %v2float %float_0 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
         %58 = OpConstantComposite %mat2v2float %55 %56
      %int_3 = OpConstant %int 3
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
         %28 = OpAccessChain %_ptr_Uniform_float %10 %int_1
         %32 = OpLoad %float %28
         %33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %36 = OpLoad %v4float %33
         %37 = OpCompositeExtract %float %36 0
         %38 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %38 %37
         %42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpCompositeExtract %float %43 1
         %45 = OpAccessChain %_ptr_Function_float %result %int_1
               OpStore %45 %44
         %49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %50 = OpLoad %v4float %49
         %51 = OpCompositeExtract %float %50 2
         %52 = OpAccessChain %_ptr_Function_float %result %int_2
               OpStore %52 %51
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpCompositeExtract %float %60 3
         %62 = OpAccessChain %_ptr_Function_float %result %int_3
               OpStore %62 %61
         %64 = OpLoad %v4float %result
               OpReturnValue %64
               OpFunctionEnd
