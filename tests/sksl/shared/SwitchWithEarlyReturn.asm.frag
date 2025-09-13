               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %21
               OpName %_UniformBuffer "_UniformBuffer"  ; id %26
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %28
               OpName %return_in_one_case_bi "return_in_one_case_bi"    ; id %6
               OpName %val "val"                                        ; id %43
               OpName %return_in_default_bi "return_in_default_bi"      ; id %7
               OpName %return_in_every_case_bi "return_in_every_case_bi"    ; id %8
               OpName %return_in_every_case_no_default_bi "return_in_every_case_no_default_bi"  ; id %9
               OpName %val_0 "val"                                                              ; id %70
               OpName %case_has_break_before_return_bi "case_has_break_before_return_bi"        ; id %10
               OpName %val_1 "val"                                                              ; id %79
               OpName %case_has_break_after_return_bi "case_has_break_after_return_bi"          ; id %11
               OpName %no_return_in_default_bi "no_return_in_default_bi"                        ; id %12
               OpName %val_2 "val"                                                              ; id %96
               OpName %empty_default_bi "empty_default_bi"                                      ; id %13
               OpName %val_3 "val"                                                              ; id %106
               OpName %return_with_fallthrough_bi "return_with_fallthrough_bi"                  ; id %14
               OpName %fallthrough_ends_in_break_bi "fallthrough_ends_in_break_bi"              ; id %15
               OpName %val_4 "val"                                                              ; id %123
               OpName %fallthrough_to_default_with_break_bi "fallthrough_to_default_with_break_bi"  ; id %16
               OpName %val_5 "val"                                                                  ; id %133
               OpName %fallthrough_to_default_with_return_bi "fallthrough_to_default_with_return_bi"    ; id %17
               OpName %fallthrough_with_loop_break_bi "fallthrough_with_loop_break_bi"                  ; id %18
               OpName %val_6 "val"                                                                      ; id %150
               OpName %i "i"                                                                            ; id %156
               OpName %fallthrough_with_loop_continue_bi "fallthrough_with_loop_continue_bi"            ; id %19
               OpName %val_7 "val"                                                                      ; id %171
               OpName %i_0 "i"                                                                          ; id %177
               OpName %main "main"                                                                      ; id %20
               OpName %x "x"                                                                            ; id %192

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %25 Binding 0
               OpDecorate %25 DescriptorSet 0
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %25 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %34 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_int = OpTypePointer Function %int
         %40 = OpTypeFunction %bool %_ptr_Function_int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
      %int_5 = OpConstant %int 5
        %189 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %30

         %31 = OpLabel
         %35 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %35 %34
         %37 =   OpFunctionCall %v4float %main %35
                 OpStore %sk_FragColor %37
                 OpReturn
               OpFunctionEnd


               ; Function return_in_one_case_bi
%return_in_one_case_bi = OpFunction %bool None %40
         %41 = OpFunctionParameter %_ptr_Function_int

         %42 = OpLabel
        %val =   OpVariable %_ptr_Function_int Function
                 OpStore %val %int_0
         %45 =   OpLoad %int %41
                 OpSelectionMerge %46 None
                 OpSwitch %45 %48 1 %47

         %47 =     OpLabel
         %50 =       OpIAdd %int %int_0 %int_1
                     OpStore %val %50
                     OpReturnValue %false

         %48 =     OpLabel
         %52 =       OpLoad %int %val
         %53 =       OpIAdd %int %52 %int_1
                     OpStore %val %53
                     OpBranch %46

         %46 = OpLabel
         %54 =   OpLoad %int %val
         %55 =   OpIEqual %bool %54 %int_1
                 OpReturnValue %55
               OpFunctionEnd


               ; Function return_in_default_bi
%return_in_default_bi = OpFunction %bool None %40
         %56 = OpFunctionParameter %_ptr_Function_int

         %57 = OpLabel
         %58 =   OpLoad %int %56
                 OpSelectionMerge %59 None
                 OpSwitch %58 %60

         %60 =     OpLabel
                     OpReturnValue %true

         %59 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function return_in_every_case_bi
