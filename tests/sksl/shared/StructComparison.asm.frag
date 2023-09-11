               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %S 2 Offset 16
               OpMemberDecorate %S 2 ColMajor
               OpMemberDecorate %S 2 MatrixStride 16
               OpMemberDecorate %S 2 RelaxedPrecision
               OpMemberDecorate %S 3 Offset 48
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %40 = OpConstantComposite %v2float %float_1 %float_0
         %41 = OpConstantComposite %v2float %float_0 %float_1
         %42 = OpConstantComposite %mat2v2float %40 %41
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
         %50 = OpConstantComposite %v2float %float_2 %float_0
         %51 = OpConstantComposite %v2float %float_0 %float_2
         %52 = OpConstantComposite %mat2v2float %50 %51
       %bool = OpTypeBool
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
      %array = OpVariable %_ptr_Function__arr_float_int_5 Function
         %s1 = OpVariable %_ptr_Function_S Function
         %s2 = OpVariable %_ptr_Function_S Function
         %s3 = OpVariable %_ptr_Function_S Function
         %95 = OpVariable %_ptr_Function_v4float Function
         %33 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_4 %float_5
               OpStore %array %33
         %43 = OpCompositeConstruct %S %int_1 %int_2 %42 %33
               OpStore %s1 %43
         %45 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
         %47 = OpLoad %_arr_float_int_5 %45
         %48 = OpCompositeConstruct %S %int_1 %int_2 %42 %47
               OpStore %s2 %48
         %53 = OpCompositeConstruct %S %int_1 %int_2 %52 %33
               OpStore %s3 %53
         %57 = OpLogicalAnd %bool %true %true
         %59 = OpFOrdEqual %v2bool %40 %40
         %60 = OpAll %bool %59
         %61 = OpFOrdEqual %v2bool %41 %41
         %62 = OpAll %bool %61
         %63 = OpLogicalAnd %bool %60 %62
         %64 = OpLogicalAnd %bool %63 %57
         %65 = OpCompositeExtract %float %47 0
         %66 = OpFOrdEqual %bool %float_1 %65
         %67 = OpCompositeExtract %float %47 1
         %68 = OpFOrdEqual %bool %float_2 %67
         %69 = OpLogicalAnd %bool %68 %66
         %70 = OpCompositeExtract %float %47 2
         %71 = OpFOrdEqual %bool %float_3 %70
         %72 = OpLogicalAnd %bool %71 %69
         %73 = OpCompositeExtract %float %47 3
         %74 = OpFOrdEqual %bool %float_4 %73
         %75 = OpLogicalAnd %bool %74 %72
         %76 = OpCompositeExtract %float %47 4
         %77 = OpFOrdEqual %bool %float_5 %76
         %78 = OpLogicalAnd %bool %77 %75
         %79 = OpLogicalAnd %bool %78 %64
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %82 = OpLogicalOr %bool %false %false
         %83 = OpFUnordNotEqual %v2bool %40 %50
         %84 = OpAny %bool %83
         %85 = OpFUnordNotEqual %v2bool %41 %51
         %86 = OpAny %bool %85
         %87 = OpLogicalOr %bool %84 %86
         %88 = OpLogicalOr %bool %87 %82
         %89 = OpLogicalOr %bool %false %false
         %90 = OpLogicalOr %bool %false %89
         %91 = OpLogicalOr %bool %false %90
         %92 = OpLogicalOr %bool %false %91
         %93 = OpLogicalOr %bool %92 %88
               OpBranch %81
         %81 = OpLabel
         %94 = OpPhi %bool %false %25 %93 %80
               OpSelectionMerge %99 None
               OpBranchConditional %94 %97 %98
         %97 = OpLabel
        %100 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %103 = OpLoad %v4float %100
               OpStore %95 %103
               OpBranch %99
         %98 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %105 = OpLoad %v4float %104
               OpStore %95 %105
               OpBranch %99
         %99 = OpLabel
        %106 = OpLoad %v4float %95
               OpReturnValue %106
               OpFunctionEnd
