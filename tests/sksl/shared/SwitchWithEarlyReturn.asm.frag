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
               OpName %return_in_one_case_bi "return_in_one_case_bi"
               OpName %val "val"
               OpName %return_in_default_bi "return_in_default_bi"
               OpName %return_in_every_case_bi "return_in_every_case_bi"
               OpName %return_in_every_case_no_default_bi "return_in_every_case_no_default_bi"
               OpName %val_0 "val"
               OpName %case_has_break_before_return_bi "case_has_break_before_return_bi"
               OpName %val_1 "val"
               OpName %case_has_break_after_return_bi "case_has_break_after_return_bi"
               OpName %no_return_in_default_bi "no_return_in_default_bi"
               OpName %val_2 "val"
               OpName %empty_default_bi "empty_default_bi"
               OpName %val_3 "val"
               OpName %return_with_fallthrough_bi "return_with_fallthrough_bi"
               OpName %fallthrough_ends_in_break_bi "fallthrough_ends_in_break_bi"
               OpName %val_4 "val"
               OpName %fallthrough_to_default_with_break_bi "fallthrough_to_default_with_break_bi"
               OpName %val_5 "val"
               OpName %fallthrough_to_default_with_return_bi "fallthrough_to_default_with_return_bi"
               OpName %fallthrough_with_loop_break_bi "fallthrough_with_loop_break_bi"
               OpName %val_6 "val"
               OpName %i "i"
               OpName %fallthrough_with_loop_continue_bi "fallthrough_with_loop_continue_bi"
               OpName %val_7 "val"
               OpName %i_0 "i"
               OpName %main "main"
               OpName %x "x"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %21 Binding 0
               OpDecorate %21 DescriptorSet 0
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %21 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %30 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %37 = OpTypeFunction %bool %_ptr_Function_int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
      %int_5 = OpConstant %int 5
        %186 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %26
         %27 = OpLabel
         %31 = OpVariable %_ptr_Function_v2float Function
               OpStore %31 %30
         %33 = OpFunctionCall %v4float %main %31
               OpStore %sk_FragColor %33
               OpReturn
               OpFunctionEnd
%return_in_one_case_bi = OpFunction %bool None %37
         %38 = OpFunctionParameter %_ptr_Function_int
         %39 = OpLabel
        %val = OpVariable %_ptr_Function_int Function
               OpStore %val %int_0
         %42 = OpLoad %int %38
               OpSelectionMerge %43 None
               OpSwitch %42 %45 1 %44
         %44 = OpLabel
         %47 = OpIAdd %int %int_0 %int_1
               OpStore %val %47
               OpReturnValue %false
         %45 = OpLabel
         %49 = OpLoad %int %val
         %50 = OpIAdd %int %49 %int_1
               OpStore %val %50
               OpBranch %43
         %43 = OpLabel
         %51 = OpLoad %int %val
         %52 = OpIEqual %bool %51 %int_1
               OpReturnValue %52
               OpFunctionEnd
%return_in_default_bi = OpFunction %bool None %37
         %53 = OpFunctionParameter %_ptr_Function_int
         %54 = OpLabel
         %55 = OpLoad %int %53
               OpSelectionMerge %56 None
               OpSwitch %55 %57
         %57 = OpLabel
               OpReturnValue %true
         %56 = OpLabel
               OpUnreachable
               OpFunctionEnd
%return_in_every_case_bi = OpFunction %bool None %37
         %59 = OpFunctionParameter %_ptr_Function_int
         %60 = OpLabel
         %61 = OpLoad %int %59
               OpSelectionMerge %62 None
               OpSwitch %61 %64 1 %63
         %63 = OpLabel
               OpReturnValue %false
         %64 = OpLabel
               OpReturnValue %true
         %62 = OpLabel
               OpUnreachable
               OpFunctionEnd
%return_in_every_case_no_default_bi = OpFunction %bool None %37
         %65 = OpFunctionParameter %_ptr_Function_int
         %66 = OpLabel
      %val_0 = OpVariable %_ptr_Function_int Function
               OpStore %val_0 %int_0
         %68 = OpLoad %int %65
               OpSelectionMerge %69 None
               OpSwitch %68 %69 1 %70 2 %71
         %70 = OpLabel
               OpReturnValue %false
         %71 = OpLabel
               OpReturnValue %true
         %69 = OpLabel
         %72 = OpIAdd %int %int_0 %int_1
               OpStore %val_0 %72
         %73 = OpIEqual %bool %72 %int_1
               OpReturnValue %73
               OpFunctionEnd
