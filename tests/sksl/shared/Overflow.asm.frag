               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %hugeH "hugeH"                    ; id %27
               OpName %hugeF "hugeF"                    ; id %44
               OpName %hugeI "hugeI"                    ; id %58
               OpName %hugeU "hugeU"                    ; id %82
               OpName %hugeS "hugeS"                    ; id %106
               OpName %hugeUS "hugeUS"                  ; id %125
               OpName %hugeNI "hugeNI"                  ; id %143
               OpName %hugeNS "hugeNS"                  ; id %164
               OpName %hugeIvec "hugeIvec"              ; id %182
               OpName %hugeUvec "hugeUvec"              ; id %202
               OpName %hugeMxM "hugeMxM"                ; id %221
               OpName %hugeMxV "hugeMxV"                ; id %228
               OpName %hugeVxM "hugeVxM"                ; id %231

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
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
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %hugeS RelaxedPrecision
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
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %hugeUS RelaxedPrecision
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
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %hugeNS RelaxedPrecision
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
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %264 RelaxedPrecision
               OpDecorate %266 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %269 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %281 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %284 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_65503_8984 = OpConstant %float 65503.8984
%float_9_99999962e_35 = OpConstant %float 9.99999962e+35
%float_1e_09 = OpConstant %float 1e+09
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
        %185 = OpConstantComposite %v4int %int_1073741824 %int_1073741824 %int_1073741824 %int_1073741824
        %186 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
        %205 = OpConstantComposite %v4uint %uint_2147483648 %uint_2147483648 %uint_2147483648 %uint_2147483648
        %206 = OpConstantComposite %v4uint %uint_2 %uint_2 %uint_2 %uint_2
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_1_00000002e_20 = OpConstant %float 1.00000002e+20
        %225 = OpConstantComposite %v4float %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20
        %226 = OpConstantComposite %mat4v4float %225 %225 %225 %225
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
        %270 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %271 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
      %hugeH =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
      %hugeF =   OpVariable %_ptr_Function_float Function
      %hugeI =   OpVariable %_ptr_Function_int Function
      %hugeU =   OpVariable %_ptr_Function_uint Function
      %hugeS =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
     %hugeUS =   OpVariable %_ptr_Function_uint Function    ; RelaxedPrecision
     %hugeNI =   OpVariable %_ptr_Function_int Function
     %hugeNS =   OpVariable %_ptr_Function_int Function     ; RelaxedPrecision
   %hugeIvec =   OpVariable %_ptr_Function_v4int Function
   %hugeUvec =   OpVariable %_ptr_Function_v4uint Function
    %hugeMxM =   OpVariable %_ptr_Function_mat4v4float Function
    %hugeMxV =   OpVariable %_ptr_Function_v4float Function
    %hugeVxM =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpFMul %float %float_65503_8984 %float_65503_8984  ; RelaxedPrecision
         %31 =   OpFMul %float %30 %float_65503_8984                ; RelaxedPrecision
         %32 =   OpFMul %float %31 %float_65503_8984                ; RelaxedPrecision
         %33 =   OpFMul %float %32 %float_65503_8984                ; RelaxedPrecision
         %34 =   OpFMul %float %33 %float_65503_8984                ; RelaxedPrecision
         %35 =   OpFMul %float %34 %float_65503_8984                ; RelaxedPrecision
         %36 =   OpFMul %float %35 %float_65503_8984                ; RelaxedPrecision
         %37 =   OpFMul %float %36 %float_65503_8984                ; RelaxedPrecision
         %38 =   OpFMul %float %37 %float_65503_8984                ; RelaxedPrecision
         %39 =   OpFMul %float %38 %float_65503_8984                ; RelaxedPrecision
         %40 =   OpFMul %float %39 %float_65503_8984                ; RelaxedPrecision
         %41 =   OpFMul %float %40 %float_65503_8984                ; RelaxedPrecision
         %42 =   OpFMul %float %41 %float_65503_8984                ; RelaxedPrecision
         %43 =   OpFMul %float %42 %float_65503_8984                ; RelaxedPrecision
                 OpStore %hugeH %43
         %47 =   OpFMul %float %float_9_99999962e_35 %float_1e_09
         %48 =   OpFMul %float %47 %float_1e_09
         %49 =   OpFMul %float %48 %float_1e_09
         %50 =   OpFMul %float %49 %float_1e_09
         %51 =   OpFMul %float %50 %float_1e_09
         %52 =   OpFMul %float %51 %float_1e_09
         %53 =   OpFMul %float %52 %float_1e_09
         %54 =   OpFMul %float %53 %float_1e_09
         %55 =   OpFMul %float %54 %float_1e_09
         %56 =   OpFMul %float %55 %float_1e_09
         %57 =   OpFMul %float %56 %float_1e_09
                 OpStore %hugeF %57
         %62 =   OpIMul %int %int_1073741824 %int_2
         %63 =   OpIMul %int %62 %int_2
         %64 =   OpIMul %int %63 %int_2
         %65 =   OpIMul %int %64 %int_2
         %66 =   OpIMul %int %65 %int_2
         %67 =   OpIMul %int %66 %int_2
         %68 =   OpIMul %int %67 %int_2
         %69 =   OpIMul %int %68 %int_2
         %70 =   OpIMul %int %69 %int_2
         %71 =   OpIMul %int %70 %int_2
         %72 =   OpIMul %int %71 %int_2
         %73 =   OpIMul %int %72 %int_2
         %74 =   OpIMul %int %73 %int_2
         %75 =   OpIMul %int %74 %int_2
         %76 =   OpIMul %int %75 %int_2
         %77 =   OpIMul %int %76 %int_2
         %78 =   OpIMul %int %77 %int_2
         %79 =   OpIMul %int %78 %int_2
         %80 =   OpIMul %int %79 %int_2
         %81 =   OpIMul %int %80 %int_2
                 OpStore %hugeI %81
         %87 =   OpIMul %uint %uint_2147483648 %uint_2
         %88 =   OpIMul %uint %87 %uint_2
         %89 =   OpIMul %uint %88 %uint_2
         %90 =   OpIMul %uint %89 %uint_2
         %91 =   OpIMul %uint %90 %uint_2
         %92 =   OpIMul %uint %91 %uint_2
         %93 =   OpIMul %uint %92 %uint_2
         %94 =   OpIMul %uint %93 %uint_2
         %95 =   OpIMul %uint %94 %uint_2
         %96 =   OpIMul %uint %95 %uint_2
         %97 =   OpIMul %uint %96 %uint_2
         %98 =   OpIMul %uint %97 %uint_2
         %99 =   OpIMul %uint %98 %uint_2
        %100 =   OpIMul %uint %99 %uint_2
        %101 =   OpIMul %uint %100 %uint_2
        %102 =   OpIMul %uint %101 %uint_2
        %103 =   OpIMul %uint %102 %uint_2
        %104 =   OpIMul %uint %103 %uint_2
        %105 =   OpIMul %uint %104 %uint_2
                 OpStore %hugeU %105
        %108 =   OpIMul %int %int_16384 %int_2      ; RelaxedPrecision
        %109 =   OpIMul %int %108 %int_2            ; RelaxedPrecision
        %110 =   OpIMul %int %109 %int_2            ; RelaxedPrecision
        %111 =   OpIMul %int %110 %int_2            ; RelaxedPrecision
        %112 =   OpIMul %int %111 %int_2            ; RelaxedPrecision
        %113 =   OpIMul %int %112 %int_2            ; RelaxedPrecision
        %114 =   OpIMul %int %113 %int_2            ; RelaxedPrecision
        %115 =   OpIMul %int %114 %int_2            ; RelaxedPrecision
        %116 =   OpIMul %int %115 %int_2            ; RelaxedPrecision
        %117 =   OpIMul %int %116 %int_2            ; RelaxedPrecision
        %118 =   OpIMul %int %117 %int_2            ; RelaxedPrecision
        %119 =   OpIMul %int %118 %int_2            ; RelaxedPrecision
        %120 =   OpIMul %int %119 %int_2            ; RelaxedPrecision
        %121 =   OpIMul %int %120 %int_2            ; RelaxedPrecision
        %122 =   OpIMul %int %121 %int_2            ; RelaxedPrecision
        %123 =   OpIMul %int %122 %int_2            ; RelaxedPrecision
        %124 =   OpIMul %int %123 %int_2            ; RelaxedPrecision
                 OpStore %hugeS %124
        %127 =   OpIMul %uint %uint_32768 %uint_2   ; RelaxedPrecision
        %128 =   OpIMul %uint %127 %uint_2          ; RelaxedPrecision
        %129 =   OpIMul %uint %128 %uint_2          ; RelaxedPrecision
        %130 =   OpIMul %uint %129 %uint_2          ; RelaxedPrecision
        %131 =   OpIMul %uint %130 %uint_2          ; RelaxedPrecision
        %132 =   OpIMul %uint %131 %uint_2          ; RelaxedPrecision
        %133 =   OpIMul %uint %132 %uint_2          ; RelaxedPrecision
        %134 =   OpIMul %uint %133 %uint_2          ; RelaxedPrecision
        %135 =   OpIMul %uint %134 %uint_2          ; RelaxedPrecision
        %136 =   OpIMul %uint %135 %uint_2          ; RelaxedPrecision
        %137 =   OpIMul %uint %136 %uint_2          ; RelaxedPrecision
        %138 =   OpIMul %uint %137 %uint_2          ; RelaxedPrecision
        %139 =   OpIMul %uint %138 %uint_2          ; RelaxedPrecision
        %140 =   OpIMul %uint %139 %uint_2          ; RelaxedPrecision
        %141 =   OpIMul %uint %140 %uint_2          ; RelaxedPrecision
        %142 =   OpIMul %uint %141 %uint_2          ; RelaxedPrecision
                 OpStore %hugeUS %142
        %145 =   OpIMul %int %int_n2147483648 %int_2
        %146 =   OpIMul %int %145 %int_2
        %147 =   OpIMul %int %146 %int_2
        %148 =   OpIMul %int %147 %int_2
        %149 =   OpIMul %int %148 %int_2
        %150 =   OpIMul %int %149 %int_2
        %151 =   OpIMul %int %150 %int_2
        %152 =   OpIMul %int %151 %int_2
        %153 =   OpIMul %int %152 %int_2
        %154 =   OpIMul %int %153 %int_2
        %155 =   OpIMul %int %154 %int_2
        %156 =   OpIMul %int %155 %int_2
        %157 =   OpIMul %int %156 %int_2
        %158 =   OpIMul %int %157 %int_2
        %159 =   OpIMul %int %158 %int_2
        %160 =   OpIMul %int %159 %int_2
        %161 =   OpIMul %int %160 %int_2
        %162 =   OpIMul %int %161 %int_2
        %163 =   OpIMul %int %162 %int_2
                 OpStore %hugeNI %163
        %166 =   OpIMul %int %int_n32768 %int_2     ; RelaxedPrecision
        %167 =   OpIMul %int %166 %int_2            ; RelaxedPrecision
        %168 =   OpIMul %int %167 %int_2            ; RelaxedPrecision
        %169 =   OpIMul %int %168 %int_2            ; RelaxedPrecision
        %170 =   OpIMul %int %169 %int_2            ; RelaxedPrecision
        %171 =   OpIMul %int %170 %int_2            ; RelaxedPrecision
        %172 =   OpIMul %int %171 %int_2            ; RelaxedPrecision
        %173 =   OpIMul %int %172 %int_2            ; RelaxedPrecision
        %174 =   OpIMul %int %173 %int_2            ; RelaxedPrecision
        %175 =   OpIMul %int %174 %int_2            ; RelaxedPrecision
        %176 =   OpIMul %int %175 %int_2            ; RelaxedPrecision
        %177 =   OpIMul %int %176 %int_2            ; RelaxedPrecision
        %178 =   OpIMul %int %177 %int_2            ; RelaxedPrecision
        %179 =   OpIMul %int %178 %int_2            ; RelaxedPrecision
        %180 =   OpIMul %int %179 %int_2            ; RelaxedPrecision
        %181 =   OpIMul %int %180 %int_2            ; RelaxedPrecision
                 OpStore %hugeNS %181
        %187 =   OpIMul %v4int %185 %186
        %188 =   OpIMul %v4int %187 %186
        %189 =   OpIMul %v4int %188 %186
        %190 =   OpIMul %v4int %189 %186
        %191 =   OpIMul %v4int %190 %186
        %192 =   OpIMul %v4int %191 %186
        %193 =   OpIMul %v4int %192 %186
        %194 =   OpIMul %v4int %193 %186
        %195 =   OpIMul %v4int %194 %186
        %196 =   OpIMul %v4int %195 %186
        %197 =   OpIMul %v4int %196 %186
        %198 =   OpIMul %v4int %197 %186
        %199 =   OpIMul %v4int %198 %186
        %200 =   OpIMul %v4int %199 %186
        %201 =   OpIMul %v4int %200 %186
                 OpStore %hugeIvec %201
        %207 =   OpIMul %v4uint %205 %206
        %208 =   OpIMul %v4uint %207 %206
        %209 =   OpIMul %v4uint %208 %206
        %210 =   OpIMul %v4uint %209 %206
        %211 =   OpIMul %v4uint %210 %206
        %212 =   OpIMul %v4uint %211 %206
        %213 =   OpIMul %v4uint %212 %206
        %214 =   OpIMul %v4uint %213 %206
        %215 =   OpIMul %v4uint %214 %206
        %216 =   OpIMul %v4uint %215 %206
        %217 =   OpIMul %v4uint %216 %206
        %218 =   OpIMul %v4uint %217 %206
        %219 =   OpIMul %v4uint %218 %206
        %220 =   OpIMul %v4uint %219 %206
                 OpStore %hugeUvec %220
        %227 =   OpMatrixTimesMatrix %mat4v4float %226 %226
                 OpStore %hugeMxM %227
        %230 =   OpMatrixTimesVector %v4float %226 %225
                 OpStore %hugeMxV %230
        %232 =   OpVectorTimesMatrix %v4float %225 %226
                 OpStore %hugeVxM %232
        %233 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %236 =   OpLoad %v4float %233               ; RelaxedPrecision
        %237 =   OpExtInst %float %5 FClamp %43 %float_0 %float_1   ; RelaxedPrecision
        %239 =   OpVectorTimesScalar %v4float %236 %237             ; RelaxedPrecision
        %240 =   OpExtInst %float %5 FClamp %57 %float_0 %float_1   ; RelaxedPrecision
        %241 =   OpVectorTimesScalar %v4float %239 %240             ; RelaxedPrecision
        %243 =   OpConvertSToF %float %81                           ; RelaxedPrecision
        %242 =   OpExtInst %float %5 FClamp %243 %float_0 %float_1  ; RelaxedPrecision
        %244 =   OpVectorTimesScalar %v4float %241 %242             ; RelaxedPrecision
        %246 =   OpConvertUToF %float %105                          ; RelaxedPrecision
        %245 =   OpExtInst %float %5 FClamp %246 %float_0 %float_1  ; RelaxedPrecision
        %247 =   OpVectorTimesScalar %v4float %244 %245             ; RelaxedPrecision
        %249 =   OpConvertSToF %float %124                          ; RelaxedPrecision
        %248 =   OpExtInst %float %5 FClamp %249 %float_0 %float_1  ; RelaxedPrecision
        %250 =   OpVectorTimesScalar %v4float %247 %248             ; RelaxedPrecision
        %252 =   OpConvertUToF %float %142                          ; RelaxedPrecision
        %251 =   OpExtInst %float %5 FClamp %252 %float_0 %float_1  ; RelaxedPrecision
        %253 =   OpVectorTimesScalar %v4float %250 %251             ; RelaxedPrecision
        %255 =   OpConvertSToF %float %163                          ; RelaxedPrecision
        %254 =   OpExtInst %float %5 FClamp %255 %float_0 %float_1  ; RelaxedPrecision
        %256 =   OpVectorTimesScalar %v4float %253 %254             ; RelaxedPrecision
        %258 =   OpConvertSToF %float %181                          ; RelaxedPrecision
        %257 =   OpExtInst %float %5 FClamp %258 %float_0 %float_1  ; RelaxedPrecision
        %259 =   OpVectorTimesScalar %v4float %256 %257             ; RelaxedPrecision
        %261 =   OpCompositeExtract %int %201 0
        %262 =   OpConvertSToF %float %261          ; RelaxedPrecision
        %263 =   OpCompositeExtract %int %201 1
        %264 =   OpConvertSToF %float %263          ; RelaxedPrecision
        %265 =   OpCompositeExtract %int %201 2
        %266 =   OpConvertSToF %float %265          ; RelaxedPrecision
        %267 =   OpCompositeExtract %int %201 3
        %268 =   OpConvertSToF %float %267          ; RelaxedPrecision
        %269 =   OpCompositeConstruct %v4float %262 %264 %266 %268  ; RelaxedPrecision
        %260 =   OpExtInst %v4float %5 FClamp %269 %270 %271        ; RelaxedPrecision
        %272 =   OpFMul %v4float %259 %260                          ; RelaxedPrecision
        %274 =   OpCompositeExtract %uint %220 0
        %275 =   OpConvertUToF %float %274          ; RelaxedPrecision
        %276 =   OpCompositeExtract %uint %220 1
        %277 =   OpConvertUToF %float %276          ; RelaxedPrecision
        %278 =   OpCompositeExtract %uint %220 2
        %279 =   OpConvertUToF %float %278          ; RelaxedPrecision
        %280 =   OpCompositeExtract %uint %220 3
        %281 =   OpConvertUToF %float %280          ; RelaxedPrecision
        %282 =   OpCompositeConstruct %v4float %275 %277 %279 %281  ; RelaxedPrecision
        %273 =   OpExtInst %v4float %5 FClamp %282 %270 %271        ; RelaxedPrecision
        %283 =   OpFMul %v4float %272 %273                          ; RelaxedPrecision
        %285 =   OpAccessChain %_ptr_Function_v4float %hugeMxM %int_0
        %286 =   OpLoad %v4float %285
        %284 =   OpExtInst %v4float %5 FClamp %286 %270 %271    ; RelaxedPrecision
        %287 =   OpFMul %v4float %283 %284                      ; RelaxedPrecision
        %288 =   OpExtInst %v4float %5 FClamp %230 %270 %271    ; RelaxedPrecision
        %289 =   OpFMul %v4float %287 %288                      ; RelaxedPrecision
        %290 =   OpExtInst %v4float %5 FClamp %232 %270 %271    ; RelaxedPrecision
        %291 =   OpFMul %v4float %289 %290                      ; RelaxedPrecision
                 OpReturnValue %291
               OpFunctionEnd
