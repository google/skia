               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expectedBW RelaxedPrecision
               OpDecorate %expectedWT RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
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
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
 %float_2_25 = OpConstant %float 2.25
         %30 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %42 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %43 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
     %v4bool = OpTypeVector %bool 4
 %float_0_25 = OpConstant %float 0.25
         %55 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
 %float_0_75 = OpConstant %float 0.75
         %57 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
         %68 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
         %69 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
         %80 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %81 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
        %107 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
        %123 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
        %136 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
       %true = OpConstantTrue %bool
      %int_4 = OpConstant %int 4
        %180 = OpConstantComposite %v2float %float_0 %float_0_5
        %194 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
        %206 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
        %215 = OpConstantComposite %v2float %float_1 %float_0_5
        %222 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
 %expectedBW = OpVariable %_ptr_Function_v4float Function
 %expectedWT = OpVariable %_ptr_Function_v4float Function
        %230 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedBW %27
               OpStore %expectedWT %30
         %34 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %38 = OpLoad %v4float %34
         %39 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %41 = OpLoad %v4float %39
         %33 = OpExtInst %v4float %1 FMix %38 %41 %42
         %44 = OpFOrdEqual %v4bool %33 %43
         %46 = OpAll %bool %44
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %51 = OpLoad %v4float %50
         %52 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %53 = OpLoad %v4float %52
         %49 = OpExtInst %v4float %1 FMix %51 %53 %55
         %58 = OpFOrdEqual %v4bool %49 %57
         %59 = OpAll %bool %58
               OpBranch %48
         %48 = OpLabel
         %60 = OpPhi %bool %false %22 %59 %47
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %67 = OpLoad %v4float %66
         %63 = OpExtInst %v4float %1 FMix %65 %67 %68
         %70 = OpFOrdEqual %v4bool %63 %69
         %71 = OpAll %bool %70
               OpBranch %62
         %62 = OpLabel
         %72 = OpPhi %bool %false %48 %71 %61
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
         %76 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %77 = OpLoad %v4float %76
         %78 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %79 = OpLoad %v4float %78
         %75 = OpExtInst %v4float %1 FMix %77 %79 %80
         %82 = OpFOrdEqual %v4bool %75 %81
         %83 = OpAll %bool %82
               OpBranch %74
         %74 = OpLabel
         %84 = OpPhi %bool %false %62 %83 %73
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %90 = OpLoad %v4float %88
         %91 = OpCompositeExtract %float %90 0
         %92 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %94 = OpLoad %v4float %92
         %95 = OpCompositeExtract %float %94 0
         %87 = OpExtInst %float %1 FMix %91 %95 %float_0_5
         %96 = OpFOrdEqual %bool %87 %float_0_5
               OpBranch %86
         %86 = OpLabel
         %97 = OpPhi %bool %false %74 %96 %85
               OpSelectionMerge %99 None
               OpBranchConditional %97 %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %102 = OpLoad %v4float %101
        %103 = OpVectorShuffle %v2float %102 %102 0 1
        %104 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %105 = OpLoad %v4float %104
        %106 = OpVectorShuffle %v2float %105 %105 0 1
        %100 = OpExtInst %v2float %1 FMix %103 %106 %107
        %108 = OpVectorShuffle %v2float %27 %27 0 1
        %109 = OpFOrdEqual %v2bool %100 %108
        %111 = OpAll %bool %109
               OpBranch %99
         %99 = OpLabel
        %112 = OpPhi %bool %false %86 %111 %98
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %116 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %117 = OpLoad %v4float %116
        %118 = OpVectorShuffle %v3float %117 %117 0 1 2
        %120 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %121 = OpLoad %v4float %120
        %122 = OpVectorShuffle %v3float %121 %121 0 1 2
        %115 = OpExtInst %v3float %1 FMix %118 %122 %123
        %124 = OpVectorShuffle %v3float %27 %27 0 1 2
        %125 = OpFOrdEqual %v3bool %115 %124
        %127 = OpAll %bool %125
               OpBranch %114
        %114 = OpLabel
        %128 = OpPhi %bool %false %99 %127 %113
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %133 = OpLoad %v4float %132
        %134 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %135 = OpLoad %v4float %134
        %131 = OpExtInst %v4float %1 FMix %133 %135 %136
        %137 = OpFOrdEqual %v4bool %131 %27
        %138 = OpAll %bool %137
               OpBranch %130
        %130 = OpLabel
        %139 = OpPhi %bool %false %114 %138 %129
               OpSelectionMerge %141 None
               OpBranchConditional %139 %140 %141
        %140 = OpLabel
               OpBranch %141
        %141 = OpLabel
        %143 = OpPhi %bool %false %130 %true %140
               OpSelectionMerge %145 None
               OpBranchConditional %143 %144 %145
        %144 = OpLabel
        %146 = OpVectorShuffle %v2float %27 %27 0 1
        %147 = OpFOrdEqual %v2bool %107 %146
        %148 = OpAll %bool %147
               OpBranch %145
        %145 = OpLabel
        %149 = OpPhi %bool %false %141 %148 %144
               OpSelectionMerge %151 None
               OpBranchConditional %149 %150 %151
        %150 = OpLabel
        %152 = OpVectorShuffle %v3float %27 %27 0 1 2
        %153 = OpFOrdEqual %v3bool %123 %152
        %154 = OpAll %bool %153
               OpBranch %151
        %151 = OpLabel
        %155 = OpPhi %bool %false %145 %154 %150
               OpSelectionMerge %157 None
               OpBranchConditional %155 %156 %157
        %156 = OpLabel
               OpBranch %157
        %157 = OpLabel
        %158 = OpPhi %bool %false %151 %true %156
               OpSelectionMerge %160 None
               OpBranchConditional %158 %159 %160
        %159 = OpLabel
        %162 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %163 = OpLoad %v4float %162
        %164 = OpCompositeExtract %float %163 0
        %165 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %167 = OpLoad %v4float %165
        %168 = OpCompositeExtract %float %167 0
        %161 = OpExtInst %float %1 FMix %164 %168 %float_0
        %169 = OpFOrdEqual %bool %161 %float_1
               OpBranch %160
        %160 = OpLabel
        %170 = OpPhi %bool %false %157 %169 %159
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %174 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %175 = OpLoad %v4float %174
        %176 = OpVectorShuffle %v2float %175 %175 0 1
        %177 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %178 = OpLoad %v4float %177
        %179 = OpVectorShuffle %v2float %178 %178 0 1
        %173 = OpExtInst %v2float %1 FMix %176 %179 %180
        %181 = OpVectorShuffle %v2float %30 %30 0 1
        %182 = OpFOrdEqual %v2bool %173 %181
        %183 = OpAll %bool %182
               OpBranch %172
        %172 = OpLabel
        %184 = OpPhi %bool %false %160 %183 %171
               OpSelectionMerge %186 None
               OpBranchConditional %184 %185 %186
        %185 = OpLabel
        %188 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %189 = OpLoad %v4float %188
        %190 = OpVectorShuffle %v3float %189 %189 0 1 2
        %191 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %192 = OpLoad %v4float %191
        %193 = OpVectorShuffle %v3float %192 %192 0 1 2
        %187 = OpExtInst %v3float %1 FMix %190 %193 %194
        %195 = OpVectorShuffle %v3float %30 %30 0 1 2
        %196 = OpFOrdEqual %v3bool %187 %195
        %197 = OpAll %bool %196
               OpBranch %186
        %186 = OpLabel
        %198 = OpPhi %bool %false %172 %197 %185
               OpSelectionMerge %200 None
               OpBranchConditional %198 %199 %200
        %199 = OpLabel
        %202 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %203 = OpLoad %v4float %202
        %204 = OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %205 = OpLoad %v4float %204
        %201 = OpExtInst %v4float %1 FMix %203 %205 %206
        %207 = OpFOrdEqual %v4bool %201 %30
        %208 = OpAll %bool %207
               OpBranch %200
        %200 = OpLabel
        %209 = OpPhi %bool %false %186 %208 %199
               OpSelectionMerge %211 None
               OpBranchConditional %209 %210 %211
        %210 = OpLabel
               OpBranch %211
        %211 = OpLabel
        %212 = OpPhi %bool %false %200 %true %210
               OpSelectionMerge %214 None
               OpBranchConditional %212 %213 %214
        %213 = OpLabel
        %216 = OpVectorShuffle %v2float %30 %30 0 1
        %217 = OpFOrdEqual %v2bool %215 %216
        %218 = OpAll %bool %217
               OpBranch %214
        %214 = OpLabel
        %219 = OpPhi %bool %false %211 %218 %213
               OpSelectionMerge %221 None
               OpBranchConditional %219 %220 %221
        %220 = OpLabel
        %223 = OpVectorShuffle %v3float %30 %30 0 1 2
        %224 = OpFOrdEqual %v3bool %222 %223
        %225 = OpAll %bool %224
               OpBranch %221
        %221 = OpLabel
        %226 = OpPhi %bool %false %214 %225 %220
               OpSelectionMerge %228 None
               OpBranchConditional %226 %227 %228
        %227 = OpLabel
               OpBranch %228
        %228 = OpLabel
        %229 = OpPhi %bool %false %221 %true %227
               OpSelectionMerge %233 None
               OpBranchConditional %229 %231 %232
        %231 = OpLabel
        %234 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %235 = OpLoad %v4float %234
               OpStore %230 %235
               OpBranch %233
        %232 = OpLabel
        %236 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %237 = OpLoad %v4float %236
               OpStore %230 %237
               OpBranch %233
        %233 = OpLabel
        %238 = OpLoad %v4float %230
               OpReturnValue %238
               OpFunctionEnd
