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
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
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
%145 = OpConstantComposite %v4float %float_1_0 %float_1_0 %float_1_0 %float_1_0
%float_0 = OpConstant %float 0
%147 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%main = OpFunction %void None %11
%12 = OpLabel
%v = OpVariable %_ptr_Function_v4bool Function
%result = OpVariable %_ptr_Function_v4bool Function
%140 = OpVariable %_ptr_Function_v4float Function
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
%34 = OpCompositeConstruct %v4bool %33 %true %true %false
OpStore %result %34
%35 = OpLoad %v4bool %v
%36 = OpCompositeExtract %bool %35 1
%37 = OpCompositeConstruct %v4bool %false %36 %true %true
OpStore %result %37
%38 = OpLoad %v4bool %v
%39 = OpVectorShuffle %v3bool %38 %38 0 1 2
%41 = OpCompositeExtract %bool %39 0
%42 = OpCompositeExtract %bool %39 1
%43 = OpCompositeExtract %bool %39 2
%44 = OpCompositeConstruct %v4bool %41 %42 %43 %true
OpStore %result %44
%45 = OpLoad %v4bool %v
%46 = OpVectorShuffle %v2bool %45 %45 0 1
%47 = OpCompositeExtract %bool %46 0
%48 = OpCompositeExtract %bool %46 1
%49 = OpCompositeConstruct %v4bool %47 %48 %true %true
OpStore %result %49
%50 = OpLoad %v4bool %v
%51 = OpCompositeExtract %bool %50 0
%52 = OpLoad %v4bool %v
%53 = OpCompositeExtract %bool %52 2
%54 = OpCompositeConstruct %v4bool %51 %false %53 %true
OpStore %result %54
%55 = OpLoad %v4bool %v
%56 = OpCompositeExtract %bool %55 0
%57 = OpCompositeConstruct %v4bool %56 %true %false %false
OpStore %result %57
%58 = OpLoad %v4bool %v
%59 = OpVectorShuffle %v2bool %58 %58 1 2
%60 = OpCompositeExtract %bool %59 0
%61 = OpCompositeExtract %bool %59 1
%62 = OpCompositeConstruct %v4bool %true %60 %61 %false
OpStore %result %62
%63 = OpLoad %v4bool %v
%64 = OpCompositeExtract %bool %63 1
%65 = OpCompositeConstruct %v4bool %false %64 %true %false
OpStore %result %65
%66 = OpLoad %v4bool %v
%67 = OpCompositeExtract %bool %66 2
%68 = OpCompositeConstruct %v4bool %true %true %67 %false
OpStore %result %68
%69 = OpLoad %v4bool %v
OpStore %result %69
%70 = OpLoad %v4bool %v
%71 = OpVectorShuffle %v3bool %70 %70 0 1 2
%72 = OpCompositeExtract %bool %71 0
%73 = OpCompositeExtract %bool %71 1
%74 = OpCompositeExtract %bool %71 2
%75 = OpCompositeConstruct %v4bool %72 %73 %74 %true
OpStore %result %75
%76 = OpLoad %v4bool %v
%77 = OpVectorShuffle %v2bool %76 %76 0 1
%78 = OpCompositeExtract %bool %77 0
%79 = OpCompositeExtract %bool %77 1
%80 = OpLoad %v4bool %v
%81 = OpCompositeExtract %bool %80 3
%82 = OpCompositeConstruct %v4bool %78 %79 %false %81
OpStore %result %82
%83 = OpLoad %v4bool %v
%84 = OpVectorShuffle %v2bool %83 %83 0 1
%85 = OpCompositeExtract %bool %84 0
%86 = OpCompositeExtract %bool %84 1
%87 = OpCompositeConstruct %v4bool %85 %86 %true %false
OpStore %result %87
%88 = OpLoad %v4bool %v
%89 = OpCompositeExtract %bool %88 0
%90 = OpLoad %v4bool %v
%91 = OpVectorShuffle %v2bool %90 %90 2 3
%92 = OpCompositeExtract %bool %91 0
%93 = OpCompositeExtract %bool %91 1
%94 = OpCompositeConstruct %v4bool %89 %true %92 %93
OpStore %result %94
%95 = OpLoad %v4bool %v
%96 = OpCompositeExtract %bool %95 0
%97 = OpLoad %v4bool %v
%98 = OpCompositeExtract %bool %97 2
%99 = OpCompositeConstruct %v4bool %96 %false %98 %true
OpStore %result %99
%100 = OpLoad %v4bool %v
%101 = OpCompositeExtract %bool %100 0
%102 = OpLoad %v4bool %v
%103 = OpCompositeExtract %bool %102 3
%104 = OpCompositeConstruct %v4bool %101 %true %true %103
OpStore %result %104
%105 = OpLoad %v4bool %v
%106 = OpCompositeExtract %bool %105 0
%107 = OpCompositeConstruct %v4bool %106 %true %false %true
OpStore %result %107
%108 = OpLoad %v4bool %v
%109 = OpVectorShuffle %v3bool %108 %108 1 2 3
%110 = OpCompositeExtract %bool %109 0
%111 = OpCompositeExtract %bool %109 1
%112 = OpCompositeExtract %bool %109 2
%113 = OpCompositeConstruct %v4bool %true %110 %111 %112
OpStore %result %113
%114 = OpLoad %v4bool %v
%115 = OpVectorShuffle %v2bool %114 %114 1 2
%116 = OpCompositeExtract %bool %115 0
%117 = OpCompositeExtract %bool %115 1
%118 = OpCompositeConstruct %v4bool %false %116 %117 %true
OpStore %result %118
%119 = OpLoad %v4bool %v
%120 = OpCompositeExtract %bool %119 1
%121 = OpLoad %v4bool %v
%122 = OpCompositeExtract %bool %121 3
%123 = OpCompositeConstruct %v4bool %false %120 %true %122
OpStore %result %123
%124 = OpLoad %v4bool %v
%125 = OpCompositeExtract %bool %124 1
%126 = OpCompositeConstruct %v4bool %true %125 %true %true
OpStore %result %126
%127 = OpLoad %v4bool %v
%128 = OpVectorShuffle %v2bool %127 %127 2 3
%129 = OpCompositeExtract %bool %128 0
%130 = OpCompositeExtract %bool %128 1
%131 = OpCompositeConstruct %v4bool %false %false %129 %130
OpStore %result %131
%132 = OpLoad %v4bool %v
%133 = OpCompositeExtract %bool %132 2
%134 = OpCompositeConstruct %v4bool %false %false %133 %true
OpStore %result %134
%135 = OpLoad %v4bool %v
%136 = OpCompositeExtract %bool %135 3
%137 = OpCompositeConstruct %v4bool %false %true %true %136
OpStore %result %137
%139 = OpLoad %v4bool %result
%138 = OpAny %bool %139
OpSelectionMerge %144 None
OpBranchConditional %138 %142 %143
%142 = OpLabel
OpStore %140 %145
OpBranch %144
%143 = OpLabel
OpStore %140 %147
OpBranch %144
%144 = OpLabel
%149 = OpLoad %v4float %140
OpStore %sk_FragColor %149
OpReturn
OpFunctionEnd
