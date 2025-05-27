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
               OpDecorate %35 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
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
%float_0_84375 = OpConstant %float 0.84375
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
         %29 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %43 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%float_n1_25 = OpConstant %float -1.25
         %97 = OpConstantComposite %v2float %float_n1_25 %float_0
 %float_0_75 = OpConstant %float 0.75
        %114 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
 %float_2_25 = OpConstant %float 2.25
        %131 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
     %v4bool = OpTypeVector %bool 4
        %141 = OpConstantComposite %v2float %float_1 %float_0
        %148 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
        %203 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %27
               OpStore %expectedB %29
               OpSelectionMerge %34 None
               OpBranchConditional %true %33 %34
         %33 = OpLabel
         %35 = OpVectorShuffle %v2float %27 %27 0 1
         %36 = OpFOrdEqual %v2bool %16 %35
         %38 = OpAll %bool %36
               OpBranch %34
         %34 = OpLabel
         %39 = OpPhi %bool %false %22 %38 %33
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %44 = OpVectorShuffle %v3float %27 %27 0 1 2
         %45 = OpFOrdEqual %v3bool %43 %44
         %47 = OpAll %bool %45
               OpBranch %41
         %41 = OpLabel
         %48 = OpPhi %bool %false %34 %47 %40
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
         %51 = OpPhi %bool %false %41 %true %49
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
               OpBranch %53
         %53 = OpLabel
         %54 = OpPhi %bool %false %50 %true %52
               OpSelectionMerge %56 None
               OpBranchConditional %54 %55 %56
         %55 = OpLabel
         %57 = OpVectorShuffle %v2float %27 %27 0 1
         %58 = OpFOrdEqual %v2bool %16 %57
         %59 = OpAll %bool %58
               OpBranch %56
         %56 = OpLabel
         %60 = OpPhi %bool %false %53 %59 %55
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %63 = OpVectorShuffle %v3float %27 %27 0 1 2
         %64 = OpFOrdEqual %v3bool %43 %63
         %65 = OpAll %bool %64
               OpBranch %62
         %62 = OpLabel
         %66 = OpPhi %bool %false %56 %65 %61
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
               OpBranch %68
         %68 = OpLabel
         %69 = OpPhi %bool %false %62 %true %67
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %77 = OpLoad %v4float %73
         %78 = OpCompositeExtract %float %77 1
         %79 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %81 = OpLoad %v4float %79
         %82 = OpCompositeExtract %float %81 1
         %72 = OpExtInst %float %1 SmoothStep %78 %82 %float_n1_25
         %84 = OpFOrdEqual %bool %72 %float_0
               OpBranch %71
         %71 = OpLabel
         %85 = OpPhi %bool %false %68 %84 %70
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %90 = OpLoad %v4float %89
         %91 = OpCompositeExtract %float %90 1
         %92 = OpCompositeConstruct %v2float %91 %91
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %94 = OpLoad %v4float %93
         %95 = OpCompositeExtract %float %94 1
         %96 = OpCompositeConstruct %v2float %95 %95
         %88 = OpExtInst %v2float %1 SmoothStep %92 %96 %97
         %98 = OpVectorShuffle %v2float %27 %27 0 1
         %99 = OpFOrdEqual %v2bool %88 %98
        %100 = OpAll %bool %99
               OpBranch %87
         %87 = OpLabel
        %101 = OpPhi %bool %false %71 %100 %86
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
        %105 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %106 = OpLoad %v4float %105
        %107 = OpCompositeExtract %float %106 1
        %108 = OpCompositeConstruct %v3float %107 %107 %107
        %109 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %110 = OpLoad %v4float %109
        %111 = OpCompositeExtract %float %110 1
        %112 = OpCompositeConstruct %v3float %111 %111 %111
        %104 = OpExtInst %v3float %1 SmoothStep %108 %112 %114
        %115 = OpVectorShuffle %v3float %27 %27 0 1 2
        %116 = OpFOrdEqual %v3bool %104 %115
        %117 = OpAll %bool %116
               OpBranch %103
        %103 = OpLabel
        %118 = OpPhi %bool %false %87 %117 %102
               OpSelectionMerge %120 None
               OpBranchConditional %118 %119 %120
        %119 = OpLabel
        %122 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %123 = OpLoad %v4float %122
        %124 = OpCompositeExtract %float %123 1
        %125 = OpCompositeConstruct %v4float %124 %124 %124 %124
        %126 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %127 = OpLoad %v4float %126
        %128 = OpCompositeExtract %float %127 1
        %129 = OpCompositeConstruct %v4float %128 %128 %128 %128
        %121 = OpExtInst %v4float %1 SmoothStep %125 %129 %131
        %132 = OpFOrdEqual %v4bool %121 %27
        %134 = OpAll %bool %132
               OpBranch %120
        %120 = OpLabel
        %135 = OpPhi %bool %false %103 %134 %119
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
               OpBranch %137
        %137 = OpLabel
        %138 = OpPhi %bool %false %120 %true %136
               OpSelectionMerge %140 None
               OpBranchConditional %138 %139 %140
        %139 = OpLabel
        %142 = OpVectorShuffle %v2float %29 %29 0 1
        %143 = OpFOrdEqual %v2bool %141 %142
        %144 = OpAll %bool %143
               OpBranch %140
        %140 = OpLabel
        %145 = OpPhi %bool %false %137 %144 %139
               OpSelectionMerge %147 None
               OpBranchConditional %145 %146 %147
        %146 = OpLabel
        %149 = OpVectorShuffle %v3float %29 %29 0 1 2
        %150 = OpFOrdEqual %v3bool %148 %149
        %151 = OpAll %bool %150
               OpBranch %147
        %147 = OpLabel
        %152 = OpPhi %bool %false %140 %151 %146
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
               OpBranch %154
        %154 = OpLabel
        %155 = OpPhi %bool %false %147 %true %153
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
        %159 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %160 = OpLoad %v4float %159
        %161 = OpCompositeExtract %float %160 0
        %162 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %163 = OpLoad %v4float %162
        %164 = OpCompositeExtract %float %163 0
        %158 = OpExtInst %float %1 SmoothStep %161 %164 %float_n1_25
        %165 = OpFOrdEqual %bool %158 %float_1
               OpBranch %157
        %157 = OpLabel
        %166 = OpPhi %bool %false %154 %165 %156
               OpSelectionMerge %168 None
               OpBranchConditional %166 %167 %168
        %167 = OpLabel
        %170 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %171 = OpLoad %v4float %170
        %172 = OpVectorShuffle %v2float %171 %171 0 1
        %173 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %174 = OpLoad %v4float %173
        %175 = OpVectorShuffle %v2float %174 %174 0 1
        %169 = OpExtInst %v2float %1 SmoothStep %172 %175 %97
        %176 = OpVectorShuffle %v2float %29 %29 0 1
        %177 = OpFOrdEqual %v2bool %169 %176
        %178 = OpAll %bool %177
               OpBranch %168
        %168 = OpLabel
        %179 = OpPhi %bool %false %157 %178 %167
               OpSelectionMerge %181 None
               OpBranchConditional %179 %180 %181
        %180 = OpLabel
        %183 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %184 = OpLoad %v4float %183
        %185 = OpVectorShuffle %v3float %184 %184 0 1 2
        %186 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %187 = OpLoad %v4float %186
        %188 = OpVectorShuffle %v3float %187 %187 0 1 2
        %182 = OpExtInst %v3float %1 SmoothStep %185 %188 %114
        %189 = OpVectorShuffle %v3float %29 %29 0 1 2
        %190 = OpFOrdEqual %v3bool %182 %189
        %191 = OpAll %bool %190
               OpBranch %181
        %181 = OpLabel
        %192 = OpPhi %bool %false %168 %191 %180
               OpSelectionMerge %194 None
               OpBranchConditional %192 %193 %194
        %193 = OpLabel
        %196 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %197 = OpLoad %v4float %196
        %198 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %199 = OpLoad %v4float %198
        %195 = OpExtInst %v4float %1 SmoothStep %197 %199 %131
        %200 = OpFOrdEqual %v4bool %195 %29
        %201 = OpAll %bool %200
               OpBranch %194
        %194 = OpLabel
        %202 = OpPhi %bool %false %181 %201 %193
               OpSelectionMerge %206 None
               OpBranchConditional %202 %204 %205
        %204 = OpLabel
        %207 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %208 = OpLoad %v4float %207
               OpStore %203 %208
               OpBranch %206
        %205 = OpLabel
        %209 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %210 = OpLoad %v4float %209
               OpStore %203 %210
               OpBranch %206
        %206 = OpLabel
        %211 = OpLoad %v4float %203
               OpReturnValue %211
               OpFunctionEnd
