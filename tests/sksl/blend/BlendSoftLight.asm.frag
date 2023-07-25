               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"
               OpName %DSqd "DSqd"
               OpName %DCub "DCub"
               OpName %DaSqd "DaSqd"
               OpName %DaCub "DaCub"
               OpName %main "main"
               OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %17 Binding 0
               OpDecorate %17 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %DSqd RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %DCub RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %DaSqd RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %DaCub RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
      %float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
    %float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %22 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
    %float_1 = OpConstant %float 1
    %float_4 = OpConstant %float 4
%_ptr_Function_float = OpTypePointer Function %float
    %float_3 = OpConstant %float 3
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
   %float_16 = OpConstant %float 16
       %void = OpTypeVoid
        %185 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0
%soft_light_component_Qhh2h2 = OpFunction %float None %22
         %23 = OpFunctionParameter %_ptr_Function_v2float
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
       %DSqd = OpVariable %_ptr_Function_float Function
       %DCub = OpVariable %_ptr_Function_float Function
      %DaSqd = OpVariable %_ptr_Function_float Function
      %DaCub = OpVariable %_ptr_Function_float Function
         %27 = OpLoad %v2float %23
         %28 = OpCompositeExtract %float %27 0
         %29 = OpFMul %float %float_2 %28
         %30 = OpLoad %v2float %23
         %31 = OpCompositeExtract %float %30 1
         %32 = OpFOrdLessThanEqual %bool %29 %31
               OpSelectionMerge %35 None
               OpBranchConditional %32 %33 %34
         %33 = OpLabel
         %36 = OpLoad %v2float %24
         %37 = OpCompositeExtract %float %36 0
         %38 = OpLoad %v2float %24
         %39 = OpCompositeExtract %float %38 0
         %40 = OpFMul %float %37 %39
         %41 = OpLoad %v2float %23
         %42 = OpCompositeExtract %float %41 1
         %43 = OpLoad %v2float %23
         %44 = OpCompositeExtract %float %43 0
         %45 = OpFMul %float %float_2 %44
         %46 = OpFSub %float %42 %45
         %47 = OpFMul %float %40 %46
         %48 = OpLoad %v2float %24
         %49 = OpCompositeExtract %float %48 1
         %50 = OpLoad %float %_kGuardedDivideEpsilon
         %51 = OpFAdd %float %49 %50
         %52 = OpFDiv %float %47 %51
         %54 = OpLoad %v2float %24
         %55 = OpCompositeExtract %float %54 1
         %56 = OpFSub %float %float_1 %55
         %57 = OpLoad %v2float %23
         %58 = OpCompositeExtract %float %57 0
         %59 = OpFMul %float %56 %58
         %60 = OpFAdd %float %52 %59
         %61 = OpLoad %v2float %24
         %62 = OpCompositeExtract %float %61 0
         %63 = OpLoad %v2float %23
         %64 = OpCompositeExtract %float %63 1
         %65 = OpFNegate %float %64
         %66 = OpLoad %v2float %23
         %67 = OpCompositeExtract %float %66 0
         %68 = OpFMul %float %float_2 %67
         %69 = OpFAdd %float %65 %68
         %70 = OpFAdd %float %69 %float_1
         %71 = OpFMul %float %62 %70
         %72 = OpFAdd %float %60 %71
               OpReturnValue %72
         %34 = OpLabel
         %74 = OpLoad %v2float %24
         %75 = OpCompositeExtract %float %74 0
         %76 = OpFMul %float %float_4 %75
         %77 = OpLoad %v2float %24
         %78 = OpCompositeExtract %float %77 1
         %79 = OpFOrdLessThanEqual %bool %76 %78
               OpSelectionMerge %82 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %85 = OpLoad %v2float %24
         %86 = OpCompositeExtract %float %85 0
         %87 = OpLoad %v2float %24
         %88 = OpCompositeExtract %float %87 0
         %89 = OpFMul %float %86 %88
               OpStore %DSqd %89
         %91 = OpLoad %v2float %24
         %92 = OpCompositeExtract %float %91 0
         %93 = OpFMul %float %89 %92
               OpStore %DCub %93
         %95 = OpLoad %v2float %24
         %96 = OpCompositeExtract %float %95 1
         %97 = OpLoad %v2float %24
         %98 = OpCompositeExtract %float %97 1
         %99 = OpFMul %float %96 %98
               OpStore %DaSqd %99
        %101 = OpLoad %v2float %24
        %102 = OpCompositeExtract %float %101 1
        %103 = OpFMul %float %99 %102
               OpStore %DaCub %103
        %104 = OpLoad %v2float %23
        %105 = OpCompositeExtract %float %104 0
        %106 = OpLoad %v2float %24
        %107 = OpCompositeExtract %float %106 0
        %109 = OpLoad %v2float %23
        %110 = OpCompositeExtract %float %109 1
        %111 = OpFMul %float %float_3 %110
        %113 = OpLoad %v2float %23
        %114 = OpCompositeExtract %float %113 0
        %115 = OpFMul %float %float_6 %114
        %116 = OpFSub %float %111 %115
        %117 = OpFSub %float %116 %float_1
        %118 = OpFMul %float %107 %117
        %119 = OpFSub %float %105 %118
        %120 = OpFMul %float %99 %119
        %122 = OpLoad %v2float %24
        %123 = OpCompositeExtract %float %122 1
        %124 = OpFMul %float %float_12 %123
        %125 = OpFMul %float %124 %89
        %126 = OpLoad %v2float %23
        %127 = OpCompositeExtract %float %126 1
        %128 = OpLoad %v2float %23
        %129 = OpCompositeExtract %float %128 0
        %130 = OpFMul %float %float_2 %129
        %131 = OpFSub %float %127 %130
        %132 = OpFMul %float %125 %131
        %133 = OpFAdd %float %120 %132
        %135 = OpFMul %float %float_16 %93
        %136 = OpLoad %v2float %23
        %137 = OpCompositeExtract %float %136 1
        %138 = OpLoad %v2float %23
        %139 = OpCompositeExtract %float %138 0
        %140 = OpFMul %float %float_2 %139
        %141 = OpFSub %float %137 %140
        %142 = OpFMul %float %135 %141
        %143 = OpFSub %float %133 %142
        %144 = OpLoad %v2float %23
        %145 = OpCompositeExtract %float %144 0
        %146 = OpFMul %float %103 %145
        %147 = OpFSub %float %143 %146
        %148 = OpLoad %float %_kGuardedDivideEpsilon
        %149 = OpFAdd %float %99 %148
        %150 = OpFDiv %float %147 %149
               OpReturnValue %150
         %81 = OpLabel
        %151 = OpLoad %v2float %24
        %152 = OpCompositeExtract %float %151 0
        %153 = OpLoad %v2float %23
        %154 = OpCompositeExtract %float %153 1
        %155 = OpLoad %v2float %23
        %156 = OpCompositeExtract %float %155 0
        %157 = OpFMul %float %float_2 %156
        %158 = OpFSub %float %154 %157
        %159 = OpFAdd %float %158 %float_1
        %160 = OpFMul %float %152 %159
        %161 = OpLoad %v2float %23
        %162 = OpCompositeExtract %float %161 0
        %163 = OpFAdd %float %160 %162
        %165 = OpLoad %v2float %24
        %166 = OpCompositeExtract %float %165 1
        %167 = OpLoad %v2float %24
        %168 = OpCompositeExtract %float %167 0
        %169 = OpFMul %float %166 %168
        %164 = OpExtInst %float %1 Sqrt %169
        %170 = OpLoad %v2float %23
        %171 = OpCompositeExtract %float %170 1
        %172 = OpLoad %v2float %23
        %173 = OpCompositeExtract %float %172 0
        %174 = OpFMul %float %float_2 %173
        %175 = OpFSub %float %171 %174
        %176 = OpFMul %float %164 %175
        %177 = OpFSub %float %163 %176
        %178 = OpLoad %v2float %24
        %179 = OpCompositeExtract %float %178 1
        %180 = OpLoad %v2float %23
        %181 = OpCompositeExtract %float %180 0
        %182 = OpFMul %float %179 %181
        %183 = OpFSub %float %177 %182
               OpReturnValue %183
         %82 = OpLabel
               OpBranch %35
         %35 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %185
        %186 = OpLabel
        %194 = OpVariable %_ptr_Function_v4float Function
        %205 = OpVariable %_ptr_Function_v2float Function
        %209 = OpVariable %_ptr_Function_v2float Function
        %214 = OpVariable %_ptr_Function_v2float Function
        %218 = OpVariable %_ptr_Function_v2float Function
        %223 = OpVariable %_ptr_Function_v2float Function
        %227 = OpVariable %_ptr_Function_v2float Function
          %9 = OpSelect %float %false %float_9_99999994en09 %float_0
               OpStore %_kGuardedDivideEpsilon %9
        %187 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %191 = OpLoad %v4float %187
        %192 = OpCompositeExtract %float %191 3
        %193 = OpFOrdEqual %bool %192 %float_0
               OpSelectionMerge %198 None
               OpBranchConditional %193 %196 %197
        %196 = OpLabel
        %199 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %201 = OpLoad %v4float %199
               OpStore %194 %201
               OpBranch %198
        %197 = OpLabel
        %202 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %203 = OpLoad %v4float %202
        %204 = OpVectorShuffle %v2float %203 %203 0 3
               OpStore %205 %204
        %206 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %207 = OpLoad %v4float %206
        %208 = OpVectorShuffle %v2float %207 %207 0 3
               OpStore %209 %208
        %210 = OpFunctionCall %float %soft_light_component_Qhh2h2 %205 %209
        %211 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %212 = OpLoad %v4float %211
        %213 = OpVectorShuffle %v2float %212 %212 1 3
               OpStore %214 %213
        %215 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %216 = OpLoad %v4float %215
        %217 = OpVectorShuffle %v2float %216 %216 1 3
               OpStore %218 %217
        %219 = OpFunctionCall %float %soft_light_component_Qhh2h2 %214 %218
        %220 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %221 = OpLoad %v4float %220
        %222 = OpVectorShuffle %v2float %221 %221 2 3
               OpStore %223 %222
        %224 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %225 = OpLoad %v4float %224
        %226 = OpVectorShuffle %v2float %225 %225 2 3
               OpStore %227 %226
        %228 = OpFunctionCall %float %soft_light_component_Qhh2h2 %223 %227
        %229 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %230 = OpLoad %v4float %229
        %231 = OpCompositeExtract %float %230 3
        %232 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
        %233 = OpLoad %v4float %232
        %234 = OpCompositeExtract %float %233 3
        %235 = OpFSub %float %float_1 %234
        %236 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
        %237 = OpLoad %v4float %236
        %238 = OpCompositeExtract %float %237 3
        %239 = OpFMul %float %235 %238
        %240 = OpFAdd %float %231 %239
        %241 = OpCompositeConstruct %v4float %210 %219 %228 %240
               OpStore %194 %241
               OpBranch %198
        %198 = OpLabel
        %242 = OpLoad %v4float %194
               OpStore %sk_FragColor %242
               OpReturn
               OpFunctionEnd
