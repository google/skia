               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %hugeH "hugeH"
               OpName %hugeF "hugeF"
               OpName %hugeI "hugeI"
               OpName %hugeU "hugeU"
               OpName %hugeS "hugeS"
               OpName %hugeUS "hugeUS"
               OpName %hugeNI "hugeNI"
               OpName %hugeNS "hugeNS"
               OpName %hugeIvec "hugeIvec"
               OpName %hugeUvec "hugeUvec"
               OpName %hugeMxM "hugeMxM"
               OpName %hugeMxV "hugeMxV"
               OpName %hugeVxM "hugeVxM"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %hugeH RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %hugeS RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %hugeUS RelaxedPrecision
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
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %hugeNS RelaxedPrecision
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
               OpDecorate %232 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %264 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %278 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %285 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_9_99999962e_35 = OpConstant %float 9.99999962e+35
%float_1e_09 = OpConstant %float 1e+09
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1073741824 = OpConstant %int 1073741824
      %int_2 = OpConstant %int 2
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_2147483648 = OpConstant %uint 2147483648
     %uint_2 = OpConstant %uint 2
  %int_16384 = OpConstant %int 16384
 %uint_32768 = OpConstant %uint 32768
%int_n2147483648 = OpConstant %int -2147483648
 %int_n32768 = OpConstant %int -32768
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
        %181 = OpConstantComposite %v4int %int_1073741824 %int_1073741824 %int_1073741824 %int_1073741824
        %182 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
        %201 = OpConstantComposite %v4uint %uint_2147483648 %uint_2147483648 %uint_2147483648 %uint_2147483648
        %202 = OpConstantComposite %v4uint %uint_2 %uint_2 %uint_2 %uint_2
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_1_00000002e_20 = OpConstant %float 1.00000002e+20
        %221 = OpConstantComposite %v4float %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20
        %222 = OpConstantComposite %mat4v4float %221 %221 %221 %221
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
        %266 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %267 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
      %hugeH = OpVariable %_ptr_Function_float Function
      %hugeF = OpVariable %_ptr_Function_float Function
      %hugeI = OpVariable %_ptr_Function_int Function
      %hugeU = OpVariable %_ptr_Function_uint Function
      %hugeS = OpVariable %_ptr_Function_int Function
     %hugeUS = OpVariable %_ptr_Function_uint Function
     %hugeNI = OpVariable %_ptr_Function_int Function
     %hugeNS = OpVariable %_ptr_Function_int Function
   %hugeIvec = OpVariable %_ptr_Function_v4int Function
   %hugeUvec = OpVariable %_ptr_Function_v4uint Function
    %hugeMxM = OpVariable %_ptr_Function_mat4v4float Function
    %hugeMxV = OpVariable %_ptr_Function_v4float Function
    %hugeVxM = OpVariable %_ptr_Function_v4float Function
         %30 = OpFMul %float %float_9_99999962e_35 %float_1e_09
         %31 = OpFMul %float %30 %float_1e_09
         %32 = OpFMul %float %31 %float_1e_09
         %33 = OpFMul %float %32 %float_1e_09
         %34 = OpFMul %float %33 %float_1e_09
         %35 = OpFMul %float %34 %float_1e_09
         %36 = OpFMul %float %35 %float_1e_09
         %37 = OpFMul %float %36 %float_1e_09
         %38 = OpFMul %float %37 %float_1e_09
         %39 = OpFMul %float %38 %float_1e_09
         %40 = OpFMul %float %39 %float_1e_09
               OpStore %hugeH %40
         %42 = OpFMul %float %float_9_99999962e_35 %float_1e_09
         %43 = OpFMul %float %42 %float_1e_09
         %44 = OpFMul %float %43 %float_1e_09
         %45 = OpFMul %float %44 %float_1e_09
         %46 = OpFMul %float %45 %float_1e_09
         %47 = OpFMul %float %46 %float_1e_09
         %48 = OpFMul %float %47 %float_1e_09
         %49 = OpFMul %float %48 %float_1e_09
         %50 = OpFMul %float %49 %float_1e_09
         %51 = OpFMul %float %50 %float_1e_09
         %52 = OpFMul %float %51 %float_1e_09
               OpStore %hugeF %52
         %58 = OpIMul %int %int_1073741824 %int_2
         %59 = OpIMul %int %58 %int_2
         %60 = OpIMul %int %59 %int_2
         %61 = OpIMul %int %60 %int_2
         %62 = OpIMul %int %61 %int_2
         %63 = OpIMul %int %62 %int_2
         %64 = OpIMul %int %63 %int_2
         %65 = OpIMul %int %64 %int_2
         %66 = OpIMul %int %65 %int_2
         %67 = OpIMul %int %66 %int_2
         %68 = OpIMul %int %67 %int_2
         %69 = OpIMul %int %68 %int_2
         %70 = OpIMul %int %69 %int_2
         %71 = OpIMul %int %70 %int_2
         %72 = OpIMul %int %71 %int_2
         %73 = OpIMul %int %72 %int_2
         %74 = OpIMul %int %73 %int_2
         %75 = OpIMul %int %74 %int_2
         %76 = OpIMul %int %75 %int_2
         %77 = OpIMul %int %76 %int_2
               OpStore %hugeI %77
         %83 = OpIMul %uint %uint_2147483648 %uint_2
         %84 = OpIMul %uint %83 %uint_2
         %85 = OpIMul %uint %84 %uint_2
         %86 = OpIMul %uint %85 %uint_2
         %87 = OpIMul %uint %86 %uint_2
         %88 = OpIMul %uint %87 %uint_2
         %89 = OpIMul %uint %88 %uint_2
         %90 = OpIMul %uint %89 %uint_2
         %91 = OpIMul %uint %90 %uint_2
         %92 = OpIMul %uint %91 %uint_2
         %93 = OpIMul %uint %92 %uint_2
         %94 = OpIMul %uint %93 %uint_2
         %95 = OpIMul %uint %94 %uint_2
         %96 = OpIMul %uint %95 %uint_2
         %97 = OpIMul %uint %96 %uint_2
         %98 = OpIMul %uint %97 %uint_2
         %99 = OpIMul %uint %98 %uint_2
        %100 = OpIMul %uint %99 %uint_2
        %101 = OpIMul %uint %100 %uint_2
               OpStore %hugeU %101
        %104 = OpIMul %int %int_16384 %int_2
        %105 = OpIMul %int %104 %int_2
        %106 = OpIMul %int %105 %int_2
        %107 = OpIMul %int %106 %int_2
        %108 = OpIMul %int %107 %int_2
        %109 = OpIMul %int %108 %int_2
        %110 = OpIMul %int %109 %int_2
        %111 = OpIMul %int %110 %int_2
        %112 = OpIMul %int %111 %int_2
        %113 = OpIMul %int %112 %int_2
        %114 = OpIMul %int %113 %int_2
        %115 = OpIMul %int %114 %int_2
        %116 = OpIMul %int %115 %int_2
        %117 = OpIMul %int %116 %int_2
        %118 = OpIMul %int %117 %int_2
        %119 = OpIMul %int %118 %int_2
        %120 = OpIMul %int %119 %int_2
               OpStore %hugeS %120
        %123 = OpIMul %uint %uint_32768 %uint_2
        %124 = OpIMul %uint %123 %uint_2
        %125 = OpIMul %uint %124 %uint_2
        %126 = OpIMul %uint %125 %uint_2
        %127 = OpIMul %uint %126 %uint_2
        %128 = OpIMul %uint %127 %uint_2
        %129 = OpIMul %uint %128 %uint_2
        %130 = OpIMul %uint %129 %uint_2
        %131 = OpIMul %uint %130 %uint_2
        %132 = OpIMul %uint %131 %uint_2
        %133 = OpIMul %uint %132 %uint_2
        %134 = OpIMul %uint %133 %uint_2
        %135 = OpIMul %uint %134 %uint_2
        %136 = OpIMul %uint %135 %uint_2
        %137 = OpIMul %uint %136 %uint_2
        %138 = OpIMul %uint %137 %uint_2
               OpStore %hugeUS %138
        %141 = OpIMul %int %int_n2147483648 %int_2
        %142 = OpIMul %int %141 %int_2
        %143 = OpIMul %int %142 %int_2
        %144 = OpIMul %int %143 %int_2
        %145 = OpIMul %int %144 %int_2
        %146 = OpIMul %int %145 %int_2
        %147 = OpIMul %int %146 %int_2
        %148 = OpIMul %int %147 %int_2
        %149 = OpIMul %int %148 %int_2
        %150 = OpIMul %int %149 %int_2
        %151 = OpIMul %int %150 %int_2
        %152 = OpIMul %int %151 %int_2
        %153 = OpIMul %int %152 %int_2
        %154 = OpIMul %int %153 %int_2
        %155 = OpIMul %int %154 %int_2
        %156 = OpIMul %int %155 %int_2
        %157 = OpIMul %int %156 %int_2
        %158 = OpIMul %int %157 %int_2
        %159 = OpIMul %int %158 %int_2
               OpStore %hugeNI %159
        %162 = OpIMul %int %int_n32768 %int_2
        %163 = OpIMul %int %162 %int_2
        %164 = OpIMul %int %163 %int_2
        %165 = OpIMul %int %164 %int_2
        %166 = OpIMul %int %165 %int_2
        %167 = OpIMul %int %166 %int_2
        %168 = OpIMul %int %167 %int_2
        %169 = OpIMul %int %168 %int_2
        %170 = OpIMul %int %169 %int_2
        %171 = OpIMul %int %170 %int_2
        %172 = OpIMul %int %171 %int_2
        %173 = OpIMul %int %172 %int_2
        %174 = OpIMul %int %173 %int_2
        %175 = OpIMul %int %174 %int_2
        %176 = OpIMul %int %175 %int_2
        %177 = OpIMul %int %176 %int_2
               OpStore %hugeNS %177
        %183 = OpIMul %v4int %181 %182
        %184 = OpIMul %v4int %183 %182
        %185 = OpIMul %v4int %184 %182
        %186 = OpIMul %v4int %185 %182
        %187 = OpIMul %v4int %186 %182
        %188 = OpIMul %v4int %187 %182
        %189 = OpIMul %v4int %188 %182
        %190 = OpIMul %v4int %189 %182
        %191 = OpIMul %v4int %190 %182
        %192 = OpIMul %v4int %191 %182
        %193 = OpIMul %v4int %192 %182
        %194 = OpIMul %v4int %193 %182
        %195 = OpIMul %v4int %194 %182
        %196 = OpIMul %v4int %195 %182
        %197 = OpIMul %v4int %196 %182
               OpStore %hugeIvec %197
        %203 = OpIMul %v4uint %201 %202
        %204 = OpIMul %v4uint %203 %202
        %205 = OpIMul %v4uint %204 %202
        %206 = OpIMul %v4uint %205 %202
        %207 = OpIMul %v4uint %206 %202
        %208 = OpIMul %v4uint %207 %202
        %209 = OpIMul %v4uint %208 %202
        %210 = OpIMul %v4uint %209 %202
        %211 = OpIMul %v4uint %210 %202
        %212 = OpIMul %v4uint %211 %202
        %213 = OpIMul %v4uint %212 %202
        %214 = OpIMul %v4uint %213 %202
        %215 = OpIMul %v4uint %214 %202
        %216 = OpIMul %v4uint %215 %202
               OpStore %hugeUvec %216
        %223 = OpMatrixTimesMatrix %mat4v4float %222 %222
               OpStore %hugeMxM %223
        %226 = OpMatrixTimesVector %v4float %222 %221
               OpStore %hugeMxV %226
        %228 = OpVectorTimesMatrix %v4float %221 %222
               OpStore %hugeVxM %228
        %229 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %232 = OpLoad %v4float %229
        %233 = OpExtInst %float %1 FClamp %40 %float_0 %float_1
        %235 = OpVectorTimesScalar %v4float %232 %233
        %236 = OpExtInst %float %1 FClamp %52 %float_0 %float_1
        %237 = OpVectorTimesScalar %v4float %235 %236
        %239 = OpConvertSToF %float %77
        %238 = OpExtInst %float %1 FClamp %239 %float_0 %float_1
        %240 = OpVectorTimesScalar %v4float %237 %238
        %242 = OpConvertUToF %float %101
        %241 = OpExtInst %float %1 FClamp %242 %float_0 %float_1
        %243 = OpVectorTimesScalar %v4float %240 %241
        %245 = OpConvertSToF %float %120
        %244 = OpExtInst %float %1 FClamp %245 %float_0 %float_1
        %246 = OpVectorTimesScalar %v4float %243 %244
        %248 = OpConvertUToF %float %138
        %247 = OpExtInst %float %1 FClamp %248 %float_0 %float_1
        %249 = OpVectorTimesScalar %v4float %246 %247
        %251 = OpConvertSToF %float %159
        %250 = OpExtInst %float %1 FClamp %251 %float_0 %float_1
        %252 = OpVectorTimesScalar %v4float %249 %250
        %254 = OpConvertSToF %float %177
        %253 = OpExtInst %float %1 FClamp %254 %float_0 %float_1
        %255 = OpVectorTimesScalar %v4float %252 %253
        %257 = OpCompositeExtract %int %197 0
        %258 = OpConvertSToF %float %257
        %259 = OpCompositeExtract %int %197 1
        %260 = OpConvertSToF %float %259
        %261 = OpCompositeExtract %int %197 2
        %262 = OpConvertSToF %float %261
        %263 = OpCompositeExtract %int %197 3
        %264 = OpConvertSToF %float %263
        %265 = OpCompositeConstruct %v4float %258 %260 %262 %264
        %256 = OpExtInst %v4float %1 FClamp %265 %266 %267
        %268 = OpFMul %v4float %255 %256
        %270 = OpCompositeExtract %uint %216 0
        %271 = OpConvertUToF %float %270
        %272 = OpCompositeExtract %uint %216 1
        %273 = OpConvertUToF %float %272
        %274 = OpCompositeExtract %uint %216 2
        %275 = OpConvertUToF %float %274
        %276 = OpCompositeExtract %uint %216 3
        %277 = OpConvertUToF %float %276
        %278 = OpCompositeConstruct %v4float %271 %273 %275 %277
        %269 = OpExtInst %v4float %1 FClamp %278 %266 %267
        %279 = OpFMul %v4float %268 %269
        %281 = OpAccessChain %_ptr_Function_v4float %hugeMxM %int_0
        %282 = OpLoad %v4float %281
        %280 = OpExtInst %v4float %1 FClamp %282 %266 %267
        %283 = OpFMul %v4float %279 %280
        %284 = OpExtInst %v4float %1 FClamp %226 %266 %267
        %285 = OpFMul %v4float %283 %284
        %286 = OpExtInst %v4float %1 FClamp %228 %266 %267
        %287 = OpFMul %v4float %285 %286
               OpReturnValue %287
               OpFunctionEnd
