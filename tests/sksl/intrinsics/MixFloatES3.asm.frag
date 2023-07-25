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
               OpName %FTFT "FTFT"
               OpName %TFTF "TFTF"
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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
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
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
     %v2bool = OpTypeVector %bool 2
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
      %int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
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
       %FTFT = OpVariable %_ptr_Function_v4bool Function
       %TFTF = OpVariable %_ptr_Function_v4bool Function
        %236 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpCompositeExtract %float %33 0
         %35 = OpFUnordNotEqual %bool %34 %float_0
         %36 = OpCompositeExtract %float %33 1
         %37 = OpFUnordNotEqual %bool %36 %float_0
         %38 = OpCompositeExtract %float %33 2
         %39 = OpFUnordNotEqual %bool %38 %float_0
         %40 = OpCompositeExtract %float %33 3
         %41 = OpFUnordNotEqual %bool %40 %float_0
         %42 = OpCompositeConstruct %v4bool %35 %37 %39 %41
               OpStore %FTFT %42
         %44 = OpVectorShuffle %v4bool %42 %42 3 2 1 0
               OpStore %TFTF %44
         %47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %49 = OpLoad %v4float %47
         %50 = OpCompositeExtract %float %49 0
         %51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %53 = OpLoad %v4float %51
         %54 = OpCompositeExtract %float %53 0
         %55 = OpCompositeExtract %bool %42 0
         %56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %57 = OpLoad %v4float %56
         %58 = OpCompositeExtract %float %57 0
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %60 = OpLoad %v4float %59
         %61 = OpCompositeExtract %float %60 0
         %46 = OpSelect %float %55 %61 %58
         %62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %63 = OpLoad %v4float %62
         %64 = OpCompositeExtract %float %63 0
         %65 = OpFOrdEqual %bool %46 %64
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %70 = OpLoad %v4float %69
         %71 = OpVectorShuffle %v2float %70 %70 0 1
         %72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %73 = OpLoad %v4float %72
         %74 = OpVectorShuffle %v2float %73 %73 0 1
         %75 = OpVectorShuffle %v2bool %42 %42 0 1
         %77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %78 = OpLoad %v4float %77
         %79 = OpVectorShuffle %v2float %78 %78 0 1
         %80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %81 = OpLoad %v4float %80
         %82 = OpVectorShuffle %v2float %81 %81 0 1
         %83 = OpVectorShuffle %v2bool %42 %42 0 1
         %68 = OpSelect %v2float %83 %82 %79
         %84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %85 = OpLoad %v4float %84
         %86 = OpCompositeExtract %float %85 0
         %88 = OpCompositeConstruct %v2float %86 %float_1
         %89 = OpFOrdEqual %v2bool %68 %88
         %90 = OpAll %bool %89
               OpBranch %67
         %67 = OpLabel
         %91 = OpPhi %bool %false %25 %90 %66
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %96 = OpLoad %v4float %95
         %97 = OpVectorShuffle %v3float %96 %96 0 1 2
         %99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %100 = OpLoad %v4float %99
        %101 = OpVectorShuffle %v3float %100 %100 0 1 2
        %102 = OpVectorShuffle %v3bool %42 %42 0 1 2
        %104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %105 = OpLoad %v4float %104
        %106 = OpVectorShuffle %v3float %105 %105 0 1 2
        %107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %108 = OpLoad %v4float %107
        %109 = OpVectorShuffle %v3float %108 %108 0 1 2
        %110 = OpVectorShuffle %v3bool %42 %42 0 1 2
         %94 = OpSelect %v3float %110 %109 %106
        %111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %112 = OpLoad %v4float %111
        %113 = OpCompositeExtract %float %112 0
        %114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %115 = OpLoad %v4float %114
        %116 = OpCompositeExtract %float %115 2
        %117 = OpCompositeConstruct %v3float %113 %float_1 %116
        %118 = OpFOrdEqual %v3bool %94 %117
        %119 = OpAll %bool %118
               OpBranch %93
         %93 = OpLabel
        %120 = OpPhi %bool %false %67 %119 %92
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %125 = OpLoad %v4float %124
        %126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %127 = OpLoad %v4float %126
        %128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %129 = OpLoad %v4float %128
        %130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %131 = OpLoad %v4float %130
        %123 = OpSelect %v4float %42 %131 %129
        %132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %133 = OpLoad %v4float %132
        %134 = OpCompositeExtract %float %133 0
        %135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %136 = OpLoad %v4float %135
        %137 = OpCompositeExtract %float %136 2
        %138 = OpCompositeConstruct %v4float %134 %float_1 %137 %float_1
        %139 = OpFOrdEqual %v4bool %123 %138
        %140 = OpAll %bool %139
               OpBranch %122
        %122 = OpLabel
        %141 = OpPhi %bool %false %93 %140 %121
               OpSelectionMerge %143 None
               OpBranchConditional %141 %142 %143
        %142 = OpLabel
        %145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %146 = OpLoad %v4float %145
        %147 = OpCompositeExtract %float %146 0
        %148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %150 = OpLoad %v4float %148
        %151 = OpCompositeExtract %float %150 0
        %152 = OpCompositeExtract %bool %44 0
        %153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %154 = OpLoad %v4float %153
        %155 = OpCompositeExtract %float %154 0
        %156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %157 = OpLoad %v4float %156
        %158 = OpCompositeExtract %float %157 0
        %144 = OpSelect %float %152 %158 %155
        %159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %160 = OpLoad %v4float %159
        %161 = OpCompositeExtract %float %160 0
        %162 = OpFOrdEqual %bool %144 %161
               OpBranch %143
        %143 = OpLabel
        %163 = OpPhi %bool %false %122 %162 %142
               OpSelectionMerge %165 None
               OpBranchConditional %163 %164 %165
        %164 = OpLabel
        %167 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %168 = OpLoad %v4float %167
        %169 = OpVectorShuffle %v2float %168 %168 0 1
        %170 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %171 = OpLoad %v4float %170
        %172 = OpVectorShuffle %v2float %171 %171 0 1
        %173 = OpVectorShuffle %v2bool %44 %44 0 1
        %174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %175 = OpLoad %v4float %174
        %176 = OpVectorShuffle %v2float %175 %175 0 1
        %177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %178 = OpLoad %v4float %177
        %179 = OpVectorShuffle %v2float %178 %178 0 1
        %180 = OpVectorShuffle %v2bool %44 %44 0 1
        %166 = OpSelect %v2float %180 %179 %176
        %181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %182 = OpLoad %v4float %181
        %183 = OpCompositeExtract %float %182 0
        %184 = OpCompositeConstruct %v2float %183 %float_1
        %185 = OpFOrdEqual %v2bool %166 %184
        %186 = OpAll %bool %185
               OpBranch %165
        %165 = OpLabel
        %187 = OpPhi %bool %false %143 %186 %164
               OpSelectionMerge %189 None
               OpBranchConditional %187 %188 %189
        %188 = OpLabel
        %191 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %192 = OpLoad %v4float %191
        %193 = OpVectorShuffle %v3float %192 %192 0 1 2
        %194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %195 = OpLoad %v4float %194
        %196 = OpVectorShuffle %v3float %195 %195 0 1 2
        %197 = OpVectorShuffle %v3bool %44 %44 0 1 2
        %198 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %199 = OpLoad %v4float %198
        %200 = OpVectorShuffle %v3float %199 %199 0 1 2
        %201 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %202 = OpLoad %v4float %201
        %203 = OpVectorShuffle %v3float %202 %202 0 1 2
        %204 = OpVectorShuffle %v3bool %44 %44 0 1 2
        %190 = OpSelect %v3float %204 %203 %200
        %205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %206 = OpLoad %v4float %205
        %207 = OpCompositeExtract %float %206 0
        %208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %209 = OpLoad %v4float %208
        %210 = OpCompositeExtract %float %209 2
        %211 = OpCompositeConstruct %v3float %207 %float_1 %210
        %212 = OpFOrdEqual %v3bool %190 %211
        %213 = OpAll %bool %212
               OpBranch %189
        %189 = OpLabel
        %214 = OpPhi %bool %false %165 %213 %188
               OpSelectionMerge %216 None
               OpBranchConditional %214 %215 %216
        %215 = OpLabel
        %218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %219 = OpLoad %v4float %218
        %220 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %221 = OpLoad %v4float %220
        %222 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %223 = OpLoad %v4float %222
        %224 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %225 = OpLoad %v4float %224
        %217 = OpSelect %v4float %44 %225 %223
        %226 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %227 = OpLoad %v4float %226
        %228 = OpCompositeExtract %float %227 0
        %229 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
        %230 = OpLoad %v4float %229
        %231 = OpCompositeExtract %float %230 2
        %232 = OpCompositeConstruct %v4float %228 %float_1 %231 %float_1
        %233 = OpFOrdEqual %v4bool %217 %232
        %234 = OpAll %bool %233
               OpBranch %216
        %216 = OpLabel
        %235 = OpPhi %bool %false %189 %234 %215
               OpSelectionMerge %240 None
               OpBranchConditional %235 %238 %239
        %238 = OpLabel
        %241 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %242 = OpLoad %v4float %241
               OpStore %236 %242
               OpBranch %240
        %239 = OpLabel
        %243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %245 = OpLoad %v4float %243
               OpStore %236 %245
               OpBranch %240
        %240 = OpLabel
        %246 = OpLoad %v4float %236
               OpReturnValue %246
               OpFunctionEnd
