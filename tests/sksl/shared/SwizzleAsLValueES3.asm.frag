               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %gAccessCount "gAccessCount"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %Z_i "Z_i"
               OpName %main "main"
               OpName %array "array"
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
               OpDecorate %_arr_v4float_int_1 ArrayStride 16
               OpDecorate %38 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
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
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %int
      %int_1 = OpConstant %int 1
         %30 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %61 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %62 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %63 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
%mat3v3float = OpTypeMatrix %v3float 3
         %65 = OpConstantComposite %mat3v3float %61 %62 %63
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
         %75 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %int_8 = OpConstant %int 8
        %110 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
     %v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
        %Z_i = OpFunction %int None %25
         %26 = OpLabel
         %28 = OpLoad %int %gAccessCount
         %29 = OpIAdd %int %28 %int_1
               OpStore %gAccessCount %29
               OpReturnValue %int_0
               OpFunctionEnd
       %main = OpFunction %v4float None %30
         %31 = OpFunctionParameter %_ptr_Function_v2float
         %32 = OpLabel
      %array = OpVariable %_ptr_Function__arr_v4float_int_1 Function
         %90 = OpVariable %_ptr_Function_float Function
        %115 = OpVariable %_ptr_Function_v4float Function
               OpStore %gAccessCount %int_0
         %36 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %38 = OpLoad %v4float %36
         %40 = OpVectorTimesScalar %v4float %38 %float_0_5
         %41 = OpFunctionCall %int %Z_i
         %42 = OpAccessChain %_ptr_Function_v4float %array %41
               OpStore %42 %40
         %45 = OpFunctionCall %int %Z_i
         %46 = OpAccessChain %_ptr_Function_v4float %array %45
         %47 = OpAccessChain %_ptr_Function_float %46 %int_3
               OpStore %47 %float_2
         %50 = OpFunctionCall %int %Z_i
         %51 = OpAccessChain %_ptr_Function_v4float %array %50
         %52 = OpAccessChain %_ptr_Function_float %51 %int_1
         %53 = OpLoad %float %52
         %55 = OpFMul %float %53 %float_4
               OpStore %52 %55
         %56 = OpFunctionCall %int %Z_i
         %57 = OpAccessChain %_ptr_Function_v4float %array %56
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v3float %58 %58 1 2 3
         %66 = OpVectorTimesMatrix %v3float %59 %65
         %67 = OpLoad %v4float %57
         %68 = OpVectorShuffle %v4float %67 %66 0 4 5 6
               OpStore %57 %68
         %69 = OpFunctionCall %int %Z_i
         %70 = OpAccessChain %_ptr_Function_v4float %array %69
         %71 = OpLoad %v4float %70
         %72 = OpVectorShuffle %v4float %71 %71 2 1 3 0
         %76 = OpFAdd %v4float %72 %75
         %77 = OpLoad %v4float %70
         %78 = OpVectorShuffle %v4float %77 %76 7 5 4 6
               OpStore %70 %78
         %79 = OpFunctionCall %int %Z_i
         %80 = OpAccessChain %_ptr_Function_v4float %array %79
         %81 = OpAccessChain %_ptr_Function_float %80 %int_0
         %82 = OpLoad %float %81
         %83 = OpFunctionCall %int %Z_i
         %84 = OpAccessChain %_ptr_Function_v4float %array %83
         %85 = OpLoad %v4float %84
         %86 = OpCompositeExtract %float %85 3
         %88 = OpFOrdLessThanEqual %bool %86 %float_1
               OpSelectionMerge %93 None
               OpBranchConditional %88 %91 %92
         %91 = OpLabel
         %94 = OpFunctionCall %int %Z_i
         %95 = OpAccessChain %_ptr_Function_v4float %array %94
         %96 = OpLoad %v4float %95
         %97 = OpCompositeExtract %float %96 2
               OpStore %90 %97
               OpBranch %93
         %92 = OpLabel
         %98 = OpFunctionCall %int %Z_i
         %99 = OpConvertSToF %float %98
               OpStore %90 %99
               OpBranch %93
         %93 = OpLabel
        %100 = OpLoad %float %90
        %101 = OpFAdd %float %82 %100
               OpStore %81 %101
        %103 = OpLoad %int %gAccessCount
        %105 = OpIEqual %bool %103 %int_8
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %108 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %109 = OpLoad %v4float %108
        %111 = OpFOrdEqual %v4bool %109 %110
        %113 = OpAll %bool %111
               OpBranch %107
        %107 = OpLabel
        %114 = OpPhi %bool %false %93 %113 %106
               OpSelectionMerge %118 None
               OpBranchConditional %114 %116 %117
        %116 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %120 = OpLoad %v4float %119
               OpStore %115 %120
               OpBranch %118
        %117 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %122 = OpLoad %v4float %121
               OpStore %115 %122
               OpBranch %118
        %118 = OpLabel
        %123 = OpLoad %v4float %115
               OpReturnValue %123
               OpFunctionEnd
