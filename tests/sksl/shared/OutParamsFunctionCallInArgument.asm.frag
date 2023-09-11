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
               OpName %out_param_func1_vh "out_param_func1_vh"
               OpName %out_param_func2_ih "out_param_func2_ih"
               OpName %main "main"
               OpName %testArray "testArray"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %testArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %26 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %35 = OpTypeFunction %int %_ptr_Function_float
      %int_1 = OpConstant %int 1
         %43 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%out_param_func1_vh = OpFunction %void None %26
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpLabel
         %29 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpCompositeExtract %float %33 1
               OpStore %27 %34
               OpReturn
               OpFunctionEnd
%out_param_func2_ih = OpFunction %int None %35
         %36 = OpFunctionParameter %_ptr_Function_float
         %37 = OpLabel
         %38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %40 = OpLoad %v4float %38
         %41 = OpCompositeExtract %float %40 0
               OpStore %36 %41
         %42 = OpConvertFToS %int %41
               OpReturnValue %42
               OpFunctionEnd
       %main = OpFunction %v4float None %43
         %44 = OpFunctionParameter %_ptr_Function_v2float
         %45 = OpLabel
  %testArray = OpVariable %_ptr_Function__arr_float_int_2 Function
         %51 = OpVariable %_ptr_Function_float Function
         %56 = OpVariable %_ptr_Function_float Function
         %70 = OpVariable %_ptr_Function_v4float Function
         %50 = OpAccessChain %_ptr_Function_float %testArray %int_0
         %52 = OpFunctionCall %int %out_param_func2_ih %51
         %53 = OpLoad %float %51
               OpStore %50 %53
         %54 = OpAccessChain %_ptr_Function_float %testArray %52
         %55 = OpLoad %float %54
               OpStore %56 %55
         %57 = OpFunctionCall %void %out_param_func1_vh %56
         %58 = OpLoad %float %56
               OpStore %54 %58
         %60 = OpAccessChain %_ptr_Function_float %testArray %int_0
         %61 = OpLoad %float %60
         %63 = OpFOrdEqual %bool %61 %float_1
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %66 = OpAccessChain %_ptr_Function_float %testArray %int_1
         %67 = OpLoad %float %66
         %68 = OpFOrdEqual %bool %67 %float_1
               OpBranch %65
         %65 = OpLabel
         %69 = OpPhi %bool %false %45 %68 %64
               OpSelectionMerge %74 None
               OpBranchConditional %69 %72 %73
         %72 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %76 = OpLoad %v4float %75
               OpStore %70 %76
               OpBranch %74
         %73 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %78 = OpLoad %v4float %77
               OpStore %70 %78
               OpBranch %74
         %74 = OpLabel
         %79 = OpLoad %v4float %70
               OpReturnValue %79
               OpFunctionEnd