%return_in_every_case_bi = OpFunction %bool None %40
         %62 = OpFunctionParameter %_ptr_Function_int

         %63 = OpLabel
         %64 =   OpLoad %int %62
                 OpSelectionMerge %65 None
                 OpSwitch %64 %67 1 %66

         %66 =     OpLabel
                     OpReturnValue %false

         %67 =     OpLabel
                     OpReturnValue %true

         %65 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function return_in_every_case_no_default_bi
%return_in_every_case_no_default_bi = OpFunction %bool None %40
         %68 = OpFunctionParameter %_ptr_Function_int

         %69 = OpLabel
      %val_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_0 %int_0
         %71 =   OpLoad %int %68
                 OpSelectionMerge %72 None
                 OpSwitch %71 %72 1 %73 2 %74

         %73 =     OpLabel
                     OpReturnValue %false

         %74 =     OpLabel
                     OpReturnValue %true

         %72 = OpLabel
         %75 =   OpIAdd %int %int_0 %int_1
                 OpStore %val_0 %75
         %76 =   OpIEqual %bool %75 %int_1
                 OpReturnValue %76
               OpFunctionEnd


               ; Function case_has_break_before_return_bi
%case_has_break_before_return_bi = OpFunction %bool None %40
         %77 = OpFunctionParameter %_ptr_Function_int

         %78 = OpLabel
      %val_1 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_1 %int_0
         %80 =   OpLoad %int %77
                 OpSelectionMerge %81 None
                 OpSwitch %80 %84 1 %82 2 %83

         %82 =     OpLabel
                     OpBranch %81

         %83 =     OpLabel
                     OpReturnValue %true

         %84 =     OpLabel
                     OpReturnValue %true

         %81 = OpLabel
         %85 =   OpIAdd %int %int_0 %int_1
                 OpStore %val_1 %85
         %86 =   OpIEqual %bool %85 %int_1
                 OpReturnValue %86
               OpFunctionEnd


               ; Function case_has_break_after_return_bi
%case_has_break_after_return_bi = OpFunction %bool None %40
         %87 = OpFunctionParameter %_ptr_Function_int

         %88 = OpLabel
         %89 =   OpLoad %int %87
                 OpSelectionMerge %90 None
                 OpSwitch %89 %93 1 %91 2 %92

         %91 =     OpLabel
                     OpReturnValue %false

         %92 =     OpLabel
                     OpReturnValue %true

         %93 =     OpLabel
                     OpReturnValue %true

         %90 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function no_return_in_default_bi
%no_return_in_default_bi = OpFunction %bool None %40
         %94 = OpFunctionParameter %_ptr_Function_int

         %95 = OpLabel
      %val_2 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_2 %int_0
         %97 =   OpLoad %int %94
                 OpSelectionMerge %98 None
                 OpSwitch %97 %101 1 %99 2 %100

         %99 =     OpLabel
                     OpReturnValue %false

        %100 =     OpLabel
                     OpReturnValue %true

        %101 =     OpLabel
                     OpBranch %98

         %98 = OpLabel
        %102 =   OpIAdd %int %int_0 %int_1
                 OpStore %val_2 %102
        %103 =   OpIEqual %bool %102 %int_1
                 OpReturnValue %103
               OpFunctionEnd


               ; Function empty_default_bi
%empty_default_bi = OpFunction %bool None %40
        %104 = OpFunctionParameter %_ptr_Function_int

        %105 = OpLabel
      %val_3 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_3 %int_0
        %107 =   OpLoad %int %104
                 OpSelectionMerge %108 None
                 OpSwitch %107 %111 1 %109 2 %110

        %109 =     OpLabel
                     OpReturnValue %false

        %110 =     OpLabel
                     OpReturnValue %true

        %111 =     OpLabel
                     OpBranch %108

        %108 = OpLabel
        %112 =   OpIAdd %int %int_0 %int_1
                 OpStore %val_3 %112
        %113 =   OpIEqual %bool %112 %int_1
                 OpReturnValue %113
               OpFunctionEnd


               ; Function return_with_fallthrough_bi
