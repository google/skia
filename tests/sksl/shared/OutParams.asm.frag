               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %out_half_vh "out_half_vh"
               OpName %out_half2_vh2 "out_half2_vh2"
               OpName %out_half3_vh3 "out_half3_vh3"
               OpName %out_half4_vh4 "out_half4_vh4"
               OpName %out_half2x2_vh22 "out_half2x2_vh22"
               OpName %out_half3x3_vh33 "out_half3x3_vh33"
               OpName %out_half4x4_vh44 "out_half4x4_vh44"
               OpName %out_int_vi "out_int_vi"
               OpName %out_int2_vi2 "out_int2_vi2"
               OpName %out_int3_vi3 "out_int3_vi3"
               OpName %out_int4_vi4 "out_int4_vi4"
               OpName %out_float_vf "out_float_vf"
               OpName %out_float2_vf2 "out_float2_vf2"
               OpName %out_float3_vf3 "out_float3_vf3"
               OpName %out_float4_vf4 "out_float4_vf4"
               OpName %out_float2x2_vf22 "out_float2x2_vf22"
               OpName %out_float3x3_vf33 "out_float3x3_vf33"
               OpName %out_float4x4_vf44 "out_float4x4_vf44"
               OpName %out_bool_vb "out_bool_vb"
               OpName %out_bool2_vb2 "out_bool2_vb2"
               OpName %out_bool3_vb3 "out_bool3_vb3"
               OpName %out_bool4_vb4 "out_bool4_vb4"
               OpName %main "main"
               OpName %h "h"
               OpName %h2 "h2"
               OpName %h3 "h3"
               OpName %h4 "h4"
               OpName %h2x2 "h2x2"
               OpName %h3x3 "h3x3"
               OpName %h4x4 "h4x4"
               OpName %i "i"
               OpName %i2 "i2"
               OpName %i3 "i3"
               OpName %i4 "i4"
               OpName %f "f"
               OpName %f2 "f2"
               OpName %f3 "f3"
               OpName %f4 "f4"
               OpName %f2x2 "f2x2"
               OpName %f3x3 "f3x3"
               OpName %f4x4 "f4x4"
               OpName %b "b"
               OpName %b2 "b2"
               OpName %b3 "b3"
               OpName %b4 "b4"
               OpName %ok "ok"
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
               OpDecorate %29 Binding 0
               OpDecorate %29 DescriptorSet 0
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %h RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %h2 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %h3 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %h4 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %265 RelaxedPrecision
               OpDecorate %266 RelaxedPrecision
               OpDecorate %267 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %272 RelaxedPrecision
               OpDecorate %h2x2 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision
               OpDecorate %h3x3 RelaxedPrecision
               OpDecorate %278 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %h4x4 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %284 RelaxedPrecision
               OpDecorate %286 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %292 RelaxedPrecision
               OpDecorate %294 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %399 RelaxedPrecision
               OpDecorate %400 RelaxedPrecision
               OpDecorate %401 RelaxedPrecision
               OpDecorate %402 RelaxedPrecision
               OpDecorate %403 RelaxedPrecision
               OpDecorate %404 RelaxedPrecision
               OpDecorate %405 RelaxedPrecision
               OpDecorate %406 RelaxedPrecision
               OpDecorate %407 RelaxedPrecision
               OpDecorate %408 RelaxedPrecision
               OpDecorate %410 RelaxedPrecision
               OpDecorate %411 RelaxedPrecision
               OpDecorate %412 RelaxedPrecision
               OpDecorate %414 RelaxedPrecision
               OpDecorate %415 RelaxedPrecision
               OpDecorate %416 RelaxedPrecision
               OpDecorate %418 RelaxedPrecision
               OpDecorate %419 RelaxedPrecision
               OpDecorate %420 RelaxedPrecision
               OpDecorate %465 RelaxedPrecision
               OpDecorate %468 RelaxedPrecision
               OpDecorate %473 RelaxedPrecision
               OpDecorate %478 RelaxedPrecision
               OpDecorate %487 RelaxedPrecision
               OpDecorate %489 RelaxedPrecision
               OpDecorate %490 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %29 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %34 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %38 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %43 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
         %52 = OpTypeFunction %void %_ptr_Function_v2float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %61 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %69 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
         %78 = OpTypeFunction %void %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
         %89 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
        %101 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
        %113 = OpTypeFunction %void %_ptr_Function_int
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
        %122 = OpTypeFunction %void %_ptr_Function_v2int
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
        %132 = OpTypeFunction %void %_ptr_Function_v3int
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
        %142 = OpTypeFunction %void %_ptr_Function_v4int
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
        %202 = OpTypeFunction %void %_ptr_Function_bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
        %211 = OpTypeFunction %void %_ptr_Function_v2bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
        %221 = OpTypeFunction %void %_ptr_Function_v3bool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
        %231 = OpTypeFunction %void %_ptr_Function_v4bool
        %239 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_0 = OpConstant %int 0
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
    %float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %34
         %35 = OpLabel
         %39 = OpVariable %_ptr_Function_v2float Function
               OpStore %39 %38
         %41 = OpFunctionCall %v4float %main %39
               OpStore %sk_FragColor %41
               OpReturn
               OpFunctionEnd
