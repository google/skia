               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %out_params_are_distinct_bhh "out_params_are_distinct_bhh"
               OpName %main "main"
               OpName %x "x"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
         %23 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %34 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%out_params_are_distinct_bhh = OpFunction %bool None %23
         %24 = OpFunctionParameter %_ptr_Function_float
         %25 = OpFunctionParameter %_ptr_Function_float
         %26 = OpLabel
               OpStore %24 %float_1
               OpStore %25 %float_2
               OpSelectionMerge %32 None
               OpBranchConditional %true %31 %32
         %31 = OpLabel
               OpBranch %32
         %32 = OpLabel
         %33 = OpPhi %bool %false %26 %true %31
               OpReturnValue %33
               OpFunctionEnd
       %main = OpFunction %v4float None %34
         %35 = OpFunctionParameter %_ptr_Function_v2float
         %36 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
         %38 = OpVariable %_ptr_Function_float Function
         %39 = OpVariable %_ptr_Function_float Function
         %43 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_0
         %40 = OpFunctionCall %bool %out_params_are_distinct_bhh %38 %39
         %41 = OpLoad %float %38
               OpStore %x %41
         %42 = OpLoad %float %39
               OpStore %x %42
               OpSelectionMerge %47 None
               OpBranchConditional %40 %45 %46
         %45 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %52 = OpLoad %v4float %48
               OpStore %43 %52
               OpBranch %47
         %46 = OpLabel
         %53 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %55 = OpLoad %v4float %53
               OpStore %43 %55
               OpBranch %47
         %47 = OpLabel
         %56 = OpLoad %v4float %43
               OpReturnValue %56
               OpFunctionEnd
