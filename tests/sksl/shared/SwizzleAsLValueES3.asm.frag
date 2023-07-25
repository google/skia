               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %gAccessCount "gAccessCount"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %Z_i "Z_i"
               OpName %main "main"
               OpName %array "array"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %_arr_v4float_int_1 ArrayStride 16
               OpDecorate %41 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%gAccessCount = OpVariable %_ptr_Private_int Private
      %int_0 = OpConstant %int 0
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %28 = OpTypeFunction %int
      %int_1 = OpConstant %int 1
         %33 = OpTypeFunction %v4float %_ptr_Function_v2float
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
  %float_0_5 = OpConstant %float 0.5
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
      %int_3 = OpConstant %int 3
    %float_4 = OpConstant %float 4
    %v3float = OpTypeVector %float 3
         %64 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %65 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %66 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
%mat3v3float = OpTypeMatrix %v3float 3
         %68 = OpConstantComposite %mat3v3float %64 %65 %66
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
         %78 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
    %float_1 = OpConstant %float 1
      %false = OpConstantFalse %bool
      %int_8 = OpConstant %int 8
        %112 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
     %v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %20
         %21 = OpLabel
         %25 = OpVariable %_ptr_Function_v2float Function
               OpStore %25 %24
         %27 = OpFunctionCall %v4float %main %25
               OpStore %sk_FragColor %27
               OpReturn
               OpFunctionEnd
        %Z_i = OpFunction %int None %28
         %29 = OpLabel
         %31 = OpLoad %int %gAccessCount
         %32 = OpIAdd %int %31 %int_1
               OpStore %gAccessCount %32
               OpReturnValue %int_0
               OpFunctionEnd
       %main = OpFunction %v4float None %33
         %34 = OpFunctionParameter %_ptr_Function_v2float
         %35 = OpLabel
      %array = OpVariable %_ptr_Function__arr_v4float_int_1 Function
         %92 = OpVariable %_ptr_Function_float Function
        %117 = OpVariable %_ptr_Function_v4float Function
               OpStore %gAccessCount %int_0
         %39 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %41 = OpLoad %v4float %39
         %43 = OpVectorTimesScalar %v4float %41 %float_0_5
         %44 = OpFunctionCall %int %Z_i
         %45 = OpAccessChain %_ptr_Function_v4float %array %44
               OpStore %45 %43
         %48 = OpFunctionCall %int %Z_i
         %49 = OpAccessChain %_ptr_Function_v4float %array %48
         %50 = OpAccessChain %_ptr_Function_float %49 %int_3
               OpStore %50 %float_2
         %53 = OpFunctionCall %int %Z_i
         %54 = OpAccessChain %_ptr_Function_v4float %array %53
         %55 = OpAccessChain %_ptr_Function_float %54 %int_1
         %56 = OpLoad %float %55
         %58 = OpFMul %float %56 %float_4
               OpStore %55 %58
         %59 = OpFunctionCall %int %Z_i
         %60 = OpAccessChain %_ptr_Function_v4float %array %59
         %61 = OpLoad %v4float %60
         %62 = OpVectorShuffle %v3float %61 %61 1 2 3
         %69 = OpVectorTimesMatrix %v3float %62 %68
         %70 = OpLoad %v4float %60
         %71 = OpVectorShuffle %v4float %70 %69 0 4 5 6
               OpStore %60 %71
         %72 = OpFunctionCall %int %Z_i
         %73 = OpAccessChain %_ptr_Function_v4float %array %72
         %74 = OpLoad %v4float %73
         %75 = OpVectorShuffle %v4float %74 %74 2 1 3 0
         %79 = OpFAdd %v4float %75 %78
         %80 = OpLoad %v4float %73
         %81 = OpVectorShuffle %v4float %80 %79 7 5 4 6
               OpStore %73 %81
         %82 = OpFunctionCall %int %Z_i
         %83 = OpAccessChain %_ptr_Function_v4float %array %82
         %84 = OpAccessChain %_ptr_Function_float %83 %int_0
         %85 = OpLoad %float %84
         %86 = OpFunctionCall %int %Z_i
         %87 = OpAccessChain %_ptr_Function_v4float %array %86
         %88 = OpLoad %v4float %87
         %89 = OpCompositeExtract %float %88 3
         %91 = OpFOrdLessThanEqual %bool %89 %float_1
               OpSelectionMerge %95 None
               OpBranchConditional %91 %93 %94
         %93 = OpLabel
         %96 = OpFunctionCall %int %Z_i
         %97 = OpAccessChain %_ptr_Function_v4float %array %96
         %98 = OpLoad %v4float %97
         %99 = OpCompositeExtract %float %98 2
               OpStore %92 %99
               OpBranch %95
         %94 = OpLabel
        %100 = OpFunctionCall %int %Z_i
        %101 = OpConvertSToF %float %100
               OpStore %92 %101
               OpBranch %95
         %95 = OpLabel
        %102 = OpLoad %float %92
        %103 = OpFAdd %float %85 %102
               OpStore %84 %103
        %105 = OpLoad %int %gAccessCount
        %107 = OpIEqual %bool %105 %int_8
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
        %110 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %111 = OpLoad %v4float %110
        %113 = OpFOrdEqual %v4bool %111 %112
        %115 = OpAll %bool %113
               OpBranch %109
        %109 = OpLabel
        %116 = OpPhi %bool %false %95 %115 %108
               OpSelectionMerge %120 None
               OpBranchConditional %116 %118 %119
        %118 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %122 = OpLoad %v4float %121
               OpStore %117 %122
               OpBranch %120
        %119 = OpLabel
        %123 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %124 = OpLoad %v4float %123
               OpStore %117 %124
               OpBranch %120
        %120 = OpLabel
        %125 = OpLoad %v4float %117
               OpReturnValue %125
               OpFunctionEnd
