OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %g "g"
OpName %h "h"
OpName %i "i"
OpName %j "j"
OpName %k "k"
OpName %l "l"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%34 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%mat2v3float = OpTypeMatrix %v3float 2
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%51 = OpConstantComposite %v2float %float_1 %float_0
%52 = OpConstantComposite %v2float %float_0 %float_1
%mat3v2float = OpTypeMatrix %v2float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%98 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%99 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
%mat2v4float = OpTypeMatrix %v4float 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
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
%result = OpVariable %_ptr_Function_float Function
%g = OpVariable %_ptr_Function_mat3v3float Function
%h = OpVariable %_ptr_Function_mat3v3float Function
%i = OpVariable %_ptr_Function_mat4v4float Function
%j = OpVariable %_ptr_Function_mat4v4float Function
%k = OpVariable %_ptr_Function_mat2v4float Function
%l = OpVariable %_ptr_Function_mat4v2float Function
%147 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%33 = OpCompositeConstruct %mat2v3float %34 %35
%38 = OpCompositeExtract %v3float %33 0
%39 = OpCompositeExtract %v3float %33 1
%40 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%37 = OpCompositeConstruct %mat3v3float %38 %39 %40
OpStore %g %37
%41 = OpLoad %float %result
%44 = OpAccessChain %_ptr_Function_v3float %g %int_0
%46 = OpLoad %v3float %44
%47 = OpCompositeExtract %float %46 0
%48 = OpFAdd %float %41 %47
OpStore %result %48
%50 = OpCompositeConstruct %mat3v2float %51 %52 %19
%55 = OpCompositeExtract %v2float %50 0
%56 = OpCompositeConstruct %v3float %55 %float_0
%57 = OpCompositeExtract %v2float %50 1
%58 = OpCompositeConstruct %v3float %57 %float_0
%59 = OpCompositeExtract %v2float %50 2
%60 = OpCompositeConstruct %v3float %59 %float_1
%54 = OpCompositeConstruct %mat3v3float %56 %58 %60
OpStore %h %54
%61 = OpLoad %float %result
%62 = OpAccessChain %_ptr_Function_v3float %h %int_0
%63 = OpLoad %v3float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpFAdd %float %61 %64
OpStore %result %65
%69 = OpCompositeConstruct %mat4v2float %51 %52 %19 %19
%72 = OpCompositeExtract %v2float %69 0
%73 = OpCompositeConstruct %v3float %72 %float_0
%74 = OpCompositeExtract %v2float %69 1
%75 = OpCompositeConstruct %v3float %74 %float_0
%76 = OpCompositeExtract %v2float %69 2
%77 = OpCompositeConstruct %v3float %76 %float_1
%78 = OpCompositeExtract %v2float %69 3
%79 = OpCompositeConstruct %v3float %78 %float_0
%71 = OpCompositeConstruct %mat4v3float %73 %75 %77 %79
%82 = OpCompositeExtract %v3float %71 0
%83 = OpCompositeConstruct %v4float %82 %float_0
%84 = OpCompositeExtract %v3float %71 1
%85 = OpCompositeConstruct %v4float %84 %float_0
%86 = OpCompositeExtract %v3float %71 2
%87 = OpCompositeConstruct %v4float %86 %float_0
%88 = OpCompositeExtract %v3float %71 3
%89 = OpCompositeConstruct %v4float %88 %float_1
%81 = OpCompositeConstruct %mat4v4float %83 %85 %87 %89
OpStore %i %81
%90 = OpLoad %float %result
%91 = OpAccessChain %_ptr_Function_v4float %i %int_0
%93 = OpLoad %v4float %91
%94 = OpCompositeExtract %float %93 0
%95 = OpFAdd %float %90 %94
OpStore %result %95
%97 = OpCompositeConstruct %mat2v4float %98 %99
%102 = OpCompositeExtract %v4float %97 0
%103 = OpCompositeExtract %v4float %97 1
%104 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%101 = OpCompositeConstruct %mat3v4float %102 %103 %104
%107 = OpCompositeExtract %v4float %101 0
%108 = OpCompositeExtract %v4float %101 1
%109 = OpCompositeExtract %v4float %101 2
%110 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%106 = OpCompositeConstruct %mat4v4float %107 %108 %109 %110
OpStore %j %106
%111 = OpLoad %float %result
%112 = OpAccessChain %_ptr_Function_v4float %j %int_0
%113 = OpLoad %v4float %112
%114 = OpCompositeExtract %float %113 0
%115 = OpFAdd %float %111 %114
OpStore %result %115
%118 = OpCompositeConstruct %mat4v2float %51 %52 %19 %19
%120 = OpCompositeExtract %v2float %118 0
%121 = OpCompositeConstruct %v4float %120 %float_0 %float_0
%122 = OpCompositeExtract %v2float %118 1
%123 = OpCompositeConstruct %v4float %122 %float_0 %float_0
%119 = OpCompositeConstruct %mat2v4float %121 %123
OpStore %k %119
%124 = OpLoad %float %result
%125 = OpAccessChain %_ptr_Function_v4float %k %int_0
%126 = OpLoad %v4float %125
%127 = OpCompositeExtract %float %126 0
%128 = OpFAdd %float %124 %127
OpStore %result %128
%131 = OpCompositeConstruct %mat2v4float %98 %99
%133 = OpCompositeExtract %v4float %131 0
%134 = OpVectorShuffle %v2float %133 %133 0 1
%135 = OpCompositeExtract %v4float %131 1
%136 = OpVectorShuffle %v2float %135 %135 0 1
%137 = OpCompositeConstruct %v2float %float_0 %float_0
%138 = OpCompositeConstruct %v2float %float_0 %float_0
%132 = OpCompositeConstruct %mat4v2float %134 %136 %137 %138
OpStore %l %132
%139 = OpLoad %float %result
%140 = OpAccessChain %_ptr_Function_v2float %l %int_0
%141 = OpLoad %v2float %140
%142 = OpCompositeExtract %float %141 0
%143 = OpFAdd %float %139 %142
OpStore %result %143
%144 = OpLoad %float %result
%146 = OpFOrdEqual %bool %144 %float_6
OpSelectionMerge %150 None
OpBranchConditional %146 %148 %149
%148 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%153 = OpLoad %v4float %151
OpStore %147 %153
OpBranch %150
%149 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%156 = OpLoad %v4float %154
OpStore %147 %156
OpBranch %150
%150 = OpLabel
%157 = OpLoad %v4float %147
OpReturnValue %157
OpFunctionEnd
