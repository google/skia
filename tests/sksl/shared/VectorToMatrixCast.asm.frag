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
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %ok "ok"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %48 = OpConstantComposite %v2float %float_n1_25 %float_0
         %49 = OpConstantComposite %v2float %float_0_75 %float_2_25
         %50 = OpConstantComposite %mat2v2float %48 %49
     %v2bool = OpTypeVector %bool 2
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %88 = OpConstantComposite %v2float %float_0 %float_1
         %89 = OpConstantComposite %mat2v2float %88 %88
      %v4int = OpTypeVector %int 4
     %v4bool = OpTypeVector %bool 4
      %int_1 = OpConstant %int 1
   %float_n1 = OpConstant %float -1
        %235 = OpConstantComposite %v2float %float_n1 %float_1
        %236 = OpConstantComposite %mat2v2float %235 %19
    %float_5 = OpConstant %float 5
        %248 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
    %float_6 = OpConstant %float 6
        %258 = OpConstantComposite %v2float %float_5 %float_6
        %259 = OpConstantComposite %mat2v2float %258 %258
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %ok = OpVariable %_ptr_Function_bool Function
        %266 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpSelectionMerge %31 None
               OpBranchConditional %true %30 %31
         %30 = OpLabel
         %32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 0
         %38 = OpCompositeExtract %float %36 1
         %39 = OpCompositeExtract %float %36 2
         %40 = OpCompositeExtract %float %36 3
         %41 = OpCompositeConstruct %v2float %37 %38
         %42 = OpCompositeConstruct %v2float %39 %40
         %44 = OpCompositeConstruct %mat2v2float %41 %42
         %52 = OpFOrdEqual %v2bool %41 %48
         %53 = OpAll %bool %52
         %54 = OpFOrdEqual %v2bool %42 %49
         %55 = OpAll %bool %54
         %56 = OpLogicalAnd %bool %53 %55
               OpBranch %31
         %31 = OpLabel
         %57 = OpPhi %bool %false %25 %56 %30
               OpStore %ok %57
               OpSelectionMerge %59 None
               OpBranchConditional %57 %58 %59
         %58 = OpLabel
         %60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %61 = OpLoad %v4float %60
         %62 = OpCompositeExtract %float %61 0
         %63 = OpCompositeExtract %float %61 1
         %64 = OpCompositeExtract %float %61 2
         %65 = OpCompositeExtract %float %61 3
         %66 = OpCompositeConstruct %v2float %62 %63
         %67 = OpCompositeConstruct %v2float %64 %65
         %68 = OpCompositeConstruct %mat2v2float %66 %67
         %69 = OpFOrdEqual %v2bool %66 %48
         %70 = OpAll %bool %69
         %71 = OpFOrdEqual %v2bool %67 %49
         %72 = OpAll %bool %71
         %73 = OpLogicalAnd %bool %70 %72
               OpBranch %59
         %59 = OpLabel
         %74 = OpPhi %bool %false %31 %73 %58
               OpStore %ok %74
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %79 = OpLoad %v4float %77
         %80 = OpCompositeExtract %float %79 0
         %81 = OpCompositeExtract %float %79 1
         %82 = OpCompositeExtract %float %79 2
         %83 = OpCompositeExtract %float %79 3
         %84 = OpCompositeConstruct %v2float %80 %81
         %85 = OpCompositeConstruct %v2float %82 %83
         %86 = OpCompositeConstruct %mat2v2float %84 %85
         %90 = OpFOrdEqual %v2bool %84 %88
         %91 = OpAll %bool %90
         %92 = OpFOrdEqual %v2bool %85 %88
         %93 = OpAll %bool %92
         %94 = OpLogicalAnd %bool %91 %93
               OpBranch %76
         %76 = OpLabel
         %95 = OpPhi %bool %false %59 %94 %75
               OpStore %ok %95
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
         %98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %99 = OpLoad %v4float %98
        %100 = OpCompositeExtract %float %99 0
        %101 = OpCompositeExtract %float %99 1
        %102 = OpCompositeExtract %float %99 2
        %103 = OpCompositeExtract %float %99 3
        %104 = OpCompositeConstruct %v2float %100 %101
        %105 = OpCompositeConstruct %v2float %102 %103
        %106 = OpCompositeConstruct %mat2v2float %104 %105
        %107 = OpFOrdEqual %v2bool %104 %88
        %108 = OpAll %bool %107
        %109 = OpFOrdEqual %v2bool %105 %88
        %110 = OpAll %bool %109
        %111 = OpLogicalAnd %bool %108 %110
               OpBranch %97
         %97 = OpLabel
        %112 = OpPhi %bool %false %76 %111 %96
               OpStore %ok %112
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %116 = OpLoad %v4float %115
        %117 = OpCompositeExtract %float %116 0
        %118 = OpConvertFToS %int %117
        %119 = OpCompositeExtract %float %116 1
        %120 = OpConvertFToS %int %119
        %121 = OpCompositeExtract %float %116 2
        %122 = OpConvertFToS %int %121
        %123 = OpCompositeExtract %float %116 3
        %124 = OpConvertFToS %int %123
        %126 = OpCompositeConstruct %v4int %118 %120 %122 %124
        %127 = OpCompositeExtract %int %126 0
        %128 = OpConvertSToF %float %127
        %129 = OpCompositeExtract %int %126 1
        %130 = OpConvertSToF %float %129
        %131 = OpCompositeExtract %int %126 2
        %132 = OpConvertSToF %float %131
        %133 = OpCompositeExtract %int %126 3
        %134 = OpConvertSToF %float %133
        %135 = OpCompositeConstruct %v4float %128 %130 %132 %134
        %136 = OpCompositeExtract %float %135 0
        %137 = OpCompositeExtract %float %135 1
        %138 = OpCompositeExtract %float %135 2
        %139 = OpCompositeExtract %float %135 3
        %140 = OpCompositeConstruct %v2float %136 %137
        %141 = OpCompositeConstruct %v2float %138 %139
        %142 = OpCompositeConstruct %mat2v2float %140 %141
        %143 = OpFOrdEqual %v2bool %140 %88
        %144 = OpAll %bool %143
        %145 = OpFOrdEqual %v2bool %141 %88
        %146 = OpAll %bool %145
        %147 = OpLogicalAnd %bool %144 %146
               OpBranch %114
        %114 = OpLabel
        %148 = OpPhi %bool %false %97 %147 %113
               OpStore %ok %148
               OpSelectionMerge %150 None
               OpBranchConditional %148 %149 %150
        %149 = OpLabel
        %151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %152 = OpLoad %v4float %151
        %153 = OpCompositeExtract %float %152 0
        %154 = OpCompositeExtract %float %152 1
        %155 = OpCompositeExtract %float %152 2
        %156 = OpCompositeExtract %float %152 3
        %157 = OpCompositeConstruct %v2float %153 %154
        %158 = OpCompositeConstruct %v2float %155 %156
        %159 = OpCompositeConstruct %mat2v2float %157 %158
        %160 = OpFOrdEqual %v2bool %157 %88
        %161 = OpAll %bool %160
        %162 = OpFOrdEqual %v2bool %158 %88
        %163 = OpAll %bool %162
        %164 = OpLogicalAnd %bool %161 %163
               OpBranch %150
        %150 = OpLabel
        %165 = OpPhi %bool %false %114 %164 %149
               OpStore %ok %165
               OpSelectionMerge %167 None
               OpBranchConditional %165 %166 %167
        %166 = OpLabel
        %168 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %169 = OpLoad %v4float %168
        %170 = OpCompositeExtract %float %169 0
        %171 = OpCompositeExtract %float %169 1
        %172 = OpCompositeExtract %float %169 2
        %173 = OpCompositeExtract %float %169 3
        %174 = OpCompositeConstruct %v2float %170 %171
        %175 = OpCompositeConstruct %v2float %172 %173
        %176 = OpCompositeConstruct %mat2v2float %174 %175
        %177 = OpFOrdEqual %v2bool %174 %88
        %178 = OpAll %bool %177
        %179 = OpFOrdEqual %v2bool %175 %88
        %180 = OpAll %bool %179
        %181 = OpLogicalAnd %bool %178 %180
               OpBranch %167
        %167 = OpLabel
        %182 = OpPhi %bool %false %150 %181 %166
               OpStore %ok %182
               OpSelectionMerge %184 None
               OpBranchConditional %182 %183 %184
        %183 = OpLabel
        %185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %186 = OpLoad %v4float %185
        %187 = OpCompositeExtract %float %186 0
        %188 = OpFUnordNotEqual %bool %187 %float_0
        %189 = OpCompositeExtract %float %186 1
        %190 = OpFUnordNotEqual %bool %189 %float_0
        %191 = OpCompositeExtract %float %186 2
        %192 = OpFUnordNotEqual %bool %191 %float_0
        %193 = OpCompositeExtract %float %186 3
        %194 = OpFUnordNotEqual %bool %193 %float_0
        %196 = OpCompositeConstruct %v4bool %188 %190 %192 %194
        %197 = OpCompositeExtract %bool %196 0
        %198 = OpSelect %float %197 %float_1 %float_0
        %199 = OpCompositeExtract %bool %196 1
        %200 = OpSelect %float %199 %float_1 %float_0
        %201 = OpCompositeExtract %bool %196 2
        %202 = OpSelect %float %201 %float_1 %float_0
        %203 = OpCompositeExtract %bool %196 3
        %204 = OpSelect %float %203 %float_1 %float_0
        %205 = OpCompositeConstruct %v4float %198 %200 %202 %204
        %206 = OpCompositeExtract %float %205 0
        %207 = OpCompositeExtract %float %205 1
        %208 = OpCompositeExtract %float %205 2
        %209 = OpCompositeExtract %float %205 3
        %210 = OpCompositeConstruct %v2float %206 %207
        %211 = OpCompositeConstruct %v2float %208 %209
        %212 = OpCompositeConstruct %mat2v2float %210 %211
        %213 = OpFOrdEqual %v2bool %210 %88
        %214 = OpAll %bool %213
        %215 = OpFOrdEqual %v2bool %211 %88
        %216 = OpAll %bool %215
        %217 = OpLogicalAnd %bool %214 %216
               OpBranch %184
        %184 = OpLabel
        %218 = OpPhi %bool %false %167 %217 %183
               OpStore %ok %218
               OpSelectionMerge %220 None
               OpBranchConditional %218 %219 %220
        %219 = OpLabel
        %221 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %222 = OpLoad %v4float %221
        %223 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %225 = OpLoad %v4float %223
        %226 = OpFSub %v4float %222 %225
        %227 = OpCompositeExtract %float %226 0
        %228 = OpCompositeExtract %float %226 1
        %229 = OpCompositeExtract %float %226 2
        %230 = OpCompositeExtract %float %226 3
        %231 = OpCompositeConstruct %v2float %227 %228
        %232 = OpCompositeConstruct %v2float %229 %230
        %233 = OpCompositeConstruct %mat2v2float %231 %232
        %237 = OpFOrdEqual %v2bool %231 %235
        %238 = OpAll %bool %237
        %239 = OpFOrdEqual %v2bool %232 %19
        %240 = OpAll %bool %239
        %241 = OpLogicalAnd %bool %238 %240
               OpBranch %220
        %220 = OpLabel
        %242 = OpPhi %bool %false %184 %241 %219
               OpStore %ok %242
               OpSelectionMerge %244 None
               OpBranchConditional %242 %243 %244
        %243 = OpLabel
        %245 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %246 = OpLoad %v4float %245
        %249 = OpFAdd %v4float %246 %248
        %250 = OpCompositeExtract %float %249 0
        %251 = OpCompositeExtract %float %249 1
        %252 = OpCompositeExtract %float %249 2
        %253 = OpCompositeExtract %float %249 3
        %254 = OpCompositeConstruct %v2float %250 %251
        %255 = OpCompositeConstruct %v2float %252 %253
        %256 = OpCompositeConstruct %mat2v2float %254 %255
        %260 = OpFOrdEqual %v2bool %254 %258
        %261 = OpAll %bool %260
        %262 = OpFOrdEqual %v2bool %255 %258
        %263 = OpAll %bool %262
        %264 = OpLogicalAnd %bool %261 %263
               OpBranch %244
        %244 = OpLabel
        %265 = OpPhi %bool %false %220 %264 %243
               OpStore %ok %265
               OpSelectionMerge %270 None
               OpBranchConditional %265 %268 %269
        %268 = OpLabel
        %271 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %272 = OpLoad %v4float %271
               OpStore %266 %272
               OpBranch %270
        %269 = OpLabel
        %273 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %274 = OpLoad %v4float %273
               OpStore %266 %274
               OpBranch %270
        %270 = OpLabel
        %275 = OpLoad %v4float %266
               OpReturnValue %275
               OpFunctionEnd
