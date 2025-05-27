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
               OpName %expectedC "expectedC"
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
               OpDecorate %expectedC RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
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
    %float_1 = OpConstant %float 1
         %26 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
         %28 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
         %30 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
  %float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %45 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %58 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %70 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %89 = OpConstantComposite %v3float %float_0 %float_0 %float_1
        %111 = OpConstantComposite %v2float %float_0 %float_1
        %122 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %132 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
        %141 = OpConstantComposite %v2float %float_1 %float_1
        %148 = OpConstantComposite %v3float %float_1 %float_1 %float_0
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
        %216 = OpConstantComposite %v3float %float_0 %float_1 %float_1
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
  %expectedC = OpVariable %_ptr_Function_v4float Function
        %224 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %26
               OpStore %expectedB %28
               OpStore %expectedC %30
         %35 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %39 = OpLoad %v4float %35
         %40 = OpCompositeExtract %float %39 0
         %33 = OpExtInst %float %1 Step %float_0_5 %40
         %41 = OpFOrdEqual %bool %33 %float_0
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpVectorShuffle %v2float %47 %47 0 1
         %44 = OpExtInst %v2float %1 Step %45 %48
         %49 = OpVectorShuffle %v2float %26 %26 0 1
         %50 = OpFOrdEqual %v2bool %44 %49
         %52 = OpAll %bool %50
               OpBranch %43
         %43 = OpLabel
         %53 = OpPhi %bool %false %22 %52 %42
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v3float %60 %60 0 1 2
         %56 = OpExtInst %v3float %1 Step %58 %61
         %62 = OpVectorShuffle %v3float %26 %26 0 1 2
         %63 = OpFOrdEqual %v3bool %56 %62
         %65 = OpAll %bool %63
               OpBranch %55
         %55 = OpLabel
         %66 = OpPhi %bool %false %43 %65 %54
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
         %71 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %72 = OpLoad %v4float %71
         %69 = OpExtInst %v4float %1 Step %70 %72
         %73 = OpFOrdEqual %v4bool %69 %26
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
         %83 = OpVectorShuffle %v2float %26 %26 0 1
         %84 = OpFOrdEqual %v2bool %16 %83
         %85 = OpAll %bool %84
               OpBranch %82
         %82 = OpLabel
         %86 = OpPhi %bool %false %78 %85 %81
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpVectorShuffle %v3float %26 %26 0 1 2
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
         %99 = OpExtInst %float %1 Step %102 %float_0
        %103 = OpFOrdEqual %bool %99 %float_1
               OpBranch %98
         %98 = OpLabel
        %104 = OpPhi %bool %false %95 %103 %97
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
        %108 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %109 = OpLoad %v4float %108
        %110 = OpVectorShuffle %v2float %109 %109 0 1
        %107 = OpExtInst %v2float %1 Step %110 %111
        %112 = OpVectorShuffle %v2float %28 %28 0 1
        %113 = OpFOrdEqual %v2bool %107 %112
        %114 = OpAll %bool %113
               OpBranch %106
        %106 = OpLabel
        %115 = OpPhi %bool %false %98 %114 %105
               OpSelectionMerge %117 None
               OpBranchConditional %115 %116 %117
        %116 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %120 = OpLoad %v4float %119
        %121 = OpVectorShuffle %v3float %120 %120 0 1 2
        %118 = OpExtInst %v3float %1 Step %121 %122
        %123 = OpVectorShuffle %v3float %28 %28 0 1 2
        %124 = OpFOrdEqual %v3bool %118 %123
        %125 = OpAll %bool %124
               OpBranch %117
        %117 = OpLabel
        %126 = OpPhi %bool %false %106 %125 %116
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %130 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %131 = OpLoad %v4float %130
        %129 = OpExtInst %v4float %1 Step %131 %132
        %133 = OpFOrdEqual %v4bool %129 %28
        %134 = OpAll %bool %133
               OpBranch %128
        %128 = OpLabel
        %135 = OpPhi %bool %false %117 %134 %127
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
               OpBranch %137
        %137 = OpLabel
        %138 = OpPhi %bool %false %128 %true %136
               OpSelectionMerge %140 None
               OpBranchConditional %138 %139 %140
        %139 = OpLabel
        %142 = OpVectorShuffle %v2float %28 %28 0 1
        %143 = OpFOrdEqual %v2bool %141 %142
        %144 = OpAll %bool %143
               OpBranch %140
        %140 = OpLabel
        %145 = OpPhi %bool %false %137 %144 %139
               OpSelectionMerge %147 None
               OpBranchConditional %145 %146 %147
        %146 = OpLabel
        %149 = OpVectorShuffle %v3float %28 %28 0 1 2
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
        %161 = OpLoad %v4float %159
        %162 = OpCompositeExtract %float %161 0
        %163 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %165 = OpLoad %v4float %163
        %166 = OpCompositeExtract %float %165 0
        %158 = OpExtInst %float %1 Step %162 %166
        %167 = OpFOrdEqual %bool %158 %float_0
               OpBranch %157
        %157 = OpLabel
        %168 = OpPhi %bool %false %154 %167 %156
               OpSelectionMerge %170 None
               OpBranchConditional %168 %169 %170
        %169 = OpLabel
        %172 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %173 = OpLoad %v4float %172
        %174 = OpVectorShuffle %v2float %173 %173 0 1
        %175 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %176 = OpLoad %v4float %175
        %177 = OpVectorShuffle %v2float %176 %176 0 1
        %171 = OpExtInst %v2float %1 Step %174 %177
        %178 = OpVectorShuffle %v2float %30 %30 0 1
        %179 = OpFOrdEqual %v2bool %171 %178
        %180 = OpAll %bool %179
               OpBranch %170
        %170 = OpLabel
        %181 = OpPhi %bool %false %157 %180 %169
               OpSelectionMerge %183 None
               OpBranchConditional %181 %182 %183
        %182 = OpLabel
        %185 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %186 = OpLoad %v4float %185
        %187 = OpVectorShuffle %v3float %186 %186 0 1 2
        %188 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %189 = OpLoad %v4float %188
        %190 = OpVectorShuffle %v3float %189 %189 0 1 2
        %184 = OpExtInst %v3float %1 Step %187 %190
        %191 = OpVectorShuffle %v3float %30 %30 0 1 2
        %192 = OpFOrdEqual %v3bool %184 %191
        %193 = OpAll %bool %192
               OpBranch %183
        %183 = OpLabel
        %194 = OpPhi %bool %false %170 %193 %182
               OpSelectionMerge %196 None
               OpBranchConditional %194 %195 %196
        %195 = OpLabel
        %198 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %199 = OpLoad %v4float %198
        %200 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %201 = OpLoad %v4float %200
        %197 = OpExtInst %v4float %1 Step %199 %201
        %202 = OpFOrdEqual %v4bool %197 %30
        %203 = OpAll %bool %202
               OpBranch %196
        %196 = OpLabel
        %204 = OpPhi %bool %false %183 %203 %195
               OpSelectionMerge %206 None
               OpBranchConditional %204 %205 %206
        %205 = OpLabel
               OpBranch %206
        %206 = OpLabel
        %207 = OpPhi %bool %false %196 %true %205
               OpSelectionMerge %209 None
               OpBranchConditional %207 %208 %209
        %208 = OpLabel
        %210 = OpVectorShuffle %v2float %30 %30 0 1
        %211 = OpFOrdEqual %v2bool %111 %210
        %212 = OpAll %bool %211
               OpBranch %209
        %209 = OpLabel
        %213 = OpPhi %bool %false %206 %212 %208
               OpSelectionMerge %215 None
               OpBranchConditional %213 %214 %215
        %214 = OpLabel
        %217 = OpVectorShuffle %v3float %30 %30 0 1 2
        %218 = OpFOrdEqual %v3bool %216 %217
        %219 = OpAll %bool %218
               OpBranch %215
        %215 = OpLabel
        %220 = OpPhi %bool %false %209 %219 %214
               OpSelectionMerge %222 None
               OpBranchConditional %220 %221 %222
        %221 = OpLabel
               OpBranch %222
        %222 = OpLabel
        %223 = OpPhi %bool %false %215 %true %221
               OpSelectionMerge %227 None
               OpBranchConditional %223 %225 %226
        %225 = OpLabel
        %228 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %229 = OpLoad %v4float %228
               OpStore %224 %229
               OpBranch %227
        %226 = OpLabel
        %230 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %231 = OpLoad %v4float %230
               OpStore %224 %231
               OpBranch %227
        %227 = OpLabel
        %232 = OpLoad %v4float %224
               OpReturnValue %232
               OpFunctionEnd