%return_with_fallthrough_bi = OpFunction %bool None %40
        %114 = OpFunctionParameter %_ptr_Function_int

        %115 = OpLabel
        %116 =   OpLoad %int %114
                 OpSelectionMerge %117 None
                 OpSwitch %116 %120 1 %119 2 %119

        %119 =     OpLabel
                     OpReturnValue %true

        %120 =     OpLabel
                     OpReturnValue %false

        %117 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function fallthrough_ends_in_break_bi
%fallthrough_ends_in_break_bi = OpFunction %bool None %40
        %121 = OpFunctionParameter %_ptr_Function_int

        %122 = OpLabel
      %val_4 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_4 %int_0
        %124 =   OpLoad %int %121
                 OpSelectionMerge %125 None
                 OpSwitch %124 %128 1 %127 2 %127

        %127 =     OpLabel
                     OpBranch %125

        %128 =     OpLabel
                     OpReturnValue %false

        %125 = OpLabel
        %129 =   OpIAdd %int %int_0 %int_1
                 OpStore %val_4 %129
        %130 =   OpIEqual %bool %129 %int_1
                 OpReturnValue %130
               OpFunctionEnd


               ; Function fallthrough_to_default_with_break_bi
%fallthrough_to_default_with_break_bi = OpFunction %bool None %40
        %131 = OpFunctionParameter %_ptr_Function_int

        %132 = OpLabel
      %val_5 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_5 %int_0
        %134 =   OpLoad %int %131
                 OpSelectionMerge %135 None
                 OpSwitch %134 %138 1 %138 2 %138

        %138 =     OpLabel
                     OpBranch %135

        %135 = OpLabel
        %139 =   OpIAdd %int %int_0 %int_1
                 OpStore %val_5 %139
        %140 =   OpIEqual %bool %139 %int_1
                 OpReturnValue %140
               OpFunctionEnd


               ; Function fallthrough_to_default_with_return_bi
%fallthrough_to_default_with_return_bi = OpFunction %bool None %40
        %141 = OpFunctionParameter %_ptr_Function_int

        %142 = OpLabel
        %143 =   OpLoad %int %141
                 OpSelectionMerge %144 None
                 OpSwitch %143 %147 1 %147 2 %147

        %147 =     OpLabel
                     OpReturnValue %true

        %144 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function fallthrough_with_loop_break_bi
%fallthrough_with_loop_break_bi = OpFunction %bool None %40
        %148 = OpFunctionParameter %_ptr_Function_int

        %149 = OpLabel
      %val_6 =   OpVariable %_ptr_Function_int Function
          %i =   OpVariable %_ptr_Function_int Function
                 OpStore %val_6 %int_0
        %151 =   OpLoad %int %148
                 OpSelectionMerge %152 None
                 OpSwitch %151 %155 1 %153 2 %155

        %153 =     OpLabel
                     OpStore %i %int_0
                     OpBranch %157

        %157 =     OpLabel
                     OpLoopMerge %161 %160 None
                     OpBranch %158

        %158 =         OpLabel
        %162 =           OpLoad %int %i
        %164 =           OpSLessThan %bool %162 %int_5
                         OpBranchConditional %164 %159 %161

        %159 =             OpLabel
        %165 =               OpLoad %int %val_6
        %166 =               OpIAdd %int %165 %int_1
                             OpStore %val_6 %166
                             OpBranch %161

        %160 =       OpLabel
        %167 =         OpLoad %int %i
        %168 =         OpIAdd %int %167 %int_1
                       OpStore %i %168
                       OpBranch %157

        %161 =     OpLabel
                     OpBranch %155

        %155 =     OpLabel
                     OpReturnValue %true

        %152 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function fallthrough_with_loop_continue_bi
