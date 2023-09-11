               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %x "x"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %out_params_are_distinct_from_global_bh "out_params_are_distinct_from_global_bh"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %31 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_float = OpTypePointer Private %float
          %x = OpVariable %_ptr_Private_float Private
    %float_1 = OpConstant %float 1
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
         %26 = OpTypeFunction %bool %_ptr_Function_float
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %37 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%out_params_are_distinct_from_global_bh = OpFunction %bool None %26
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpLabel
               OpStore %27 %float_2
         %31 = OpLoad %float %x
         %32 = OpFOrdEqual %bool %31 %float_1
               OpSelectionMerge %34 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
               OpBranch %34
         %34 = OpLabel
         %36 = OpPhi %bool %false %28 %true %33
               OpReturnValue %36
               OpFunctionEnd
       %main = OpFunction %v4float None %37
         %38 = OpFunctionParameter %_ptr_Function_v2float
         %39 = OpLabel
         %40 = OpVariable %_ptr_Function_float Function
         %43 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
         %41 = OpFunctionCall %bool %out_params_are_distinct_from_global_bh %40
         %42 = OpLoad %float %40
               OpStore %x %42
               OpSelectionMerge %47 None
               OpBranchConditional %41 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %52 = OpLoad %v4float %48
               OpStore %43 %52
               OpBranch %47
         %46 = OpLabel
         %53 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %55 = OpLoad %v4float %53
               OpStore %43 %55
               OpBranch %47
         %47 = OpLabel
         %56 = OpLoad %v4float %43
               OpReturnValue %56
               OpFunctionEnd
