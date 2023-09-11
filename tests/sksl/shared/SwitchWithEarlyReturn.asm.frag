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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %24 Binding 0
               OpDecorate %24 DescriptorSet 0
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %24 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %29 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %33 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %39 = OpTypeFunction %bool %_ptr_Function_int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
      %int_5 = OpConstant %int 5
        %188 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %29
         %30 = OpLabel
         %34 = OpVariable %_ptr_Function_v2float Function
               OpStore %34 %33
         %36 = OpFunctionCall %v4float %main %34
               OpStore %sk_FragColor %36
               OpReturn
               OpFunctionEnd
%return_in_one_case_bi = OpFunction %bool None %39
         %40 = OpFunctionParameter %_ptr_Function_int
         %41 = OpLabel
        %val = OpVariable %_ptr_Function_int Function
               OpStore %val %int_0
         %44 = OpLoad %int %40
               OpSelectionMerge %45 None
               OpSwitch %44 %47 1 %46
         %46 = OpLabel
         %49 = OpIAdd %int %int_0 %int_1
               OpStore %val %49
               OpReturnValue %false
         %47 = OpLabel
         %51 = OpLoad %int %val
         %52 = OpIAdd %int %51 %int_1
               OpStore %val %52
               OpBranch %45
         %45 = OpLabel
         %53 = OpLoad %int %val
         %54 = OpIEqual %bool %53 %int_1
               OpReturnValue %54
               OpFunctionEnd
%return_in_default_bi = OpFunction %bool None %39
         %55 = OpFunctionParameter %_ptr_Function_int
         %56 = OpLabel
         %57 = OpLoad %int %55
               OpSelectionMerge %58 None
               OpSwitch %57 %59
         %59 = OpLabel
               OpReturnValue %true
         %58 = OpLabel
               OpUnreachable
               OpFunctionEnd
%return_in_every_case_bi = OpFunction %bool None %39
         %61 = OpFunctionParameter %_ptr_Function_int
         %62 = OpLabel
         %63 = OpLoad %int %61
               OpSelectionMerge %64 None
               OpSwitch %63 %66 1 %65
         %65 = OpLabel
               OpReturnValue %false
         %66 = OpLabel
               OpReturnValue %true
         %64 = OpLabel
               OpUnreachable
               OpFunctionEnd
%return_in_every_case_no_default_bi = OpFunction %bool None %39
         %67 = OpFunctionParameter %_ptr_Function_int
         %68 = OpLabel
      %val_0 = OpVariable %_ptr_Function_int Function
               OpStore %val_0 %int_0
         %70 = OpLoad %int %67
               OpSelectionMerge %71 None
               OpSwitch %70 %71 1 %72 2 %73
         %72 = OpLabel
               OpReturnValue %false
         %73 = OpLabel
               OpReturnValue %true
         %71 = OpLabel
         %74 = OpIAdd %int %int_0 %int_1
               OpStore %val_0 %74
         %75 = OpIEqual %bool %74 %int_1
               OpReturnValue %75
               OpFunctionEnd
%case_has_break_before_return_bi = OpFunction %bool None %39
         %76 = OpFunctionParameter %_ptr_Function_int
         %77 = OpLabel
      %val_1 = OpVariable %_ptr_Function_int Function
               OpStore %val_1 %int_0
         %79 = OpLoad %int %76
               OpSelectionMerge %80 None
               OpSwitch %79 %83 1 %81 2 %82
         %81 = OpLabel
               OpBranch %80
         %82 = OpLabel
               OpReturnValue %true
         %83 = OpLabel
               OpReturnValue %true
         %80 = OpLabel
         %84 = OpIAdd %int %int_0 %int_1
               OpStore %val_1 %84
         %85 = OpIEqual %bool %84 %int_1
               OpReturnValue %85
               OpFunctionEnd
%case_has_break_after_return_bi = OpFunction %bool None %39
         %86 = OpFunctionParameter %_ptr_Function_int
         %87 = OpLabel
         %88 = OpLoad %int %86
               OpSelectionMerge %89 None
               OpSwitch %88 %92 1 %90 2 %91
         %90 = OpLabel
               OpReturnValue %false
         %91 = OpLabel
               OpReturnValue %true
         %92 = OpLabel
               OpReturnValue %true
         %89 = OpLabel
               OpUnreachable
               OpFunctionEnd