%out_half_vh = OpFunction %void None %43
         %44 = OpFunctionParameter %_ptr_Function_float
         %45 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
         %50 = OpLoad %v4float %46
         %51 = OpCompositeExtract %float %50 0
               OpStore %44 %51
               OpReturn
               OpFunctionEnd
%out_half2_vh2 = OpFunction %void None %52
         %53 = OpFunctionParameter %_ptr_Function_v2float
         %54 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
         %56 = OpLoad %v4float %55
         %57 = OpCompositeExtract %float %56 1
         %58 = OpCompositeConstruct %v2float %57 %57
               OpStore %53 %58
               OpReturn
               OpFunctionEnd
%out_half3_vh3 = OpFunction %void None %61
         %62 = OpFunctionParameter %_ptr_Function_v3float
         %63 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
         %65 = OpLoad %v4float %64
         %66 = OpCompositeExtract %float %65 2
         %67 = OpCompositeConstruct %v3float %66 %66 %66
               OpStore %62 %67
               OpReturn
               OpFunctionEnd
%out_half4_vh4 = OpFunction %void None %69
         %70 = OpFunctionParameter %_ptr_Function_v4float
         %71 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
         %73 = OpLoad %v4float %72
         %74 = OpCompositeExtract %float %73 3
         %75 = OpCompositeConstruct %v4float %74 %74 %74 %74
               OpStore %70 %75
               OpReturn
               OpFunctionEnd
%out_half2x2_vh22 = OpFunction %void None %78
         %79 = OpFunctionParameter %_ptr_Function_mat2v2float
         %80 = OpLabel
         %81 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
         %82 = OpLoad %v4float %81
         %83 = OpCompositeExtract %float %82 0
         %84 = OpCompositeConstruct %v2float %83 %float_0
         %85 = OpCompositeConstruct %v2float %float_0 %83
         %86 = OpCompositeConstruct %mat2v2float %84 %85
               OpStore %79 %86
               OpReturn
               OpFunctionEnd
%out_half3x3_vh33 = OpFunction %void None %89
         %90 = OpFunctionParameter %_ptr_Function_mat3v3float
         %91 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
         %93 = OpLoad %v4float %92
         %94 = OpCompositeExtract %float %93 1
         %95 = OpCompositeConstruct %v3float %94 %float_0 %float_0
         %96 = OpCompositeConstruct %v3float %float_0 %94 %float_0
         %97 = OpCompositeConstruct %v3float %float_0 %float_0 %94
         %98 = OpCompositeConstruct %mat3v3float %95 %96 %97
               OpStore %90 %98
               OpReturn
               OpFunctionEnd
%out_half4x4_vh44 = OpFunction %void None %101
        %102 = OpFunctionParameter %_ptr_Function_mat4v4float
        %103 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %105 = OpLoad %v4float %104
        %106 = OpCompositeExtract %float %105 2
        %107 = OpCompositeConstruct %v4float %106 %float_0 %float_0 %float_0
        %108 = OpCompositeConstruct %v4float %float_0 %106 %float_0 %float_0
        %109 = OpCompositeConstruct %v4float %float_0 %float_0 %106 %float_0
        %110 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %106
        %111 = OpCompositeConstruct %mat4v4float %107 %108 %109 %110
               OpStore %102 %111
               OpReturn
               OpFunctionEnd
 %out_int_vi = OpFunction %void None %113
        %114 = OpFunctionParameter %_ptr_Function_int
        %115 = OpLabel
        %116 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %117 = OpLoad %v4float %116
        %118 = OpCompositeExtract %float %117 0
        %119 = OpConvertFToS %int %118
               OpStore %114 %119
               OpReturn
               OpFunctionEnd
