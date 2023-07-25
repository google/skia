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
               OpMemberName %_UniformBuffer 2 "colorBlack"
               OpMemberName %_UniformBuffer 3 "colorWhite"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedBW "expectedBW"
               OpName %expectedWT "expectedWT"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 64
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expectedBW RelaxedPrecision
               OpDecorate %expectedWT RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
 %float_2_25 = OpConstant %float 2.25
         %33 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %44 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %45 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
     %v4bool = OpTypeVector %bool 4
 %float_0_25 = OpConstant %float 0.25
         %57 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
 %float_0_75 = OpConstant %float 0.75
         %59 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
         %70 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
         %71 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
         %82 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %83 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
        %109 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
        %125 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
        %138 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
       %true = OpConstantTrue %bool
      %int_4 = OpConstant %int 4
        %182 = OpConstantComposite %v2float %float_0 %float_0_5
        %196 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
        %208 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
        %217 = OpConstantComposite %v2float %float_1 %float_0_5
        %224 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
 %expectedBW = OpVariable %_ptr_Function_v4float Function
 %expectedWT = OpVariable %_ptr_Function_v4float Function
        %232 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedBW %30
               OpStore %expectedWT %33
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %40 = OpLoad %v4float %36
         %41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %43 = OpLoad %v4float %41
         %35 = OpExtInst %v4float %1 FMix %40 %43 %44
         %46 = OpFOrdEqual %v4bool %35 %45
         %48 = OpAll %bool %46
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %53 = OpLoad %v4float %52
         %54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %55 = OpLoad %v4float %54
         %51 = OpExtInst %v4float %1 FMix %53 %55 %57
         %60 = OpFOrdEqual %v4bool %51 %59
         %61 = OpAll %bool %60
               OpBranch %50
         %50 = OpLabel
         %62 = OpPhi %bool %false %25 %61 %49
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %67 = OpLoad %v4float %66
         %68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %69 = OpLoad %v4float %68
         %65 = OpExtInst %v4float %1 FMix %67 %69 %70
         %72 = OpFOrdEqual %v4bool %65 %71
         %73 = OpAll %bool %72
               OpBranch %64
         %64 = OpLabel
         %74 = OpPhi %bool %false %50 %73 %63
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %79 = OpLoad %v4float %78
         %80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %81 = OpLoad %v4float %80
         %77 = OpExtInst %v4float %1 FMix %79 %81 %82
         %84 = OpFOrdEqual %v4bool %77 %83
         %85 = OpAll %bool %84
               OpBranch %76
         %76 = OpLabel
         %86 = OpPhi %bool %false %64 %85 %75
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %92 = OpLoad %v4float %90
         %93 = OpCompositeExtract %float %92 0
         %94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %96 = OpLoad %v4float %94
         %97 = OpCompositeExtract %float %96 0
         %89 = OpExtInst %float %1 FMix %93 %97 %float_0_5
         %98 = OpFOrdEqual %bool %89 %float_0_5
               OpBranch %88
         %88 = OpLabel
         %99 = OpPhi %bool %false %76 %98 %87
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
        %103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %104 = OpLoad %v4float %103
        %105 = OpVectorShuffle %v2float %104 %104 0 1
        %106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %107 = OpLoad %v4float %106
        %108 = OpVectorShuffle %v2float %107 %107 0 1
        %102 = OpExtInst %v2float %1 FMix %105 %108 %109
        %110 = OpVectorShuffle %v2float %30 %30 0 1
        %111 = OpFOrdEqual %v2bool %102 %110
        %113 = OpAll %bool %111
               OpBranch %101
        %101 = OpLabel
        %114 = OpPhi %bool %false %88 %113 %100
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %119 = OpLoad %v4float %118
        %120 = OpVectorShuffle %v3float %119 %119 0 1 2
        %122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %123 = OpLoad %v4float %122
        %124 = OpVectorShuffle %v3float %123 %123 0 1 2
        %117 = OpExtInst %v3float %1 FMix %120 %124 %125
        %126 = OpVectorShuffle %v3float %30 %30 0 1 2
        %127 = OpFOrdEqual %v3bool %117 %126
        %129 = OpAll %bool %127
               OpBranch %116
        %116 = OpLabel
        %130 = OpPhi %bool %false %101 %129 %115
               OpSelectionMerge %132 None
               OpBranchConditional %130 %131 %132
        %131 = OpLabel
        %134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %135 = OpLoad %v4float %134
        %136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %137 = OpLoad %v4float %136
        %133 = OpExtInst %v4float %1 FMix %135 %137 %138
        %139 = OpFOrdEqual %v4bool %133 %30
        %140 = OpAll %bool %139
               OpBranch %132
        %132 = OpLabel
        %141 = OpPhi %bool %false %116 %140 %131
               OpSelectionMerge %143 None
               OpBranchConditional %141 %142 %143
        %142 = OpLabel
               OpBranch %143
        %143 = OpLabel
        %145 = OpPhi %bool %false %132 %true %142
               OpSelectionMerge %147 None
               OpBranchConditional %145 %146 %147
        %146 = OpLabel
        %148 = OpVectorShuffle %v2float %30 %30 0 1
        %149 = OpFOrdEqual %v2bool %109 %148
        %150 = OpAll %bool %149
               OpBranch %147
        %147 = OpLabel
        %151 = OpPhi %bool %false %143 %150 %146
               OpSelectionMerge %153 None
               OpBranchConditional %151 %152 %153
        %152 = OpLabel
        %154 = OpVectorShuffle %v3float %30 %30 0 1 2
        %155 = OpFOrdEqual %v3bool %125 %154
        %156 = OpAll %bool %155
               OpBranch %153
        %153 = OpLabel
        %157 = OpPhi %bool %false %147 %156 %152
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
               OpBranch %159
        %159 = OpLabel
        %160 = OpPhi %bool %false %153 %true %158
               OpSelectionMerge %162 None
               OpBranchConditional %160 %161 %162
        %161 = OpLabel
        %164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %165 = OpLoad %v4float %164
        %166 = OpCompositeExtract %float %165 0
        %167 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %169 = OpLoad %v4float %167
        %170 = OpCompositeExtract %float %169 0
        %163 = OpExtInst %float %1 FMix %166 %170 %float_0
        %171 = OpFOrdEqual %bool %163 %float_1
               OpBranch %162
        %162 = OpLabel
        %172 = OpPhi %bool %false %159 %171 %161
               OpSelectionMerge %174 None
               OpBranchConditional %172 %173 %174
        %173 = OpLabel
        %176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %177 = OpLoad %v4float %176
        %178 = OpVectorShuffle %v2float %177 %177 0 1
        %179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %180 = OpLoad %v4float %179
        %181 = OpVectorShuffle %v2float %180 %180 0 1
        %175 = OpExtInst %v2float %1 FMix %178 %181 %182
        %183 = OpVectorShuffle %v2float %33 %33 0 1
        %184 = OpFOrdEqual %v2bool %175 %183
        %185 = OpAll %bool %184
               OpBranch %174
        %174 = OpLabel
        %186 = OpPhi %bool %false %162 %185 %173
               OpSelectionMerge %188 None
               OpBranchConditional %186 %187 %188
        %187 = OpLabel
        %190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %191 = OpLoad %v4float %190
        %192 = OpVectorShuffle %v3float %191 %191 0 1 2
        %193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %194 = OpLoad %v4float %193
        %195 = OpVectorShuffle %v3float %194 %194 0 1 2
        %189 = OpExtInst %v3float %1 FMix %192 %195 %196
        %197 = OpVectorShuffle %v3float %33 %33 0 1 2
        %198 = OpFOrdEqual %v3bool %189 %197
        %199 = OpAll %bool %198
               OpBranch %188
        %188 = OpLabel
        %200 = OpPhi %bool %false %174 %199 %187
               OpSelectionMerge %202 None
               OpBranchConditional %200 %201 %202
        %201 = OpLabel
        %204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %205 = OpLoad %v4float %204
        %206 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %207 = OpLoad %v4float %206
        %203 = OpExtInst %v4float %1 FMix %205 %207 %208
        %209 = OpFOrdEqual %v4bool %203 %33
        %210 = OpAll %bool %209
               OpBranch %202
        %202 = OpLabel
        %211 = OpPhi %bool %false %188 %210 %201
               OpSelectionMerge %213 None
               OpBranchConditional %211 %212 %213
        %212 = OpLabel
               OpBranch %213
        %213 = OpLabel
        %214 = OpPhi %bool %false %202 %true %212
               OpSelectionMerge %216 None
               OpBranchConditional %214 %215 %216
        %215 = OpLabel
        %218 = OpVectorShuffle %v2float %33 %33 0 1
        %219 = OpFOrdEqual %v2bool %217 %218
        %220 = OpAll %bool %219
               OpBranch %216
        %216 = OpLabel
        %221 = OpPhi %bool %false %213 %220 %215
               OpSelectionMerge %223 None
               OpBranchConditional %221 %222 %223
        %222 = OpLabel
        %225 = OpVectorShuffle %v3float %33 %33 0 1 2
        %226 = OpFOrdEqual %v3bool %224 %225
        %227 = OpAll %bool %226
               OpBranch %223
        %223 = OpLabel
        %228 = OpPhi %bool %false %216 %227 %222
               OpSelectionMerge %230 None
               OpBranchConditional %228 %229 %230
        %229 = OpLabel
               OpBranch %230
        %230 = OpLabel
        %231 = OpPhi %bool %false %223 %true %229
               OpSelectionMerge %235 None
               OpBranchConditional %231 %233 %234
        %233 = OpLabel
        %236 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %237 = OpLoad %v4float %236
               OpStore %232 %237
               OpBranch %235
        %234 = OpLabel
        %238 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %239 = OpLoad %v4float %238
               OpStore %232 %239
               OpBranch %235
        %235 = OpLabel
        %240 = OpLoad %v4float %232
               OpReturnValue %240
               OpFunctionEnd
