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
OpDecorate %52 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
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
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
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
%s2ui = OpVariable %_ptr_Private_uint Private
%i2ui = OpVariable %_ptr_Private_uint Private
%us2ui = OpVariable %_ptr_Private_uint Private
%ui2ui = OpVariable %_ptr_Private_uint Private
%h2ui = OpVariable %_ptr_Private_uint Private
%f2ui = OpVariable %_ptr_Private_uint Private
%b2ui = OpVariable %_ptr_Private_uint Private
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%s2f = OpVariable %_ptr_Private_float Private
%i2f = OpVariable %_ptr_Private_float Private
%us2f = OpVariable %_ptr_Private_float Private
%ui2f = OpVariable %_ptr_Private_float Private
%h2f = OpVariable %_ptr_Private_float Private
%f2f = OpVariable %_ptr_Private_float Private
%b2f = OpVariable %_ptr_Private_float Private
%float_0 = OpConstant %float 0
%void = OpTypeVoid
%138 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %138
%139 = OpLabel
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
%52 = OpLoad %bool %b
%53 = OpSelect %int %52 %int_1 %int_0
OpStore %b2s %53
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
%73 = OpLoad %bool %b
%74 = OpSelect %int %73 %int_1 %int_0
OpStore %b2i %74
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
%92 = OpLoad %bool %b
%94 = OpBitcast %uint %int_1
%95 = OpBitcast %uint %int_0
%93 = OpSelect %uint %92 %94 %95
OpStore %b2us %93
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
%113 = OpLoad %bool %b
%114 = OpSelect %uint %113 %uint_1 %uint_0
OpStore %b2ui %114
%119 = OpLoad %int %s
%118 = OpConvertSToF %float %119
OpStore %s2f %118
%122 = OpLoad %int %i
%121 = OpConvertSToF %float %122
OpStore %i2f %121
%125 = OpLoad %uint %us
%124 = OpConvertUToF %float %125
OpStore %us2f %124
%128 = OpLoad %uint %ui
%127 = OpConvertUToF %float %128
OpStore %ui2f %127
%130 = OpLoad %float %h
OpStore %h2f %130
%132 = OpLoad %float %f
OpStore %f2f %132
%134 = OpLoad %bool %b
%135 = OpSelect %float %134 %float_1 %float_0
OpStore %b2f %135
%141 = OpLoad %int %s
%142 = OpLoad %int %i
%143 = OpIAdd %int %141 %142
%145 = OpLoad %uint %us
%144 = OpBitcast %int %145
%146 = OpIAdd %int %143 %144
%140 = OpConvertSToF %float %146
%148 = OpLoad %uint %ui
%147 = OpConvertUToF %float %148
%149 = OpFAdd %float %140 %147
%150 = OpLoad %float %h
%151 = OpFAdd %float %149 %150
%152 = OpLoad %float %f
%153 = OpFAdd %float %151 %152
%155 = OpLoad %int %s2s
%154 = OpConvertSToF %float %155
%156 = OpFAdd %float %153 %154
%158 = OpLoad %int %i2s
%157 = OpConvertSToF %float %158
%159 = OpFAdd %float %156 %157
%161 = OpLoad %int %us2s
%160 = OpConvertSToF %float %161
%162 = OpFAdd %float %159 %160
%164 = OpLoad %int %ui2s
%163 = OpConvertSToF %float %164
%165 = OpFAdd %float %162 %163
%167 = OpLoad %int %h2s
%166 = OpConvertSToF %float %167
%168 = OpFAdd %float %165 %166
%170 = OpLoad %int %f2s
%169 = OpConvertSToF %float %170
%171 = OpFAdd %float %168 %169
%173 = OpLoad %int %b2s
%172 = OpConvertSToF %float %173
%174 = OpFAdd %float %171 %172
%176 = OpLoad %int %s2i
%175 = OpConvertSToF %float %176
%177 = OpFAdd %float %174 %175
%179 = OpLoad %int %i2i
%178 = OpConvertSToF %float %179
%180 = OpFAdd %float %177 %178
%182 = OpLoad %int %us2i
%181 = OpConvertSToF %float %182
%183 = OpFAdd %float %180 %181
%185 = OpLoad %int %ui2i
%184 = OpConvertSToF %float %185
%186 = OpFAdd %float %183 %184
%188 = OpLoad %int %h2i
%187 = OpConvertSToF %float %188
%189 = OpFAdd %float %186 %187
%191 = OpLoad %int %f2i
%190 = OpConvertSToF %float %191
%192 = OpFAdd %float %189 %190
%194 = OpLoad %int %b2i
%193 = OpConvertSToF %float %194
%195 = OpFAdd %float %192 %193
%197 = OpLoad %uint %s2us
%196 = OpConvertUToF %float %197
%198 = OpFAdd %float %195 %196
%200 = OpLoad %uint %i2us
%199 = OpConvertUToF %float %200
%201 = OpFAdd %float %198 %199
%203 = OpLoad %uint %us2us
%202 = OpConvertUToF %float %203
%204 = OpFAdd %float %201 %202
%205 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %205 %204
%207 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%208 = OpLoad %float %207
%210 = OpLoad %uint %ui2us
%211 = OpLoad %uint %h2us
%212 = OpIAdd %uint %210 %211
%213 = OpLoad %uint %f2us
%214 = OpIAdd %uint %212 %213
%215 = OpLoad %uint %b2us
%216 = OpIAdd %uint %214 %215
%209 = OpConvertUToF %float %216
%218 = OpLoad %uint %s2ui
%217 = OpConvertUToF %float %218
%219 = OpFAdd %float %209 %217
%221 = OpLoad %uint %i2ui
%220 = OpConvertUToF %float %221
%222 = OpFAdd %float %219 %220
%224 = OpLoad %uint %us2ui
%223 = OpConvertUToF %float %224
%225 = OpFAdd %float %222 %223
%227 = OpLoad %uint %ui2ui
%226 = OpConvertUToF %float %227
%228 = OpFAdd %float %225 %226
%230 = OpLoad %uint %h2ui
%229 = OpConvertUToF %float %230
%231 = OpFAdd %float %228 %229
%233 = OpLoad %uint %f2ui
%232 = OpConvertUToF %float %233
%234 = OpFAdd %float %231 %232
%236 = OpLoad %uint %b2ui
%235 = OpConvertUToF %float %236
%237 = OpFAdd %float %234 %235
%238 = OpLoad %float %s2f
%239 = OpFAdd %float %237 %238
%240 = OpLoad %float %i2f
%241 = OpFAdd %float %239 %240
%242 = OpLoad %float %us2f
%243 = OpFAdd %float %241 %242
%244 = OpLoad %float %ui2f
%245 = OpFAdd %float %243 %244
%246 = OpLoad %float %h2f
%247 = OpFAdd %float %245 %246
%248 = OpLoad %float %f2f
%249 = OpFAdd %float %247 %248
%250 = OpLoad %float %b2f
%251 = OpFAdd %float %249 %250
%252 = OpFAdd %float %208 %251
OpStore %207 %252
OpReturn
OpFunctionEnd
