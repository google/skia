OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
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
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %b RelaxedPrecision
OpDecorate %s RelaxedPrecision
OpDecorate %us RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %s2s RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %i2s RelaxedPrecision
OpDecorate %us2s RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %ui2s RelaxedPrecision
OpDecorate %h2s RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %f2s RelaxedPrecision
OpDecorate %b2s RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %s2us RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %i2us RelaxedPrecision
OpDecorate %us2us RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %ui2us RelaxedPrecision
OpDecorate %h2us RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %f2us RelaxedPrecision
OpDecorate %b2us RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_bool = OpTypePointer Private %bool
%b = OpVariable %_ptr_Private_bool Private
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%s = OpVariable %_ptr_Private_int Private
%float_1 = OpConstant %float 1
%i = OpVariable %_ptr_Private_int Private
%uint = OpTypeInt 32 0
%_ptr_Private_uint = OpTypePointer Private %uint
%us = OpVariable %_ptr_Private_uint Private
%ui = OpVariable %_ptr_Private_uint Private
%_ptr_Private_float = OpTypePointer Private %float
%h = OpVariable %_ptr_Private_float Private
%f = OpVariable %_ptr_Private_float Private
%s2s = OpVariable %_ptr_Private_int Private
%i2s = OpVariable %_ptr_Private_int Private
%us2s = OpVariable %_ptr_Private_int Private
%ui2s = OpVariable %_ptr_Private_int Private
%h2s = OpVariable %_ptr_Private_int Private
%f2s = OpVariable %_ptr_Private_int Private
%b2s = OpVariable %_ptr_Private_int Private
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%s2i = OpVariable %_ptr_Private_int Private
%i2i = OpVariable %_ptr_Private_int Private
%us2i = OpVariable %_ptr_Private_int Private
%ui2i = OpVariable %_ptr_Private_int Private
%h2i = OpVariable %_ptr_Private_int Private
%f2i = OpVariable %_ptr_Private_int Private
%b2i = OpVariable %_ptr_Private_int Private
%s2us = OpVariable %_ptr_Private_uint Private
%i2us = OpVariable %_ptr_Private_uint Private
%us2us = OpVariable %_ptr_Private_uint Private
%ui2us = OpVariable %_ptr_Private_uint Private
%h2us = OpVariable %_ptr_Private_uint Private
%f2us = OpVariable %_ptr_Private_uint Private
%b2us = OpVariable %_ptr_Private_uint Private
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%s2ui = OpVariable %_ptr_Private_uint Private
%i2ui = OpVariable %_ptr_Private_uint Private
%us2ui = OpVariable %_ptr_Private_uint Private
%ui2ui = OpVariable %_ptr_Private_uint Private
%h2ui = OpVariable %_ptr_Private_uint Private
%f2ui = OpVariable %_ptr_Private_uint Private
%b2ui = OpVariable %_ptr_Private_uint Private
%s2f = OpVariable %_ptr_Private_float Private
%i2f = OpVariable %_ptr_Private_float Private
%us2f = OpVariable %_ptr_Private_float Private
%ui2f = OpVariable %_ptr_Private_float Private
%h2f = OpVariable %_ptr_Private_float Private
%f2f = OpVariable %_ptr_Private_float Private
%b2f = OpVariable %_ptr_Private_float Private
%float_0 = OpConstant %float 0
%void = OpTypeVoid
%136 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %136
%137 = OpLabel
OpStore %b %true
%17 = OpExtInst %float %1 Sqrt %float_1
%16 = OpConvertFToS %int %17
OpStore %s %16
%21 = OpExtInst %float %1 Sqrt %float_1
%20 = OpConvertFToS %int %21
OpStore %i %20
%26 = OpExtInst %float %1 Sqrt %float_1
%25 = OpConvertFToU %uint %26
OpStore %us %25
%29 = OpExtInst %float %1 Sqrt %float_1
%28 = OpConvertFToU %uint %29
OpStore %ui %28
%32 = OpExtInst %float %1 Sqrt %float_1
OpStore %h %32
%34 = OpExtInst %float %1 Sqrt %float_1
OpStore %f %34
%36 = OpLoad %int %s
OpStore %s2s %36
%38 = OpLoad %int %i
OpStore %i2s %38
%41 = OpLoad %uint %us
%40 = OpBitcast %int %41
OpStore %us2s %40
%44 = OpLoad %uint %ui
%43 = OpBitcast %int %44
OpStore %ui2s %43
%47 = OpLoad %float %h
%46 = OpConvertFToS %int %47
OpStore %h2s %46
%50 = OpLoad %float %f
%49 = OpConvertFToS %int %50
OpStore %f2s %49
%53 = OpLoad %bool %b
%52 = OpSelect %int %53 %int_1 %int_0
OpStore %b2s %52
%57 = OpLoad %int %s
OpStore %s2i %57
%59 = OpLoad %int %i
OpStore %i2i %59
%62 = OpLoad %uint %us
%61 = OpBitcast %int %62
OpStore %us2i %61
%65 = OpLoad %uint %ui
%64 = OpBitcast %int %65
OpStore %ui2i %64
%68 = OpLoad %float %h
%67 = OpConvertFToS %int %68
OpStore %h2i %67
%71 = OpLoad %float %f
%70 = OpConvertFToS %int %71
OpStore %f2i %70
%74 = OpLoad %bool %b
%73 = OpSelect %int %74 %int_1 %int_0
OpStore %b2i %73
%77 = OpLoad %int %s
%76 = OpBitcast %uint %77
OpStore %s2us %76
%80 = OpLoad %int %i
%79 = OpBitcast %uint %80
OpStore %i2us %79
%82 = OpLoad %uint %us
OpStore %us2us %82
%84 = OpLoad %uint %ui
OpStore %ui2us %84
%87 = OpLoad %float %h
%86 = OpConvertFToU %uint %87
OpStore %h2us %86
%90 = OpLoad %float %f
%89 = OpConvertFToU %uint %90
OpStore %f2us %89
%93 = OpLoad %bool %b
%92 = OpSelect %uint %93 %uint_1 %uint_0
OpStore %b2us %92
%98 = OpLoad %int %s
%97 = OpBitcast %uint %98
OpStore %s2ui %97
%101 = OpLoad %int %i
%100 = OpBitcast %uint %101
OpStore %i2ui %100
%103 = OpLoad %uint %us
OpStore %us2ui %103
%105 = OpLoad %uint %ui
OpStore %ui2ui %105
%108 = OpLoad %float %h
%107 = OpConvertFToU %uint %108
OpStore %h2ui %107
%111 = OpLoad %float %f
%110 = OpConvertFToU %uint %111
OpStore %f2ui %110
%114 = OpLoad %bool %b
%113 = OpSelect %uint %114 %uint_1 %uint_0
OpStore %b2ui %113
%117 = OpLoad %int %s
%116 = OpConvertSToF %float %117
OpStore %s2f %116
%120 = OpLoad %int %i
%119 = OpConvertSToF %float %120
OpStore %i2f %119
%123 = OpLoad %uint %us
%122 = OpConvertUToF %float %123
OpStore %us2f %122
%126 = OpLoad %uint %ui
%125 = OpConvertUToF %float %126
OpStore %ui2f %125
%128 = OpLoad %float %h
OpStore %h2f %128
%130 = OpLoad %float %f
OpStore %f2f %130
%133 = OpLoad %bool %b
%132 = OpSelect %float %133 %float_1 %float_0
OpStore %b2f %132
%139 = OpLoad %int %s
%138 = OpConvertSToF %float %139
%141 = OpLoad %int %i
%140 = OpConvertSToF %float %141
%142 = OpFAdd %float %138 %140
%144 = OpLoad %uint %us
%143 = OpConvertUToF %float %144
%145 = OpFAdd %float %142 %143
%147 = OpLoad %uint %ui
%146 = OpConvertUToF %float %147
%148 = OpFAdd %float %145 %146
%149 = OpLoad %float %h
%150 = OpFAdd %float %148 %149
%151 = OpLoad %float %f
%152 = OpFAdd %float %150 %151
%154 = OpLoad %int %s2s
%153 = OpConvertSToF %float %154
%155 = OpFAdd %float %152 %153
%157 = OpLoad %int %i2s
%156 = OpConvertSToF %float %157
%158 = OpFAdd %float %155 %156
%160 = OpLoad %int %us2s
%159 = OpConvertSToF %float %160
%161 = OpFAdd %float %158 %159
%163 = OpLoad %int %ui2s
%162 = OpConvertSToF %float %163
%164 = OpFAdd %float %161 %162
%166 = OpLoad %int %h2s
%165 = OpConvertSToF %float %166
%167 = OpFAdd %float %164 %165
%169 = OpLoad %int %f2s
%168 = OpConvertSToF %float %169
%170 = OpFAdd %float %167 %168
%172 = OpLoad %int %b2s
%171 = OpConvertSToF %float %172
%173 = OpFAdd %float %170 %171
%175 = OpLoad %int %s2i
%174 = OpConvertSToF %float %175
%176 = OpFAdd %float %173 %174
%178 = OpLoad %int %i2i
%177 = OpConvertSToF %float %178
%179 = OpFAdd %float %176 %177
%181 = OpLoad %int %us2i
%180 = OpConvertSToF %float %181
%182 = OpFAdd %float %179 %180
%184 = OpLoad %int %ui2i
%183 = OpConvertSToF %float %184
%185 = OpFAdd %float %182 %183
%187 = OpLoad %int %h2i
%186 = OpConvertSToF %float %187
%188 = OpFAdd %float %185 %186
%190 = OpLoad %int %f2i
%189 = OpConvertSToF %float %190
%191 = OpFAdd %float %188 %189
%193 = OpLoad %int %b2i
%192 = OpConvertSToF %float %193
%194 = OpFAdd %float %191 %192
%196 = OpLoad %uint %s2us
%195 = OpConvertUToF %float %196
%197 = OpFAdd %float %194 %195
%199 = OpLoad %uint %i2us
%198 = OpConvertUToF %float %199
%200 = OpFAdd %float %197 %198
%202 = OpLoad %uint %us2us
%201 = OpConvertUToF %float %202
%203 = OpFAdd %float %200 %201
%204 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %204 %203
%206 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%207 = OpLoad %float %206
%209 = OpLoad %uint %ui2us
%208 = OpConvertUToF %float %209
%211 = OpLoad %uint %h2us
%210 = OpConvertUToF %float %211
%212 = OpFAdd %float %208 %210
%214 = OpLoad %uint %f2us
%213 = OpConvertUToF %float %214
%215 = OpFAdd %float %212 %213
%217 = OpLoad %uint %b2us
%216 = OpConvertUToF %float %217
%218 = OpFAdd %float %215 %216
%220 = OpLoad %uint %s2ui
%219 = OpConvertUToF %float %220
%221 = OpFAdd %float %218 %219
%223 = OpLoad %uint %i2ui
%222 = OpConvertUToF %float %223
%224 = OpFAdd %float %221 %222
%226 = OpLoad %uint %us2ui
%225 = OpConvertUToF %float %226
%227 = OpFAdd %float %224 %225
%229 = OpLoad %uint %ui2ui
%228 = OpConvertUToF %float %229
%230 = OpFAdd %float %227 %228
%232 = OpLoad %uint %h2ui
%231 = OpConvertUToF %float %232
%233 = OpFAdd %float %230 %231
%235 = OpLoad %uint %f2ui
%234 = OpConvertUToF %float %235
%236 = OpFAdd %float %233 %234
%238 = OpLoad %uint %b2ui
%237 = OpConvertUToF %float %238
%239 = OpFAdd %float %236 %237
%240 = OpLoad %float %s2f
%241 = OpFAdd %float %239 %240
%242 = OpLoad %float %i2f
%243 = OpFAdd %float %241 %242
%244 = OpLoad %float %us2f
%245 = OpFAdd %float %243 %244
%246 = OpLoad %float %ui2f
%247 = OpFAdd %float %245 %246
%248 = OpLoad %float %h2f
%249 = OpFAdd %float %247 %248
%250 = OpLoad %float %f2f
%251 = OpFAdd %float %249 %250
%252 = OpLoad %float %b2f
%253 = OpFAdd %float %251 %252
%254 = OpFAdd %float %207 %253
OpStore %206 %254
OpReturn
OpFunctionEnd