%case_has_break_before_return_bi = OpFunction %bool None %37
         %74 = OpFunctionParameter %_ptr_Function_int
         %75 = OpLabel
      %val_1 = OpVariable %_ptr_Function_int Function
               OpStore %val_1 %int_0
         %77 = OpLoad %int %74
               OpSelectionMerge %78 None
               OpSwitch %77 %81 1 %79 2 %80
         %79 = OpLabel
               OpBranch %78
         %80 = OpLabel
               OpReturnValue %true
         %81 = OpLabel
               OpReturnValue %true
         %78 = OpLabel
         %82 = OpIAdd %int %int_0 %int_1
               OpStore %val_1 %82
         %83 = OpIEqual %bool %82 %int_1
               OpReturnValue %83
               OpFunctionEnd
%case_has_break_after_return_bi = OpFunction %bool None %37
         %84 = OpFunctionParameter %_ptr_Function_int
         %85 = OpLabel
         %86 = OpLoad %int %84
               OpSelectionMerge %87 None
               OpSwitch %86 %90 1 %88 2 %89
         %88 = OpLabel
               OpReturnValue %false
         %89 = OpLabel
               OpReturnValue %true
         %90 = OpLabel
               OpReturnValue %true
         %87 = OpLabel
               OpUnreachable
               OpFunctionEnd
%no_return_in_default_bi = OpFunction %bool None %37
         %91 = OpFunctionParameter %_ptr_Function_int
         %92 = OpLabel
      %val_2 = OpVariable %_ptr_Function_int Function
               OpStore %val_2 %int_0
         %94 = OpLoad %int %91
               OpSelectionMerge %95 None
               OpSwitch %94 %98 1 %96 2 %97
         %96 = OpLabel
               OpReturnValue %false
         %97 = OpLabel
               OpReturnValue %true
         %98 = OpLabel
               OpBranch %95
         %95 = OpLabel
         %99 = OpIAdd %int %int_0 %int_1
               OpStore %val_2 %99
        %100 = OpIEqual %bool %99 %int_1
               OpReturnValue %100
               OpFunctionEnd
%empty_default_bi = OpFunction %bool None %37
        %101 = OpFunctionParameter %_ptr_Function_int
        %102 = OpLabel
      %val_3 = OpVariable %_ptr_Function_int Function
               OpStore %val_3 %int_0
        %104 = OpLoad %int %101
               OpSelectionMerge %105 None
               OpSwitch %104 %108 1 %106 2 %107
        %106 = OpLabel
               OpReturnValue %false
        %107 = OpLabel
               OpReturnValue %true
        %108 = OpLabel
               OpBranch %105
        %105 = OpLabel
        %109 = OpIAdd %int %int_0 %int_1
               OpStore %val_3 %109
        %110 = OpIEqual %bool %109 %int_1
               OpReturnValue %110
               OpFunctionEnd
%return_with_fallthrough_bi = OpFunction %bool None %37
        %111 = OpFunctionParameter %_ptr_Function_int
        %112 = OpLabel
        %113 = OpLoad %int %111
               OpSelectionMerge %114 None
               OpSwitch %113 %117 1 %116 2 %116
        %116 = OpLabel
               OpReturnValue %true
        %117 = OpLabel
               OpReturnValue %false
        %114 = OpLabel
               OpUnreachable
               OpFunctionEnd
%fallthrough_ends_in_break_bi = OpFunction %bool None %37
        %118 = OpFunctionParameter %_ptr_Function_int
        %119 = OpLabel
      %val_4 = OpVariable %_ptr_Function_int Function
               OpStore %val_4 %int_0
        %121 = OpLoad %int %118
               OpSelectionMerge %122 None
               OpSwitch %121 %125 1 %124 2 %124
        %124 = OpLabel
               OpBranch %122
        %125 = OpLabel
               OpReturnValue %false
        %122 = OpLabel
        %126 = OpIAdd %int %int_0 %int_1
               OpStore %val_4 %126
        %127 = OpIEqual %bool %126 %int_1
               OpReturnValue %127
               OpFunctionEnd
%fallthrough_to_default_with_break_bi = OpFunction %bool None %37
        %128 = OpFunctionParameter %_ptr_Function_int
        %129 = OpLabel
      %val_5 = OpVariable %_ptr_Function_int Function
               OpStore %val_5 %int_0
        %131 = OpLoad %int %128
               OpSelectionMerge %132 None
               OpSwitch %131 %135 1 %135 2 %135
        %135 = OpLabel
               OpBranch %132
        %132 = OpLabel
        %136 = OpIAdd %int %int_0 %int_1
               OpStore %val_5 %136
        %137 = OpIEqual %bool %136 %int_1
               OpReturnValue %137
               OpFunctionEnd
