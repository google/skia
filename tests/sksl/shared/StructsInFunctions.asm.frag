               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %38 RelaxedPrecision
               OpMemberDecorate %Nested 0 Offset 0
               OpMemberDecorate %Nested 0 RelaxedPrecision
               OpMemberDecorate %Nested 1 Offset 16
               OpMemberDecorate %Nested 1 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpMemberDecorate %Compound 0 Offset 0
               OpMemberDecorate %Compound 1 Offset 16
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %float %int
         %26 = OpTypeFunction %S
%_ptr_Function_S = OpTypePointer Function %S
    %float_1 = OpConstant %float 1
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%_ptr_Function_int = OpTypePointer Function %int
    %float_2 = OpConstant %float 2
      %int_3 = OpConstant %int 3
         %43 = OpTypeFunction %float %_ptr_Function_S
         %52 = OpTypeFunction %void %_ptr_Function_S
         %61 = OpTypeFunction %v4float %_ptr_Function_v2float
     %Nested = OpTypeStruct %S %S
%_ptr_Function_Nested = OpTypePointer Function %Nested
      %v3int = OpTypeVector %int 3
   %Compound = OpTypeStruct %v4float %v3int
%_ptr_Function_Compound = OpTypePointer Function %Compound
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %96 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
      %int_7 = OpConstant %int 7
        %100 = OpConstantComposite %v3int %int_5 %int_6 %int_7
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
%returns_a_struct_S = OpFunction %S None %26
         %27 = OpLabel
          %s = OpVariable %_ptr_Function_S Function
         %32 = OpAccessChain %_ptr_Function_float %s %int_0
               OpStore %32 %float_1
         %36 = OpAccessChain %_ptr_Function_int %s %int_1
               OpStore %36 %int_2
         %38 = OpLoad %S %s
               OpReturnValue %38
               OpFunctionEnd
%constructs_a_struct_S = OpFunction %S None %26
         %39 = OpLabel
         %42 = OpCompositeConstruct %S %float_2 %int_3
               OpReturnValue %42
               OpFunctionEnd
%accepts_a_struct_fS = OpFunction %float None %43
         %44 = OpFunctionParameter %_ptr_Function_S
         %45 = OpLabel
         %46 = OpAccessChain %_ptr_Function_float %44 %int_0
         %47 = OpLoad %float %46
         %48 = OpAccessChain %_ptr_Function_int %44 %int_1
         %49 = OpLoad %int %48
         %50 = OpConvertSToF %float %49
         %51 = OpFAdd %float %47 %50
               OpReturnValue %51
               OpFunctionEnd
