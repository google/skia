OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %v "v"
OpName %result "result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %21 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1_0 = OpConstant %float 1
%175 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%float_0 = OpConstant %float 0
%177 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %11
%12 = OpLabel
%v = OpVariable %_ptr_Function_v4bool Function
%result = OpVariable %_ptr_Function_v4bool Function
%170 = OpVariable %_ptr_Function_v4float Function
%16 = OpExtInst %float %1 Sqrt %float_1
%18 = OpFOrdEqual %bool %16 %float_1
%19 = OpCompositeConstruct %v4bool %18 %18 %18 %18
OpStore %v %19
%21 = OpLoad %v4bool %v
%22 = OpCompositeExtract %bool %21 0
%24 = OpCompositeConstruct %v4bool %22 %true %true %true
OpStore %result %24
%25 = OpLoad %v4bool %v
%26 = OpVectorShuffle %v2bool %25 %25 0 1
%28 = OpCompositeExtract %bool %26 0
%29 = OpCompositeExtract %bool %26 1
%31 = OpCompositeConstruct %v4bool %28 %29 %false %true
OpStore %result %31
%32 = OpLoad %v4bool %v
%33 = OpCompositeExtract %bool %32 0
%34 = OpCompositeConstruct %v2bool %33 %true
%35 = OpCompositeExtract %bool %34 0
%36 = OpCompositeExtract %bool %34 1
%37 = OpCompositeConstruct %v4bool %35 %36 %true %false
OpStore %result %37
%38 = OpLoad %v4bool %v
%39 = OpCompositeExtract %bool %38 1
%40 = OpCompositeConstruct %v2bool %false %39
%41 = OpCompositeExtract %bool %40 0
%42 = OpCompositeExtract %bool %40 1
%43 = OpCompositeConstruct %v4bool %41 %42 %true %true
OpStore %result %43
%44 = OpLoad %v4bool %v
%45 = OpVectorShuffle %v3bool %44 %44 0 1 2
%47 = OpCompositeExtract %bool %45 0
%48 = OpCompositeExtract %bool %45 1
%49 = OpCompositeExtract %bool %45 2
%50 = OpCompositeConstruct %v4bool %47 %48 %49 %true
OpStore %result %50
%51 = OpLoad %v4bool %v
%52 = OpVectorShuffle %v2bool %51 %51 0 1
%53 = OpCompositeExtract %bool %52 0
%54 = OpCompositeExtract %bool %52 1
%55 = OpCompositeConstruct %v3bool %53 %54 %true
%56 = OpCompositeExtract %bool %55 0
%57 = OpCompositeExtract %bool %55 1
%58 = OpCompositeExtract %bool %55 2
%59 = OpCompositeConstruct %v4bool %56 %57 %58 %true
OpStore %result %59
%60 = OpLoad %v4bool %v
%61 = OpCompositeExtract %bool %60 0
%62 = OpLoad %v4bool %v
%63 = OpCompositeExtract %bool %62 2
%64 = OpCompositeConstruct %v3bool %61 %false %63
%65 = OpCompositeExtract %bool %64 0
%66 = OpCompositeExtract %bool %64 1
%67 = OpCompositeExtract %bool %64 2
%68 = OpCompositeConstruct %v4bool %65 %66 %67 %true
OpStore %result %68
%69 = OpLoad %v4bool %v
%70 = OpCompositeExtract %bool %69 0
%71 = OpCompositeConstruct %v3bool %70 %true %false
%72 = OpCompositeExtract %bool %71 0
%73 = OpCompositeExtract %bool %71 1
%74 = OpCompositeExtract %bool %71 2
%75 = OpCompositeConstruct %v4bool %72 %73 %74 %false
OpStore %result %75
%76 = OpLoad %v4bool %v
%77 = OpVectorShuffle %v2bool %76 %76 1 2
%78 = OpCompositeExtract %bool %77 0
%79 = OpCompositeExtract %bool %77 1
%80 = OpCompositeConstruct %v3bool %true %78 %79
%81 = OpCompositeExtract %bool %80 0
%82 = OpCompositeExtract %bool %80 1
%83 = OpCompositeExtract %bool %80 2
%84 = OpCompositeConstruct %v4bool %81 %82 %83 %false
OpStore %result %84
%85 = OpLoad %v4bool %v
%86 = OpCompositeExtract %bool %85 1
%87 = OpCompositeConstruct %v3bool %false %86 %true
%88 = OpCompositeExtract %bool %87 0
%89 = OpCompositeExtract %bool %87 1
%90 = OpCompositeExtract %bool %87 2
%91 = OpCompositeConstruct %v4bool %88 %89 %90 %false
OpStore %result %91
%92 = OpLoad %v4bool %v
%93 = OpCompositeExtract %bool %92 2
%94 = OpCompositeConstruct %v3bool %true %true %93
%95 = OpCompositeExtract %bool %94 0
%96 = OpCompositeExtract %bool %94 1
%97 = OpCompositeExtract %bool %94 2
%98 = OpCompositeConstruct %v4bool %95 %96 %97 %false
OpStore %result %98
%99 = OpLoad %v4bool %v
OpStore %result %99
%100 = OpLoad %v4bool %v
%101 = OpVectorShuffle %v3bool %100 %100 0 1 2
%102 = OpCompositeExtract %bool %101 0
%103 = OpCompositeExtract %bool %101 1
%104 = OpCompositeExtract %bool %101 2
%105 = OpCompositeConstruct %v4bool %102 %103 %104 %true
OpStore %result %105
%106 = OpLoad %v4bool %v
%107 = OpVectorShuffle %v2bool %106 %106 0 1
%108 = OpCompositeExtract %bool %107 0
%109 = OpCompositeExtract %bool %107 1
%110 = OpLoad %v4bool %v
%111 = OpCompositeExtract %bool %110 3
%112 = OpCompositeConstruct %v4bool %108 %109 %false %111
OpStore %result %112
%113 = OpLoad %v4bool %v
%114 = OpVectorShuffle %v2bool %113 %113 0 1
%115 = OpCompositeExtract %bool %114 0
%116 = OpCompositeExtract %bool %114 1
%117 = OpCompositeConstruct %v4bool %115 %116 %true %false
OpStore %result %117
%118 = OpLoad %v4bool %v
%119 = OpCompositeExtract %bool %118 0
%120 = OpLoad %v4bool %v
%121 = OpVectorShuffle %v2bool %120 %120 2 3
%122 = OpCompositeExtract %bool %121 0
%123 = OpCompositeExtract %bool %121 1
%124 = OpCompositeConstruct %v4bool %119 %true %122 %123
OpStore %result %124
%125 = OpLoad %v4bool %v
%126 = OpCompositeExtract %bool %125 0
%127 = OpLoad %v4bool %v
%128 = OpCompositeExtract %bool %127 2
%129 = OpCompositeConstruct %v4bool %126 %false %128 %true
OpStore %result %129
%130 = OpLoad %v4bool %v
%131 = OpCompositeExtract %bool %130 0
%132 = OpLoad %v4bool %v
%133 = OpCompositeExtract %bool %132 3
%134 = OpCompositeConstruct %v4bool %131 %true %true %133
OpStore %result %134
%135 = OpLoad %v4bool %v
%136 = OpCompositeExtract %bool %135 0
%137 = OpCompositeConstruct %v4bool %136 %true %false %true
OpStore %result %137
%138 = OpLoad %v4bool %v
%139 = OpVectorShuffle %v3bool %138 %138 1 2 3
%140 = OpCompositeExtract %bool %139 0
%141 = OpCompositeExtract %bool %139 1
%142 = OpCompositeExtract %bool %139 2
%143 = OpCompositeConstruct %v4bool %true %140 %141 %142
OpStore %result %143
%144 = OpLoad %v4bool %v
%145 = OpVectorShuffle %v2bool %144 %144 1 2
%146 = OpCompositeExtract %bool %145 0
%147 = OpCompositeExtract %bool %145 1
%148 = OpCompositeConstruct %v4bool %false %146 %147 %true
OpStore %result %148
%149 = OpLoad %v4bool %v
%150 = OpCompositeExtract %bool %149 1
%151 = OpLoad %v4bool %v
%152 = OpCompositeExtract %bool %151 3
%153 = OpCompositeConstruct %v4bool %false %150 %true %152
OpStore %result %153
%154 = OpLoad %v4bool %v
%155 = OpCompositeExtract %bool %154 1
%156 = OpCompositeConstruct %v4bool %true %155 %true %true
OpStore %result %156
%157 = OpLoad %v4bool %v
%158 = OpVectorShuffle %v2bool %157 %157 2 3
%159 = OpCompositeExtract %bool %158 0
%160 = OpCompositeExtract %bool %158 1
%161 = OpCompositeConstruct %v4bool %false %false %159 %160
OpStore %result %161
%162 = OpLoad %v4bool %v
%163 = OpCompositeExtract %bool %162 2
%164 = OpCompositeConstruct %v4bool %false %false %163 %true
OpStore %result %164
%165 = OpLoad %v4bool %v
%166 = OpCompositeExtract %bool %165 3
%167 = OpCompositeConstruct %v4bool %false %true %true %166
OpStore %result %167
%169 = OpLoad %v4bool %result
%168 = OpAny %bool %169
OpSelectionMerge %174 None
OpBranchConditional %168 %172 %173
%172 = OpLabel
OpStore %170 %175
OpBranch %174
%173 = OpLabel
OpStore %170 %177
OpBranch %174
%174 = OpLabel
%179 = OpLoad %v4float %170
OpStore %sk_FragColor %179
OpReturn
OpFunctionEnd
