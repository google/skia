               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "y"
               OpName %returns_a_struct_S "returns_a_struct_S"
               OpName %s "s"
               OpName %constructs_a_struct_S "constructs_a_struct_S"
               OpName %accepts_a_struct_fS "accepts_a_struct_fS"
               OpName %modifies_a_struct_vS "modifies_a_struct_vS"
               OpName %main "main"
               OpName %s_0 "s"
               OpName %x "x"
               OpName %expected "expected"
               OpName %Nested "Nested"
               OpMemberName %Nested 0 "a"
               OpMemberName %Nested 1 "b"
               OpName %n1 "n1"
               OpName %n2 "n2"
               OpName %n3 "n3"
               OpName %Compound "Compound"
               OpMemberName %Compound 0 "f4"
               OpMemberName %Compound 1 "i3"
               OpName %c1 "c1"
               OpName %c2 "c2"
               OpName %c3 "c3"
               OpName %valid "valid"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %14 Binding 0
               OpDecorate %14 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %41 RelaxedPrecision
               OpMemberDecorate %Nested 0 Offset 0
               OpMemberDecorate %Nested 0 RelaxedPrecision
               OpMemberDecorate %Nested 1 Offset 16
               OpMemberDecorate %Nested 1 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpMemberDecorate %Compound 0 Offset 0
               OpMemberDecorate %Compound 1 Offset 16
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %float %int
         %29 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
    %float_1 = OpConstant %float 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
    %float_2 = OpConstant %float 2
      %int_3 = OpConstant %int 3
         %46 = OpTypeFunction %float %_ptr_Function_S
         %55 = OpTypeFunction %void %_ptr_Function_S
         %64 = OpTypeFunction %v4float %_ptr_Function_v2float
     %Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
      %v3int = OpTypeVector %int 3
   %Compound = OpTypeStruct %v4float %v3int
%_ptr_Function_Compound = OpTypePointer Function %Compound
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %99 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
      %int_7 = OpConstant %int 7
        %103 = OpConstantComposite %v3int %int_5 %int_6 %int_7
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
         %20 = OpLabel
         %24 = OpVariable %_ptr_Function_v2float Function
               OpStore %24 %23
         %26 = OpFunctionCall %v4float %main %24
               OpStore %sk_FragColor %26
               OpReturn
               OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %29
         %30 = OpLabel
          %s = OpVariable %_ptr_Function_S Function
         %35 = OpAccessChain %_ptr_Function_float %s %int_0
               OpStore %35 %float_1
         %39 = OpAccessChain %_ptr_Function_int %s %int_1
               OpStore %39 %int_2
         %41 = OpLoad %S %s
               OpReturnValue %41
               OpFunctionEnd