%modifies_a_struct_vS = OpFunction %void None %52
         %53 = OpFunctionParameter %_ptr_Function_S
         %54 = OpLabel
         %55 = OpAccessChain %_ptr_Function_float %53 %int_0
         %56 = OpLoad %float %55
         %57 = OpFAdd %float %56 %float_1
               OpStore %55 %57
         %58 = OpAccessChain %_ptr_Function_int %53 %int_1
         %59 = OpLoad %int %58
         %60 = OpIAdd %int %59 %int_1
               OpStore %58 %60
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %61
         %62 = OpFunctionParameter %_ptr_Function_v2float
         %63 = OpLabel
        %s_0 = OpVariable %_ptr_Function_S Function
          %x = OpVariable %_ptr_Function_float Function
         %67 = OpVariable %_ptr_Function_S Function
         %69 = OpVariable %_ptr_Function_S Function
   %expected = OpVariable %_ptr_Function_S Function
         %n1 = OpVariable %_ptr_Function_Nested Function
         %n2 = OpVariable %_ptr_Function_Nested Function
         %n3 = OpVariable %_ptr_Function_Nested Function
         %87 = OpVariable %_ptr_Function_S Function
         %c1 = OpVariable %_ptr_Function_Compound Function
         %c2 = OpVariable %_ptr_Function_Compound Function
         %c3 = OpVariable %_ptr_Function_Compound Function
      %valid = OpVariable %_ptr_Function_bool Function
        %249 = OpVariable %_ptr_Function_v4float Function
         %65 = OpFunctionCall %S %returns_a_struct_S
               OpStore %s_0 %65
               OpStore %67 %65
         %68 = OpFunctionCall %float %accepts_a_struct_fS %67
               OpStore %x %68
               OpStore %69 %65
         %70 = OpFunctionCall %void %modifies_a_struct_vS %69
         %71 = OpLoad %S %69
               OpStore %s_0 %71
         %73 = OpFunctionCall %S %constructs_a_struct_S
               OpStore %expected %73
         %79 = OpFunctionCall %S %returns_a_struct_S
         %80 = OpAccessChain %_ptr_Function_S %n1 %int_0
               OpStore %80 %79
         %81 = OpAccessChain %_ptr_Function_S %n1 %int_0
         %82 = OpLoad %S %81
         %83 = OpAccessChain %_ptr_Function_S %n1 %int_1
               OpStore %83 %82
         %84 = OpLoad %Nested %n1
               OpStore %n2 %84
               OpStore %n3 %84
         %85 = OpAccessChain %_ptr_Function_S %n3 %int_1
         %86 = OpLoad %S %85
               OpStore %87 %86
         %88 = OpFunctionCall %void %modifies_a_struct_vS %87
         %89 = OpLoad %S %87
               OpStore %85 %89
        %101 = OpCompositeConstruct %Compound %96 %100
               OpStore %c1 %101
        %103 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %105 = OpLoad %v4float %103
        %106 = OpCompositeExtract %float %105 1
        %107 = OpCompositeConstruct %v4float %106 %float_2 %float_3 %float_4
        %108 = OpCompositeConstruct %Compound %107 %100
               OpStore %c2 %108
        %110 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %111 = OpLoad %v4float %110
        %112 = OpCompositeExtract %float %111 0
        %113 = OpCompositeConstruct %v4float %112 %float_2 %float_3 %float_4
        %114 = OpCompositeConstruct %Compound %113 %100
               OpStore %c3 %114
        %119 = OpLoad %float %x
        %120 = OpFOrdEqual %bool %119 %float_3
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %123 = OpAccessChain %_ptr_Function_float %s_0 %int_0
        %124 = OpLoad %float %123
        %125 = OpFOrdEqual %bool %124 %float_2
               OpBranch %122
        %122 = OpLabel
        %126 = OpPhi %bool %false %63 %125 %121
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
        %129 = OpAccessChain %_ptr_Function_int %s_0 %int_1
        %130 = OpLoad %int %129
        %131 = OpIEqual %bool %130 %int_3
               OpBranch %128
        %128 = OpLabel
        %132 = OpPhi %bool %false %122 %131 %127
               OpSelectionMerge %134 None
               OpBranchConditional %132 %133 %134
        %133 = OpLabel
        %135 = OpLoad %S %s_0
        %136 = OpLoad %S %expected
        %137 = OpCompositeExtract %float %135 0
        %138 = OpCompositeExtract %float %136 0
        %139 = OpFOrdEqual %bool %137 %138
        %140 = OpCompositeExtract %int %135 1
        %141 = OpCompositeExtract %int %136 1
        %142 = OpIEqual %bool %140 %141
        %143 = OpLogicalAnd %bool %142 %139
               OpBranch %134
        %134 = OpLabel
        %144 = OpPhi %bool %false %128 %143 %133
               OpSelectionMerge %146 None
               OpBranchConditional %144 %145 %146
        %145 = OpLabel
        %147 = OpLoad %S %s_0
        %148 = OpCompositeConstruct %S %float_2 %int_3
        %149 = OpCompositeExtract %float %147 0
        %150 = OpFOrdEqual %bool %149 %float_2
        %151 = OpCompositeExtract %int %147 1
        %152 = OpIEqual %bool %151 %int_3
        %153 = OpLogicalAnd %bool %152 %150
               OpBranch %146
        %146 = OpLabel
        %154 = OpPhi %bool %false %134 %153 %145
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %157 = OpLoad %S %s_0
        %158 = OpFunctionCall %S %returns_a_struct_S
        %159 = OpCompositeExtract %float %157 0
        %160 = OpCompositeExtract %float %158 0
        %161 = OpFUnordNotEqual %bool %159 %160
        %162 = OpCompositeExtract %int %157 1
        %163 = OpCompositeExtract %int %158 1
        %164 = OpINotEqual %bool %162 %163
        %165 = OpLogicalOr %bool %164 %161
               OpBranch %156
        %156 = OpLabel
        %166 = OpPhi %bool %false %146 %165 %155
               OpSelectionMerge %168 None
               OpBranchConditional %166 %167 %168
        %167 = OpLabel
        %169 = OpLoad %Nested %n1
        %170 = OpLoad %Nested %n2
        %171 = OpCompositeExtract %S %169 0
        %172 = OpCompositeExtract %S %170 0
        %173 = OpCompositeExtract %float %171 0
        %174 = OpCompositeExtract %float %172 0
        %175 = OpFOrdEqual %bool %173 %174
        %176 = OpCompositeExtract %int %171 1
        %177 = OpCompositeExtract %int %172 1
        %178 = OpIEqual %bool %176 %177
        %179 = OpLogicalAnd %bool %178 %175
        %180 = OpCompositeExtract %S %169 1
        %181 = OpCompositeExtract %S %170 1
        %182 = OpCompositeExtract %float %180 0
        %183 = OpCompositeExtract %float %181 0
        %184 = OpFOrdEqual %bool %182 %183
        %185 = OpCompositeExtract %int %180 1
        %186 = OpCompositeExtract %int %181 1
        %187 = OpIEqual %bool %185 %186
        %188 = OpLogicalAnd %bool %187 %184
        %189 = OpLogicalAnd %bool %188 %179
               OpBranch %168
        %168 = OpLabel
        %190 = OpPhi %bool %false %156 %189 %167
               OpSelectionMerge %192 None
               OpBranchConditional %190 %191 %192
        %191 = OpLabel
        %193 = OpLoad %Nested %n1
        %194 = OpLoad %Nested %n3
        %195 = OpCompositeExtract %S %193 0
        %196 = OpCompositeExtract %S %194 0
        %197 = OpCompositeExtract %float %195 0
        %198 = OpCompositeExtract %float %196 0
        %199 = OpFUnordNotEqual %bool %197 %198
        %200 = OpCompositeExtract %int %195 1
        %201 = OpCompositeExtract %int %196 1
        %202 = OpINotEqual %bool %200 %201
        %203 = OpLogicalOr %bool %202 %199
        %204 = OpCompositeExtract %S %193 1
        %205 = OpCompositeExtract %S %194 1
        %206 = OpCompositeExtract %float %204 0
        %207 = OpCompositeExtract %float %205 0
        %208 = OpFUnordNotEqual %bool %206 %207
        %209 = OpCompositeExtract %int %204 1
        %210 = OpCompositeExtract %int %205 1
        %211 = OpINotEqual %bool %209 %210
        %212 = OpLogicalOr %bool %211 %208
        %213 = OpLogicalOr %bool %212 %203
               OpBranch %192
        %192 = OpLabel
        %214 = OpPhi %bool %false %168 %213 %191
               OpSelectionMerge %216 None
               OpBranchConditional %214 %215 %216
        %215 = OpLabel
        %217 = OpLoad %Nested %n3
        %218 = OpCompositeConstruct %S %float_1 %int_2
        %219 = OpCompositeConstruct %S %float_2 %int_3
        %220 = OpCompositeConstruct %Nested %218 %219
        %221 = OpCompositeExtract %S %217 0
        %222 = OpCompositeExtract %float %221 0
        %223 = OpFOrdEqual %bool %222 %float_1
        %224 = OpCompositeExtract %int %221 1
        %225 = OpIEqual %bool %224 %int_2
        %226 = OpLogicalAnd %bool %225 %223
        %227 = OpCompositeExtract %S %217 1
        %228 = OpCompositeExtract %float %227 0
        %229 = OpFOrdEqual %bool %228 %float_2
        %230 = OpCompositeExtract %int %227 1
        %231 = OpIEqual %bool %230 %int_3
        %232 = OpLogicalAnd %bool %231 %229
        %233 = OpLogicalAnd %bool %232 %226
               OpBranch %216
        %216 = OpLabel
        %234 = OpPhi %bool %false %192 %233 %215
               OpSelectionMerge %236 None
               OpBranchConditional %234 %235 %236
        %235 = OpLabel
        %237 = OpFOrdEqual %v4bool %96 %107
        %239 = OpAll %bool %237
        %241 = OpLogicalAnd %bool %true %239
               OpBranch %236
        %236 = OpLabel
        %242 = OpPhi %bool %false %216 %241 %235
               OpSelectionMerge %244 None
               OpBranchConditional %242 %243 %244
        %243 = OpLabel
        %245 = OpFUnordNotEqual %v4bool %107 %113
        %246 = OpAny %bool %245
        %247 = OpLogicalOr %bool %false %246
               OpBranch %244
        %244 = OpLabel
        %248 = OpPhi %bool %false %236 %247 %243
               OpStore %valid %248
               OpSelectionMerge %253 None
               OpBranchConditional %248 %251 %252
        %251 = OpLabel
        %254 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %255 = OpLoad %v4float %254
               OpStore %249 %255
               OpBranch %253
        %252 = OpLabel
        %256 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %257 = OpLoad %v4float %256
               OpStore %249 %257
               OpBranch %253
        %253 = OpLabel
        %258 = OpLoad %v4float %249
               OpReturnValue %258
               OpFunctionEnd
