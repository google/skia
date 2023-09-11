               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %ok "ok"
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
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
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
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %46 = OpConstantComposite %v2float %float_n1_25 %float_0
         %47 = OpConstantComposite %v2float %float_0_75 %float_2_25
         %48 = OpConstantComposite %mat2v2float %46 %47
     %v2bool = OpTypeVector %bool 2
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %86 = OpConstantComposite %v2float %float_0 %float_1
         %87 = OpConstantComposite %mat2v2float %86 %86
      %v4int = OpTypeVector %int 4
     %v4bool = OpTypeVector %bool 4
      %int_1 = OpConstant %int 1
   %float_n1 = OpConstant %float -1
        %233 = OpConstantComposite %v2float %float_n1 %float_1
        %234 = OpConstantComposite %mat2v2float %233 %16
    %float_5 = OpConstant %float 5
        %246 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
    %float_6 = OpConstant %float 6
        %256 = OpConstantComposite %v2float %float_5 %float_6
        %257 = OpConstantComposite %mat2v2float %256 %256
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %ok = OpVariable %_ptr_Function_bool Function
        %264 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpSelectionMerge %29 None
               OpBranchConditional %true %28 %29
         %28 = OpLabel
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %34 = OpLoad %v4float %30
         %35 = OpCompositeExtract %float %34 0
         %36 = OpCompositeExtract %float %34 1
         %37 = OpCompositeExtract %float %34 2
         %38 = OpCompositeExtract %float %34 3
         %39 = OpCompositeConstruct %v2float %35 %36
         %40 = OpCompositeConstruct %v2float %37 %38
         %42 = OpCompositeConstruct %mat2v2float %39 %40
         %50 = OpFOrdEqual %v2bool %39 %46
         %51 = OpAll %bool %50
         %52 = OpFOrdEqual %v2bool %40 %47
         %53 = OpAll %bool %52
         %54 = OpLogicalAnd %bool %51 %53
               OpBranch %29
         %29 = OpLabel
         %55 = OpPhi %bool %false %22 %54 %28
               OpStore %ok %55
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %58 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %59 = OpLoad %v4float %58
         %60 = OpCompositeExtract %float %59 0
         %61 = OpCompositeExtract %float %59 1
         %62 = OpCompositeExtract %float %59 2
         %63 = OpCompositeExtract %float %59 3
         %64 = OpCompositeConstruct %v2float %60 %61
         %65 = OpCompositeConstruct %v2float %62 %63
         %66 = OpCompositeConstruct %mat2v2float %64 %65
         %67 = OpFOrdEqual %v2bool %64 %46
         %68 = OpAll %bool %67
         %69 = OpFOrdEqual %v2bool %65 %47
         %70 = OpAll %bool %69
         %71 = OpLogicalAnd %bool %68 %70
               OpBranch %57
         %57 = OpLabel
         %72 = OpPhi %bool %false %29 %71 %56
               OpStore %ok %72
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %75 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %77 = OpLoad %v4float %75
         %78 = OpCompositeExtract %float %77 0
         %79 = OpCompositeExtract %float %77 1
         %80 = OpCompositeExtract %float %77 2
         %81 = OpCompositeExtract %float %77 3
         %82 = OpCompositeConstruct %v2float %78 %79
         %83 = OpCompositeConstruct %v2float %80 %81
         %84 = OpCompositeConstruct %mat2v2float %82 %83
         %88 = OpFOrdEqual %v2bool %82 %86
         %89 = OpAll %bool %88
         %90 = OpFOrdEqual %v2bool %83 %86
         %91 = OpAll %bool %90
         %92 = OpLogicalAnd %bool %89 %91
               OpBranch %74
         %74 = OpLabel
         %93 = OpPhi %bool %false %57 %92 %73
               OpStore %ok %93
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %96 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %97 = OpLoad %v4float %96
         %98 = OpCompositeExtract %float %97 0
         %99 = OpCompositeExtract %float %97 1
        %100 = OpCompositeExtract %float %97 2
        %101 = OpCompositeExtract %float %97 3
        %102 = OpCompositeConstruct %v2float %98 %99
        %103 = OpCompositeConstruct %v2float %100 %101
        %104 = OpCompositeConstruct %mat2v2float %102 %103
        %105 = OpFOrdEqual %v2bool %102 %86
        %106 = OpAll %bool %105
        %107 = OpFOrdEqual %v2bool %103 %86
        %108 = OpAll %bool %107
        %109 = OpLogicalAnd %bool %106 %108
               OpBranch %95
         %95 = OpLabel
        %110 = OpPhi %bool %false %74 %109 %94
               OpStore %ok %110
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %113 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %114 = OpLoad %v4float %113
        %115 = OpCompositeExtract %float %114 0
        %116 = OpConvertFToS %int %115
        %117 = OpCompositeExtract %float %114 1
        %118 = OpConvertFToS %int %117
        %119 = OpCompositeExtract %float %114 2
        %120 = OpConvertFToS %int %119
        %121 = OpCompositeExtract %float %114 3
        %122 = OpConvertFToS %int %121
        %124 = OpCompositeConstruct %v4int %116 %118 %120 %122
        %125 = OpCompositeExtract %int %124 0
        %126 = OpConvertSToF %float %125
        %127 = OpCompositeExtract %int %124 1
        %128 = OpConvertSToF %float %127
        %129 = OpCompositeExtract %int %124 2
        %130 = OpConvertSToF %float %129
        %131 = OpCompositeExtract %int %124 3
        %132 = OpConvertSToF %float %131
        %133 = OpCompositeConstruct %v4float %126 %128 %130 %132
        %134 = OpCompositeExtract %float %133 0
        %135 = OpCompositeExtract %float %133 1
        %136 = OpCompositeExtract %float %133 2
        %137 = OpCompositeExtract %float %133 3
        %138 = OpCompositeConstruct %v2float %134 %135
        %139 = OpCompositeConstruct %v2float %136 %137
        %140 = OpCompositeConstruct %mat2v2float %138 %139
        %141 = OpFOrdEqual %v2bool %138 %86
        %142 = OpAll %bool %141
        %143 = OpFOrdEqual %v2bool %139 %86
        %144 = OpAll %bool %143
        %145 = OpLogicalAnd %bool %142 %144
               OpBranch %112
        %112 = OpLabel
        %146 = OpPhi %bool %false %95 %145 %111
               OpStore %ok %146
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %149 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %150 = OpLoad %v4float %149
        %151 = OpCompositeExtract %float %150 0
        %152 = OpCompositeExtract %float %150 1
        %153 = OpCompositeExtract %float %150 2
        %154 = OpCompositeExtract %float %150 3
        %155 = OpCompositeConstruct %v2float %151 %152
        %156 = OpCompositeConstruct %v2float %153 %154
        %157 = OpCompositeConstruct %mat2v2float %155 %156
        %158 = OpFOrdEqual %v2bool %155 %86
        %159 = OpAll %bool %158
        %160 = OpFOrdEqual %v2bool %156 %86
        %161 = OpAll %bool %160
        %162 = OpLogicalAnd %bool %159 %161
               OpBranch %148
        %148 = OpLabel
        %163 = OpPhi %bool %false %112 %162 %147
               OpStore %ok %163
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
        %166 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %167 = OpLoad %v4float %166
        %168 = OpCompositeExtract %float %167 0
        %169 = OpCompositeExtract %float %167 1
        %170 = OpCompositeExtract %float %167 2
        %171 = OpCompositeExtract %float %167 3
        %172 = OpCompositeConstruct %v2float %168 %169
        %173 = OpCompositeConstruct %v2float %170 %171
        %174 = OpCompositeConstruct %mat2v2float %172 %173
        %175 = OpFOrdEqual %v2bool %172 %86
        %176 = OpAll %bool %175
        %177 = OpFOrdEqual %v2bool %173 %86
        %178 = OpAll %bool %177
        %179 = OpLogicalAnd %bool %176 %178
               OpBranch %165
        %165 = OpLabel
        %180 = OpPhi %bool %false %148 %179 %164
               OpStore %ok %180
               OpSelectionMerge %182 None
               OpBranchConditional %180 %181 %182
        %181 = OpLabel
        %183 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %184 = OpLoad %v4float %183
        %185 = OpCompositeExtract %float %184 0
        %186 = OpFUnordNotEqual %bool %185 %float_0
        %187 = OpCompositeExtract %float %184 1
        %188 = OpFUnordNotEqual %bool %187 %float_0
        %189 = OpCompositeExtract %float %184 2
        %190 = OpFUnordNotEqual %bool %189 %float_0
        %191 = OpCompositeExtract %float %184 3
        %192 = OpFUnordNotEqual %bool %191 %float_0
        %194 = OpCompositeConstruct %v4bool %186 %188 %190 %192
        %195 = OpCompositeExtract %bool %194 0
        %196 = OpSelect %float %195 %float_1 %float_0
        %197 = OpCompositeExtract %bool %194 1
        %198 = OpSelect %float %197 %float_1 %float_0
        %199 = OpCompositeExtract %bool %194 2
        %200 = OpSelect %float %199 %float_1 %float_0
        %201 = OpCompositeExtract %bool %194 3
        %202 = OpSelect %float %201 %float_1 %float_0
        %203 = OpCompositeConstruct %v4float %196 %198 %200 %202
        %204 = OpCompositeExtract %float %203 0
        %205 = OpCompositeExtract %float %203 1
        %206 = OpCompositeExtract %float %203 2
        %207 = OpCompositeExtract %float %203 3
        %208 = OpCompositeConstruct %v2float %204 %205
        %209 = OpCompositeConstruct %v2float %206 %207
        %210 = OpCompositeConstruct %mat2v2float %208 %209
        %211 = OpFOrdEqual %v2bool %208 %86
        %212 = OpAll %bool %211
        %213 = OpFOrdEqual %v2bool %209 %86
        %214 = OpAll %bool %213
        %215 = OpLogicalAnd %bool %212 %214
               OpBranch %182
        %182 = OpLabel
        %216 = OpPhi %bool %false %165 %215 %181
               OpStore %ok %216
               OpSelectionMerge %218 None
               OpBranchConditional %216 %217 %218
        %217 = OpLabel
        %219 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %220 = OpLoad %v4float %219
        %221 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %223 = OpLoad %v4float %221
        %224 = OpFSub %v4float %220 %223
        %225 = OpCompositeExtract %float %224 0
        %226 = OpCompositeExtract %float %224 1
        %227 = OpCompositeExtract %float %224 2
        %228 = OpCompositeExtract %float %224 3
        %229 = OpCompositeConstruct %v2float %225 %226
        %230 = OpCompositeConstruct %v2float %227 %228
        %231 = OpCompositeConstruct %mat2v2float %229 %230
        %235 = OpFOrdEqual %v2bool %229 %233
        %236 = OpAll %bool %235
        %237 = OpFOrdEqual %v2bool %230 %16
        %238 = OpAll %bool %237
        %239 = OpLogicalAnd %bool %236 %238
               OpBranch %218
        %218 = OpLabel
        %240 = OpPhi %bool %false %182 %239 %217
               OpStore %ok %240
               OpSelectionMerge %242 None
               OpBranchConditional %240 %241 %242
        %241 = OpLabel
        %243 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %244 = OpLoad %v4float %243
        %247 = OpFAdd %v4float %244 %246
        %248 = OpCompositeExtract %float %247 0
        %249 = OpCompositeExtract %float %247 1
        %250 = OpCompositeExtract %float %247 2
        %251 = OpCompositeExtract %float %247 3
        %252 = OpCompositeConstruct %v2float %248 %249
        %253 = OpCompositeConstruct %v2float %250 %251
        %254 = OpCompositeConstruct %mat2v2float %252 %253
        %258 = OpFOrdEqual %v2bool %252 %256
        %259 = OpAll %bool %258
        %260 = OpFOrdEqual %v2bool %253 %256
        %261 = OpAll %bool %260
        %262 = OpLogicalAnd %bool %259 %261
               OpBranch %242
        %242 = OpLabel
        %263 = OpPhi %bool %false %218 %262 %241
               OpStore %ok %263
               OpSelectionMerge %268 None
               OpBranchConditional %263 %266 %267
        %266 = OpLabel
        %269 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %270 = OpLoad %v4float %269
               OpStore %264 %270
               OpBranch %268
        %267 = OpLabel
        %271 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %272 = OpLoad %v4float %271
               OpStore %264 %272
               OpBranch %268
        %268 = OpLabel
        %273 = OpLoad %v4float %264
               OpReturnValue %273
               OpFunctionEnd
