               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedA "expectedA"
               OpName %expectedB "expectedB"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
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
  %float_0_5 = OpConstant %float 0.5
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %28 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
    %float_1 = OpConstant %float 1
         %31 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %48 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %61 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %72 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %89 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
      %int_1 = OpConstant %int 1
        %150 = OpConstantComposite %v2float %float_0 %float_1
        %157 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
      %int_2 = OpConstant %int 2
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
  %expectedA = OpVariable %_ptr_Function_v4float Function
  %expectedB = OpVariable %_ptr_Function_v4float Function
        %165 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %28
               OpStore %expectedB %31
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %39 = OpLoad %v4float %35
         %40 = OpCompositeExtract %float %39 0
         %34 = OpExtInst %float %1 FMax %40 %float_0_5
         %41 = OpFOrdEqual %bool %34 %float_0_5
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %45 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v2float %46 %46 0 1
         %44 = OpExtInst %v2float %1 FMax %47 %48
         %49 = OpVectorShuffle %v2float %28 %28 0 1
         %50 = OpFOrdEqual %v2bool %44 %49
         %52 = OpAll %bool %50
               OpBranch %43
         %43 = OpLabel
         %53 = OpPhi %bool %false %22 %52 %42
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %58 = OpLoad %v4float %57
         %59 = OpVectorShuffle %v3float %58 %58 0 1 2
         %56 = OpExtInst %v3float %1 FMax %59 %61
         %62 = OpVectorShuffle %v3float %28 %28 0 1 2
         %63 = OpFOrdEqual %v3bool %56 %62
         %65 = OpAll %bool %63
               OpBranch %55
         %55 = OpLabel
         %66 = OpPhi %bool %false %43 %65 %54
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
         %70 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %71 = OpLoad %v4float %70
         %69 = OpExtInst %v4float %1 FMax %71 %72
         %73 = OpFOrdEqual %v4bool %69 %28
         %75 = OpAll %bool %73
               OpBranch %68
         %68 = OpLabel
         %76 = OpPhi %bool %false %55 %75 %67
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %80 = OpPhi %bool %false %68 %true %77
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %83 = OpVectorShuffle %v2float %28 %28 0 1
         %84 = OpFOrdEqual %v2bool %48 %83
         %85 = OpAll %bool %84
               OpBranch %82
         %82 = OpLabel
         %86 = OpPhi %bool %false %78 %85 %81
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpVectorShuffle %v3float %28 %28 0 1 2
         %91 = OpFOrdEqual %v3bool %89 %90
         %92 = OpAll %bool %91
               OpBranch %88
         %88 = OpLabel
         %93 = OpPhi %bool %false %82 %92 %87
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
               OpBranch %95
         %95 = OpLabel
         %96 = OpPhi %bool %false %88 %true %94
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
        %100 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %101 = OpLoad %v4float %100
        %102 = OpCompositeExtract %float %101 0
        %103 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %105 = OpLoad %v4float %103
        %106 = OpCompositeExtract %float %105 0
         %99 = OpExtInst %float %1 FMax %102 %106
        %107 = OpFOrdEqual %bool %99 %float_0
               OpBranch %98
         %98 = OpLabel
        %108 = OpPhi %bool %false %95 %107 %97
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %112 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %113 = OpLoad %v4float %112
        %114 = OpVectorShuffle %v2float %113 %113 0 1
        %115 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %116 = OpLoad %v4float %115
        %117 = OpVectorShuffle %v2float %116 %116 0 1
        %111 = OpExtInst %v2float %1 FMax %114 %117
        %118 = OpVectorShuffle %v2float %31 %31 0 1
        %119 = OpFOrdEqual %v2bool %111 %118
        %120 = OpAll %bool %119
               OpBranch %110
        %110 = OpLabel
        %121 = OpPhi %bool %false %98 %120 %109
               OpSelectionMerge %123 None
               OpBranchConditional %121 %122 %123
        %122 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %126 = OpLoad %v4float %125
        %127 = OpVectorShuffle %v3float %126 %126 0 1 2
        %128 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %129 = OpLoad %v4float %128
        %130 = OpVectorShuffle %v3float %129 %129 0 1 2
        %124 = OpExtInst %v3float %1 FMax %127 %130
        %131 = OpVectorShuffle %v3float %31 %31 0 1 2
        %132 = OpFOrdEqual %v3bool %124 %131
        %133 = OpAll %bool %132
               OpBranch %123
        %123 = OpLabel
        %134 = OpPhi %bool %false %110 %133 %122
               OpSelectionMerge %136 None
               OpBranchConditional %134 %135 %136
        %135 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %139 = OpLoad %v4float %138
        %140 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %141 = OpLoad %v4float %140
        %137 = OpExtInst %v4float %1 FMax %139 %141
        %142 = OpFOrdEqual %v4bool %137 %31
        %143 = OpAll %bool %142
               OpBranch %136
        %136 = OpLabel
        %144 = OpPhi %bool %false %123 %143 %135
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
               OpBranch %146
        %146 = OpLabel
        %147 = OpPhi %bool %false %136 %true %145
               OpSelectionMerge %149 None
               OpBranchConditional %147 %148 %149
        %148 = OpLabel
        %151 = OpVectorShuffle %v2float %31 %31 0 1
        %152 = OpFOrdEqual %v2bool %150 %151
        %153 = OpAll %bool %152
               OpBranch %149
        %149 = OpLabel
        %154 = OpPhi %bool %false %146 %153 %148
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %158 = OpVectorShuffle %v3float %31 %31 0 1 2
        %159 = OpFOrdEqual %v3bool %157 %158
        %160 = OpAll %bool %159
               OpBranch %156
        %156 = OpLabel
        %161 = OpPhi %bool %false %149 %160 %155
               OpSelectionMerge %163 None
               OpBranchConditional %161 %162 %163
        %162 = OpLabel
               OpBranch %163
        %163 = OpLabel
        %164 = OpPhi %bool %false %156 %true %162
               OpSelectionMerge %168 None
               OpBranchConditional %164 %166 %167
        %166 = OpLabel
        %169 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %170 = OpLoad %v4float %169
               OpStore %165 %170
               OpBranch %168
        %167 = OpLabel
        %171 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %173 = OpLoad %v4float %171
               OpStore %165 %173
               OpBranch %168
        %168 = OpLabel
        %174 = OpLoad %v4float %165
               OpReturnValue %174
               OpFunctionEnd
