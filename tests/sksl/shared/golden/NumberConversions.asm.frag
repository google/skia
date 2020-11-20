OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
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
OpName %s2i "s2i"
OpName %i2i "i2i"
OpName %us2i "us2i"
OpName %ui2i "ui2i"
OpName %h2i "h2i"
OpName %f2i "f2i"
OpName %s2us "s2us"
OpName %i2us "i2us"
OpName %us2us "us2us"
OpName %ui2us "ui2us"
OpName %h2us "h2us"
OpName %f2us "f2us"
OpName %s2ui "s2ui"
OpName %i2ui "i2ui"
OpName %us2ui "us2ui"
OpName %ui2ui "ui2ui"
OpName %h2ui "h2ui"
OpName %f2ui "f2ui"
OpName %s2f "s2f"
OpName %i2f "i2f"
OpName %us2f "us2f"
OpName %ui2f "ui2f"
OpName %h2f "h2f"
OpName %f2f "f2f"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %s RelaxedPrecision
OpDecorate %us RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %s2s RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %i2s RelaxedPrecision
OpDecorate %us2s RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %ui2s RelaxedPrecision
OpDecorate %h2s RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %f2s RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %s2us RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %i2us RelaxedPrecision
OpDecorate %us2us RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %ui2us RelaxedPrecision
OpDecorate %h2us RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %f2us RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
%s2i = OpVariable %_ptr_Private_int Private
%i2i = OpVariable %_ptr_Private_int Private
%us2i = OpVariable %_ptr_Private_int Private
%ui2i = OpVariable %_ptr_Private_int Private
%h2i = OpVariable %_ptr_Private_int Private
%f2i = OpVariable %_ptr_Private_int Private
%s2us = OpVariable %_ptr_Private_uint Private
%i2us = OpVariable %_ptr_Private_uint Private
%us2us = OpVariable %_ptr_Private_uint Private
%ui2us = OpVariable %_ptr_Private_uint Private
%h2us = OpVariable %_ptr_Private_uint Private
%f2us = OpVariable %_ptr_Private_uint Private
%s2ui = OpVariable %_ptr_Private_uint Private
%i2ui = OpVariable %_ptr_Private_uint Private
%us2ui = OpVariable %_ptr_Private_uint Private
%ui2ui = OpVariable %_ptr_Private_uint Private
%h2ui = OpVariable %_ptr_Private_uint Private
%f2ui = OpVariable %_ptr_Private_uint Private
%s2f = OpVariable %_ptr_Private_float Private
%i2f = OpVariable %_ptr_Private_float Private
%us2f = OpVariable %_ptr_Private_float Private
%ui2f = OpVariable %_ptr_Private_float Private
%h2f = OpVariable %_ptr_Private_float Private
%f2f = OpVariable %_ptr_Private_float Private
%void = OpTypeVoid
%113 = OpTypeFunction %void
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %113
%114 = OpLabel
%14 = OpExtInst %float %1 Sqrt %float_1
%13 = OpConvertFToS %int %14
OpStore %s %13
%18 = OpExtInst %float %1 Sqrt %float_1
%17 = OpConvertFToS %int %18
OpStore %i %17
%23 = OpExtInst %float %1 Sqrt %float_1
%22 = OpConvertFToU %uint %23
OpStore %us %22
%26 = OpExtInst %float %1 Sqrt %float_1
%25 = OpConvertFToU %uint %26
OpStore %ui %25
%29 = OpExtInst %float %1 Sqrt %float_1
OpStore %h %29
%31 = OpExtInst %float %1 Sqrt %float_1
OpStore %f %31
%33 = OpLoad %int %s
OpStore %s2s %33
%35 = OpLoad %int %i
OpStore %i2s %35
%38 = OpLoad %uint %us
%37 = OpBitcast %int %38
OpStore %us2s %37
%41 = OpLoad %uint %ui
%40 = OpBitcast %int %41
OpStore %ui2s %40
%44 = OpLoad %float %h
%43 = OpConvertFToS %int %44
OpStore %h2s %43
%47 = OpLoad %float %f
%46 = OpConvertFToS %int %47
OpStore %f2s %46
%49 = OpLoad %int %s
OpStore %s2i %49
%51 = OpLoad %int %i
OpStore %i2i %51
%54 = OpLoad %uint %us
%53 = OpBitcast %int %54
OpStore %us2i %53
%57 = OpLoad %uint %ui
%56 = OpBitcast %int %57
OpStore %ui2i %56
%60 = OpLoad %float %h
%59 = OpConvertFToS %int %60
OpStore %h2i %59
%63 = OpLoad %float %f
%62 = OpConvertFToS %int %63
OpStore %f2i %62
%66 = OpLoad %int %s
%65 = OpBitcast %uint %66
OpStore %s2us %65
%69 = OpLoad %int %i
%68 = OpBitcast %uint %69
OpStore %i2us %68
%71 = OpLoad %uint %us
OpStore %us2us %71
%73 = OpLoad %uint %ui
OpStore %ui2us %73
%76 = OpLoad %float %h
%75 = OpConvertFToU %uint %76
OpStore %h2us %75
%79 = OpLoad %float %f
%78 = OpConvertFToU %uint %79
OpStore %f2us %78
%82 = OpLoad %int %s
%81 = OpBitcast %uint %82
OpStore %s2ui %81
%85 = OpLoad %int %i
%84 = OpBitcast %uint %85
OpStore %i2ui %84
%87 = OpLoad %uint %us
OpStore %us2ui %87
%89 = OpLoad %uint %ui
OpStore %ui2ui %89
%92 = OpLoad %float %h
%91 = OpConvertFToU %uint %92
OpStore %h2ui %91
%95 = OpLoad %float %f
%94 = OpConvertFToU %uint %95
OpStore %f2ui %94
%98 = OpLoad %int %s
%97 = OpConvertSToF %float %98
OpStore %s2f %97
%101 = OpLoad %int %i
%100 = OpConvertSToF %float %101
OpStore %i2f %100
%104 = OpLoad %uint %us
%103 = OpConvertUToF %float %104
OpStore %us2f %103
%107 = OpLoad %uint %ui
%106 = OpConvertUToF %float %107
OpStore %ui2f %106
%109 = OpLoad %float %h
OpStore %h2f %109
%111 = OpLoad %float %f
OpStore %f2f %111
%116 = OpLoad %int %s
%117 = OpLoad %int %i
%118 = OpIAdd %int %116 %117
%120 = OpLoad %uint %us
%119 = OpBitcast %int %120
%121 = OpIAdd %int %118 %119
%115 = OpConvertSToF %float %121
%123 = OpLoad %uint %ui
%122 = OpConvertUToF %float %123
%124 = OpFAdd %float %115 %122
%125 = OpLoad %float %h
%126 = OpFAdd %float %124 %125
%127 = OpLoad %float %f
%128 = OpFAdd %float %126 %127
%130 = OpLoad %int %s2s
%129 = OpConvertSToF %float %130
%131 = OpFAdd %float %128 %129
%133 = OpLoad %int %i2s
%132 = OpConvertSToF %float %133
%134 = OpFAdd %float %131 %132
%136 = OpLoad %int %us2s
%135 = OpConvertSToF %float %136
%137 = OpFAdd %float %134 %135
%139 = OpLoad %int %ui2s
%138 = OpConvertSToF %float %139
%140 = OpFAdd %float %137 %138
%142 = OpLoad %int %h2s
%141 = OpConvertSToF %float %142
%143 = OpFAdd %float %140 %141
%145 = OpLoad %int %f2s
%144 = OpConvertSToF %float %145
%146 = OpFAdd %float %143 %144
%148 = OpLoad %int %s2i
%147 = OpConvertSToF %float %148
%149 = OpFAdd %float %146 %147
%151 = OpLoad %int %i2i
%150 = OpConvertSToF %float %151
%152 = OpFAdd %float %149 %150
%154 = OpLoad %int %us2i
%153 = OpConvertSToF %float %154
%155 = OpFAdd %float %152 %153
%157 = OpLoad %int %ui2i
%156 = OpConvertSToF %float %157
%158 = OpFAdd %float %155 %156
%160 = OpLoad %int %h2i
%159 = OpConvertSToF %float %160
%161 = OpFAdd %float %158 %159
%163 = OpLoad %int %f2i
%162 = OpConvertSToF %float %163
%164 = OpFAdd %float %161 %162
%166 = OpLoad %uint %s2us
%165 = OpConvertUToF %float %166
%167 = OpFAdd %float %164 %165
%169 = OpLoad %uint %i2us
%168 = OpConvertUToF %float %169
%170 = OpFAdd %float %167 %168
%172 = OpLoad %uint %us2us
%171 = OpConvertUToF %float %172
%173 = OpFAdd %float %170 %171
%174 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %174 %173
%177 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
%178 = OpLoad %float %177
%180 = OpLoad %uint %ui2us
%181 = OpLoad %uint %h2us
%182 = OpIAdd %uint %180 %181
%183 = OpLoad %uint %f2us
%184 = OpIAdd %uint %182 %183
%179 = OpConvertUToF %float %184
%186 = OpLoad %uint %s2ui
%185 = OpConvertUToF %float %186
%187 = OpFAdd %float %179 %185
%189 = OpLoad %uint %i2ui
%188 = OpConvertUToF %float %189
%190 = OpFAdd %float %187 %188
%192 = OpLoad %uint %us2ui
%191 = OpConvertUToF %float %192
%193 = OpFAdd %float %190 %191
%195 = OpLoad %uint %ui2ui
%194 = OpConvertUToF %float %195
%196 = OpFAdd %float %193 %194
%198 = OpLoad %uint %h2ui
%197 = OpConvertUToF %float %198
%199 = OpFAdd %float %196 %197
%201 = OpLoad %uint %f2ui
%200 = OpConvertUToF %float %201
%202 = OpFAdd %float %199 %200
%203 = OpLoad %float %s2f
%204 = OpFAdd %float %202 %203
%205 = OpLoad %float %i2f
%206 = OpFAdd %float %204 %205
%207 = OpLoad %float %us2f
%208 = OpFAdd %float %206 %207
%209 = OpLoad %float %ui2f
%210 = OpFAdd %float %208 %209
%211 = OpLoad %float %h2f
%212 = OpFAdd %float %210 %211
%213 = OpLoad %float %f2f
%214 = OpFAdd %float %212 %213
%215 = OpFAdd %float %178 %214
OpStore %177 %215
OpReturn
OpFunctionEnd
