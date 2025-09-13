               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %_UniformBuffer "_UniformBuffer"  ; id %16
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %18
               OpName %S "S"                            ; id %28
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "y"
               OpName %returns_a_struct_S "returns_a_struct_S"  ; id %6
               OpName %s "s"                                    ; id %31
               OpName %constructs_a_struct_S "constructs_a_struct_S"    ; id %7
               OpName %accepts_a_struct_fS "accepts_a_struct_fS"        ; id %8
               OpName %modifies_a_struct_vS "modifies_a_struct_vS"      ; id %9
               OpName %main "main"                                      ; id %10
               OpName %s_0 "s"                                          ; id %67
               OpName %x "x"                                            ; id %69
               OpName %expected "expected"                              ; id %75
               OpName %Nested "Nested"                                  ; id %78
               OpMemberName %Nested 0 "a"
               OpMemberName %Nested 1 "b"
               OpName %n1 "n1"                      ; id %77
               OpName %n2 "n2"                      ; id %80
               OpName %n3 "n3"                      ; id %81
               OpName %Compound "Compound"          ; id %95
               OpMemberName %Compound 0 "f4"
               OpMemberName %Compound 1 "i3"
               OpName %c1 "c1"                      ; id %93
               OpName %c2 "c2"                      ; id %105
               OpName %c3 "c3"                      ; id %112
               OpName %valid "valid"                ; id %118

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
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
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
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
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
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %20

         %21 = OpLabel
         %25 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %25 %24
         %27 =   OpFunctionCall %v4float %main %25
                 OpStore %sk_FragColor %27
                 OpReturn
               OpFunctionEnd


               ; Function returns_a_struct_S
%returns_a_struct_S = OpFunction %S None %29

         %30 = OpLabel
          %s =   OpVariable %_ptr_Function_S Function
         %35 =   OpAccessChain %_ptr_Function_float %s %int_0
                 OpStore %35 %float_1
         %39 =   OpAccessChain %_ptr_Function_int %s %int_1
                 OpStore %39 %int_2
         %41 =   OpLoad %S %s                       ; RelaxedPrecision
                 OpReturnValue %41
               OpFunctionEnd


               ; Function constructs_a_struct_S
%constructs_a_struct_S = OpFunction %S None %29

         %42 = OpLabel
         %45 =   OpCompositeConstruct %S %float_2 %int_3
                 OpReturnValue %45
               OpFunctionEnd


               ; Function accepts_a_struct_fS
%accepts_a_struct_fS = OpFunction %float None %46
         %47 = OpFunctionParameter %_ptr_Function_S

         %48 = OpLabel
         %49 =   OpAccessChain %_ptr_Function_float %47 %int_0
         %50 =   OpLoad %float %49
         %51 =   OpAccessChain %_ptr_Function_int %47 %int_1
         %52 =   OpLoad %int %51
         %53 =   OpConvertSToF %float %52
         %54 =   OpFAdd %float %50 %53
                 OpReturnValue %54
               OpFunctionEnd


               ; Function modifies_a_struct_vS
