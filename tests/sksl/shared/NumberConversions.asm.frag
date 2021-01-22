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
OpDecorate %40 RelaxedPrecision
OpDecorate %ui2s RelaxedPrecision
OpDecorate %h2s RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %f2s RelaxedPrecision
OpDecorate %b2s RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %s2us RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %i2us RelaxedPrecision
OpDecorate %us2us RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %ui2us RelaxedPrecision
OpDecorate %h2us RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %f2us RelaxedPrecision
OpDecorate %b2us RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
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
%16 = OpExtInst %float %1 Sqrt %float_1
%18 = OpConvertFToS %int %16
OpStore %s %18
%20 = OpExtInst %float %1 Sqrt %float_1
%21 = OpConvertFToS %int %20
OpStore %i %21
%25 = OpExtInst %float %1 Sqrt %float_1
%26 = OpConvertFToU %uint %25
OpStore %us %26
%28 = OpExtInst %float %1 Sqrt %float_1
%29 = OpConvertFToU %uint %28
OpStore %ui %29
%32 = OpExtInst %float %1 Sqrt %float_1
OpStore %h %32
%34 = OpExtInst %float %1 Sqrt %float_1
OpStore %f %34
%36 = OpLoad %int %s
OpStore %s2s %36
%38 = OpLoad %int %i
OpStore %i2s %38
%40 = OpLoad %uint %us
%41 = OpBitcast %int %40
OpStore %us2s %41
%43 = OpLoad %uint %ui
%44 = OpBitcast %int %43
OpStore %ui2s %44
%46 = OpLoad %float %h
%47 = OpConvertFToS %int %46
OpStore %h2s %47
%49 = OpLoad %float %f
%50 = OpConvertFToS %int %49
OpStore %f2s %50
%52 = OpLoad %bool %b
%53 = OpSelect %int %52 %int_1 %int_0
OpStore %b2s %53
%57 = OpLoad %int %s
OpStore %s2i %57
%59 = OpLoad %int %i
OpStore %i2i %59
%61 = OpLoad %uint %us
%62 = OpBitcast %int %61
OpStore %us2i %62
%64 = OpLoad %uint %ui
%65 = OpBitcast %int %64
OpStore %ui2i %65
%67 = OpLoad %float %h
%68 = OpConvertFToS %int %67
OpStore %h2i %68
%70 = OpLoad %float %f
%71 = OpConvertFToS %int %70
OpStore %f2i %71
%73 = OpLoad %bool %b
%74 = OpSelect %int %73 %int_1 %int_0
OpStore %b2i %74
%76 = OpLoad %int %s
%77 = OpBitcast %uint %76
OpStore %s2us %77
%79 = OpLoad %int %i
%80 = OpBitcast %uint %79
OpStore %i2us %80
%82 = OpLoad %uint %us
OpStore %us2us %82
%84 = OpLoad %uint %ui
OpStore %ui2us %84
%86 = OpLoad %float %h
%87 = OpConvertFToU %uint %86
OpStore %h2us %87
%89 = OpLoad %float %f
%90 = OpConvertFToU %uint %89
OpStore %f2us %90
%92 = OpLoad %bool %b
%93 = OpSelect %uint %92 %uint_1 %uint_0
OpStore %b2us %93
%97 = OpLoad %int %s
%98 = OpBitcast %uint %97
OpStore %s2ui %98
%100 = OpLoad %int %i
%101 = OpBitcast %uint %100
OpStore %i2ui %101
%103 = OpLoad %uint %us
OpStore %us2ui %103
%105 = OpLoad %uint %ui
OpStore %ui2ui %105
%107 = OpLoad %float %h
%108 = OpConvertFToU %uint %107
OpStore %h2ui %108
%110 = OpLoad %float %f
%111 = OpConvertFToU %uint %110
OpStore %f2ui %111
%113 = OpLoad %bool %b
%114 = OpSelect %uint %113 %uint_1 %uint_0
OpStore %b2ui %114
%116 = OpLoad %int %s
%117 = OpConvertSToF %float %116
OpStore %s2f %117
%119 = OpLoad %int %i
%120 = OpConvertSToF %float %119
OpStore %i2f %120
%122 = OpLoad %uint %us
%123 = OpConvertUToF %float %122
OpStore %us2f %123
%125 = OpLoad %uint %ui
%126 = OpConvertUToF %float %125
OpStore %ui2f %126
%128 = OpLoad %float %h
OpStore %h2f %128
%130 = OpLoad %float %f
OpStore %f2f %130
%132 = OpLoad %bool %b
%133 = OpSelect %float %132 %float_1 %float_0
OpStore %b2f %133
%138 = OpLoad %int %s
%139 = OpConvertSToF %float %138
%140 = OpLoad %int %i
%141 = OpConvertSToF %float %140
%142 = OpFAdd %float %139 %141
%143 = OpLoad %uint %us
%144 = OpConvertUToF %float %143
%145 = OpFAdd %float %142 %144
%146 = OpLoad %uint %ui
%147 = OpConvertUToF %float %146
%148 = OpFAdd %float %145 %147
%149 = OpLoad %float %h
%150 = OpFAdd %float %148 %149
%151 = OpLoad %float %f
%152 = OpFAdd %float %150 %151
%153 = OpLoad %int %s2s
%154 = OpConvertSToF %float %153
%155 = OpFAdd %float %152 %154
%156 = OpLoad %int %i2s
%157 = OpConvertSToF %float %156
%158 = OpFAdd %float %155 %157
%159 = OpLoad %int %us2s
%160 = OpConvertSToF %float %159
%161 = OpFAdd %float %158 %160
%162 = OpLoad %int %ui2s
%163 = OpConvertSToF %float %162
%164 = OpFAdd %float %161 %163
%165 = OpLoad %int %h2s
%166 = OpConvertSToF %float %165
%167 = OpFAdd %float %164 %166
%168 = OpLoad %int %f2s
%169 = OpConvertSToF %float %168
%170 = OpFAdd %float %167 %169
%171 = OpLoad %int %b2s
%172 = OpConvertSToF %float %171
%173 = OpFAdd %float %170 %172
%174 = OpLoad %int %s2i
%175 = OpConvertSToF %float %174
%176 = OpFAdd %float %173 %175
%177 = OpLoad %int %i2i
%178 = OpConvertSToF %float %177
%179 = OpFAdd %float %176 %178
%180 = OpLoad %int %us2i
%181 = OpConvertSToF %float %180
%182 = OpFAdd %float %179 %181
%183 = OpLoad %int %ui2i
%184 = OpConvertSToF %float %183
%185 = OpFAdd %float %182 %184
%186 = OpLoad %int %h2i
%187 = OpConvertSToF %float %186
%188 = OpFAdd %float %185 %187
%189 = OpLoad %int %f2i
%190 = OpConvertSToF %float %189
%191 = OpFAdd %float %188 %190
%192 = OpLoad %int %b2i
%193 = OpConvertSToF %float %192
%194 = OpFAdd %float %191 %193
%195 = OpLoad %uint %s2us
%196 = OpConvertUToF %float %195
%197 = OpFAdd %float %194 %196
%198 = OpLoad %uint %i2us
%199 = OpConvertUToF %float %198
%200 = OpFAdd %float %197 %199
%201 = OpLoad %uint %us2us
%202 = OpConvertUToF %float %201
%203 = OpFAdd %float %200 %202
%204 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %204 %203
%206 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%207 = OpLoad %float %206
%208 = OpLoad %uint %ui2us
%209 = OpConvertUToF %float %208
%210 = OpLoad %uint %h2us
%211 = OpConvertUToF %float %210
%212 = OpFAdd %float %209 %211
%213 = OpLoad %uint %f2us
%214 = OpConvertUToF %float %213
%215 = OpFAdd %float %212 %214
%216 = OpLoad %uint %b2us
%217 = OpConvertUToF %float %216
%218 = OpFAdd %float %215 %217
%219 = OpLoad %uint %s2ui
%220 = OpConvertUToF %float %219
%221 = OpFAdd %float %218 %220
%222 = OpLoad %uint %i2ui
%223 = OpConvertUToF %float %222
%224 = OpFAdd %float %221 %223
%225 = OpLoad %uint %us2ui
%226 = OpConvertUToF %float %225
%227 = OpFAdd %float %224 %226
%228 = OpLoad %uint %ui2ui
%229 = OpConvertUToF %float %228
%230 = OpFAdd %float %227 %229
%231 = OpLoad %uint %h2ui
%232 = OpConvertUToF %float %231
%233 = OpFAdd %float %230 %232
%234 = OpLoad %uint %f2ui
%235 = OpConvertUToF %float %234
%236 = OpFAdd %float %233 %235
%237 = OpLoad %uint %b2ui
%238 = OpConvertUToF %float %237
%239 = OpFAdd %float %236 %238
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
