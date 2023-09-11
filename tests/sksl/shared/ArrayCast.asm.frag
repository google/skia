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
               OpName %f "f"
               OpName %h "h"
               OpName %i3 "i3"
               OpName %s3 "s3"
               OpName %h2x2 "h2x2"
               OpName %f2x2 "f2x2"
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
               OpDecorate %h RelaxedPrecision
               OpDecorate %_arr_v3int_int_3 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
               OpDecorate %96 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
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
      %v3int = OpTypeVector %int 3
      %int_3 = OpConstant %int 3
%_arr_v3int_int_3 = OpTypeArray %v3int %int_3
%_ptr_Function__arr_v3int_int_3 = OpTypePointer Function %_arr_v3int_int_3
      %int_1 = OpConstant %int 1
         %40 = OpConstantComposite %v3int %int_1 %int_1 %int_1
      %int_2 = OpConstant %int 2
         %42 = OpConstantComposite %v3int %int_2 %int_2 %int_2
         %43 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
         %50 = OpConstantComposite %v2float %float_1 %float_2
         %51 = OpConstantComposite %v2float %float_3 %float_4
         %52 = OpConstantComposite %mat2v2float %50 %51
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
         %57 = OpConstantComposite %v2float %float_5 %float_6
         %58 = OpConstantComposite %v2float %float_7 %float_8
         %59 = OpConstantComposite %mat2v2float %57 %58
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
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
          %f = OpVariable %_ptr_Function__arr_float_int_4 Function
          %h = OpVariable %_ptr_Function__arr_float_int_4 Function
         %i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
         %s3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
       %h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
       %f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
         %88 = OpVariable %_ptr_Function_v4float Function
         %32 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
               OpStore %f %32
               OpStore %h %32
               OpStore %f %32
               OpStore %h %32
         %44 = OpCompositeConstruct %_arr_v3int_int_3 %40 %42 %43
               OpStore %i3 %44
               OpStore %s3 %44
               OpStore %i3 %44
               OpStore %s3 %44
         %60 = OpCompositeConstruct %_arr_mat2v2float_int_2 %52 %59
               OpStore %h2x2 %60
               OpStore %f2x2 %60
               OpStore %f2x2 %60
               OpStore %h2x2 %60
         %65 = OpLogicalAnd %bool %true %true
         %66 = OpLogicalAnd %bool %true %65
         %67 = OpLogicalAnd %bool %true %66
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
         %70 = OpLogicalAnd %bool %true %true
         %71 = OpLogicalAnd %bool %true %70
               OpBranch %69
         %69 = OpLabel
         %72 = OpPhi %bool %false %22 %71 %68
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %76 = OpFOrdEqual %v2bool %50 %50
         %77 = OpAll %bool %76
         %78 = OpFOrdEqual %v2bool %51 %51
         %79 = OpAll %bool %78
         %80 = OpLogicalAnd %bool %77 %79
         %81 = OpFOrdEqual %v2bool %57 %57
         %82 = OpAll %bool %81
         %83 = OpFOrdEqual %v2bool %58 %58
         %84 = OpAll %bool %83
         %85 = OpLogicalAnd %bool %82 %84
         %86 = OpLogicalAnd %bool %85 %80
               OpBranch %74
         %74 = OpLabel
         %87 = OpPhi %bool %false %69 %86 %73
               OpSelectionMerge %92 None
               OpBranchConditional %87 %90 %91
         %90 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %96 = OpLoad %v4float %93
               OpStore %88 %96
               OpBranch %92
         %91 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %98 = OpLoad %v4float %97
               OpStore %88 %98
               OpBranch %92
         %92 = OpLabel
         %99 = OpLoad %v4float %88
               OpReturnValue %99
               OpFunctionEnd