%fallthrough_with_loop_continue_bi = OpFunction %bool None %40
        %169 = OpFunctionParameter %_ptr_Function_int

        %170 = OpLabel
      %val_7 =   OpVariable %_ptr_Function_int Function
        %i_0 =   OpVariable %_ptr_Function_int Function
                 OpStore %val_7 %int_0
        %172 =   OpLoad %int %169
                 OpSelectionMerge %173 None
                 OpSwitch %172 %176 1 %174 2 %176

        %174 =     OpLabel
                     OpStore %i_0 %int_0
                     OpBranch %178

        %178 =     OpLabel
                     OpLoopMerge %182 %181 None
                     OpBranch %179

        %179 =         OpLabel
        %183 =           OpLoad %int %i_0
        %184 =           OpSLessThan %bool %183 %int_5
                         OpBranchConditional %184 %180 %182

        %180 =             OpLabel
        %185 =               OpLoad %int %val_7
        %186 =               OpIAdd %int %185 %int_1
                             OpStore %val_7 %186
                             OpBranch %181

        %181 =       OpLabel
        %187 =         OpLoad %int %i_0
        %188 =         OpIAdd %int %187 %int_1
                       OpStore %i_0 %188
                       OpBranch %178

        %182 =     OpLabel
                     OpBranch %176

        %176 =     OpLabel
                     OpReturnValue %true

        %173 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %189        ; RelaxedPrecision
        %190 = OpFunctionParameter %_ptr_Function_v2float

        %191 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
        %198 =   OpVariable %_ptr_Function_int Function
        %202 =   OpVariable %_ptr_Function_int Function
        %207 =   OpVariable %_ptr_Function_int Function
        %212 =   OpVariable %_ptr_Function_int Function
        %217 =   OpVariable %_ptr_Function_int Function
        %222 =   OpVariable %_ptr_Function_int Function
        %227 =   OpVariable %_ptr_Function_int Function
        %232 =   OpVariable %_ptr_Function_int Function
        %237 =   OpVariable %_ptr_Function_int Function
        %242 =   OpVariable %_ptr_Function_int Function
        %247 =   OpVariable %_ptr_Function_int Function
        %252 =   OpVariable %_ptr_Function_int Function
        %257 =   OpVariable %_ptr_Function_int Function
        %262 =   OpVariable %_ptr_Function_int Function
        %265 =   OpVariable %_ptr_Function_v4float Function
        %193 =   OpAccessChain %_ptr_Uniform_v4float %25 %int_0
        %195 =   OpLoad %v4float %193               ; RelaxedPrecision
        %196 =   OpCompositeExtract %float %195 1   ; RelaxedPrecision
        %197 =   OpConvertFToS %int %196
                 OpStore %x %197
                 OpStore %198 %197
        %199 =   OpFunctionCall %bool %return_in_one_case_bi %198
                 OpSelectionMerge %201 None
                 OpBranchConditional %199 %200 %201

        %200 =     OpLabel
                     OpStore %202 %197
        %203 =       OpFunctionCall %bool %return_in_default_bi %202
                     OpBranch %201

        %201 = OpLabel
        %204 =   OpPhi %bool %false %191 %203 %200
                 OpSelectionMerge %206 None
                 OpBranchConditional %204 %205 %206

        %205 =     OpLabel
                     OpStore %207 %197
        %208 =       OpFunctionCall %bool %return_in_every_case_bi %207
                     OpBranch %206

        %206 = OpLabel
        %209 =   OpPhi %bool %false %201 %208 %205
                 OpSelectionMerge %211 None
                 OpBranchConditional %209 %210 %211

        %210 =     OpLabel
                     OpStore %212 %197
        %213 =       OpFunctionCall %bool %return_in_every_case_no_default_bi %212
                     OpBranch %211

        %211 = OpLabel
        %214 =   OpPhi %bool %false %206 %213 %210
                 OpSelectionMerge %216 None
                 OpBranchConditional %214 %215 %216

        %215 =     OpLabel
                     OpStore %217 %197
        %218 =       OpFunctionCall %bool %case_has_break_before_return_bi %217
                     OpBranch %216

        %216 = OpLabel
        %219 =   OpPhi %bool %false %211 %218 %215
                 OpSelectionMerge %221 None
                 OpBranchConditional %219 %220 %221

        %220 =     OpLabel
                     OpStore %222 %197
        %223 =       OpFunctionCall %bool %case_has_break_after_return_bi %222
                     OpBranch %221

        %221 = OpLabel
        %224 =   OpPhi %bool %false %216 %223 %220
                 OpSelectionMerge %226 None
                 OpBranchConditional %224 %225 %226

        %225 =     OpLabel
                     OpStore %227 %197
        %228 =       OpFunctionCall %bool %no_return_in_default_bi %227
                     OpBranch %226

        %226 = OpLabel
        %229 =   OpPhi %bool %false %221 %228 %225
                 OpSelectionMerge %231 None
                 OpBranchConditional %229 %230 %231

        %230 =     OpLabel
                     OpStore %232 %197
        %233 =       OpFunctionCall %bool %empty_default_bi %232
                     OpBranch %231

        %231 = OpLabel
        %234 =   OpPhi %bool %false %226 %233 %230
                 OpSelectionMerge %236 None
                 OpBranchConditional %234 %235 %236

        %235 =     OpLabel
                     OpStore %237 %197
        %238 =       OpFunctionCall %bool %return_with_fallthrough_bi %237
                     OpBranch %236

        %236 = OpLabel
        %239 =   OpPhi %bool %false %231 %238 %235
                 OpSelectionMerge %241 None
                 OpBranchConditional %239 %240 %241

        %240 =     OpLabel
                     OpStore %242 %197
        %243 =       OpFunctionCall %bool %fallthrough_ends_in_break_bi %242
                     OpBranch %241

        %241 = OpLabel
        %244 =   OpPhi %bool %false %236 %243 %240
                 OpSelectionMerge %246 None
                 OpBranchConditional %244 %245 %246

        %245 =     OpLabel
                     OpStore %247 %197
        %248 =       OpFunctionCall %bool %fallthrough_to_default_with_break_bi %247
                     OpBranch %246

        %246 = OpLabel
        %249 =   OpPhi %bool %false %241 %248 %245
                 OpSelectionMerge %251 None
                 OpBranchConditional %249 %250 %251

        %250 =     OpLabel
                     OpStore %252 %197
        %253 =       OpFunctionCall %bool %fallthrough_to_default_with_return_bi %252
                     OpBranch %251

        %251 = OpLabel
        %254 =   OpPhi %bool %false %246 %253 %250
                 OpSelectionMerge %256 None
                 OpBranchConditional %254 %255 %256

        %255 =     OpLabel
                     OpStore %257 %197
        %258 =       OpFunctionCall %bool %fallthrough_with_loop_break_bi %257
                     OpBranch %256

        %256 = OpLabel
        %259 =   OpPhi %bool %false %251 %258 %255
                 OpSelectionMerge %261 None
                 OpBranchConditional %259 %260 %261

        %260 =     OpLabel
                     OpStore %262 %197
        %263 =       OpFunctionCall %bool %fallthrough_with_loop_continue_bi %262
                     OpBranch %261

        %261 = OpLabel
        %264 =   OpPhi %bool %false %256 %263 %260
                 OpSelectionMerge %269 None
                 OpBranchConditional %264 %267 %268

        %267 =     OpLabel
        %270 =       OpAccessChain %_ptr_Uniform_v4float %25 %int_0
        %271 =       OpLoad %v4float %270           ; RelaxedPrecision
                     OpStore %265 %271
                     OpBranch %269

        %268 =     OpLabel
        %272 =       OpAccessChain %_ptr_Uniform_v4float %25 %int_1
        %273 =       OpLoad %v4float %272           ; RelaxedPrecision
                     OpStore %265 %273
                     OpBranch %269

        %269 = OpLabel
        %274 =   OpLoad %v4float %265               ; RelaxedPrecision
                 OpReturnValue %274
               OpFunctionEnd
