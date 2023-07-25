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
               OpName %main "main"
               OpName %f "f"
               OpName %h "h"
               OpName %i3 "i3"
               OpName %s3 "s3"
               OpName %h2x2 "h2x2"
               OpName %f2x2 "f2x2"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %_arr_float_int_4 ArrayStride 16
               OpDecorate %h RelaxedPrecision
               OpDecorate %_arr_v3int_int_3 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_2 ArrayStride 32
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %43 = OpConstantComposite %v3int %int_1 %int_1 %int_1
      %int_2 = OpConstant %int 2
         %45 = OpConstantComposite %v3int %int_2 %int_2 %int_2
         %46 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_2 = OpTypeArray %mat2v2float %int_2
%_ptr_Function__arr_mat2v2float_int_2 = OpTypePointer Function %_arr_mat2v2float_int_2
         %53 = OpConstantComposite %v2float %float_1 %float_2
         %54 = OpConstantComposite %v2float %float_3 %float_4
         %55 = OpConstantComposite %mat2v2float %53 %54
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
         %60 = OpConstantComposite %v2float %float_5 %float_6
         %61 = OpConstantComposite %v2float %float_7 %float_8
         %62 = OpConstantComposite %mat2v2float %60 %61
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
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
          %f = OpVariable %_ptr_Function__arr_float_int_4 Function
          %h = OpVariable %_ptr_Function__arr_float_int_4 Function
         %i3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
         %s3 = OpVariable %_ptr_Function__arr_v3int_int_3 Function
       %h2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
       %f2x2 = OpVariable %_ptr_Function__arr_mat2v2float_int_2 Function
         %90 = OpVariable %_ptr_Function_v4float Function
         %35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
               OpStore %f %35
               OpStore %h %35
               OpStore %f %35
               OpStore %h %35
         %47 = OpCompositeConstruct %_arr_v3int_int_3 %43 %45 %46
               OpStore %i3 %47
               OpStore %s3 %47
               OpStore %i3 %47
               OpStore %s3 %47
         %63 = OpCompositeConstruct %_arr_mat2v2float_int_2 %55 %62
               OpStore %h2x2 %63
               OpStore %f2x2 %63
               OpStore %f2x2 %63
               OpStore %h2x2 %63
         %67 = OpLogicalAnd %bool %true %true
         %68 = OpLogicalAnd %bool %true %67
         %69 = OpLogicalAnd %bool %true %68
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %72 = OpLogicalAnd %bool %true %true
         %73 = OpLogicalAnd %bool %true %72
               OpBranch %71
         %71 = OpLabel
         %74 = OpPhi %bool %false %25 %73 %70
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpFOrdEqual %v2bool %53 %53
         %79 = OpAll %bool %78
         %80 = OpFOrdEqual %v2bool %54 %54
         %81 = OpAll %bool %80
         %82 = OpLogicalAnd %bool %79 %81
         %83 = OpFOrdEqual %v2bool %60 %60
         %84 = OpAll %bool %83
         %85 = OpFOrdEqual %v2bool %61 %61
         %86 = OpAll %bool %85
         %87 = OpLogicalAnd %bool %84 %86
         %88 = OpLogicalAnd %bool %87 %82
               OpBranch %76
         %76 = OpLabel
         %89 = OpPhi %bool %false %71 %88 %75
               OpSelectionMerge %94 None
               OpBranchConditional %89 %92 %93
         %92 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %98 = OpLoad %v4float %95
               OpStore %90 %98
               OpBranch %94
         %93 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %100 = OpLoad %v4float %99
               OpStore %90 %100
               OpBranch %94
         %94 = OpLabel
        %101 = OpLoad %v4float %90
               OpReturnValue %101
               OpFunctionEnd
