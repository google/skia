OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %main "main"
OpName %b "b"
OpName %s "s"
OpName %i "i"
OpName %us "us"
OpName %ui "ui"
OpName %h "h"
OpName %f "f"
OpName %s2s "s2s"
OpName %i2s "i2s"
OpName %us2s "us2s"
OpName %ui2s "ui2s"
OpName %h2s "h2s"
OpName %f2s "f2s"
OpName %b2s "b2s"
OpName %s2i "s2i"
OpName %i2i "i2i"
OpName %us2i "us2i"
OpName %ui2i "ui2i"
OpName %h2i "h2i"
OpName %f2i "f2i"
OpName %b2i "b2i"
OpName %s2us "s2us"
OpName %i2us "i2us"
OpName %us2us "us2us"
OpName %ui2us "ui2us"
OpName %h2us "h2us"
OpName %f2us "f2us"
OpName %b2us "b2us"
OpName %s2ui "s2ui"
OpName %i2ui "i2ui"
OpName %us2ui "us2ui"
OpName %ui2ui "ui2ui"
OpName %h2ui "h2ui"
OpName %f2ui "f2ui"
OpName %b2ui "b2ui"
OpName %s2f "s2f"
OpName %i2f "i2f"
OpName %us2f "us2f"
OpName %ui2f "ui2f"
OpName %h2f "h2f"
OpName %f2f "f2f"
OpName %b2f "b2f"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %s RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %us RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %s2s RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %i2s RelaxedPrecision
OpDecorate %us2s RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %ui2s RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %h2s RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %f2s RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %b2s RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %s2us RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %i2us RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %us2us RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %ui2us RelaxedPrecision
OpDecorate %h2us RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %f2us RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %b2us RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
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
OpDecorate %253 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %14
%15 = OpLabel
%b = OpVariable %_ptr_Function_bool Function
%s = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
%us = OpVariable %_ptr_Function_uint Function
%ui = OpVariable %_ptr_Function_uint Function
%h = OpVariable %_ptr_Function_float Function
%f = OpVariable %_ptr_Function_float Function
%s2s = OpVariable %_ptr_Function_int Function
%i2s = OpVariable %_ptr_Function_int Function
%us2s = OpVariable %_ptr_Function_int Function
%ui2s = OpVariable %_ptr_Function_int Function
%h2s = OpVariable %_ptr_Function_int Function
%f2s = OpVariable %_ptr_Function_int Function
%b2s = OpVariable %_ptr_Function_int Function
%s2i = OpVariable %_ptr_Function_int Function
%i2i = OpVariable %_ptr_Function_int Function
%us2i = OpVariable %_ptr_Function_int Function
%ui2i = OpVariable %_ptr_Function_int Function
%h2i = OpVariable %_ptr_Function_int Function
%f2i = OpVariable %_ptr_Function_int Function
%b2i = OpVariable %_ptr_Function_int Function
%s2us = OpVariable %_ptr_Function_uint Function
%i2us = OpVariable %_ptr_Function_uint Function
%us2us = OpVariable %_ptr_Function_uint Function
%ui2us = OpVariable %_ptr_Function_uint Function
%h2us = OpVariable %_ptr_Function_uint Function
%f2us = OpVariable %_ptr_Function_uint Function
%b2us = OpVariable %_ptr_Function_uint Function
%s2ui = OpVariable %_ptr_Function_uint Function
%i2ui = OpVariable %_ptr_Function_uint Function
%us2ui = OpVariable %_ptr_Function_uint Function
%ui2ui = OpVariable %_ptr_Function_uint Function
%h2ui = OpVariable %_ptr_Function_uint Function
%f2ui = OpVariable %_ptr_Function_uint Function
%b2ui = OpVariable %_ptr_Function_uint Function
%s2f = OpVariable %_ptr_Function_float Function
%i2f = OpVariable %_ptr_Function_float Function
%us2f = OpVariable %_ptr_Function_float Function
%ui2f = OpVariable %_ptr_Function_float Function
%h2f = OpVariable %_ptr_Function_float Function
%f2f = OpVariable %_ptr_Function_float Function
%b2f = OpVariable %_ptr_Function_float Function
OpStore %b %true
%22 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%25 = OpLoad %float %22
%26 = OpConvertFToS %int %25
OpStore %s %26
%28 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%29 = OpLoad %float %28
%30 = OpConvertFToS %int %29
OpStore %i %30
%34 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%35 = OpLoad %float %34
%36 = OpConvertFToU %uint %35
OpStore %us %36
%38 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%39 = OpLoad %float %38
%40 = OpConvertFToU %uint %39
OpStore %ui %40
%43 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%44 = OpLoad %float %43
OpStore %h %44
%46 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%47 = OpLoad %float %46
OpStore %f %47
%49 = OpLoad %int %s
OpStore %s2s %49
%51 = OpLoad %int %i
OpStore %i2s %51
%53 = OpLoad %uint %us
%54 = OpBitcast %int %53
OpStore %us2s %54
%56 = OpLoad %uint %ui
%57 = OpBitcast %int %56
OpStore %ui2s %57
%59 = OpLoad %float %h
%60 = OpConvertFToS %int %59
OpStore %h2s %60
%62 = OpLoad %float %f
%63 = OpConvertFToS %int %62
OpStore %f2s %63
%65 = OpLoad %bool %b
%66 = OpSelect %int %65 %int_1 %int_0
OpStore %b2s %66
%69 = OpLoad %int %s
OpStore %s2i %69
%71 = OpLoad %int %i
OpStore %i2i %71
%73 = OpLoad %uint %us
%74 = OpBitcast %int %73
OpStore %us2i %74
%76 = OpLoad %uint %ui
%77 = OpBitcast %int %76
OpStore %ui2i %77
%79 = OpLoad %float %h
%80 = OpConvertFToS %int %79
OpStore %h2i %80
%82 = OpLoad %float %f
%83 = OpConvertFToS %int %82
OpStore %f2i %83
%85 = OpLoad %bool %b
%86 = OpSelect %int %85 %int_1 %int_0
OpStore %b2i %86
%88 = OpLoad %int %s
%89 = OpBitcast %uint %88
OpStore %s2us %89
%91 = OpLoad %int %i
%92 = OpBitcast %uint %91
OpStore %i2us %92
%94 = OpLoad %uint %us
OpStore %us2us %94
%96 = OpLoad %uint %ui
OpStore %ui2us %96
%98 = OpLoad %float %h
%99 = OpConvertFToU %uint %98
OpStore %h2us %99
%101 = OpLoad %float %f
%102 = OpConvertFToU %uint %101
OpStore %f2us %102
%104 = OpLoad %bool %b
%105 = OpSelect %uint %104 %uint_1 %uint_0
OpStore %b2us %105
%109 = OpLoad %int %s
%110 = OpBitcast %uint %109
OpStore %s2ui %110
%112 = OpLoad %int %i
%113 = OpBitcast %uint %112
OpStore %i2ui %113
%115 = OpLoad %uint %us
OpStore %us2ui %115
%117 = OpLoad %uint %ui
OpStore %ui2ui %117
%119 = OpLoad %float %h
%120 = OpConvertFToU %uint %119
OpStore %h2ui %120
%122 = OpLoad %float %f
%123 = OpConvertFToU %uint %122
OpStore %f2ui %123
%125 = OpLoad %bool %b
%126 = OpSelect %uint %125 %uint_1 %uint_0
OpStore %b2ui %126
%128 = OpLoad %int %s
%129 = OpConvertSToF %float %128
OpStore %s2f %129
%131 = OpLoad %int %i
%132 = OpConvertSToF %float %131
OpStore %i2f %132
%134 = OpLoad %uint %us
%135 = OpConvertUToF %float %134
OpStore %us2f %135
%137 = OpLoad %uint %ui
%138 = OpConvertUToF %float %137
OpStore %ui2f %138
%140 = OpLoad %float %h
OpStore %h2f %140
%142 = OpLoad %float %f
OpStore %f2f %142
%144 = OpLoad %bool %b
%145 = OpSelect %float %144 %float_1 %float_0
OpStore %b2f %145
%148 = OpLoad %int %s
%149 = OpConvertSToF %float %148
%150 = OpLoad %int %i
%151 = OpConvertSToF %float %150
%152 = OpFAdd %float %149 %151
%153 = OpLoad %uint %us
%154 = OpConvertUToF %float %153
%155 = OpFAdd %float %152 %154
%156 = OpLoad %uint %ui
%157 = OpConvertUToF %float %156
%158 = OpFAdd %float %155 %157
%159 = OpLoad %float %h
%160 = OpFAdd %float %158 %159
%161 = OpLoad %float %f
%162 = OpFAdd %float %160 %161
%163 = OpLoad %int %s2s
%164 = OpConvertSToF %float %163
%165 = OpFAdd %float %162 %164
%166 = OpLoad %int %i2s
%167 = OpConvertSToF %float %166
%168 = OpFAdd %float %165 %167
%169 = OpLoad %int %us2s
%170 = OpConvertSToF %float %169
%171 = OpFAdd %float %168 %170
%172 = OpLoad %int %ui2s
%173 = OpConvertSToF %float %172
%174 = OpFAdd %float %171 %173
%175 = OpLoad %int %h2s
%176 = OpConvertSToF %float %175
%177 = OpFAdd %float %174 %176
%178 = OpLoad %int %f2s
%179 = OpConvertSToF %float %178
%180 = OpFAdd %float %177 %179
%181 = OpLoad %int %b2s
%182 = OpConvertSToF %float %181
%183 = OpFAdd %float %180 %182
%184 = OpLoad %int %s2i
%185 = OpConvertSToF %float %184
%186 = OpFAdd %float %183 %185
%187 = OpLoad %int %i2i
%188 = OpConvertSToF %float %187
%189 = OpFAdd %float %186 %188
%190 = OpLoad %int %us2i
%191 = OpConvertSToF %float %190
%192 = OpFAdd %float %189 %191
%193 = OpLoad %int %ui2i
%194 = OpConvertSToF %float %193
%195 = OpFAdd %float %192 %194
%196 = OpLoad %int %h2i
%197 = OpConvertSToF %float %196
%198 = OpFAdd %float %195 %197
%199 = OpLoad %int %f2i
%200 = OpConvertSToF %float %199
%201 = OpFAdd %float %198 %200
%202 = OpLoad %int %b2i
%203 = OpConvertSToF %float %202
%204 = OpFAdd %float %201 %203
%205 = OpLoad %uint %s2us
%206 = OpConvertUToF %float %205
%207 = OpFAdd %float %204 %206
%208 = OpLoad %uint %i2us
%209 = OpConvertUToF %float %208
%210 = OpFAdd %float %207 %209
%211 = OpLoad %uint %us2us
%212 = OpConvertUToF %float %211
%213 = OpFAdd %float %210 %212
%214 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %214 %213
%216 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%217 = OpLoad %float %216
%218 = OpLoad %uint %ui2us
%219 = OpConvertUToF %float %218
%220 = OpLoad %uint %h2us
%221 = OpConvertUToF %float %220
%222 = OpFAdd %float %219 %221
%223 = OpLoad %uint %f2us
%224 = OpConvertUToF %float %223
%225 = OpFAdd %float %222 %224
%226 = OpLoad %uint %b2us
%227 = OpConvertUToF %float %226
%228 = OpFAdd %float %225 %227
%229 = OpLoad %uint %s2ui
%230 = OpConvertUToF %float %229
%231 = OpFAdd %float %228 %230
%232 = OpLoad %uint %i2ui
%233 = OpConvertUToF %float %232
%234 = OpFAdd %float %231 %233
%235 = OpLoad %uint %us2ui
%236 = OpConvertUToF %float %235
%237 = OpFAdd %float %234 %236
%238 = OpLoad %uint %ui2ui
%239 = OpConvertUToF %float %238
%240 = OpFAdd %float %237 %239
%241 = OpLoad %uint %h2ui
%242 = OpConvertUToF %float %241
%243 = OpFAdd %float %240 %242
%244 = OpLoad %uint %f2ui
%245 = OpConvertUToF %float %244
%246 = OpFAdd %float %243 %245
%247 = OpLoad %uint %b2ui
%248 = OpConvertUToF %float %247
%249 = OpFAdd %float %246 %248
%250 = OpLoad %float %s2f
%251 = OpFAdd %float %249 %250
%252 = OpLoad %float %i2f
%253 = OpFAdd %float %251 %252
%254 = OpLoad %float %us2f
%255 = OpFAdd %float %253 %254
%256 = OpLoad %float %ui2f
%257 = OpFAdd %float %255 %256
%258 = OpLoad %float %h2f
%259 = OpFAdd %float %257 %258
%260 = OpLoad %float %f2f
%261 = OpFAdd %float %259 %260
%262 = OpLoad %float %b2f
%263 = OpFAdd %float %261 %262
%264 = OpFAdd %float %217 %263
OpStore %216 %264
OpReturn
OpFunctionEnd