%constructs_a_struct_S = OpFunction %S None %29
         %42 = OpLabel
         %45 = OpCompositeConstruct %S %float_2 %int_3
               OpReturnValue %45
               OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %46
         %47 = OpFunctionParameter %_ptr_Function_S
         %48 = OpLabel
         %49 = OpAccessChain %_ptr_Function_float %47 %int_0
         %50 = OpLoad %float %49
         %51 = OpAccessChain %_ptr_Function_int %47 %int_1
         %52 = OpLoad %int %51
         %53 = OpConvertSToF %float %52
         %54 = OpFAdd %float %50 %53
               OpReturnValue %54
               OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %55
         %56 = OpFunctionParameter %_ptr_Function_S
         %57 = OpLabel
         %58 = OpAccessChain %_ptr_Function_float %56 %int_0
         %59 = OpLoad %float %58
         %60 = OpFAdd %float %59 %float_1
               OpStore %58 %60
         %61 = OpAccessChain %_ptr_Function_int %56 %int_1
         %62 = OpLoad %int %61
         %63 = OpIAdd %int %62 %int_1
               OpStore %61 %63
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %64
         %65 = OpFunctionParameter %_ptr_Function_v2float
         %66 = OpLabel
        %s_0 = OpVariable %_ptr_Function_S Function
          %x = OpVariable %_ptr_Function_float Function
         %70 = OpVariable %_ptr_Function_S Function
         %72 = OpVariable %_ptr_Function_S Function
   %expected = OpVariable %_ptr_Function_S Function
         %n1 = OpVariable %_ptr_Function_Nested Function
         %n2 = OpVariable %_ptr_Function_Nested Function
         %n3 = OpVariable %_ptr_Function_Nested Function
         %90 = OpVariable %_ptr_Function_S Function
         %c1 = OpVariable %_ptr_Function_Compound Function
         %c2 = OpVariable %_ptr_Function_Compound Function
         %c3 = OpVariable %_ptr_Function_Compound Function
      %valid = OpVariable %_ptr_Function_bool Function
        %251 = OpVariable %_ptr_Function_v4float Function
         %68 = OpFunctionCall %S %returns_a_struct_S
               OpStore %s_0 %68
               OpStore %70 %68
         %71 = OpFunctionCall %float %accepts_a_struct_fS %70
               OpStore %x %71
               OpStore %72 %68
         %73 = OpFunctionCall %void %modifies_a_struct_vS %72
         %74 = OpLoad %S %72
               OpStore %s_0 %74
         %76 = OpFunctionCall %S %constructs_a_struct_S
               OpStore %expected %76
         %82 = OpFunctionCall %S %returns_a_struct_S
         %83 = OpAccessChain %_ptr_Function_S %n1 %int_0
               OpStore %83 %82
         %84 = OpAccessChain %_ptr_Function_S %n1 %int_0
         %85 = OpLoad %S %84
         %86 = OpAccessChain %_ptr_Function_S %n1 %int_1
               OpStore %86 %85
         %87 = OpLoad %Nested %n1
               OpStore %n2 %87
               OpStore %n3 %87
         %88 = OpAccessChain %_ptr_Function_S %n3 %int_1
         %89 = OpLoad %S %88
               OpStore %90 %89
         %91 = OpFunctionCall %void %modifies_a_struct_vS %90
         %92 = OpLoad %S %90
               OpStore %88 %92
        %104 = OpCompositeConstruct %Compound %99 %103
               OpStore %c1 %104
        %106 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %108 = OpLoad %v4float %106
        %109 = OpCompositeExtract %float %108 1
        %110 = OpCompositeConstruct %v4float %109 %float_2 %float_3 %float_4
        %111 = OpCompositeConstruct %Compound %110 %103
               OpStore %c2 %111
        %113 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %114 = OpLoad %v4float %113
        %115 = OpCompositeExtract %float %114 0
        %116 = OpCompositeConstruct %v4float %115 %float_2 %float_3 %float_4
        %117 = OpCompositeConstruct %Compound %116 %103
               OpStore %c3 %117
        %121 = OpLoad %float %x
        %122 = OpFOrdEqual %bool %121 %float_3
               OpSelectionMerge %124 None
               OpBranchConditional %122 %123 %124
        %123 = OpLabel
        %125 = OpAccessChain %_ptr_Function_float %s_0 %int_0
        %126 = OpLoad %float %125
        %127 = OpFOrdEqual %bool %126 %float_2
               OpBranch %124
        %124 = OpLabel
        %128 = OpPhi %bool %false %66 %127 %123
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %131 = OpAccessChain %_ptr_Function_int %s_0 %int_1
        %132 = OpLoad %int %131
        %133 = OpIEqual %bool %132 %int_3
               OpBranch %130
        %130 = OpLabel
        %134 = OpPhi %bool %false %124 %133 %129
               OpSelectionMerge %136 None
               OpBranchConditional %134 %135 %136
        %135 = OpLabel
        %137 = OpLoad %S %s_0
        %138 = OpLoad %S %expected
        %139 = OpCompositeExtract %float %137 0
        %140 = OpCompositeExtract %float %138 0
        %141 = OpFOrdEqual %bool %139 %140
        %142 = OpCompositeExtract %int %137 1
        %143 = OpCompositeExtract %int %138 1
        %144 = OpIEqual %bool %142 %143
        %145 = OpLogicalAnd %bool %144 %141
               OpBranch %136
        %136 = OpLabel
        %146 = OpPhi %bool %false %130 %145 %135
               OpSelectionMerge %148 None
               OpBranchConditional %146 %147 %148
        %147 = OpLabel
        %149 = OpLoad %S %s_0
        %150 = OpCompositeConstruct %S %float_2 %int_3
        %151 = OpCompositeExtract %float %149 0
        %152 = OpFOrdEqual %bool %151 %float_2
        %153 = OpCompositeExtract %int %149 1
        %154 = OpIEqual %bool %153 %int_3
        %155 = OpLogicalAnd %bool %154 %152
               OpBranch %148
        %148 = OpLabel
        %156 = OpPhi %bool %false %136 %155 %147
               OpSelectionMerge %158 None
               OpBranchConditional %156 %157 %158
        %157 = OpLabel
        %159 = OpLoad %S %s_0
        %160 = OpFunctionCall %S %returns_a_struct_S
        %161 = OpCompositeExtract %float %159 0
        %162 = OpCompositeExtract %float %160 0
        %163 = OpFUnordNotEqual %bool %161 %162
        %164 = OpCompositeExtract %int %159 1
        %165 = OpCompositeExtract %int %160 1
        %166 = OpINotEqual %bool %164 %165
        %167 = OpLogicalOr %bool %166 %163
               OpBranch %158
        %158 = OpLabel
        %168 = OpPhi %bool %false %148 %167 %157
               OpSelectionMerge %170 None
               OpBranchConditional %168 %169 %170
        %169 = OpLabel
        %171 = OpLoad %Nested %n1
        %172 = OpLoad %Nested %n2
        %173 = OpCompositeExtract %S %171 0
        %174 = OpCompositeExtract %S %172 0
        %175 = OpCompositeExtract %float %173 0
        %176 = OpCompositeExtract %float %174 0
        %177 = OpFOrdEqual %bool %175 %176
        %178 = OpCompositeExtract %int %173 1
        %179 = OpCompositeExtract %int %174 1
        %180 = OpIEqual %bool %178 %179
        %181 = OpLogicalAnd %bool %180 %177
        %182 = OpCompositeExtract %S %171 1
        %183 = OpCompositeExtract %S %172 1
        %184 = OpCompositeExtract %float %182 0
        %185 = OpCompositeExtract %float %183 0
        %186 = OpFOrdEqual %bool %184 %185
        %187 = OpCompositeExtract %int %182 1
        %188 = OpCompositeExtract %int %183 1
        %189 = OpIEqual %bool %187 %188
        %190 = OpLogicalAnd %bool %189 %186
        %191 = OpLogicalAnd %bool %190 %181
               OpBranch %170
        %170 = OpLabel
        %192 = OpPhi %bool %false %158 %191 %169
               OpSelectionMerge %194 None
               OpBranchConditional %192 %193 %194
        %193 = OpLabel
        %195 = OpLoad %Nested %n1
        %196 = OpLoad %Nested %n3
        %197 = OpCompositeExtract %S %195 0
        %198 = OpCompositeExtract %S %196 0
        %199 = OpCompositeExtract %float %197 0
        %200 = OpCompositeExtract %float %198 0
        %201 = OpFUnordNotEqual %bool %199 %200
        %202 = OpCompositeExtract %int %197 1
        %203 = OpCompositeExtract %int %198 1
        %204 = OpINotEqual %bool %202 %203
        %205 = OpLogicalOr %bool %204 %201
        %206 = OpCompositeExtract %S %195 1
        %207 = OpCompositeExtract %S %196 1
        %208 = OpCompositeExtract %float %206 0
        %209 = OpCompositeExtract %float %207 0
        %210 = OpFUnordNotEqual %bool %208 %209
        %211 = OpCompositeExtract %int %206 1
        %212 = OpCompositeExtract %int %207 1
        %213 = OpINotEqual %bool %211 %212
        %214 = OpLogicalOr %bool %213 %210
        %215 = OpLogicalOr %bool %214 %205
               OpBranch %194
        %194 = OpLabel
        %216 = OpPhi %bool %false %170 %215 %193
               OpSelectionMerge %218 None
               OpBranchConditional %216 %217 %218
        %217 = OpLabel
        %219 = OpLoad %Nested %n3
        %220 = OpCompositeConstruct %S %float_1 %int_2
        %221 = OpCompositeConstruct %S %float_2 %int_3
        %222 = OpCompositeConstruct %Nested %220 %221
        %223 = OpCompositeExtract %S %219 0
        %224 = OpCompositeExtract %float %223 0
        %225 = OpFOrdEqual %bool %224 %float_1
        %226 = OpCompositeExtract %int %223 1
        %227 = OpIEqual %bool %226 %int_2
        %228 = OpLogicalAnd %bool %227 %225
        %229 = OpCompositeExtract %S %219 1
        %230 = OpCompositeExtract %float %229 0
        %231 = OpFOrdEqual %bool %230 %float_2
        %232 = OpCompositeExtract %int %229 1
        %233 = OpIEqual %bool %232 %int_3
        %234 = OpLogicalAnd %bool %233 %231
        %235 = OpLogicalAnd %bool %234 %228
               OpBranch %218
        %218 = OpLabel
        %236 = OpPhi %bool %false %194 %235 %217
               OpSelectionMerge %238 None
               OpBranchConditional %236 %237 %238
        %237 = OpLabel
        %239 = OpFOrdEqual %v4bool %99 %110
        %241 = OpAll %bool %239
        %243 = OpLogicalAnd %bool %true %241
               OpBranch %238
        %238 = OpLabel
        %244 = OpPhi %bool %false %218 %243 %237
               OpSelectionMerge %246 None
               OpBranchConditional %244 %245 %246
        %245 = OpLabel
        %247 = OpFUnordNotEqual %v4bool %110 %116
        %248 = OpAny %bool %247
        %249 = OpLogicalOr %bool %false %248
               OpBranch %246
        %246 = OpLabel
        %250 = OpPhi %bool %false %238 %249 %245
               OpStore %valid %250
               OpSelectionMerge %255 None
               OpBranchConditional %250 %253 %254
        %253 = OpLabel
        %256 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
        %257 = OpLoad %v4float %256
               OpStore %251 %257
               OpBranch %255
        %254 = OpLabel
        %258 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
        %259 = OpLoad %v4float %258
               OpStore %251 %259
               OpBranch %255
        %255 = OpLabel
        %260 = OpLoad %v4float %251
               OpReturnValue %260
               OpFunctionEnd