%fallthrough_to_default_with_return_bi = OpFunction %bool None %37
        %138 = OpFunctionParameter %_ptr_Function_int
        %139 = OpLabel
        %140 = OpLoad %int %138
               OpSelectionMerge %141 None
               OpSwitch %140 %144 1 %144 2 %144
        %144 = OpLabel
               OpReturnValue %true
        %141 = OpLabel
               OpUnreachable
               OpFunctionEnd
%fallthrough_with_loop_break_bi = OpFunction %bool None %37
        %145 = OpFunctionParameter %_ptr_Function_int
        %146 = OpLabel
      %val_6 = OpVariable %_ptr_Function_int Function
          %i = OpVariable %_ptr_Function_int Function
               OpStore %val_6 %int_0
        %148 = OpLoad %int %145
               OpSelectionMerge %149 None
               OpSwitch %148 %152 1 %150 2 %152
        %150 = OpLabel
               OpStore %i %int_0
               OpBranch %154
        %154 = OpLabel
               OpLoopMerge %158 %157 None
               OpBranch %155
        %155 = OpLabel
        %159 = OpLoad %int %i
        %161 = OpSLessThan %bool %159 %int_5
               OpBranchConditional %161 %156 %158
        %156 = OpLabel
        %162 = OpLoad %int %val_6
        %163 = OpIAdd %int %162 %int_1
               OpStore %val_6 %163
               OpBranch %158
        %157 = OpLabel
        %164 = OpLoad %int %i
        %165 = OpIAdd %int %164 %int_1
               OpStore %i %165
               OpBranch %154
        %158 = OpLabel
               OpBranch %152
        %152 = OpLabel
               OpReturnValue %true
        %149 = OpLabel
               OpUnreachable
               OpFunctionEnd