%modifies_a_struct_vS = OpFunction %void None %55
         %56 = OpFunctionParameter %_ptr_Function_S

         %57 = OpLabel
         %58 =   OpAccessChain %_ptr_Function_float %56 %int_0
         %59 =   OpLoad %float %58
         %60 =   OpFAdd %float %59 %float_1
                 OpStore %58 %60
         %61 =   OpAccessChain %_ptr_Function_int %56 %int_1
         %62 =   OpLoad %int %61
         %63 =   OpIAdd %int %62 %int_1
                 OpStore %61 %63
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %64         ; RelaxedPrecision
         %65 = OpFunctionParameter %_ptr_Function_v2float

         %66 = OpLabel
        %s_0 =   OpVariable %_ptr_Function_S Function
          %x =   OpVariable %_ptr_Function_float Function
         %70 =   OpVariable %_ptr_Function_S Function
         %72 =   OpVariable %_ptr_Function_S Function
   %expected =   OpVariable %_ptr_Function_S Function
         %n1 =   OpVariable %_ptr_Function_Nested Function
         %n2 =   OpVariable %_ptr_Function_Nested Function
         %n3 =   OpVariable %_ptr_Function_Nested Function
         %90 =   OpVariable %_ptr_Function_S Function
         %c1 =   OpVariable %_ptr_Function_Compound Function
         %c2 =   OpVariable %_ptr_Function_Compound Function
         %c3 =   OpVariable %_ptr_Function_Compound Function
      %valid =   OpVariable %_ptr_Function_bool Function
        %252 =   OpVariable %_ptr_Function_v4float Function
         %68 =   OpFunctionCall %S %returns_a_struct_S
                 OpStore %s_0 %68
                 OpStore %70 %68
         %71 =   OpFunctionCall %float %accepts_a_struct_fS %70
                 OpStore %x %71
                 OpStore %72 %68
         %73 =   OpFunctionCall %void %modifies_a_struct_vS %72
         %74 =   OpLoad %S %72
                 OpStore %s_0 %74
         %76 =   OpFunctionCall %S %constructs_a_struct_S
                 OpStore %expected %76
         %82 =   OpFunctionCall %S %returns_a_struct_S
         %83 =   OpAccessChain %_ptr_Function_S %n1 %int_0
                 OpStore %83 %82
         %84 =   OpAccessChain %_ptr_Function_S %n1 %int_0
         %85 =   OpLoad %S %84                      ; RelaxedPrecision
         %86 =   OpAccessChain %_ptr_Function_S %n1 %int_1
                 OpStore %86 %85
         %87 =   OpLoad %Nested %n1                 ; RelaxedPrecision
                 OpStore %n2 %87
                 OpStore %n3 %87
         %88 =   OpAccessChain %_ptr_Function_S %n3 %int_1
         %89 =   OpLoad %S %88                      ; RelaxedPrecision
                 OpStore %90 %89
         %91 =   OpFunctionCall %void %modifies_a_struct_vS %90
         %92 =   OpLoad %S %90
                 OpStore %88 %92
        %104 =   OpCompositeConstruct %Compound %99 %103
                 OpStore %c1 %104
        %106 =   OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %108 =   OpLoad %v4float %106               ; RelaxedPrecision
        %109 =   OpCompositeExtract %float %108 1   ; RelaxedPrecision
        %110 =   OpCompositeConstruct %v4float %109 %float_2 %float_3 %float_4
        %111 =   OpCompositeConstruct %Compound %110 %103
                 OpStore %c2 %111
        %113 =   OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %114 =   OpLoad %v4float %113               ; RelaxedPrecision
        %115 =   OpCompositeExtract %float %114 0   ; RelaxedPrecision
        %116 =   OpCompositeConstruct %v4float %115 %float_2 %float_3 %float_4
        %117 =   OpCompositeConstruct %Compound %116 %103
                 OpStore %c3 %117
        %122 =   OpLoad %float %x
        %123 =   OpFOrdEqual %bool %122 %float_3
                 OpSelectionMerge %125 None
                 OpBranchConditional %123 %124 %125

        %124 =     OpLabel
        %126 =       OpAccessChain %_ptr_Function_float %s_0 %int_0
        %127 =       OpLoad %float %126
        %128 =       OpFOrdEqual %bool %127 %float_2
                     OpBranch %125

        %125 = OpLabel
        %129 =   OpPhi %bool %false %66 %128 %124
                 OpSelectionMerge %131 None
                 OpBranchConditional %129 %130 %131

        %130 =     OpLabel
        %132 =       OpAccessChain %_ptr_Function_int %s_0 %int_1
        %133 =       OpLoad %int %132
        %134 =       OpIEqual %bool %133 %int_3
                     OpBranch %131

        %131 = OpLabel
        %135 =   OpPhi %bool %false %125 %134 %130
                 OpSelectionMerge %137 None
                 OpBranchConditional %135 %136 %137

        %136 =     OpLabel
        %138 =       OpLoad %S %s_0                 ; RelaxedPrecision
        %139 =       OpLoad %S %expected            ; RelaxedPrecision
        %140 =       OpCompositeExtract %float %138 0
        %141 =       OpCompositeExtract %float %139 0
        %142 =       OpFOrdEqual %bool %140 %141
        %143 =       OpCompositeExtract %int %138 1
        %144 =       OpCompositeExtract %int %139 1
        %145 =       OpIEqual %bool %143 %144
        %146 =       OpLogicalAnd %bool %145 %142
                     OpBranch %137

        %137 = OpLabel
        %147 =   OpPhi %bool %false %131 %146 %136
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
        %150 =       OpLoad %S %s_0                 ; RelaxedPrecision
        %151 =       OpCompositeConstruct %S %float_2 %int_3
        %152 =       OpCompositeExtract %float %150 0
        %153 =       OpFOrdEqual %bool %152 %float_2
        %154 =       OpCompositeExtract %int %150 1
        %155 =       OpIEqual %bool %154 %int_3
        %156 =       OpLogicalAnd %bool %155 %153
                     OpBranch %149

        %149 = OpLabel
        %157 =   OpPhi %bool %false %137 %156 %148
                 OpSelectionMerge %159 None
                 OpBranchConditional %157 %158 %159

        %158 =     OpLabel
        %160 =       OpLoad %S %s_0                 ; RelaxedPrecision
        %161 =       OpFunctionCall %S %returns_a_struct_S
        %162 =       OpCompositeExtract %float %160 0
        %163 =       OpCompositeExtract %float %161 0
        %164 =       OpFUnordNotEqual %bool %162 %163
        %165 =       OpCompositeExtract %int %160 1
        %166 =       OpCompositeExtract %int %161 1
        %167 =       OpINotEqual %bool %165 %166
        %168 =       OpLogicalOr %bool %167 %164
                     OpBranch %159

        %159 = OpLabel
        %169 =   OpPhi %bool %false %149 %168 %158
                 OpSelectionMerge %171 None
                 OpBranchConditional %169 %170 %171

        %170 =     OpLabel
        %172 =       OpLoad %Nested %n1             ; RelaxedPrecision
        %173 =       OpLoad %Nested %n2             ; RelaxedPrecision
        %174 =       OpCompositeExtract %S %172 0
        %175 =       OpCompositeExtract %S %173 0
        %176 =       OpCompositeExtract %float %174 0
        %177 =       OpCompositeExtract %float %175 0
        %178 =       OpFOrdEqual %bool %176 %177
        %179 =       OpCompositeExtract %int %174 1
        %180 =       OpCompositeExtract %int %175 1
        %181 =       OpIEqual %bool %179 %180
        %182 =       OpLogicalAnd %bool %181 %178
        %183 =       OpCompositeExtract %S %172 1
        %184 =       OpCompositeExtract %S %173 1
        %185 =       OpCompositeExtract %float %183 0
        %186 =       OpCompositeExtract %float %184 0
        %187 =       OpFOrdEqual %bool %185 %186
        %188 =       OpCompositeExtract %int %183 1
        %189 =       OpCompositeExtract %int %184 1
        %190 =       OpIEqual %bool %188 %189
        %191 =       OpLogicalAnd %bool %190 %187
        %192 =       OpLogicalAnd %bool %191 %182
                     OpBranch %171

        %171 = OpLabel
        %193 =   OpPhi %bool %false %159 %192 %170
                 OpSelectionMerge %195 None
                 OpBranchConditional %193 %194 %195

        %194 =     OpLabel
        %196 =       OpLoad %Nested %n1             ; RelaxedPrecision
        %197 =       OpLoad %Nested %n3             ; RelaxedPrecision
        %198 =       OpCompositeExtract %S %196 0
        %199 =       OpCompositeExtract %S %197 0
        %200 =       OpCompositeExtract %float %198 0
        %201 =       OpCompositeExtract %float %199 0
        %202 =       OpFUnordNotEqual %bool %200 %201
        %203 =       OpCompositeExtract %int %198 1
        %204 =       OpCompositeExtract %int %199 1
        %205 =       OpINotEqual %bool %203 %204
        %206 =       OpLogicalOr %bool %205 %202
        %207 =       OpCompositeExtract %S %196 1
        %208 =       OpCompositeExtract %S %197 1
        %209 =       OpCompositeExtract %float %207 0
        %210 =       OpCompositeExtract %float %208 0
        %211 =       OpFUnordNotEqual %bool %209 %210
        %212 =       OpCompositeExtract %int %207 1
        %213 =       OpCompositeExtract %int %208 1
        %214 =       OpINotEqual %bool %212 %213
        %215 =       OpLogicalOr %bool %214 %211
        %216 =       OpLogicalOr %bool %215 %206
                     OpBranch %195

        %195 = OpLabel
        %217 =   OpPhi %bool %false %171 %216 %194
                 OpSelectionMerge %219 None
                 OpBranchConditional %217 %218 %219

        %218 =     OpLabel
        %220 =       OpLoad %Nested %n3             ; RelaxedPrecision
        %221 =       OpCompositeConstruct %S %float_1 %int_2
        %222 =       OpCompositeConstruct %S %float_2 %int_3
        %223 =       OpCompositeConstruct %Nested %221 %222
        %224 =       OpCompositeExtract %S %220 0
        %225 =       OpCompositeExtract %float %224 0
        %226 =       OpFOrdEqual %bool %225 %float_1
        %227 =       OpCompositeExtract %int %224 1
        %228 =       OpIEqual %bool %227 %int_2
        %229 =       OpLogicalAnd %bool %228 %226
        %230 =       OpCompositeExtract %S %220 1
        %231 =       OpCompositeExtract %float %230 0
        %232 =       OpFOrdEqual %bool %231 %float_2
        %233 =       OpCompositeExtract %int %230 1
        %234 =       OpIEqual %bool %233 %int_3
        %235 =       OpLogicalAnd %bool %234 %232
        %236 =       OpLogicalAnd %bool %235 %229
                     OpBranch %219

        %219 = OpLabel
        %237 =   OpPhi %bool %false %195 %236 %218
                 OpSelectionMerge %239 None
                 OpBranchConditional %237 %238 %239

        %238 =     OpLabel
        %240 =       OpFOrdEqual %v4bool %99 %110
        %242 =       OpAll %bool %240
        %244 =       OpLogicalAnd %bool %true %242
                     OpBranch %239

        %239 = OpLabel
        %245 =   OpPhi %bool %false %219 %244 %238
                 OpSelectionMerge %247 None
                 OpBranchConditional %245 %246 %247

        %246 =     OpLabel
        %248 =       OpFUnordNotEqual %v4bool %110 %116
        %249 =       OpAny %bool %248
        %250 =       OpLogicalOr %bool %false %249
                     OpBranch %247

        %247 = OpLabel
        %251 =   OpPhi %bool %false %239 %250 %246
                 OpStore %valid %251
                 OpSelectionMerge %256 None
                 OpBranchConditional %251 %254 %255

        %254 =     OpLabel
        %257 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %258 =       OpLoad %v4float %257           ; RelaxedPrecision
                     OpStore %252 %258
                     OpBranch %256

        %255 =     OpLabel
        %259 =       OpAccessChain %_ptr_Uniform_v4float %15 %int_0
        %260 =       OpLoad %v4float %259           ; RelaxedPrecision
                     OpStore %252 %260
                     OpBranch %256

        %256 = OpLabel
        %261 =   OpLoad %v4float %252               ; RelaxedPrecision
                 OpReturnValue %261
               OpFunctionEnd
