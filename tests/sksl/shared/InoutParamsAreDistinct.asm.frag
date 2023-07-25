               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %inout_params_are_distinct_bhh "inout_params_are_distinct_bhh"
               OpName %main "main"
               OpName %x "x"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %25 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %36 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
%inout_params_are_distinct_bhh = OpFunction %bool None %25
         %26 = OpFunctionParameter %_ptr_Function_float
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpLabel
               OpStore %26 %float_1
               OpStore %27 %float_2
               OpSelectionMerge %34 None
               OpBranchConditional %true %33 %34
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
         %35 = OpPhi %bool %false %28 %true %33
               OpReturnValue %35
               OpFunctionEnd
       %main = OpFunction %v4float None %36
         %37 = OpFunctionParameter %_ptr_Function_v2float
         %38 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
         %40 = OpVariable %_ptr_Function_float Function
         %41 = OpVariable %_ptr_Function_float Function
         %45 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_0
               OpStore %40 %float_0
               OpStore %41 %float_0
         %42 = OpFunctionCall %bool %inout_params_are_distinct_bhh %40 %41
         %43 = OpLoad %float %40
               OpStore %x %43
         %44 = OpLoad %float %41
               OpStore %x %44
               OpSelectionMerge %49 None
               OpBranchConditional %42 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 = OpLoad %v4float %50
               OpStore %45 %54
               OpBranch %49
         %48 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %57 = OpLoad %v4float %55
               OpStore %45 %57
               OpBranch %49
         %49 = OpLabel
         %58 = OpLoad %v4float %45
               OpReturnValue %58
               OpFunctionEnd
