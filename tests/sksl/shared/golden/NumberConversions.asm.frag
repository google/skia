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
OpDecorate %143 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
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
%140 = OpLoad %int %i
%141 = OpIAdd %int %139 %140
%143 = OpLoad %uint %us
%142 = OpBitcast %int %143
%144 = OpIAdd %int %141 %142
%138 = OpConvertSToF %float %144
%146 = OpLoad %uint %ui
%145 = OpConvertUToF %float %146
%147 = OpFAdd %float %138 %145
%148 = OpLoad %float %h
%149 = OpFAdd %float %147 %148
%150 = OpLoad %float %f
%151 = OpFAdd %float %149 %150
%153 = OpLoad %int %s2s
%152 = OpConvertSToF %float %153
%154 = OpFAdd %float %151 %152
%156 = OpLoad %int %i2s
%155 = OpConvertSToF %float %156
%157 = OpFAdd %float %154 %155
%159 = OpLoad %int %us2s
%158 = OpConvertSToF %float %159
%160 = OpFAdd %float %157 %158
%162 = OpLoad %int %ui2s
%161 = OpConvertSToF %float %162
%163 = OpFAdd %float %160 %161
%165 = OpLoad %int %h2s
%164 = OpConvertSToF %float %165
%166 = OpFAdd %float %163 %164
%168 = OpLoad %int %f2s
%167 = OpConvertSToF %float %168
%169 = OpFAdd %float %166 %167
%171 = OpLoad %int %b2s
%170 = OpConvertSToF %float %171
%172 = OpFAdd %float %169 %170
%174 = OpLoad %int %s2i
%173 = OpConvertSToF %float %174
%175 = OpFAdd %float %172 %173
%177 = OpLoad %int %i2i
%176 = OpConvertSToF %float %177
%178 = OpFAdd %float %175 %176
%180 = OpLoad %int %us2i
%179 = OpConvertSToF %float %180
%181 = OpFAdd %float %178 %179
%183 = OpLoad %int %ui2i
%182 = OpConvertSToF %float %183
%184 = OpFAdd %float %181 %182
%186 = OpLoad %int %h2i
%185 = OpConvertSToF %float %186
%187 = OpFAdd %float %184 %185
%189 = OpLoad %int %f2i
%188 = OpConvertSToF %float %189
%190 = OpFAdd %float %187 %188
%192 = OpLoad %int %b2i
%191 = OpConvertSToF %float %192
%193 = OpFAdd %float %190 %191
%195 = OpLoad %uint %s2us
%194 = OpConvertUToF %float %195
%196 = OpFAdd %float %193 %194
%198 = OpLoad %uint %i2us
%197 = OpConvertUToF %float %198
%199 = OpFAdd %float %196 %197
%201 = OpLoad %uint %us2us
%200 = OpConvertUToF %float %201
%202 = OpFAdd %float %199 %200
%203 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %203 %202
%205 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%206 = OpLoad %float %205
%208 = OpLoad %uint %ui2us
%209 = OpLoad %uint %h2us
%210 = OpIAdd %uint %208 %209
%211 = OpLoad %uint %f2us
%212 = OpIAdd %uint %210 %211
%213 = OpLoad %uint %b2us
%214 = OpIAdd %uint %212 %213
%207 = OpConvertUToF %float %214
%216 = OpLoad %uint %s2ui
%215 = OpConvertUToF %float %216
%217 = OpFAdd %float %207 %215
%219 = OpLoad %uint %i2ui
%218 = OpConvertUToF %float %219
%220 = OpFAdd %float %217 %218
%222 = OpLoad %uint %us2ui
%221 = OpConvertUToF %float %222
%223 = OpFAdd %float %220 %221
%225 = OpLoad %uint %ui2ui
%224 = OpConvertUToF %float %225
%226 = OpFAdd %float %223 %224
%228 = OpLoad %uint %h2ui
%227 = OpConvertUToF %float %228
%229 = OpFAdd %float %226 %227
%231 = OpLoad %uint %f2ui
%230 = OpConvertUToF %float %231
%232 = OpFAdd %float %229 %230
%234 = OpLoad %uint %b2ui
%233 = OpConvertUToF %float %234
%235 = OpFAdd %float %232 %233
%236 = OpLoad %float %s2f
%237 = OpFAdd %float %235 %236
%238 = OpLoad %float %i2f
%239 = OpFAdd %float %237 %238
%240 = OpLoad %float %us2f
%241 = OpFAdd %float %239 %240
%242 = OpLoad %float %ui2f
%243 = OpFAdd %float %241 %242
%244 = OpLoad %float %h2f
%245 = OpFAdd %float %243 %244
%246 = OpLoad %float %f2f
%247 = OpFAdd %float %245 %246
%248 = OpLoad %float %b2f
%249 = OpFAdd %float %247 %248
%250 = OpFAdd %float %206 %249
OpStore %205 %250
OpReturn
OpFunctionEnd
