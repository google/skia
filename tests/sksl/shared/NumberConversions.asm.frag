OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
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
OpDecorate %s RelaxedPrecision
OpDecorate %us RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %s2s RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %i2s RelaxedPrecision
OpDecorate %us2s RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %ui2s RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %h2s RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %f2s RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %b2s RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %s2us RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %i2us RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %us2us RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %ui2us RelaxedPrecision
OpDecorate %h2us RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %f2us RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %b2us RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
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
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_1 = OpConstant %uint 1
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%int_0 = OpConstant %int 0
%uint_0 = OpConstant %uint 0
%float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%main = OpFunction %void None %11
%12 = OpLabel
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
OpStore %s %int_1
OpStore %i %int_1
OpStore %us %uint_1
OpStore %ui %uint_1
OpStore %h %float_1
OpStore %f %float_1
%31 = OpLoad %int %s
OpStore %s2s %31
%33 = OpLoad %int %i
OpStore %i2s %33
%35 = OpLoad %uint %us
%36 = OpBitcast %int %35
OpStore %us2s %36
%38 = OpLoad %uint %ui
%39 = OpBitcast %int %38
OpStore %ui2s %39
%41 = OpLoad %float %h
%42 = OpConvertFToS %int %41
OpStore %h2s %42
%44 = OpLoad %float %f
%45 = OpConvertFToS %int %44
OpStore %f2s %45
%47 = OpLoad %bool %b
%48 = OpSelect %int %47 %int_1 %int_0
OpStore %b2s %48
%51 = OpLoad %int %s
OpStore %s2i %51
%53 = OpLoad %int %i
OpStore %i2i %53
%55 = OpLoad %uint %us
%56 = OpBitcast %int %55
OpStore %us2i %56
%58 = OpLoad %uint %ui
%59 = OpBitcast %int %58
OpStore %ui2i %59
%61 = OpLoad %float %h
%62 = OpConvertFToS %int %61
OpStore %h2i %62
%64 = OpLoad %float %f
%65 = OpConvertFToS %int %64
OpStore %f2i %65
%67 = OpLoad %bool %b
%68 = OpSelect %int %67 %int_1 %int_0
OpStore %b2i %68
%70 = OpLoad %int %s
%71 = OpBitcast %uint %70
OpStore %s2us %71
%73 = OpLoad %int %i
%74 = OpBitcast %uint %73
OpStore %i2us %74
%76 = OpLoad %uint %us
OpStore %us2us %76
%78 = OpLoad %uint %ui
OpStore %ui2us %78
%80 = OpLoad %float %h
%81 = OpConvertFToU %uint %80
OpStore %h2us %81
%83 = OpLoad %float %f
%84 = OpConvertFToU %uint %83
OpStore %f2us %84
%86 = OpLoad %bool %b
%87 = OpSelect %uint %86 %uint_1 %uint_0
OpStore %b2us %87
%90 = OpLoad %int %s
%91 = OpBitcast %uint %90
OpStore %s2ui %91
%93 = OpLoad %int %i
%94 = OpBitcast %uint %93
OpStore %i2ui %94
%96 = OpLoad %uint %us
OpStore %us2ui %96
%98 = OpLoad %uint %ui
OpStore %ui2ui %98
%100 = OpLoad %float %h
%101 = OpConvertFToU %uint %100
OpStore %h2ui %101
%103 = OpLoad %float %f
%104 = OpConvertFToU %uint %103
OpStore %f2ui %104
%106 = OpLoad %bool %b
%107 = OpSelect %uint %106 %uint_1 %uint_0
OpStore %b2ui %107
%109 = OpLoad %int %s
%110 = OpConvertSToF %float %109
OpStore %s2f %110
%112 = OpLoad %int %i
%113 = OpConvertSToF %float %112
OpStore %i2f %113
%115 = OpLoad %uint %us
%116 = OpConvertUToF %float %115
OpStore %us2f %116
%118 = OpLoad %uint %ui
%119 = OpConvertUToF %float %118
OpStore %ui2f %119
%121 = OpLoad %float %h
OpStore %h2f %121
%123 = OpLoad %float %f
OpStore %f2f %123
%125 = OpLoad %bool %b
%126 = OpSelect %float %125 %float_1 %float_0
OpStore %b2f %126
%128 = OpLoad %int %s
%129 = OpConvertSToF %float %128
%130 = OpLoad %int %i
%131 = OpConvertSToF %float %130
%132 = OpFAdd %float %129 %131
%133 = OpLoad %uint %us
%134 = OpConvertUToF %float %133
%135 = OpFAdd %float %132 %134
%136 = OpLoad %uint %ui
%137 = OpConvertUToF %float %136
%138 = OpFAdd %float %135 %137
%139 = OpLoad %float %h
%140 = OpFAdd %float %138 %139
%141 = OpLoad %float %f
%142 = OpFAdd %float %140 %141
%143 = OpLoad %int %s2s
%144 = OpConvertSToF %float %143
%145 = OpFAdd %float %142 %144
%146 = OpLoad %int %i2s
%147 = OpConvertSToF %float %146
%148 = OpFAdd %float %145 %147
%149 = OpLoad %int %us2s
%150 = OpConvertSToF %float %149
%151 = OpFAdd %float %148 %150
%152 = OpLoad %int %ui2s
%153 = OpConvertSToF %float %152
%154 = OpFAdd %float %151 %153
%155 = OpLoad %int %h2s
%156 = OpConvertSToF %float %155
%157 = OpFAdd %float %154 %156
%158 = OpLoad %int %f2s
%159 = OpConvertSToF %float %158
%160 = OpFAdd %float %157 %159
%161 = OpLoad %int %b2s
%162 = OpConvertSToF %float %161
%163 = OpFAdd %float %160 %162
%164 = OpLoad %int %s2i
%165 = OpConvertSToF %float %164
%166 = OpFAdd %float %163 %165
%167 = OpLoad %int %i2i
%168 = OpConvertSToF %float %167
%169 = OpFAdd %float %166 %168
%170 = OpLoad %int %us2i
%171 = OpConvertSToF %float %170
%172 = OpFAdd %float %169 %171
%173 = OpLoad %int %ui2i
%174 = OpConvertSToF %float %173
%175 = OpFAdd %float %172 %174
%176 = OpLoad %int %h2i
%177 = OpConvertSToF %float %176
%178 = OpFAdd %float %175 %177
%179 = OpLoad %int %f2i
%180 = OpConvertSToF %float %179
%181 = OpFAdd %float %178 %180
%182 = OpLoad %int %b2i
%183 = OpConvertSToF %float %182
%184 = OpFAdd %float %181 %183
%185 = OpLoad %uint %s2us
%186 = OpConvertUToF %float %185
%187 = OpFAdd %float %184 %186
%188 = OpLoad %uint %i2us
%189 = OpConvertUToF %float %188
%190 = OpFAdd %float %187 %189
%191 = OpLoad %uint %us2us
%192 = OpConvertUToF %float %191
%193 = OpFAdd %float %190 %192
%194 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %194 %193
%196 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%197 = OpLoad %float %196
%198 = OpLoad %uint %ui2us
%199 = OpConvertUToF %float %198
%200 = OpLoad %uint %h2us
%201 = OpConvertUToF %float %200
%202 = OpFAdd %float %199 %201
%203 = OpLoad %uint %f2us
%204 = OpConvertUToF %float %203
%205 = OpFAdd %float %202 %204
%206 = OpLoad %uint %b2us
%207 = OpConvertUToF %float %206
%208 = OpFAdd %float %205 %207
%209 = OpLoad %uint %s2ui
%210 = OpConvertUToF %float %209
%211 = OpFAdd %float %208 %210
%212 = OpLoad %uint %i2ui
%213 = OpConvertUToF %float %212
%214 = OpFAdd %float %211 %213
%215 = OpLoad %uint %us2ui
%216 = OpConvertUToF %float %215
%217 = OpFAdd %float %214 %216
%218 = OpLoad %uint %ui2ui
%219 = OpConvertUToF %float %218
%220 = OpFAdd %float %217 %219
%221 = OpLoad %uint %h2ui
%222 = OpConvertUToF %float %221
%223 = OpFAdd %float %220 %222
%224 = OpLoad %uint %f2ui
%225 = OpConvertUToF %float %224
%226 = OpFAdd %float %223 %225
%227 = OpLoad %uint %b2ui
%228 = OpConvertUToF %float %227
%229 = OpFAdd %float %226 %228
%230 = OpLoad %float %s2f
%231 = OpFAdd %float %229 %230
%232 = OpLoad %float %i2f
%233 = OpFAdd %float %231 %232
%234 = OpLoad %float %us2f
%235 = OpFAdd %float %233 %234
%236 = OpLoad %float %ui2f
%237 = OpFAdd %float %235 %236
%238 = OpLoad %float %h2f
%239 = OpFAdd %float %237 %238
%240 = OpLoad %float %f2f
%241 = OpFAdd %float %239 %240
%242 = OpLoad %float %b2f
%243 = OpFAdd %float %241 %242
%244 = OpFAdd %float %197 %243
OpStore %196 %244
OpReturn
OpFunctionEnd
