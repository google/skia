               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %x "x"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %out_params_are_distinct_from_global_bh "out_params_are_distinct_from_global_bh"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
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
%_ptr_Private_float = OpTypePointer Private %float
          %x = OpVariable %_ptr_Private_float Private
    %float_1 = OpConstant %float 1
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %28 = OpTypeFunction %bool %_ptr_Function_float
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %39 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %19
         %20 = OpLabel
         %24 = OpVariable %_ptr_Function_v2float Function
               OpStore %24 %23
         %26 = OpFunctionCall %v4float %main %24
               OpStore %sk_FragColor %26
               OpReturn
               OpFunctionEnd
%out_params_are_distinct_from_global_bh = OpFunction %bool None %28
         %29 = OpFunctionParameter %_ptr_Function_float
         %30 = OpLabel
               OpStore %29 %float_2
         %33 = OpLoad %float %x
         %34 = OpFOrdEqual %bool %33 %float_1
               OpSelectionMerge %36 None
               OpBranchConditional %34 %35 %36
         %35 = OpLabel
               OpBranch %36
         %36 = OpLabel
         %38 = OpPhi %bool %false %30 %true %35
               OpReturnValue %38
               OpFunctionEnd
       %main = OpFunction %v4float None %39
         %40 = OpFunctionParameter %_ptr_Function_v2float
         %41 = OpLabel
         %42 = OpVariable %_ptr_Function_float Function
         %45 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
         %43 = OpFunctionCall %bool %out_params_are_distinct_from_global_bh %42
         %44 = OpLoad %float %42
               OpStore %x %44
               OpSelectionMerge %49 None
               OpBranchConditional %43 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
         %54 = OpLoad %v4float %50
               OpStore %45 %54
               OpBranch %49
         %48 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
         %57 = OpLoad %v4float %55
               OpStore %45 %57
               OpBranch %49
         %49 = OpLabel
         %58 = OpLoad %v4float %45
               OpReturnValue %58
               OpFunctionEnd
