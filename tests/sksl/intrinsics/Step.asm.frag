               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedA "expectedA"
               OpName %expectedB "expectedB"
               OpName %expectedC "expectedC"
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
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %expectedC RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
         %31 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
         %33 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
      %false = OpConstantFalse %bool
  %float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %47 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %60 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %72 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %91 = OpConstantComposite %v3float %float_0 %float_0 %float_1
        %113 = OpConstantComposite %v2float %float_0 %float_1
        %124 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %134 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
        %143 = OpConstantComposite %v2float %float_1 %float_1
        %150 = OpConstantComposite %v3float %float_1 %float_1 %float_0
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
        %218 = OpConstantComposite %v3float %float_0 %float_1 %float_1
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
  %expectedA = OpVariable %_ptr_Function_v4float Function
  %expectedB = OpVariable %_ptr_Function_v4float Function
  %expectedC = OpVariable %_ptr_Function_v4float Function
        %226 = OpVariable %_ptr_Function_v4float Function
               OpStore %expectedA %29
               OpStore %expectedB %31
               OpStore %expectedC %33
         %37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %41 = OpLoad %v4float %37
         %42 = OpCompositeExtract %float %41 0
         %35 = OpExtInst %float %1 Step %float_0_5 %42
         %43 = OpFOrdEqual %bool %35 %float_0
               OpSelectionMerge %45 None
               OpBranchConditional %43 %44 %45
         %44 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v2float %49 %49 0 1
         %46 = OpExtInst %v2float %1 Step %47 %50
         %51 = OpVectorShuffle %v2float %29 %29 0 1
         %52 = OpFOrdEqual %v2bool %46 %51
         %54 = OpAll %bool %52
               OpBranch %45
         %45 = OpLabel
         %55 = OpPhi %bool %false %25 %54 %44
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %62 = OpLoad %v4float %61
         %63 = OpVectorShuffle %v3float %62 %62 0 1 2
         %58 = OpExtInst %v3float %1 Step %60 %63
         %64 = OpVectorShuffle %v3float %29 %29 0 1 2
         %65 = OpFOrdEqual %v3bool %58 %64
         %67 = OpAll %bool %65
               OpBranch %57
         %57 = OpLabel
         %68 = OpPhi %bool %false %45 %67 %56
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %74 = OpLoad %v4float %73
         %71 = OpExtInst %v4float %1 Step %72 %74
         %75 = OpFOrdEqual %v4bool %71 %29
         %77 = OpAll %bool %75
               OpBranch %70
         %70 = OpLabel
         %78 = OpPhi %bool %false %57 %77 %69
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
               OpBranch %80
         %80 = OpLabel
         %82 = OpPhi %bool %false %70 %true %79
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %85 = OpVectorShuffle %v2float %29 %29 0 1
         %86 = OpFOrdEqual %v2bool %19 %85
         %87 = OpAll %bool %86
               OpBranch %84
         %84 = OpLabel
         %88 = OpPhi %bool %false %80 %87 %83
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %92 = OpVectorShuffle %v3float %29 %29 0 1 2
         %93 = OpFOrdEqual %v3bool %91 %92
         %94 = OpAll %bool %93
               OpBranch %90
         %90 = OpLabel
         %95 = OpPhi %bool %false %84 %94 %89
               OpSelectionMerge %97 None
               OpBranchConditional %95 %96 %97
         %96 = OpLabel
               OpBranch %97
         %97 = OpLabel
         %98 = OpPhi %bool %false %90 %true %96
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %103 = OpLoad %v4float %102
        %104 = OpCompositeExtract %float %103 0
        %101 = OpExtInst %float %1 Step %104 %float_0
        %105 = OpFOrdEqual %bool %101 %float_1
               OpBranch %100
        %100 = OpLabel
        %106 = OpPhi %bool %false %97 %105 %99
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %111 = OpLoad %v4float %110
        %112 = OpVectorShuffle %v2float %111 %111 0 1
        %109 = OpExtInst %v2float %1 Step %112 %113
        %114 = OpVectorShuffle %v2float %31 %31 0 1
        %115 = OpFOrdEqual %v2bool %109 %114
        %116 = OpAll %bool %115
               OpBranch %108
        %108 = OpLabel
        %117 = OpPhi %bool %false %100 %116 %107
               OpSelectionMerge %119 None
               OpBranchConditional %117 %118 %119
        %118 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %122 = OpLoad %v4float %121
        %123 = OpVectorShuffle %v3float %122 %122 0 1 2
        %120 = OpExtInst %v3float %1 Step %123 %124
        %125 = OpVectorShuffle %v3float %31 %31 0 1 2
        %126 = OpFOrdEqual %v3bool %120 %125
        %127 = OpAll %bool %126
               OpBranch %119
        %119 = OpLabel
        %128 = OpPhi %bool %false %108 %127 %118
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %133 = OpLoad %v4float %132
        %131 = OpExtInst %v4float %1 Step %133 %134
        %135 = OpFOrdEqual %v4bool %131 %31
        %136 = OpAll %bool %135
               OpBranch %130
        %130 = OpLabel
        %137 = OpPhi %bool %false %119 %136 %129
               OpSelectionMerge %139 None
               OpBranchConditional %137 %138 %139
        %138 = OpLabel
               OpBranch %139
        %139 = OpLabel
        %140 = OpPhi %bool %false %130 %true %138
               OpSelectionMerge %142 None
               OpBranchConditional %140 %141 %142
        %141 = OpLabel
        %144 = OpVectorShuffle %v2float %31 %31 0 1
        %145 = OpFOrdEqual %v2bool %143 %144
        %146 = OpAll %bool %145
               OpBranch %142
        %142 = OpLabel
        %147 = OpPhi %bool %false %139 %146 %141
               OpSelectionMerge %149 None
               OpBranchConditional %147 %148 %149
        %148 = OpLabel
        %151 = OpVectorShuffle %v3float %31 %31 0 1 2
        %152 = OpFOrdEqual %v3bool %150 %151
        %153 = OpAll %bool %152
               OpBranch %149
        %149 = OpLabel
        %154 = OpPhi %bool %false %142 %153 %148
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
               OpBranch %156
        %156 = OpLabel
        %157 = OpPhi %bool %false %149 %true %155
               OpSelectionMerge %159 None
               OpBranchConditional %157 %158 %159
        %158 = OpLabel
        %161 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %163 = OpLoad %v4float %161
        %164 = OpCompositeExtract %float %163 0
        %165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %167 = OpLoad %v4float %165
        %168 = OpCompositeExtract %float %167 0
        %160 = OpExtInst %float %1 Step %164 %168
        %169 = OpFOrdEqual %bool %160 %float_0
               OpBranch %159
        %159 = OpLabel
        %170 = OpPhi %bool %false %156 %169 %158
               OpSelectionMerge %172 None
               OpBranchConditional %170 %171 %172
        %171 = OpLabel
        %174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %175 = OpLoad %v4float %174
        %176 = OpVectorShuffle %v2float %175 %175 0 1
        %177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %178 = OpLoad %v4float %177
        %179 = OpVectorShuffle %v2float %178 %178 0 1
        %173 = OpExtInst %v2float %1 Step %176 %179
        %180 = OpVectorShuffle %v2float %33 %33 0 1
        %181 = OpFOrdEqual %v2bool %173 %180
        %182 = OpAll %bool %181
               OpBranch %172
        %172 = OpLabel
        %183 = OpPhi %bool %false %159 %182 %171
               OpSelectionMerge %185 None
               OpBranchConditional %183 %184 %185
        %184 = OpLabel
        %187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %188 = OpLoad %v4float %187
        %189 = OpVectorShuffle %v3float %188 %188 0 1 2
        %190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %191 = OpLoad %v4float %190
        %192 = OpVectorShuffle %v3float %191 %191 0 1 2
        %186 = OpExtInst %v3float %1 Step %189 %192
        %193 = OpVectorShuffle %v3float %33 %33 0 1 2
        %194 = OpFOrdEqual %v3bool %186 %193
        %195 = OpAll %bool %194
               OpBranch %185
        %185 = OpLabel
        %196 = OpPhi %bool %false %172 %195 %184
               OpSelectionMerge %198 None
               OpBranchConditional %196 %197 %198
        %197 = OpLabel
        %200 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %201 = OpLoad %v4float %200
        %202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %203 = OpLoad %v4float %202
        %199 = OpExtInst %v4float %1 Step %201 %203
        %204 = OpFOrdEqual %v4bool %199 %33
        %205 = OpAll %bool %204
               OpBranch %198
        %198 = OpLabel
        %206 = OpPhi %bool %false %185 %205 %197
               OpSelectionMerge %208 None
               OpBranchConditional %206 %207 %208
        %207 = OpLabel
               OpBranch %208
        %208 = OpLabel
        %209 = OpPhi %bool %false %198 %true %207
               OpSelectionMerge %211 None
               OpBranchConditional %209 %210 %211
        %210 = OpLabel
        %212 = OpVectorShuffle %v2float %33 %33 0 1
        %213 = OpFOrdEqual %v2bool %113 %212
        %214 = OpAll %bool %213
               OpBranch %211
        %211 = OpLabel
        %215 = OpPhi %bool %false %208 %214 %210
               OpSelectionMerge %217 None
               OpBranchConditional %215 %216 %217
        %216 = OpLabel
        %219 = OpVectorShuffle %v3float %33 %33 0 1 2
        %220 = OpFOrdEqual %v3bool %218 %219
        %221 = OpAll %bool %220
               OpBranch %217
        %217 = OpLabel
        %222 = OpPhi %bool %false %211 %221 %216
               OpSelectionMerge %224 None
               OpBranchConditional %222 %223 %224
        %223 = OpLabel
               OpBranch %224
        %224 = OpLabel
        %225 = OpPhi %bool %false %217 %true %223
               OpSelectionMerge %229 None
               OpBranchConditional %225 %227 %228
        %227 = OpLabel
        %230 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %231 = OpLoad %v4float %230
               OpStore %226 %231
               OpBranch %229
        %228 = OpLabel
        %232 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %233 = OpLoad %v4float %232
               OpStore %226 %233
               OpBranch %229
        %229 = OpLabel
        %234 = OpLoad %v4float %226
               OpReturnValue %234
               OpFunctionEnd
