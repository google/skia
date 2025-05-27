               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %hugeH RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %hugeS RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
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
               OpDecorate %hugeUS RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
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
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %hugeNS RelaxedPrecision
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
               OpDecorate %229 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
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
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %284 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
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
        %178 = OpConstantComposite %v4int %int_1073741824 %int_1073741824 %int_1073741824 %int_1073741824
        %179 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
        %198 = OpConstantComposite %v4uint %uint_2147483648 %uint_2147483648 %uint_2147483648 %uint_2147483648
        %199 = OpConstantComposite %v4uint %uint_2 %uint_2 %uint_2 %uint_2
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_1_00000002e_20 = OpConstant %float 1.00000002e+20
        %218 = OpConstantComposite %v4float %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20
        %219 = OpConstantComposite %mat4v4float %218 %218 %218 %218
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
        %263 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %264 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
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
         %27 = OpFMul %float %float_9_99999962e_35 %float_1e_09
         %28 = OpFMul %float %27 %float_1e_09
         %29 = OpFMul %float %28 %float_1e_09
         %30 = OpFMul %float %29 %float_1e_09
         %31 = OpFMul %float %30 %float_1e_09
         %32 = OpFMul %float %31 %float_1e_09
         %33 = OpFMul %float %32 %float_1e_09
         %34 = OpFMul %float %33 %float_1e_09
         %35 = OpFMul %float %34 %float_1e_09
         %36 = OpFMul %float %35 %float_1e_09
         %37 = OpFMul %float %36 %float_1e_09
               OpStore %hugeH %37
         %39 = OpFMul %float %float_9_99999962e_35 %float_1e_09
         %40 = OpFMul %float %39 %float_1e_09
         %41 = OpFMul %float %40 %float_1e_09
         %42 = OpFMul %float %41 %float_1e_09
         %43 = OpFMul %float %42 %float_1e_09
         %44 = OpFMul %float %43 %float_1e_09
         %45 = OpFMul %float %44 %float_1e_09
         %46 = OpFMul %float %45 %float_1e_09
         %47 = OpFMul %float %46 %float_1e_09
         %48 = OpFMul %float %47 %float_1e_09
         %49 = OpFMul %float %48 %float_1e_09
               OpStore %hugeF %49
         %55 = OpIMul %int %int_1073741824 %int_2
         %56 = OpIMul %int %55 %int_2
         %57 = OpIMul %int %56 %int_2
         %58 = OpIMul %int %57 %int_2
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
               OpStore %hugeI %74
         %80 = OpIMul %uint %uint_2147483648 %uint_2
         %81 = OpIMul %uint %80 %uint_2
         %82 = OpIMul %uint %81 %uint_2
         %83 = OpIMul %uint %82 %uint_2
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
               OpStore %hugeU %98
        %101 = OpIMul %int %int_16384 %int_2
        %102 = OpIMul %int %101 %int_2
        %103 = OpIMul %int %102 %int_2
        %104 = OpIMul %int %103 %int_2
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
               OpStore %hugeS %117
        %120 = OpIMul %uint %uint_32768 %uint_2
        %121 = OpIMul %uint %120 %uint_2
        %122 = OpIMul %uint %121 %uint_2
        %123 = OpIMul %uint %122 %uint_2
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
               OpStore %hugeUS %135
        %138 = OpIMul %int %int_n2147483648 %int_2
        %139 = OpIMul %int %138 %int_2
        %140 = OpIMul %int %139 %int_2
        %141 = OpIMul %int %140 %int_2
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
               OpStore %hugeNI %156
        %159 = OpIMul %int %int_n32768 %int_2
        %160 = OpIMul %int %159 %int_2
        %161 = OpIMul %int %160 %int_2
        %162 = OpIMul %int %161 %int_2
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
               OpStore %hugeNS %174
        %180 = OpIMul %v4int %178 %179
        %181 = OpIMul %v4int %180 %179
        %182 = OpIMul %v4int %181 %179
        %183 = OpIMul %v4int %182 %179
        %184 = OpIMul %v4int %183 %179
        %185 = OpIMul %v4int %184 %179
        %186 = OpIMul %v4int %185 %179
        %187 = OpIMul %v4int %186 %179
        %188 = OpIMul %v4int %187 %179
        %189 = OpIMul %v4int %188 %179
        %190 = OpIMul %v4int %189 %179
        %191 = OpIMul %v4int %190 %179
        %192 = OpIMul %v4int %191 %179
        %193 = OpIMul %v4int %192 %179
        %194 = OpIMul %v4int %193 %179
               OpStore %hugeIvec %194
        %200 = OpIMul %v4uint %198 %199
        %201 = OpIMul %v4uint %200 %199
        %202 = OpIMul %v4uint %201 %199
        %203 = OpIMul %v4uint %202 %199
        %204 = OpIMul %v4uint %203 %199
        %205 = OpIMul %v4uint %204 %199
        %206 = OpIMul %v4uint %205 %199
        %207 = OpIMul %v4uint %206 %199
        %208 = OpIMul %v4uint %207 %199
        %209 = OpIMul %v4uint %208 %199
        %210 = OpIMul %v4uint %209 %199
        %211 = OpIMul %v4uint %210 %199
        %212 = OpIMul %v4uint %211 %199
        %213 = OpIMul %v4uint %212 %199
               OpStore %hugeUvec %213
        %220 = OpMatrixTimesMatrix %mat4v4float %219 %219
               OpStore %hugeMxM %220
        %223 = OpMatrixTimesVector %v4float %219 %218
               OpStore %hugeMxV %223
        %225 = OpVectorTimesMatrix %v4float %218 %219
               OpStore %hugeVxM %225
        %226 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %229 = OpLoad %v4float %226
        %230 = OpExtInst %float %1 FClamp %37 %float_0 %float_1
        %232 = OpVectorTimesScalar %v4float %229 %230
        %233 = OpExtInst %float %1 FClamp %49 %float_0 %float_1
        %234 = OpVectorTimesScalar %v4float %232 %233
        %236 = OpConvertSToF %float %74
        %235 = OpExtInst %float %1 FClamp %236 %float_0 %float_1
        %237 = OpVectorTimesScalar %v4float %234 %235
        %239 = OpConvertUToF %float %98
        %238 = OpExtInst %float %1 FClamp %239 %float_0 %float_1
        %240 = OpVectorTimesScalar %v4float %237 %238
        %242 = OpConvertSToF %float %117
        %241 = OpExtInst %float %1 FClamp %242 %float_0 %float_1
        %243 = OpVectorTimesScalar %v4float %240 %241
        %245 = OpConvertUToF %float %135
        %244 = OpExtInst %float %1 FClamp %245 %float_0 %float_1
        %246 = OpVectorTimesScalar %v4float %243 %244
        %248 = OpConvertSToF %float %156
        %247 = OpExtInst %float %1 FClamp %248 %float_0 %float_1
        %249 = OpVectorTimesScalar %v4float %246 %247
        %251 = OpConvertSToF %float %174
        %250 = OpExtInst %float %1 FClamp %251 %float_0 %float_1
        %252 = OpVectorTimesScalar %v4float %249 %250
        %254 = OpCompositeExtract %int %194 0
        %255 = OpConvertSToF %float %254
        %256 = OpCompositeExtract %int %194 1
        %257 = OpConvertSToF %float %256
        %258 = OpCompositeExtract %int %194 2
        %259 = OpConvertSToF %float %258
        %260 = OpCompositeExtract %int %194 3
        %261 = OpConvertSToF %float %260
        %262 = OpCompositeConstruct %v4float %255 %257 %259 %261
        %253 = OpExtInst %v4float %1 FClamp %262 %263 %264
        %265 = OpFMul %v4float %252 %253
        %267 = OpCompositeExtract %uint %213 0
        %268 = OpConvertUToF %float %267
        %269 = OpCompositeExtract %uint %213 1
        %270 = OpConvertUToF %float %269
        %271 = OpCompositeExtract %uint %213 2
        %272 = OpConvertUToF %float %271
        %273 = OpCompositeExtract %uint %213 3
        %274 = OpConvertUToF %float %273
        %275 = OpCompositeConstruct %v4float %268 %270 %272 %274
        %266 = OpExtInst %v4float %1 FClamp %275 %263 %264
        %276 = OpFMul %v4float %265 %266
        %278 = OpAccessChain %_ptr_Function_v4float %hugeMxM %int_0
        %279 = OpLoad %v4float %278
        %277 = OpExtInst %v4float %1 FClamp %279 %263 %264
        %280 = OpFMul %v4float %276 %277
        %281 = OpExtInst %v4float %1 FClamp %223 %263 %264
        %282 = OpFMul %v4float %280 %281
        %283 = OpExtInst %v4float %1 FClamp %225 %263 %264
        %284 = OpFMul %v4float %282 %283
               OpReturnValue %284
               OpFunctionEnd