%no_return_in_default_bi = OpFunction %bool None %39
         %93 = OpFunctionParameter %_ptr_Function_int
         %94 = OpLabel
      %val_2 = OpVariable %_ptr_Function_int Function
               OpStore %val_2 %int_0
         %96 = OpLoad %int %93
               OpSelectionMerge %97 None
               OpSwitch %96 %100 1 %98 2 %99
         %98 = OpLabel
               OpReturnValue %false
         %99 = OpLabel
               OpReturnValue %true
        %100 = OpLabel
               OpBranch %97
         %97 = OpLabel
        %101 = OpIAdd %int %int_0 %int_1
               OpStore %val_2 %101
        %102 = OpIEqual %bool %101 %int_1
               OpReturnValue %102
               OpFunctionEnd
%empty_default_bi = OpFunction %bool None %39
        %103 = OpFunctionParameter %_ptr_Function_int
        %104 = OpLabel
      %val_3 = OpVariable %_ptr_Function_int Function
               OpStore %val_3 %int_0
        %106 = OpLoad %int %103
               OpSelectionMerge %107 None
               OpSwitch %106 %110 1 %108 2 %109
        %108 = OpLabel
               OpReturnValue %false
        %109 = OpLabel
               OpReturnValue %true
        %110 = OpLabel
               OpBranch %107
        %107 = OpLabel
        %111 = OpIAdd %int %int_0 %int_1
               OpStore %val_3 %111
        %112 = OpIEqual %bool %111 %int_1
               OpReturnValue %112
               OpFunctionEnd
%return_with_fallthrough_bi = OpFunction %bool None %39
        %113 = OpFunctionParameter %_ptr_Function_int
        %114 = OpLabel
        %115 = OpLoad %int %113
               OpSelectionMerge %116 None
               OpSwitch %115 %119 1 %118 2 %118
        %118 = OpLabel
               OpReturnValue %true
        %119 = OpLabel
               OpReturnValue %false
        %116 = OpLabel
               OpUnreachable
               OpFunctionEnd
%fallthrough_ends_in_break_bi = OpFunction %bool None %39
        %120 = OpFunctionParameter %_ptr_Function_int
        %121 = OpLabel
      %val_4 = OpVariable %_ptr_Function_int Function
               OpStore %val_4 %int_0
        %123 = OpLoad %int %120
               OpSelectionMerge %124 None
               OpSwitch %123 %127 1 %126 2 %126
        %126 = OpLabel
               OpBranch %124
        %127 = OpLabel
               OpReturnValue %false
        %124 = OpLabel
        %128 = OpIAdd %int %int_0 %int_1
               OpStore %val_4 %128
        %129 = OpIEqual %bool %128 %int_1
               OpReturnValue %129
               OpFunctionEnd
%fallthrough_to_default_with_break_bi = OpFunction %bool None %39
        %130 = OpFunctionParameter %_ptr_Function_int
        %131 = OpLabel
      %val_5 = OpVariable %_ptr_Function_int Function
               OpStore %val_5 %int_0
        %133 = OpLoad %int %130
               OpSelectionMerge %134 None
               OpSwitch %133 %137 1 %137 2 %137
        %137 = OpLabel
               OpBranch %134
        %134 = OpLabel
        %138 = OpIAdd %int %int_0 %int_1
               OpStore %val_5 %138
        %139 = OpIEqual %bool %138 %int_1
               OpReturnValue %139
               OpFunctionEnd
%fallthrough_to_default_with_return_bi = OpFunction %bool None %39
        %140 = OpFunctionParameter %_ptr_Function_int
        %141 = OpLabel
        %142 = OpLoad %int %140
               OpSelectionMerge %143 None
               OpSwitch %142 %146 1 %146 2 %146
        %146 = OpLabel
               OpReturnValue %true
        %143 = OpLabel
               OpUnreachable
               OpFunctionEnd
