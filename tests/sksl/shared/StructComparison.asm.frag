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
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %array "array"
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "y"
               OpMemberName %S 2 "m"
               OpMemberName %S 3 "a"
               OpName %s1 "s1"
               OpName %s2 "s2"
               OpName %s3 "s3"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %S 2 Offset 16
               OpMemberDecorate %S 2 ColMajor
               OpMemberDecorate %S 2 MatrixStride 16
               OpMemberDecorate %S 2 RelaxedPrecision
               OpMemberDecorate %S 3 Offset 48
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_5 = OpTypePointer Function %_arr_float_int_5
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
%mat2v2float = OpTypeMatrix %v2float 2
          %S = OpTypeStruct %int %int %mat2v2float %_arr_float_int_5
%_ptr_Function_S = OpTypePointer Function %S
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
         %43 = OpConstantComposite %v2float %float_1 %float_0
         %44 = OpConstantComposite %v2float %float_0 %float_1
         %45 = OpConstantComposite %mat2v2float %43 %44
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
         %53 = OpConstantComposite %v2float %float_2 %float_0
         %54 = OpConstantComposite %v2float %float_0 %float_2
         %55 = OpConstantComposite %mat2v2float %53 %54
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_Function_v2float
         %28 = OpLabel
      %array = OpVariable %_ptr_Function__arr_float_int_5 Function
         %s1 = OpVariable %_ptr_Function_S Function
         %s2 = OpVariable %_ptr_Function_S Function
         %s3 = OpVariable %_ptr_Function_S Function
         %97 = OpVariable %_ptr_Function_v4float Function
         %36 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_4 %float_5
               OpStore %array %36
         %46 = OpCompositeConstruct %S %int_1 %int_2 %45 %36
               OpStore %s1 %46
         %48 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
         %50 = OpLoad %_arr_float_int_5 %48
         %51 = OpCompositeConstruct %S %int_1 %int_2 %45 %50
               OpStore %s2 %51
         %56 = OpCompositeConstruct %S %int_1 %int_2 %55 %36
               OpStore %s3 %56
         %59 = OpLogicalAnd %bool %true %true
         %61 = OpFOrdEqual %v2bool %43 %43
         %62 = OpAll %bool %61
         %63 = OpFOrdEqual %v2bool %44 %44
         %64 = OpAll %bool %63
         %65 = OpLogicalAnd %bool %62 %64
         %66 = OpLogicalAnd %bool %65 %59
         %67 = OpCompositeExtract %float %50 0
         %68 = OpFOrdEqual %bool %float_1 %67
         %69 = OpCompositeExtract %float %50 1
         %70 = OpFOrdEqual %bool %float_2 %69
         %71 = OpLogicalAnd %bool %70 %68
         %72 = OpCompositeExtract %float %50 2
         %73 = OpFOrdEqual %bool %float_3 %72
         %74 = OpLogicalAnd %bool %73 %71
         %75 = OpCompositeExtract %float %50 3
         %76 = OpFOrdEqual %bool %float_4 %75
         %77 = OpLogicalAnd %bool %76 %74
         %78 = OpCompositeExtract %float %50 4
         %79 = OpFOrdEqual %bool %float_5 %78
         %80 = OpLogicalAnd %bool %79 %77
         %81 = OpLogicalAnd %bool %80 %66
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %84 = OpLogicalOr %bool %false %false
         %85 = OpFUnordNotEqual %v2bool %43 %53
         %86 = OpAny %bool %85
         %87 = OpFUnordNotEqual %v2bool %44 %54
         %88 = OpAny %bool %87
         %89 = OpLogicalOr %bool %86 %88
         %90 = OpLogicalOr %bool %89 %84
         %91 = OpLogicalOr %bool %false %false
         %92 = OpLogicalOr %bool %false %91
         %93 = OpLogicalOr %bool %false %92
         %94 = OpLogicalOr %bool %false %93
         %95 = OpLogicalOr %bool %94 %90
               OpBranch %83
         %83 = OpLabel
         %96 = OpPhi %bool %false %28 %95 %82
               OpSelectionMerge %101 None
               OpBranchConditional %96 %99 %100
         %99 = OpLabel
        %102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %105 = OpLoad %v4float %102
               OpStore %97 %105
               OpBranch %101
        %100 = OpLabel
        %106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %107 = OpLoad %v4float %106
               OpStore %97 %107
               OpBranch %101
        %101 = OpLabel
        %108 = OpLoad %v4float %97
               OpReturnValue %108
               OpFunctionEnd