%fallthrough_with_loop_continue_bi = OpFunction %bool None %37
        %166 = OpFunctionParameter %_ptr_Function_int
        %167 = OpLabel
      %val_7 = OpVariable %_ptr_Function_int Function
        %i_0 = OpVariable %_ptr_Function_int Function
               OpStore %val_7 %int_0
        %169 = OpLoad %int %166
               OpSelectionMerge %170 None
               OpSwitch %169 %173 1 %171 2 %173
        %171 = OpLabel
               OpStore %i_0 %int_0
               OpBranch %175
        %175 = OpLabel
               OpLoopMerge %179 %178 None
               OpBranch %176
        %176 = OpLabel
        %180 = OpLoad %int %i_0
        %181 = OpSLessThan %bool %180 %int_5
               OpBranchConditional %181 %177 %179
        %177 = OpLabel
        %182 = OpLoad %int %val_7
        %183 = OpIAdd %int %182 %int_1
               OpStore %val_7 %183
               OpBranch %178
        %178 = OpLabel
        %184 = OpLoad %int %i_0
        %185 = OpIAdd %int %184 %int_1
               OpStore %i_0 %185
               OpBranch %175
        %179 = OpLabel
               OpBranch %173
        %173 = OpLabel
               OpReturnValue %true
        %170 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %186
        %187 = OpFunctionParameter %_ptr_Function_v2float
        %188 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
        %195 = OpVariable %_ptr_Function_int Function
        %199 = OpVariable %_ptr_Function_int Function
        %204 = OpVariable %_ptr_Function_int Function
        %209 = OpVariable %_ptr_Function_int Function
        %214 = OpVariable %_ptr_Function_int Function
        %219 = OpVariable %_ptr_Function_int Function
        %224 = OpVariable %_ptr_Function_int Function
        %229 = OpVariable %_ptr_Function_int Function
        %234 = OpVariable %_ptr_Function_int Function
        %239 = OpVariable %_ptr_Function_int Function
        %244 = OpVariable %_ptr_Function_int Function
        %249 = OpVariable %_ptr_Function_int Function
        %254 = OpVariable %_ptr_Function_int Function
        %259 = OpVariable %_ptr_Function_int Function
        %262 = OpVariable %_ptr_Function_v4float Function
        %190 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
        %192 = OpLoad %v4float %190
        %193 = OpCompositeExtract %float %192 1
        %194 = OpConvertFToS %int %193
               OpStore %x %194
               OpStore %195 %194
        %196 = OpFunctionCall %bool %return_in_one_case_bi %195
               OpSelectionMerge %198 None
               OpBranchConditional %196 %197 %198
        %197 = OpLabel
               OpStore %199 %194
        %200 = OpFunctionCall %bool %return_in_default_bi %199
               OpBranch %198
        %198 = OpLabel
        %201 = OpPhi %bool %false %188 %200 %197
               OpSelectionMerge %203 None
               OpBranchConditional %201 %202 %203
        %202 = OpLabel
               OpStore %204 %194
        %205 = OpFunctionCall %bool %return_in_every_case_bi %204
               OpBranch %203
        %203 = OpLabel
        %206 = OpPhi %bool %false %198 %205 %202
               OpSelectionMerge %208 None
               OpBranchConditional %206 %207 %208
        %207 = OpLabel
               OpStore %209 %194
        %210 = OpFunctionCall %bool %return_in_every_case_no_default_bi %209
               OpBranch %208
        %208 = OpLabel
        %211 = OpPhi %bool %false %203 %210 %207
               OpSelectionMerge %213 None
               OpBranchConditional %211 %212 %213
        %212 = OpLabel
               OpStore %214 %194
        %215 = OpFunctionCall %bool %case_has_break_before_return_bi %214
               OpBranch %213
        %213 = OpLabel
        %216 = OpPhi %bool %false %208 %215 %212
               OpSelectionMerge %218 None
               OpBranchConditional %216 %217 %218
        %217 = OpLabel
               OpStore %219 %194
        %220 = OpFunctionCall %bool %case_has_break_after_return_bi %219
               OpBranch %218
        %218 = OpLabel
        %221 = OpPhi %bool %false %213 %220 %217
               OpSelectionMerge %223 None
               OpBranchConditional %221 %222 %223
        %222 = OpLabel
               OpStore %224 %194
        %225 = OpFunctionCall %bool %no_return_in_default_bi %224
               OpBranch %223
        %223 = OpLabel
        %226 = OpPhi %bool %false %218 %225 %222
               OpSelectionMerge %228 None
               OpBranchConditional %226 %227 %228
        %227 = OpLabel
               OpStore %229 %194
        %230 = OpFunctionCall %bool %empty_default_bi %229
               OpBranch %228
        %228 = OpLabel
        %231 = OpPhi %bool %false %223 %230 %227
               OpSelectionMerge %233 None
               OpBranchConditional %231 %232 %233
        %232 = OpLabel
               OpStore %234 %194
        %235 = OpFunctionCall %bool %return_with_fallthrough_bi %234
               OpBranch %233
        %233 = OpLabel
        %236 = OpPhi %bool %false %228 %235 %232
               OpSelectionMerge %238 None
               OpBranchConditional %236 %237 %238
        %237 = OpLabel
               OpStore %239 %194
        %240 = OpFunctionCall %bool %fallthrough_ends_in_break_bi %239
               OpBranch %238
        %238 = OpLabel
        %241 = OpPhi %bool %false %233 %240 %237
               OpSelectionMerge %243 None
               OpBranchConditional %241 %242 %243
        %242 = OpLabel
               OpStore %244 %194
        %245 = OpFunctionCall %bool %fallthrough_to_default_with_break_bi %244
               OpBranch %243
        %243 = OpLabel
        %246 = OpPhi %bool %false %238 %245 %242
               OpSelectionMerge %248 None
               OpBranchConditional %246 %247 %248
        %247 = OpLabel
               OpStore %249 %194
        %250 = OpFunctionCall %bool %fallthrough_to_default_with_return_bi %249
               OpBranch %248
        %248 = OpLabel
        %251 = OpPhi %bool %false %243 %250 %247
               OpSelectionMerge %253 None
               OpBranchConditional %251 %252 %253
        %252 = OpLabel
               OpStore %254 %194
        %255 = OpFunctionCall %bool %fallthrough_with_loop_break_bi %254
               OpBranch %253
        %253 = OpLabel
        %256 = OpPhi %bool %false %248 %255 %252
               OpSelectionMerge %258 None
               OpBranchConditional %256 %257 %258
        %257 = OpLabel
               OpStore %259 %194
        %260 = OpFunctionCall %bool %fallthrough_with_loop_continue_bi %259
               OpBranch %258
        %258 = OpLabel
        %261 = OpPhi %bool %false %253 %260 %257
               OpSelectionMerge %266 None
               OpBranchConditional %261 %264 %265
        %264 = OpLabel
        %267 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
        %268 = OpLoad %v4float %267
               OpStore %262 %268
               OpBranch %266
        %265 = OpLabel
        %269 = OpAccessChain %_ptr_Uniform_v4float %21 %int_1
        %270 = OpLoad %v4float %269
               OpStore %262 %270
               OpBranch %266
        %266 = OpLabel
        %271 = OpLoad %v4float %262
               OpReturnValue %271
               OpFunctionEnd