%fallthrough_with_loop_break_bi = OpFunction %bool None %39
        %147 = OpFunctionParameter %_ptr_Function_int
        %148 = OpLabel
      %val_6 = OpVariable %_ptr_Function_int Function
          %i = OpVariable %_ptr_Function_int Function
               OpStore %val_6 %int_0
        %150 = OpLoad %int %147
               OpSelectionMerge %151 None
               OpSwitch %150 %154 1 %152 2 %154
        %152 = OpLabel
               OpStore %i %int_0
               OpBranch %156
        %156 = OpLabel
               OpLoopMerge %160 %159 None
               OpBranch %157
        %157 = OpLabel
        %161 = OpLoad %int %i
        %163 = OpSLessThan %bool %161 %int_5
               OpBranchConditional %163 %158 %160
        %158 = OpLabel
        %164 = OpLoad %int %val_6
        %165 = OpIAdd %int %164 %int_1
               OpStore %val_6 %165
               OpBranch %160
        %159 = OpLabel
        %166 = OpLoad %int %i
        %167 = OpIAdd %int %166 %int_1
               OpStore %i %167
               OpBranch %156
        %160 = OpLabel
               OpBranch %154
        %154 = OpLabel
               OpReturnValue %true
        %151 = OpLabel
               OpUnreachable
               OpFunctionEnd
