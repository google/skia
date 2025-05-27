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
               OpName %main "main"
               OpName %test1 "test1"
               OpName %test2 "test2"
               OpName %test3 "test3"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %_arr_float_int_4 ArrayStride 16
               OpDecorate %_arr_v2float_int_2 ArrayStride 16
               OpDecorate %_arr_mat4v4float_int_1 ArrayStride 64
               OpDecorate %75 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
         %37 = OpConstantComposite %v2float %float_1 %float_2
         %38 = OpConstantComposite %v2float %float_3 %float_4
%mat4v4float = OpTypeMatrix %v4float 4
      %int_1 = OpConstant %int 1
%_arr_mat4v4float_int_1 = OpTypeArray %mat4v4float %int_1
%_ptr_Function__arr_mat4v4float_int_1 = OpTypePointer Function %_arr_mat4v4float_int_1
   %float_16 = OpConstant %float 16
         %46 = OpConstantComposite %v4float %float_16 %float_0 %float_0 %float_0
         %47 = OpConstantComposite %v4float %float_0 %float_16 %float_0 %float_0
         %48 = OpConstantComposite %v4float %float_0 %float_0 %float_16 %float_0
         %49 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_16
         %50 = OpConstantComposite %mat4v4float %46 %47 %48 %49
      %int_3 = OpConstant %int 3
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %float_24 = OpConstant %float 24
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
      %test1 = OpVariable %_ptr_Function__arr_float_int_4 Function
      %test2 = OpVariable %_ptr_Function__arr_v2float_int_2 Function
      %test3 = OpVariable %_ptr_Function__arr_mat4v4float_int_1 Function
         %69 = OpVariable %_ptr_Function_v4float Function
         %32 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
               OpStore %test1 %32
         %39 = OpCompositeConstruct %_arr_v2float_int_2 %37 %38
               OpStore %test2 %39
         %51 = OpCompositeConstruct %_arr_mat4v4float_int_1 %50
               OpStore %test3 %51
         %53 = OpAccessChain %_ptr_Function_float %test1 %int_3
         %55 = OpLoad %float %53
         %56 = OpAccessChain %_ptr_Function_v2float %test2 %int_1
         %57 = OpLoad %v2float %56
         %58 = OpCompositeExtract %float %57 1
         %59 = OpFAdd %float %55 %58
         %61 = OpAccessChain %_ptr_Function_v4float %test3 %int_0 %int_3
         %63 = OpLoad %v4float %61
         %64 = OpCompositeExtract %float %63 3
         %65 = OpFAdd %float %59 %64
         %67 = OpFOrdEqual %bool %65 %float_24
               OpSelectionMerge %72 None
               OpBranchConditional %67 %70 %71
         %70 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %75 = OpLoad %v4float %73
               OpStore %69 %75
               OpBranch %72
         %71 = OpLabel
         %76 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %77 = OpLoad %v4float %76
               OpStore %69 %77
               OpBranch %72
         %72 = OpLabel
         %78 = OpLoad %v4float %69
               OpReturnValue %78
               OpFunctionEnd
