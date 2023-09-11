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
               OpName %scalar "scalar"
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %_arr_v4float_int_1 ArrayStride 16
               OpDecorate %36 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
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
         %50 = OpConstantComposite %v3float %float_0_5 %float_0 %float_0
         %51 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
         %52 = OpConstantComposite %v3float %float_0 %float_0 %float_0_5
%mat3v3float = OpTypeMatrix %v3float 3
         %54 = OpConstantComposite %mat3v3float %50 %51 %52
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
         %62 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
    %float_1 = OpConstant %float 1
      %false = OpConstantFalse %bool
        %118 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
     %v4bool = OpTypeVector %bool 4
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
     %scalar = OpVariable %_ptr_Function_v4float Function
      %array = OpVariable %_ptr_Function__arr_v4float_int_1 Function
         %71 = OpVariable %_ptr_Function_float Function
        %107 = OpVariable %_ptr_Function_float Function
        %129 = OpVariable %_ptr_Function_v4float Function
         %33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %36 = OpLoad %v4float %33
         %38 = OpVectorTimesScalar %v4float %36 %float_0_5
               OpStore %scalar %38
         %40 = OpAccessChain %_ptr_Function_float %scalar %int_3
               OpStore %40 %float_2
         %43 = OpAccessChain %_ptr_Function_float %scalar %int_1
         %44 = OpLoad %float %43
         %46 = OpFMul %float %44 %float_4
               OpStore %43 %46
         %47 = OpLoad %v4float %scalar
         %48 = OpVectorShuffle %v3float %47 %47 1 2 3
         %55 = OpVectorTimesMatrix %v3float %48 %54
         %56 = OpLoad %v4float %scalar
         %57 = OpVectorShuffle %v4float %56 %55 0 4 5 6
               OpStore %scalar %57
         %58 = OpLoad %v4float %scalar
         %59 = OpVectorShuffle %v4float %58 %58 2 1 3 0
         %63 = OpFAdd %v4float %59 %62
         %64 = OpLoad %v4float %scalar
         %65 = OpVectorShuffle %v4float %64 %63 7 5 4 6
               OpStore %scalar %65
         %66 = OpAccessChain %_ptr_Function_float %scalar %int_0
         %67 = OpLoad %float %66
         %68 = OpCompositeExtract %float %65 3
         %70 = OpFOrdLessThanEqual %bool %68 %float_1
               OpSelectionMerge %74 None
               OpBranchConditional %70 %72 %73
         %72 = OpLabel
         %75 = OpCompositeExtract %float %65 2
               OpStore %71 %75
               OpBranch %74
         %73 = OpLabel
               OpStore %71 %float_0
               OpBranch %74
         %74 = OpLabel
         %76 = OpLoad %float %71
         %77 = OpFAdd %float %67 %76
               OpStore %66 %77
         %78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %79 = OpLoad %v4float %78
         %80 = OpVectorTimesScalar %v4float %79 %float_0_5
         %81 = OpAccessChain %_ptr_Function_v4float %array %int_0
               OpStore %81 %80
         %82 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %83 = OpAccessChain %_ptr_Function_float %82 %int_3
               OpStore %83 %float_2
         %84 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %85 = OpAccessChain %_ptr_Function_float %84 %int_1
         %86 = OpLoad %float %85
         %87 = OpFMul %float %86 %float_4
               OpStore %85 %87
         %88 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %89 = OpLoad %v4float %88
         %90 = OpVectorShuffle %v3float %89 %89 1 2 3
         %91 = OpVectorTimesMatrix %v3float %90 %54
         %92 = OpLoad %v4float %88
         %93 = OpVectorShuffle %v4float %92 %91 0 4 5 6
               OpStore %88 %93
         %94 = OpAccessChain %_ptr_Function_v4float %array %int_0
         %95 = OpLoad %v4float %94
         %96 = OpVectorShuffle %v4float %95 %95 2 1 3 0
         %97 = OpFAdd %v4float %96 %62
         %98 = OpLoad %v4float %94
         %99 = OpVectorShuffle %v4float %98 %97 7 5 4 6
               OpStore %94 %99
        %100 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %101 = OpAccessChain %_ptr_Function_float %100 %int_0
        %102 = OpLoad %float %101
        %103 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %104 = OpLoad %v4float %103
        %105 = OpCompositeExtract %float %104 3
        %106 = OpFOrdLessThanEqual %bool %105 %float_1
               OpSelectionMerge %110 None
               OpBranchConditional %106 %108 %109
        %108 = OpLabel
        %111 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %112 = OpLoad %v4float %111
        %113 = OpCompositeExtract %float %112 2
               OpStore %107 %113
               OpBranch %110
        %109 = OpLabel
               OpStore %107 %float_0
               OpBranch %110
        %110 = OpLabel
        %114 = OpLoad %float %107
        %115 = OpFAdd %float %102 %114
               OpStore %101 %115
        %117 = OpLoad %v4float %scalar
        %119 = OpFOrdEqual %v4bool %117 %118
        %121 = OpAll %bool %119
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %124 = OpAccessChain %_ptr_Function_v4float %array %int_0
        %125 = OpLoad %v4float %124
        %126 = OpFOrdEqual %v4bool %125 %118
        %127 = OpAll %bool %126
               OpBranch %123
        %123 = OpLabel
        %128 = OpPhi %bool %false %110 %127 %122
               OpSelectionMerge %132 None
               OpBranchConditional %128 %130 %131
        %130 = OpLabel
        %133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %134 = OpLoad %v4float %133
               OpStore %129 %134
               OpBranch %132
        %131 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %136 = OpLoad %v4float %135
               OpStore %129 %136
               OpBranch %132
        %132 = OpLabel
        %137 = OpLoad %v4float %129
               OpReturnValue %137
               OpFunctionEnd