%fallthrough_with_loop_continue_bi = OpFunction %bool None %39
        %168 = OpFunctionParameter %_ptr_Function_int
        %169 = OpLabel
      %val_7 = OpVariable %_ptr_Function_int Function
        %i_0 = OpVariable %_ptr_Function_int Function
               OpStore %val_7 %int_0
        %171 = OpLoad %int %168
               OpSelectionMerge %172 None
               OpSwitch %171 %175 1 %173 2 %175
        %173 = OpLabel
               OpStore %i_0 %int_0
               OpBranch %177
        %177 = OpLabel
               OpLoopMerge %181 %180 None
               OpBranch %178
        %178 = OpLabel
        %182 = OpLoad %int %i_0
        %183 = OpSLessThan %bool %182 %int_5
               OpBranchConditional %183 %179 %181
        %179 = OpLabel
        %184 = OpLoad %int %val_7
        %185 = OpIAdd %int %184 %int_1
               OpStore %val_7 %185
               OpBranch %180
        %180 = OpLabel
        %186 = OpLoad %int %i_0
        %187 = OpIAdd %int %186 %int_1
               OpStore %i_0 %187
               OpBranch %177
        %181 = OpLabel
               OpBranch %175
        %175 = OpLabel
               OpReturnValue %true
        %172 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %v4float None %188
        %189 = OpFunctionParameter %_ptr_Function_v2float
        %190 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
        %197 = OpVariable %_ptr_Function_int Function
        %201 = OpVariable %_ptr_Function_int Function
        %206 = OpVariable %_ptr_Function_int Function
        %211 = OpVariable %_ptr_Function_int Function
        %216 = OpVariable %_ptr_Function_int Function
        %221 = OpVariable %_ptr_Function_int Function
        %226 = OpVariable %_ptr_Function_int Function
        %231 = OpVariable %_ptr_Function_int Function
        %236 = OpVariable %_ptr_Function_int Function
        %241 = OpVariable %_ptr_Function_int Function
        %246 = OpVariable %_ptr_Function_int Function
        %251 = OpVariable %_ptr_Function_int Function
        %256 = OpVariable %_ptr_Function_int Function
        %261 = OpVariable %_ptr_Function_int Function
        %264 = OpVariable %_ptr_Function_v4float Function
        %192 = OpAccessChain %_ptr_Uniform_v4float %24 %int_0
        %194 = OpLoad %v4float %192
        %195 = OpCompositeExtract %float %194 1
        %196 = OpConvertFToS %int %195
               OpStore %x %196
               OpStore %197 %196
        %198 = OpFunctionCall %bool %return_in_one_case_bi %197
               OpSelectionMerge %200 None
               OpBranchConditional %198 %199 %200
        %199 = OpLabel
               OpStore %201 %196
        %202 = OpFunctionCall %bool %return_in_default_bi %201
               OpBranch %200
        %200 = OpLabel
        %203 = OpPhi %bool %false %190 %202 %199
               OpSelectionMerge %205 None
               OpBranchConditional %203 %204 %205
        %204 = OpLabel
               OpStore %206 %196
        %207 = OpFunctionCall %bool %return_in_every_case_bi %206
               OpBranch %205
        %205 = OpLabel
        %208 = OpPhi %bool %false %200 %207 %204
               OpSelectionMerge %210 None
               OpBranchConditional %208 %209 %210
        %209 = OpLabel
               OpStore %211 %196
        %212 = OpFunctionCall %bool %return_in_every_case_no_default_bi %211
               OpBranch %210
        %210 = OpLabel
        %213 = OpPhi %bool %false %205 %212 %209
               OpSelectionMerge %215 None
               OpBranchConditional %213 %214 %215
        %214 = OpLabel
               OpStore %216 %196
        %217 = OpFunctionCall %bool %case_has_break_before_return_bi %216
               OpBranch %215
        %215 = OpLabel
        %218 = OpPhi %bool %false %210 %217 %214
               OpSelectionMerge %220 None
               OpBranchConditional %218 %219 %220
        %219 = OpLabel
               OpStore %221 %196
        %222 = OpFunctionCall %bool %case_has_break_after_return_bi %221
               OpBranch %220
        %220 = OpLabel
        %223 = OpPhi %bool %false %215 %222 %219
               OpSelectionMerge %225 None
               OpBranchConditional %223 %224 %225
        %224 = OpLabel
               OpStore %226 %196
        %227 = OpFunctionCall %bool %no_return_in_default_bi %226
               OpBranch %225
        %225 = OpLabel
        %228 = OpPhi %bool %false %220 %227 %224
               OpSelectionMerge %230 None
               OpBranchConditional %228 %229 %230
        %229 = OpLabel
               OpStore %231 %196
        %232 = OpFunctionCall %bool %empty_default_bi %231
               OpBranch %230
        %230 = OpLabel
        %233 = OpPhi %bool %false %225 %232 %229
               OpSelectionMerge %235 None
               OpBranchConditional %233 %234 %235
        %234 = OpLabel
               OpStore %236 %196
        %237 = OpFunctionCall %bool %return_with_fallthrough_bi %236
               OpBranch %235
        %235 = OpLabel
        %238 = OpPhi %bool %false %230 %237 %234
               OpSelectionMerge %240 None
               OpBranchConditional %238 %239 %240
        %239 = OpLabel
               OpStore %241 %196
        %242 = OpFunctionCall %bool %fallthrough_ends_in_break_bi %241
               OpBranch %240
        %240 = OpLabel
        %243 = OpPhi %bool %false %235 %242 %239
               OpSelectionMerge %245 None
               OpBranchConditional %243 %244 %245
        %244 = OpLabel
               OpStore %246 %196
        %247 = OpFunctionCall %bool %fallthrough_to_default_with_break_bi %246
               OpBranch %245
        %245 = OpLabel
        %248 = OpPhi %bool %false %240 %247 %244
               OpSelectionMerge %250 None
               OpBranchConditional %248 %249 %250
        %249 = OpLabel
               OpStore %251 %196
        %252 = OpFunctionCall %bool %fallthrough_to_default_with_return_bi %251
               OpBranch %250
        %250 = OpLabel
        %253 = OpPhi %bool %false %245 %252 %249
               OpSelectionMerge %255 None
               OpBranchConditional %253 %254 %255
        %254 = OpLabel
               OpStore %256 %196
        %257 = OpFunctionCall %bool %fallthrough_with_loop_break_bi %256
               OpBranch %255
        %255 = OpLabel
        %258 = OpPhi %bool %false %250 %257 %254
               OpSelectionMerge %260 None
               OpBranchConditional %258 %259 %260
        %259 = OpLabel
               OpStore %261 %196
        %262 = OpFunctionCall %bool %fallthrough_with_loop_continue_bi %261
               OpBranch %260
        %260 = OpLabel
        %263 = OpPhi %bool %false %255 %262 %259
               OpSelectionMerge %268 None
               OpBranchConditional %263 %266 %267
        %266 = OpLabel
        %269 = OpAccessChain %_ptr_Uniform_v4float %24 %int_0
        %270 = OpLoad %v4float %269
               OpStore %264 %270
               OpBranch %268
        %267 = OpLabel
        %271 = OpAccessChain %_ptr_Uniform_v4float %24 %int_1
        %272 = OpLoad %v4float %271
               OpStore %264 %272
               OpBranch %268
        %268 = OpLabel
        %273 = OpLoad %v4float %264
               OpReturnValue %273
               OpFunctionEnd
