               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "expected"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %25 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
  %float_0_5 = OpConstant %float 0.5
         %86 = OpConstantComposite %v2float %float_1 %float_0_5
 %float_0_25 = OpConstant %float 0.25
         %96 = OpConstantComposite %v3float %float_1 %float_0_5 %float_0_25
%float_0_125 = OpConstant %float 0.125
        %106 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0_25 %float_0_125
   %float_n1 = OpConstant %float -1
   %float_n4 = OpConstant %float -4
        %125 = OpConstantComposite %v2float %float_n1 %float_n4
  %float_n16 = OpConstant %float -16
        %136 = OpConstantComposite %v3float %float_n1 %float_n4 %float_n16
  %float_n64 = OpConstant %float -64
        %147 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
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
        %153 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %26
         %31 = OpCompositeExtract %float %30 0
         %25 = OpExtInst %float %1 InverseSqrt %31
         %32 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %34 = OpLoad %v4float %32
         %35 = OpCompositeExtract %float %34 0
         %36 = OpFOrdEqual %bool %25 %35
               OpSelectionMerge %38 None
               OpBranchConditional %36 %37 %38
         %37 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpVectorShuffle %v2float %41 %41 0 1
         %39 = OpExtInst %v2float %1 InverseSqrt %42
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %46 = OpFOrdEqual %v2bool %39 %45
         %48 = OpAll %bool %46
               OpBranch %38
         %38 = OpLabel
         %49 = OpPhi %bool %false %22 %48 %37
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %53 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %54 = OpLoad %v4float %53
         %55 = OpVectorShuffle %v3float %54 %54 0 1 2
         %52 = OpExtInst %v3float %1 InverseSqrt %55
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v3float %58 %58 0 1 2
         %60 = OpFOrdEqual %v3bool %52 %59
         %62 = OpAll %bool %60
               OpBranch %51
         %51 = OpLabel
         %63 = OpPhi %bool %false %38 %62 %50
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %67 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %68 = OpLoad %v4float %67
         %66 = OpExtInst %v4float %1 InverseSqrt %68
         %69 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %70 = OpLoad %v4float %69
         %71 = OpFOrdEqual %v4bool %66 %70
         %73 = OpAll %bool %71
               OpBranch %65
         %65 = OpLabel
         %74 = OpPhi %bool %false %51 %73 %64
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %79 = OpLoad %v4float %78
         %80 = OpCompositeExtract %float %79 0
         %81 = OpFOrdEqual %bool %float_1 %80
               OpBranch %76
         %76 = OpLabel
         %82 = OpPhi %bool %false %65 %81 %75
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %88 = OpLoad %v4float %87
         %89 = OpVectorShuffle %v2float %88 %88 0 1
         %90 = OpFOrdEqual %v2bool %86 %89
         %91 = OpAll %bool %90
               OpBranch %84
         %84 = OpLabel
         %92 = OpPhi %bool %false %76 %91 %83
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %98 = OpLoad %v4float %97
         %99 = OpVectorShuffle %v3float %98 %98 0 1 2
        %100 = OpFOrdEqual %v3bool %96 %99
        %101 = OpAll %bool %100
               OpBranch %94
         %94 = OpLabel
        %102 = OpPhi %bool %false %84 %101 %93
               OpSelectionMerge %104 None
               OpBranchConditional %102 %103 %104
        %103 = OpLabel
        %107 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %108 = OpLoad %v4float %107
        %109 = OpFOrdEqual %v4bool %106 %108
        %110 = OpAll %bool %109
               OpBranch %104
        %104 = OpLabel
        %111 = OpPhi %bool %false %94 %110 %103
               OpSelectionMerge %113 None
               OpBranchConditional %111 %112 %113
        %112 = OpLabel
        %114 = OpExtInst %float %1 InverseSqrt %float_n1
        %116 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %117 = OpLoad %v4float %116
        %118 = OpCompositeExtract %float %117 0
        %119 = OpFOrdEqual %bool %114 %118
               OpBranch %113
        %113 = OpLabel
        %120 = OpPhi %bool %false %104 %119 %112
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %123 = OpExtInst %v2float %1 InverseSqrt %125
        %126 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %127 = OpLoad %v4float %126
        %128 = OpVectorShuffle %v2float %127 %127 0 1
        %129 = OpFOrdEqual %v2bool %123 %128
        %130 = OpAll %bool %129
               OpBranch %122
        %122 = OpLabel
        %131 = OpPhi %bool %false %113 %130 %121
               OpSelectionMerge %133 None
               OpBranchConditional %131 %132 %133
        %132 = OpLabel
        %134 = OpExtInst %v3float %1 InverseSqrt %136
        %137 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %138 = OpLoad %v4float %137
        %139 = OpVectorShuffle %v3float %138 %138 0 1 2
        %140 = OpFOrdEqual %v3bool %134 %139
        %141 = OpAll %bool %140
               OpBranch %133
        %133 = OpLabel
        %142 = OpPhi %bool %false %122 %141 %132
               OpSelectionMerge %144 None
               OpBranchConditional %142 %143 %144
        %143 = OpLabel
        %145 = OpExtInst %v4float %1 InverseSqrt %147
        %148 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %149 = OpLoad %v4float %148
        %150 = OpFOrdEqual %v4bool %145 %149
        %151 = OpAll %bool %150
               OpBranch %144
        %144 = OpLabel
        %152 = OpPhi %bool %false %133 %151 %143
               OpSelectionMerge %157 None
               OpBranchConditional %152 %155 %156
        %155 = OpLabel
        %158 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %160 = OpLoad %v4float %158
               OpStore %153 %160
               OpBranch %157
        %156 = OpLabel
        %161 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %163 = OpLoad %v4float %161
               OpStore %153 %163
               OpBranch %157
        %157 = OpLabel
        %164 = OpLoad %v4float %153
               OpReturnValue %164
               OpFunctionEnd
