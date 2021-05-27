OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %huge "huge"
OpName %hugeI "hugeI"
OpName %hugeU "hugeU"
OpName %hugeS "hugeS"
OpName %hugeUS "hugeUS"
OpName %hugeNI "hugeNI"
OpName %hugeNS "hugeNS"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %huge RelaxedPrecision
OpDecorate %hugeS RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %hugeUS RelaxedPrecision
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
OpDecorate %125 RelaxedPrecision
OpDecorate %hugeNS RelaxedPrecision
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
OpDecorate %164 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%huge = OpVariable %_ptr_Function_float Function
%hugeI = OpVariable %_ptr_Function_int Function
%hugeU = OpVariable %_ptr_Function_uint Function
%hugeS = OpVariable %_ptr_Function_int Function
%hugeUS = OpVariable %_ptr_Function_uint Function
%hugeNI = OpVariable %_ptr_Function_int Function
%hugeNS = OpVariable %_ptr_Function_int Function
%30 = OpFMul %float %float_9_99999962e_35 %float_1e_09
%31 = OpFMul %float %30 %float_1e_09
%32 = OpFMul %float %31 %float_1e_09
%33 = OpFMul %float %32 %float_1e_09
%34 = OpFMul %float %33 %float_1e_09
%35 = OpFMul %float %34 %float_1e_09
%36 = OpFMul %float %35 %float_1e_09
%37 = OpFMul %float %36 %float_1e_09
%38 = OpFMul %float %37 %float_1e_09
%39 = OpFMul %float %38 %float_1e_09
OpStore %huge %39
%45 = OpIMul %int %int_1073741824 %int_2
%46 = OpIMul %int %45 %int_2
%47 = OpIMul %int %46 %int_2
%48 = OpIMul %int %47 %int_2
%49 = OpIMul %int %48 %int_2
%50 = OpIMul %int %49 %int_2
%51 = OpIMul %int %50 %int_2
%52 = OpIMul %int %51 %int_2
%53 = OpIMul %int %52 %int_2
%54 = OpIMul %int %53 %int_2
%55 = OpIMul %int %54 %int_2
%56 = OpIMul %int %55 %int_2
%57 = OpIMul %int %56 %int_2
%58 = OpIMul %int %57 %int_2
%59 = OpIMul %int %58 %int_2
%60 = OpIMul %int %59 %int_2
%61 = OpIMul %int %60 %int_2
%62 = OpIMul %int %61 %int_2
%63 = OpIMul %int %62 %int_2
%64 = OpIMul %int %63 %int_2
OpStore %hugeI %64
%70 = OpIMul %uint %uint_2147483648 %uint_2
%71 = OpIMul %uint %70 %uint_2
%72 = OpIMul %uint %71 %uint_2
%73 = OpIMul %uint %72 %uint_2
%74 = OpIMul %uint %73 %uint_2
%75 = OpIMul %uint %74 %uint_2
%76 = OpIMul %uint %75 %uint_2
%77 = OpIMul %uint %76 %uint_2
%78 = OpIMul %uint %77 %uint_2
%79 = OpIMul %uint %78 %uint_2
%80 = OpIMul %uint %79 %uint_2
%81 = OpIMul %uint %80 %uint_2
%82 = OpIMul %uint %81 %uint_2
%83 = OpIMul %uint %82 %uint_2
%84 = OpIMul %uint %83 %uint_2
%85 = OpIMul %uint %84 %uint_2
%86 = OpIMul %uint %85 %uint_2
%87 = OpIMul %uint %86 %uint_2
%88 = OpIMul %uint %87 %uint_2
OpStore %hugeU %88
%91 = OpIMul %int %int_16384 %int_2
%92 = OpIMul %int %91 %int_2
%93 = OpIMul %int %92 %int_2
%94 = OpIMul %int %93 %int_2
%95 = OpIMul %int %94 %int_2
%96 = OpIMul %int %95 %int_2
%97 = OpIMul %int %96 %int_2
%98 = OpIMul %int %97 %int_2
%99 = OpIMul %int %98 %int_2
%100 = OpIMul %int %99 %int_2
%101 = OpIMul %int %100 %int_2
%102 = OpIMul %int %101 %int_2
%103 = OpIMul %int %102 %int_2
%104 = OpIMul %int %103 %int_2
%105 = OpIMul %int %104 %int_2
%106 = OpIMul %int %105 %int_2
%107 = OpIMul %int %106 %int_2
OpStore %hugeS %107
%110 = OpIMul %uint %uint_32768 %uint_2
%111 = OpIMul %uint %110 %uint_2
%112 = OpIMul %uint %111 %uint_2
%113 = OpIMul %uint %112 %uint_2
%114 = OpIMul %uint %113 %uint_2
%115 = OpIMul %uint %114 %uint_2
%116 = OpIMul %uint %115 %uint_2
%117 = OpIMul %uint %116 %uint_2
%118 = OpIMul %uint %117 %uint_2
%119 = OpIMul %uint %118 %uint_2
%120 = OpIMul %uint %119 %uint_2
%121 = OpIMul %uint %120 %uint_2
%122 = OpIMul %uint %121 %uint_2
%123 = OpIMul %uint %122 %uint_2
%124 = OpIMul %uint %123 %uint_2
%125 = OpIMul %uint %124 %uint_2
OpStore %hugeUS %125
%128 = OpIMul %int %int_n2147483648 %int_2
%129 = OpIMul %int %128 %int_2
%130 = OpIMul %int %129 %int_2
%131 = OpIMul %int %130 %int_2
%132 = OpIMul %int %131 %int_2
%133 = OpIMul %int %132 %int_2
%134 = OpIMul %int %133 %int_2
%135 = OpIMul %int %134 %int_2
%136 = OpIMul %int %135 %int_2
%137 = OpIMul %int %136 %int_2
%138 = OpIMul %int %137 %int_2
%139 = OpIMul %int %138 %int_2
%140 = OpIMul %int %139 %int_2
%141 = OpIMul %int %140 %int_2
%142 = OpIMul %int %141 %int_2
%143 = OpIMul %int %142 %int_2
%144 = OpIMul %int %143 %int_2
%145 = OpIMul %int %144 %int_2
%146 = OpIMul %int %145 %int_2
OpStore %hugeNI %146
%149 = OpIMul %int %int_n32768 %int_2
%150 = OpIMul %int %149 %int_2
%151 = OpIMul %int %150 %int_2
%152 = OpIMul %int %151 %int_2
%153 = OpIMul %int %152 %int_2
%154 = OpIMul %int %153 %int_2
%155 = OpIMul %int %154 %int_2
%156 = OpIMul %int %155 %int_2
%157 = OpIMul %int %156 %int_2
%158 = OpIMul %int %157 %int_2
%159 = OpIMul %int %158 %int_2
%160 = OpIMul %int %159 %int_2
%161 = OpIMul %int %160 %int_2
%162 = OpIMul %int %161 %int_2
%163 = OpIMul %int %162 %int_2
%164 = OpIMul %int %163 %int_2
OpStore %hugeNS %164
%165 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%168 = OpLoad %v4float %165
%170 = OpLoad %float %huge
%169 = OpExtInst %float %1 FClamp %170 %float_0 %float_1
%172 = OpVectorTimesScalar %v4float %168 %169
%174 = OpLoad %int %hugeI
%175 = OpConvertSToF %float %174
%173 = OpExtInst %float %1 FClamp %175 %float_0 %float_1
%176 = OpVectorTimesScalar %v4float %172 %173
%178 = OpLoad %uint %hugeU
%179 = OpConvertUToF %float %178
%177 = OpExtInst %float %1 FClamp %179 %float_0 %float_1
%180 = OpVectorTimesScalar %v4float %176 %177
%182 = OpLoad %int %hugeS
%183 = OpConvertSToF %float %182
%181 = OpExtInst %float %1 FClamp %183 %float_0 %float_1
%184 = OpVectorTimesScalar %v4float %180 %181
%186 = OpLoad %uint %hugeUS
%187 = OpConvertUToF %float %186
%185 = OpExtInst %float %1 FClamp %187 %float_0 %float_1
%188 = OpVectorTimesScalar %v4float %184 %185
%190 = OpLoad %int %hugeNI
%191 = OpConvertSToF %float %190
%189 = OpExtInst %float %1 FClamp %191 %float_0 %float_1
%192 = OpVectorTimesScalar %v4float %188 %189
%194 = OpLoad %int %hugeNS
%195 = OpConvertSToF %float %194
%193 = OpExtInst %float %1 FClamp %195 %float_0 %float_1
%196 = OpVectorTimesScalar %v4float %192 %193
OpReturnValue %196
OpFunctionEnd