%out_int2_vi2 = OpFunction %void None %122
        %123 = OpFunctionParameter %_ptr_Function_v2int
        %124 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %126 = OpLoad %v4float %125
        %127 = OpCompositeExtract %float %126 1
        %128 = OpConvertFToS %int %127
        %129 = OpCompositeConstruct %v2int %128 %128
               OpStore %123 %129
               OpReturn
               OpFunctionEnd
%out_int3_vi3 = OpFunction %void None %132
        %133 = OpFunctionParameter %_ptr_Function_v3int
        %134 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %136 = OpLoad %v4float %135
        %137 = OpCompositeExtract %float %136 2
        %138 = OpConvertFToS %int %137
        %139 = OpCompositeConstruct %v3int %138 %138 %138
               OpStore %133 %139
               OpReturn
               OpFunctionEnd
%out_int4_vi4 = OpFunction %void None %142
        %143 = OpFunctionParameter %_ptr_Function_v4int
        %144 = OpLabel
        %145 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %146 = OpLoad %v4float %145
        %147 = OpCompositeExtract %float %146 3
        %148 = OpConvertFToS %int %147
        %149 = OpCompositeConstruct %v4int %148 %148 %148 %148
               OpStore %143 %149
               OpReturn
               OpFunctionEnd
%out_float_vf = OpFunction %void None %43
        %150 = OpFunctionParameter %_ptr_Function_float
        %151 = OpLabel
        %152 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %153 = OpLoad %v4float %152
        %154 = OpCompositeExtract %float %153 0
               OpStore %150 %154
               OpReturn
               OpFunctionEnd
%out_float2_vf2 = OpFunction %void None %52
        %155 = OpFunctionParameter %_ptr_Function_v2float
        %156 = OpLabel
        %157 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %158 = OpLoad %v4float %157
        %159 = OpCompositeExtract %float %158 1
        %160 = OpCompositeConstruct %v2float %159 %159
               OpStore %155 %160
               OpReturn
               OpFunctionEnd
%out_float3_vf3 = OpFunction %void None %61
        %161 = OpFunctionParameter %_ptr_Function_v3float
        %162 = OpLabel
        %163 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %164 = OpLoad %v4float %163
        %165 = OpCompositeExtract %float %164 2
        %166 = OpCompositeConstruct %v3float %165 %165 %165
               OpStore %161 %166
               OpReturn
               OpFunctionEnd
%out_float4_vf4 = OpFunction %void None %69
        %167 = OpFunctionParameter %_ptr_Function_v4float
        %168 = OpLabel
        %169 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %170 = OpLoad %v4float %169
        %171 = OpCompositeExtract %float %170 3
        %172 = OpCompositeConstruct %v4float %171 %171 %171 %171
               OpStore %167 %172
               OpReturn
               OpFunctionEnd
%out_float2x2_vf22 = OpFunction %void None %78
        %173 = OpFunctionParameter %_ptr_Function_mat2v2float
        %174 = OpLabel
        %175 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %176 = OpLoad %v4float %175
        %177 = OpCompositeExtract %float %176 0
        %178 = OpCompositeConstruct %v2float %177 %float_0
        %179 = OpCompositeConstruct %v2float %float_0 %177
        %180 = OpCompositeConstruct %mat2v2float %178 %179
               OpStore %173 %180
               OpReturn
               OpFunctionEnd
%out_float3x3_vf33 = OpFunction %void None %89
        %181 = OpFunctionParameter %_ptr_Function_mat3v3float
        %182 = OpLabel
        %183 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %184 = OpLoad %v4float %183
        %185 = OpCompositeExtract %float %184 1
        %186 = OpCompositeConstruct %v3float %185 %float_0 %float_0
        %187 = OpCompositeConstruct %v3float %float_0 %185 %float_0
        %188 = OpCompositeConstruct %v3float %float_0 %float_0 %185
        %189 = OpCompositeConstruct %mat3v3float %186 %187 %188
               OpStore %181 %189
               OpReturn
               OpFunctionEnd
