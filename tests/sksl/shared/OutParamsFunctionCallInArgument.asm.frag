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
               OpName %out_param_func1_vh "out_param_func1_vh"
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
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %testArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %23 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %32 = OpTypeFunction %int %_ptr_Function_float
      %int_1 = OpConstant %int 1
         %40 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%out_param_func1_vh = OpFunction %void None %23
         %24 = OpFunctionParameter %_ptr_Function_float
         %25 = OpLabel
         %26 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %30 = OpLoad %v4float %26
         %31 = OpCompositeExtract %float %30 1
               OpStore %24 %31
               OpReturn
               OpFunctionEnd
%out_param_func2_ih = OpFunction %int None %32
         %33 = OpFunctionParameter %_ptr_Function_float
         %34 = OpLabel
         %35 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %37 = OpLoad %v4float %35
         %38 = OpCompositeExtract %float %37 0
               OpStore %33 %38
         %39 = OpConvertFToS %int %38
               OpReturnValue %39
               OpFunctionEnd
       %main = OpFunction %v4float None %40
         %41 = OpFunctionParameter %_ptr_Function_v2float
         %42 = OpLabel
  %testArray = OpVariable %_ptr_Function__arr_float_int_2 Function
         %48 = OpVariable %_ptr_Function_float Function
         %53 = OpVariable %_ptr_Function_float Function
         %68 = OpVariable %_ptr_Function_v4float Function
         %47 = OpAccessChain %_ptr_Function_float %testArray %int_0
         %49 = OpFunctionCall %int %out_param_func2_ih %48
         %50 = OpLoad %float %48
               OpStore %47 %50
         %51 = OpAccessChain %_ptr_Function_float %testArray %49
         %52 = OpLoad %float %51
               OpStore %53 %52
         %54 = OpFunctionCall %void %out_param_func1_vh %53
         %55 = OpLoad %float %53
               OpStore %51 %55
         %58 = OpAccessChain %_ptr_Function_float %testArray %int_0
         %59 = OpLoad %float %58
         %61 = OpFOrdEqual %bool %59 %float_1
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
         %64 = OpAccessChain %_ptr_Function_float %testArray %int_1
         %65 = OpLoad %float %64
         %66 = OpFOrdEqual %bool %65 %float_1
               OpBranch %63
         %63 = OpLabel
         %67 = OpPhi %bool %false %42 %66 %62
               OpSelectionMerge %72 None
               OpBranchConditional %67 %70 %71
         %70 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %74 = OpLoad %v4float %73
               OpStore %68 %74
               OpBranch %72
         %71 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %76 = OpLoad %v4float %75
               OpStore %68 %76
               OpBranch %72
         %72 = OpLabel
         %77 = OpLoad %v4float %68
               OpReturnValue %77
               OpFunctionEnd
