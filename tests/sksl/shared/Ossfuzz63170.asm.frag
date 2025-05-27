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
               OpName %out_param_func2_ih "out_param_func2_ih"
               OpName %main "main"
               OpName %testArray "testArray"
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
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %testArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
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
        %int = OpTypeInt 32 1
%_ptr_Function_float = OpTypePointer Function %float
         %23 = OpTypeFunction %int %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
         %32 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%out_param_func2_ih = OpFunction %int None %23
         %24 = OpFunctionParameter %_ptr_Function_float
         %25 = OpLabel
         %26 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %29 = OpLoad %v4float %26
         %30 = OpCompositeExtract %float %29 0
               OpStore %24 %30
         %31 = OpConvertFToS %int %30
               OpReturnValue %31
               OpFunctionEnd
       %main = OpFunction %v4float None %32
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
  %testArray = OpVariable %_ptr_Function__arr_float_int_2 Function
         %41 = OpVariable %_ptr_Function_float Function
         %58 = OpVariable %_ptr_Function_v4float Function
         %40 = OpAccessChain %_ptr_Function_float %testArray %int_0
         %42 = OpFunctionCall %int %out_param_func2_ih %41
         %43 = OpLoad %float %41
               OpStore %40 %43
         %44 = OpAccessChain %_ptr_Function_float %testArray %42
         %45 = OpLoad %float %44
         %48 = OpAccessChain %_ptr_Function_float %testArray %int_0
         %49 = OpLoad %float %48
         %51 = OpFOrdEqual %bool %49 %float_1
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %54 = OpAccessChain %_ptr_Function_float %testArray %int_1
         %55 = OpLoad %float %54
         %56 = OpFOrdEqual %bool %55 %float_1
               OpBranch %53
         %53 = OpLabel
         %57 = OpPhi %bool %false %34 %56 %52
               OpSelectionMerge %62 None
               OpBranchConditional %57 %60 %61
         %60 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %64 = OpLoad %v4float %63
               OpStore %58 %64
               OpBranch %62
         %61 = OpLabel
         %65 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %66 = OpLoad %v4float %65
               OpStore %58 %66
               OpBranch %62
         %62 = OpLabel
         %67 = OpLoad %v4float %58
               OpReturnValue %67
               OpFunctionEnd