%out_float4x4_vf44 = OpFunction %void None %101
        %190 = OpFunctionParameter %_ptr_Function_mat4v4float
        %191 = OpLabel
        %192 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %193 = OpLoad %v4float %192
        %194 = OpCompositeExtract %float %193 2
        %195 = OpCompositeConstruct %v4float %194 %float_0 %float_0 %float_0
        %196 = OpCompositeConstruct %v4float %float_0 %194 %float_0 %float_0
        %197 = OpCompositeConstruct %v4float %float_0 %float_0 %194 %float_0
        %198 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %194
        %199 = OpCompositeConstruct %mat4v4float %195 %196 %197 %198
               OpStore %190 %199
               OpReturn
               OpFunctionEnd
%out_bool_vb = OpFunction %void None %202
        %203 = OpFunctionParameter %_ptr_Function_bool
        %204 = OpLabel
        %205 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %206 = OpLoad %v4float %205
        %207 = OpCompositeExtract %float %206 0
        %208 = OpFUnordNotEqual %bool %207 %float_0
               OpStore %203 %208
               OpReturn
               OpFunctionEnd
%out_bool2_vb2 = OpFunction %void None %211
        %212 = OpFunctionParameter %_ptr_Function_v2bool
        %213 = OpLabel
        %214 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %215 = OpLoad %v4float %214
        %216 = OpCompositeExtract %float %215 1
        %217 = OpFUnordNotEqual %bool %216 %float_0
        %218 = OpCompositeConstruct %v2bool %217 %217
               OpStore %212 %218
               OpReturn
               OpFunctionEnd
%out_bool3_vb3 = OpFunction %void None %221
        %222 = OpFunctionParameter %_ptr_Function_v3bool
        %223 = OpLabel
        %224 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %225 = OpLoad %v4float %224
        %226 = OpCompositeExtract %float %225 2
        %227 = OpFUnordNotEqual %bool %226 %float_0
        %228 = OpCompositeConstruct %v3bool %227 %227 %227
               OpStore %222 %228
               OpReturn
               OpFunctionEnd
