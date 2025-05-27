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
               OpName %scalar "scalar"
               OpName %array "array"
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
               OpDecorate %_arr_v4float_int_1 ArrayStride 16
               OpDecorate %33 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_arr_v4float_int_1 = OpTypeArray %v4float %int_1
%_ptr_Function__arr_v4float_int_1 = OpTypePointer Function %_arr_v4float_int_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_0_5 = OpConstant %float 0.5
    %float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
      %int_3 = OpConstant %int 3
    %float_4 = OpConstant %float 4
    %v3float = OpTypeVector %float 3
         %47 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %48 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %49 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
%mat3v3float = OpTypeMatrix %v3float 3
         %51 = OpConstantComposite %mat3v3float %47 %48 %49
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
         %59 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
        %116 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
     %v4bool = OpTypeVector %bool 4
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
     %scalar = OpVariable %_ptr_Function_v4float Function
      %array = OpVariable %_ptr_Function__arr_v4float_int_1 Function
         %69 = OpVariable %_ptr_Function_float Function
        %105 = OpVariable %_ptr_Function_float Function
        %127 = OpVariable %_ptr_Function_v4float Function
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %33 = OpLoad %v4float %30
         %35 = OpVectorTimesScalar %v4float %33 %float_0_5
               OpStore %scalar %35
         %37 = OpAccessChain %_ptr_Function_float %scalar %int_3
               OpStore %37 %float_2
         %40 = OpAccessChain %_ptr_Function_float %scalar %int_1
         %41 = OpLoad %float %40
         %43 = OpFMul %float %41 %float_4
               OpStore %40 %43
         %44 = OpLoad %v4float %scalar
         %45 = OpVectorShuffle %v3float %44 %44 1 2 3
         %52 = OpVectorTimesMatrix %v3float %45 %51
         %53 = OpLoad %v4float %scalar
         %54 = OpVectorShuffle %v4float %53 %52 0 4 5 6
               OpStore %scalar %54
         %55 = OpLoad %v4float %scalar
         %56 = OpVectorShuffle %v4float %55 %55 2 1 3 0
         %60 = OpFAdd %v4float %56 %59
         %61 = OpLoad %v4float %scalar
         %62 = OpVectorShuffle %v4float %61 %60 7 5 4 6
               OpStore %scalar %62
         %63 = OpAccessChain %_ptr_Function_float %scalar %int_0
         %64 = OpLoad %float %63
         %65 = OpCompositeExtract %float %62 3
         %67 = OpFOrdLessThanEqual %bool %65 %float_1
               OpSelectionMerge %72 None
               OpBranchConditional %67 %70 %71
         %70 = OpLabel
         %73 = OpCompositeExtract %float %62 2
               OpStore %69 %73
               OpBranch %72
         %71 = OpLabel
               OpStore %69 %float_0
               OpBranch %72
         %72 = OpLabel
         %74 = OpLoad %float %69
         %75 = OpFAdd %float %64 %74
               OpStore %63 %75
         %76 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %77 = OpLoad %v4float %76
         %78 = OpVectorTimesScalar %v4float %77 %float_0_5
         %79 = OpAccessChain %_ptr_Function_v4float %array %int_0
               OpStore %79 %78
         %80 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %81 = OpAccessChain %_ptr_Function_float %80 %int_3
               OpStore %81 %float_2
         %82 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %83 = OpAccessChain %_ptr_Function_float %82 %int_1
         %84 = OpLoad %float %83
         %85 = OpFMul %float %84 %float_4
               OpStore %83 %85
         %86 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %87 = OpLoad %v4float %86
         %88 = OpVectorShuffle %v3float %87 %87 1 2 3
         %89 = OpVectorTimesMatrix %v3float %88 %51
         %90 = OpLoad %v4float %86
         %91 = OpVectorShuffle %v4float %90 %89 0 4 5 6
               OpStore %86 %91
         %92 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %93 = OpLoad %v4float %92
         %94 = OpVectorShuffle %v4float %93 %93 2 1 3 0
         %95 = OpFAdd %v4float %94 %59
         %96 = OpLoad %v4float %92
         %97 = OpVectorShuffle %v4float %96 %95 7 5 4 6
               OpStore %92 %97
         %98 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %99 = OpAccessChain %_ptr_Function_float %98 %int_0
        %100 = OpLoad %float %99
        %101 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %102 = OpLoad %v4float %101
        %103 = OpCompositeExtract %float %102 3
        %104 = OpFOrdLessThanEqual %bool %103 %float_1
               OpSelectionMerge %108 None
               OpBranchConditional %104 %106 %107
        %106 = OpLabel
        %109 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %110 = OpLoad %v4float %109
        %111 = OpCompositeExtract %float %110 2
               OpStore %105 %111
               OpBranch %108
        %107 = OpLabel
               OpStore %105 %float_0
               OpBranch %108
        %108 = OpLabel
        %112 = OpLoad %float %105
        %113 = OpFAdd %float %100 %112
               OpStore %99 %113
        %115 = OpLoad %v4float %scalar
        %117 = OpFOrdEqual %v4bool %115 %116
        %119 = OpAll %bool %117
               OpSelectionMerge %121 None
               OpBranchConditional %119 %120 %121
        %120 = OpLabel
        %122 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %123 = OpLoad %v4float %122
        %124 = OpFOrdEqual %v4bool %123 %116
        %125 = OpAll %bool %124
               OpBranch %121
        %121 = OpLabel
        %126 = OpPhi %bool %false %108 %125 %120
               OpSelectionMerge %130 None
               OpBranchConditional %126 %128 %129
        %128 = OpLabel
        %131 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %132 = OpLoad %v4float %131
               OpStore %127 %132
               OpBranch %130
        %129 = OpLabel
        %133 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %134 = OpLoad %v4float %133
               OpStore %127 %134
               OpBranch %130
        %130 = OpLabel
        %135 = OpLoad %v4float %127
               OpReturnValue %135
               OpFunctionEnd