%out_bool4_vb4 = OpFunction %void None %231
        %232 = OpFunctionParameter %_ptr_Function_v4bool
        %233 = OpLabel
        %234 = OpAccessChain %_ptr_Uniform_v4float %29 %int_2
        %235 = OpLoad %v4float %234
        %236 = OpCompositeExtract %float %235 3
        %237 = OpFUnordNotEqual %bool %236 %float_0
        %238 = OpCompositeConstruct %v4bool %237 %237 %237 %237
               OpStore %232 %238
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %239
        %240 = OpFunctionParameter %_ptr_Function_v2float
        %241 = OpLabel
          %h = OpVariable %_ptr_Function_float Function
        %243 = OpVariable %_ptr_Function_float Function
         %h2 = OpVariable %_ptr_Function_v2float Function
        %247 = OpVariable %_ptr_Function_v2float Function
         %h3 = OpVariable %_ptr_Function_v3float Function
        %251 = OpVariable %_ptr_Function_v3float Function
         %h4 = OpVariable %_ptr_Function_v4float Function
        %255 = OpVariable %_ptr_Function_v4float Function
        %260 = OpVariable %_ptr_Function_float Function
        %263 = OpVariable %_ptr_Function_v2float Function
        %268 = OpVariable %_ptr_Function_v4float Function
       %h2x2 = OpVariable %_ptr_Function_mat2v2float Function
        %274 = OpVariable %_ptr_Function_mat2v2float Function
       %h3x3 = OpVariable %_ptr_Function_mat3v3float Function
        %278 = OpVariable %_ptr_Function_mat3v3float Function
       %h4x4 = OpVariable %_ptr_Function_mat4v4float Function
        %282 = OpVariable %_ptr_Function_mat4v4float Function
        %286 = OpVariable %_ptr_Function_v3float Function
        %292 = OpVariable %_ptr_Function_float Function
        %298 = OpVariable %_ptr_Function_float Function
          %i = OpVariable %_ptr_Function_int Function
        %302 = OpVariable %_ptr_Function_int Function
         %i2 = OpVariable %_ptr_Function_v2int Function
        %306 = OpVariable %_ptr_Function_v2int Function
         %i3 = OpVariable %_ptr_Function_v3int Function
        %310 = OpVariable %_ptr_Function_v3int Function
         %i4 = OpVariable %_ptr_Function_v4int Function
        %314 = OpVariable %_ptr_Function_v4int Function
        %317 = OpVariable %_ptr_Function_v3int Function
        %323 = OpVariable %_ptr_Function_int Function
          %f = OpVariable %_ptr_Function_float Function
        %327 = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_v2float Function
        %331 = OpVariable %_ptr_Function_v2float Function
         %f3 = OpVariable %_ptr_Function_v3float Function
        %335 = OpVariable %_ptr_Function_v3float Function
         %f4 = OpVariable %_ptr_Function_v4float Function
        %339 = OpVariable %_ptr_Function_v4float Function
        %342 = OpVariable %_ptr_Function_v2float Function
        %348 = OpVariable %_ptr_Function_float Function
       %f2x2 = OpVariable %_ptr_Function_mat2v2float Function
        %352 = OpVariable %_ptr_Function_mat2v2float Function
       %f3x3 = OpVariable %_ptr_Function_mat3v3float Function
        %356 = OpVariable %_ptr_Function_mat3v3float Function
       %f4x4 = OpVariable %_ptr_Function_mat4v4float Function
        %360 = OpVariable %_ptr_Function_mat4v4float Function
        %365 = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_bool Function
        %369 = OpVariable %_ptr_Function_bool Function
         %b2 = OpVariable %_ptr_Function_v2bool Function
        %373 = OpVariable %_ptr_Function_v2bool Function
         %b3 = OpVariable %_ptr_Function_v3bool Function
        %377 = OpVariable %_ptr_Function_v3bool Function
         %b4 = OpVariable %_ptr_Function_v4bool Function
        %381 = OpVariable %_ptr_Function_v4bool Function
        %384 = OpVariable %_ptr_Function_v2bool Function
        %390 = OpVariable %_ptr_Function_bool Function
         %ok = OpVariable %_ptr_Function_bool Function
        %482 = OpVariable %_ptr_Function_v4float Function
        %244 = OpFunctionCall %void %out_half_vh %243
        %245 = OpLoad %float %243
               OpStore %h %245
        %248 = OpFunctionCall %void %out_half2_vh2 %247
        %249 = OpLoad %v2float %247
               OpStore %h2 %249
        %252 = OpFunctionCall %void %out_half3_vh3 %251
        %253 = OpLoad %v3float %251
               OpStore %h3 %253
        %256 = OpFunctionCall %void %out_half4_vh4 %255
        %257 = OpLoad %v4float %255
               OpStore %h4 %257
        %258 = OpAccessChain %_ptr_Function_float %h3 %int_1
        %261 = OpFunctionCall %void %out_half_vh %260
        %262 = OpLoad %float %260
               OpStore %258 %262
        %264 = OpFunctionCall %void %out_half2_vh2 %263
        %265 = OpLoad %v2float %263
        %266 = OpLoad %v3float %h3
        %267 = OpVectorShuffle %v3float %266 %265 3 1 4
               OpStore %h3 %267
        %269 = OpFunctionCall %void %out_half4_vh4 %268
        %270 = OpLoad %v4float %268
        %271 = OpLoad %v4float %h4
        %272 = OpVectorShuffle %v4float %271 %270 6 7 4 5
               OpStore %h4 %272
        %275 = OpFunctionCall %void %out_half2x2_vh22 %274
        %276 = OpLoad %mat2v2float %274
               OpStore %h2x2 %276
        %279 = OpFunctionCall %void %out_half3x3_vh33 %278
        %280 = OpLoad %mat3v3float %278
               OpStore %h3x3 %280
        %283 = OpFunctionCall %void %out_half4x4_vh44 %282
        %284 = OpLoad %mat4v4float %282
               OpStore %h4x4 %284
        %285 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
        %287 = OpFunctionCall %void %out_half3_vh3 %286
        %288 = OpLoad %v3float %286
               OpStore %285 %288
        %290 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
        %291 = OpAccessChain %_ptr_Function_float %290 %int_3
        %293 = OpFunctionCall %void %out_half_vh %292
        %294 = OpLoad %float %292
               OpStore %291 %294
        %296 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
        %297 = OpAccessChain %_ptr_Function_float %296 %int_0
        %299 = OpFunctionCall %void %out_half_vh %298
        %300 = OpLoad %float %298
               OpStore %297 %300
        %303 = OpFunctionCall %void %out_int_vi %302
        %304 = OpLoad %int %302
               OpStore %i %304
        %307 = OpFunctionCall %void %out_int2_vi2 %306
        %308 = OpLoad %v2int %306
               OpStore %i2 %308
        %311 = OpFunctionCall %void %out_int3_vi3 %310
        %312 = OpLoad %v3int %310
               OpStore %i3 %312
        %315 = OpFunctionCall %void %out_int4_vi4 %314
        %316 = OpLoad %v4int %314
               OpStore %i4 %316
        %318 = OpFunctionCall %void %out_int3_vi3 %317
        %319 = OpLoad %v3int %317
        %320 = OpLoad %v4int %i4
        %321 = OpVectorShuffle %v4int %320 %319 4 5 6 3
               OpStore %i4 %321
        %322 = OpAccessChain %_ptr_Function_int %i2 %int_1
        %324 = OpFunctionCall %void %out_int_vi %323
        %325 = OpLoad %int %323
               OpStore %322 %325
        %328 = OpFunctionCall %void %out_float_vf %327
        %329 = OpLoad %float %327
               OpStore %f %329
        %332 = OpFunctionCall %void %out_float2_vf2 %331
        %333 = OpLoad %v2float %331
               OpStore %f2 %333
        %336 = OpFunctionCall %void %out_float3_vf3 %335
        %337 = OpLoad %v3float %335
               OpStore %f3 %337
        %340 = OpFunctionCall %void %out_float4_vf4 %339
        %341 = OpLoad %v4float %339
               OpStore %f4 %341
        %343 = OpFunctionCall %void %out_float2_vf2 %342
        %344 = OpLoad %v2float %342
        %345 = OpLoad %v3float %f3
        %346 = OpVectorShuffle %v3float %345 %344 3 4 2
               OpStore %f3 %346
        %347 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %349 = OpFunctionCall %void %out_float_vf %348
        %350 = OpLoad %float %348
               OpStore %347 %350
        %353 = OpFunctionCall %void %out_float2x2_vf22 %352
        %354 = OpLoad %mat2v2float %352
               OpStore %f2x2 %354
        %357 = OpFunctionCall %void %out_float3x3_vf33 %356
        %358 = OpLoad %mat3v3float %356
               OpStore %f3x3 %358
        %361 = OpFunctionCall %void %out_float4x4_vf44 %360
        %362 = OpLoad %mat4v4float %360
               OpStore %f4x4 %362
        %363 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
        %364 = OpAccessChain %_ptr_Function_float %363 %int_0
        %366 = OpFunctionCall %void %out_float_vf %365
        %367 = OpLoad %float %365
               OpStore %364 %367
        %370 = OpFunctionCall %void %out_bool_vb %369
        %371 = OpLoad %bool %369
               OpStore %b %371
        %374 = OpFunctionCall %void %out_bool2_vb2 %373
        %375 = OpLoad %v2bool %373
               OpStore %b2 %375
        %378 = OpFunctionCall %void %out_bool3_vb3 %377
        %379 = OpLoad %v3bool %377
               OpStore %b3 %379
        %382 = OpFunctionCall %void %out_bool4_vb4 %381
        %383 = OpLoad %v4bool %381
               OpStore %b4 %383
        %385 = OpFunctionCall %void %out_bool2_vb2 %384
        %386 = OpLoad %v2bool %384
        %387 = OpLoad %v4bool %b4
        %388 = OpVectorShuffle %v4bool %387 %386 4 1 2 5
               OpStore %b4 %388
        %389 = OpAccessChain %_ptr_Function_bool %b3 %int_2
        %391 = OpFunctionCall %void %out_bool_vb %390
        %392 = OpLoad %bool %390
               OpStore %389 %392
               OpStore %ok %true
               OpSelectionMerge %397 None
               OpBranchConditional %true %396 %397
        %396 = OpLabel
        %399 = OpLoad %float %h
        %400 = OpLoad %v2float %h2
        %401 = OpCompositeExtract %float %400 0
        %402 = OpFMul %float %399 %401
        %403 = OpLoad %v3float %h3
        %404 = OpCompositeExtract %float %403 0
        %405 = OpFMul %float %402 %404
        %406 = OpLoad %v4float %h4
        %407 = OpCompositeExtract %float %406 0
        %408 = OpFMul %float %405 %407
        %409 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
        %410 = OpLoad %v2float %409
        %411 = OpCompositeExtract %float %410 0
        %412 = OpFMul %float %408 %411
        %413 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
        %414 = OpLoad %v3float %413
        %415 = OpCompositeExtract %float %414 0
        %416 = OpFMul %float %412 %415
        %417 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
        %418 = OpLoad %v4float %417
        %419 = OpCompositeExtract %float %418 0
        %420 = OpFMul %float %416 %419
        %421 = OpFOrdEqual %bool %float_1 %420
               OpBranch %397
        %397 = OpLabel
        %422 = OpPhi %bool %false %241 %421 %396
               OpStore %ok %422
               OpSelectionMerge %424 None
               OpBranchConditional %422 %423 %424
        %423 = OpLabel
        %425 = OpLoad %float %f
        %426 = OpLoad %v2float %f2
        %427 = OpCompositeExtract %float %426 0
        %428 = OpFMul %float %425 %427
        %429 = OpLoad %v3float %f3
        %430 = OpCompositeExtract %float %429 0
        %431 = OpFMul %float %428 %430
        %432 = OpLoad %v4float %f4
        %433 = OpCompositeExtract %float %432 0
        %434 = OpFMul %float %431 %433
        %435 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
        %436 = OpLoad %v2float %435
        %437 = OpCompositeExtract %float %436 0
        %438 = OpFMul %float %434 %437
        %439 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
        %440 = OpLoad %v3float %439
        %441 = OpCompositeExtract %float %440 0
        %442 = OpFMul %float %438 %441
        %443 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
        %444 = OpLoad %v4float %443
        %445 = OpCompositeExtract %float %444 0
        %446 = OpFMul %float %442 %445
        %447 = OpFOrdEqual %bool %float_1 %446
               OpBranch %424
        %424 = OpLabel
        %448 = OpPhi %bool %false %397 %447 %423
               OpStore %ok %448
               OpSelectionMerge %450 None
               OpBranchConditional %448 %449 %450
        %449 = OpLabel
        %451 = OpLoad %int %i
        %452 = OpLoad %v2int %i2
        %453 = OpCompositeExtract %int %452 0
        %454 = OpIMul %int %451 %453
        %455 = OpLoad %v3int %i3
        %456 = OpCompositeExtract %int %455 0
        %457 = OpIMul %int %454 %456
        %458 = OpLoad %v4int %i4
        %459 = OpCompositeExtract %int %458 0
        %460 = OpIMul %int %457 %459
        %461 = OpIEqual %bool %int_1 %460
               OpBranch %450
        %450 = OpLabel
        %462 = OpPhi %bool %false %424 %461 %449
               OpStore %ok %462
               OpSelectionMerge %464 None
               OpBranchConditional %462 %463 %464
        %463 = OpLabel
        %465 = OpLoad %bool %b
               OpSelectionMerge %467 None
               OpBranchConditional %465 %466 %467
        %466 = OpLabel
        %468 = OpLoad %v2bool %b2
        %469 = OpCompositeExtract %bool %468 0
               OpBranch %467
        %467 = OpLabel
        %470 = OpPhi %bool %false %463 %469 %466
               OpSelectionMerge %472 None
               OpBranchConditional %470 %471 %472
        %471 = OpLabel
        %473 = OpLoad %v3bool %b3
        %474 = OpCompositeExtract %bool %473 0
               OpBranch %472
        %472 = OpLabel
        %475 = OpPhi %bool %false %467 %474 %471
               OpSelectionMerge %477 None
               OpBranchConditional %475 %476 %477
        %476 = OpLabel
        %478 = OpLoad %v4bool %b4
        %479 = OpCompositeExtract %bool %478 0
               OpBranch %477
        %477 = OpLabel
        %480 = OpPhi %bool %false %472 %479 %476
               OpBranch %464
        %464 = OpLabel
        %481 = OpPhi %bool %false %450 %480 %477
               OpStore %ok %481
               OpSelectionMerge %485 None
               OpBranchConditional %481 %483 %484
        %483 = OpLabel
        %486 = OpAccessChain %_ptr_Uniform_v4float %29 %int_0
        %487 = OpLoad %v4float %486
               OpStore %482 %487
               OpBranch %485
        %484 = OpLabel
        %488 = OpAccessChain %_ptr_Uniform_v4float %29 %int_1
        %489 = OpLoad %v4float %488
               OpStore %482 %489
               OpBranch %485
        %485 = OpLabel
        %490 = OpLoad %v4float %482
               OpReturnValue %490
               OpFunctionEnd
