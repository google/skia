OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main" %3 %sk_InstanceID %sk_VertexID %vcoord_Stage0
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InstanceID "sk_InstanceID"
OpName %sk_VertexID "sk_VertexID"
OpName %vcoord_Stage0 "vcoord_Stage0"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %ileft "ileft"
OpName %iright "iright"
OpName %itop "itop"
OpName %ibot "ibot"
OpName %outset "outset"
OpName %l "l"
OpName %r "r"
OpName %t "t"
OpName %b "b"
OpName %vertexpos "vertexpos"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %sk_PerVertex Block
OpDecorate %sk_InstanceID BuiltIn InstanceIndex
OpDecorate %sk_VertexID BuiltIn VertexIndex
OpDecorate %vcoord_Stage0 Location 1
OpDecorate %vcoord_Stage0 NoPerspective
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_InstanceID = OpVariable %_ptr_Input_int Input
%sk_VertexID = OpVariable %_ptr_Input_int Input
%v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
%vcoord_Stage0 = OpVariable %_ptr_Output_v2float Output
%void = OpTypeVoid
%16 = OpTypeFunction %void
%_ptr_Function_int = OpTypePointer Function %int
%int_200 = OpConstant %int 200
%int_929 = OpConstant %int 929
%int_17 = OpConstant %int 17
%int_1 = OpConstant %int 1
%int_1637 = OpConstant %int 1637
%int_313 = OpConstant %int 313
%int_1901 = OpConstant %int 1901
%_ptr_Function_float = OpTypePointer Function %float
%float_0_03125 = OpConstant %float 0.03125
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%bool = OpTypeBool
%float_n0_03125 = OpConstant %float -0.03125
%float_16 = OpConstant %float 16
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int_n1 = OpConstant %int -1
%_ptr_Output_float = OpTypePointer Output %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%main = OpFunction %void None %16
%17 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_int Function
%ileft = OpVariable %_ptr_Function_int Function
%iright = OpVariable %_ptr_Function_int Function
%itop = OpVariable %_ptr_Function_int Function
%ibot = OpVariable %_ptr_Function_int Function
%outset = OpVariable %_ptr_Function_float Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
%t = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%vertexpos = OpVariable %_ptr_Function_v2float Function
%103 = OpVariable %_ptr_Function_float Function
%117 = OpVariable %_ptr_Function_float Function
%20 = OpLoad %int %sk_InstanceID
%22 = OpSMod %int %20 %int_200
OpStore %x %22
%24 = OpLoad %int %sk_InstanceID
%25 = OpSDiv %int %24 %int_200
OpStore %y %25
%27 = OpLoad %int %sk_InstanceID
%29 = OpIMul %int %27 %int_929
%31 = OpSMod %int %29 %int_17
OpStore %ileft %31
%33 = OpLoad %int %ileft
%35 = OpIAdd %int %33 %int_1
%36 = OpLoad %int %sk_InstanceID
%38 = OpIMul %int %36 %int_1637
%39 = OpLoad %int %ileft
%40 = OpISub %int %int_17 %39
%41 = OpSMod %int %38 %40
%42 = OpIAdd %int %35 %41
OpStore %iright %42
%44 = OpLoad %int %sk_InstanceID
%46 = OpIMul %int %44 %int_313
%47 = OpSMod %int %46 %int_17
OpStore %itop %47
%49 = OpLoad %int %itop
%50 = OpIAdd %int %49 %int_1
%51 = OpLoad %int %sk_InstanceID
%53 = OpIMul %int %51 %int_1901
%54 = OpLoad %int %itop
%55 = OpISub %int %int_17 %54
%56 = OpSMod %int %53 %55
%57 = OpIAdd %int %50 %56
OpStore %ibot %57
OpStore %outset %float_0_03125
%62 = OpLoad %int %x
%63 = OpLoad %int %y
%64 = OpIAdd %int %62 %63
%66 = OpSMod %int %64 %int_2
%67 = OpIEqual %bool %int_0 %66
%69 = OpSelect %float %67 %float_n0_03125 %float_0_03125
OpStore %outset %69
%72 = OpLoad %int %ileft
%73 = OpConvertSToF %float %72
%75 = OpFDiv %float %73 %float_16
%76 = OpLoad %float %outset
%77 = OpFSub %float %75 %76
OpStore %l %77
%79 = OpLoad %int %iright
%80 = OpConvertSToF %float %79
%81 = OpFDiv %float %80 %float_16
%82 = OpLoad %float %outset
%83 = OpFAdd %float %81 %82
OpStore %r %83
%85 = OpLoad %int %itop
%86 = OpConvertSToF %float %85
%87 = OpFDiv %float %86 %float_16
%88 = OpLoad %float %outset
%89 = OpFSub %float %87 %88
OpStore %t %89
%91 = OpLoad %int %ibot
%92 = OpConvertSToF %float %91
%93 = OpFDiv %float %92 %float_16
%94 = OpLoad %float %outset
%95 = OpFAdd %float %93 %94
OpStore %b %95
%98 = OpLoad %int %x
%99 = OpConvertSToF %float %98
%100 = OpLoad %int %sk_VertexID
%101 = OpSMod %int %100 %int_2
%102 = OpIEqual %bool %int_0 %101
OpSelectionMerge %106 None
OpBranchConditional %102 %104 %105
%104 = OpLabel
%107 = OpLoad %float %l
OpStore %103 %107
OpBranch %106
%105 = OpLabel
%108 = OpLoad %float %r
OpStore %103 %108
OpBranch %106
%106 = OpLabel
%109 = OpLoad %float %103
%110 = OpFAdd %float %99 %109
%111 = OpAccessChain %_ptr_Function_float %vertexpos %int_0
OpStore %111 %110
%112 = OpLoad %int %y
%113 = OpConvertSToF %float %112
%114 = OpLoad %int %sk_VertexID
%115 = OpSDiv %int %114 %int_2
%116 = OpIEqual %bool %int_0 %115
OpSelectionMerge %120 None
OpBranchConditional %116 %118 %119
%118 = OpLabel
%121 = OpLoad %float %t
OpStore %117 %121
OpBranch %120
%119 = OpLabel
%122 = OpLoad %float %b
OpStore %117 %122
OpBranch %120
%120 = OpLabel
%123 = OpLoad %float %117
%124 = OpFAdd %float %113 %123
%125 = OpAccessChain %_ptr_Function_float %vertexpos %int_1
OpStore %125 %124
%126 = OpLoad %int %sk_VertexID
%127 = OpSMod %int %126 %int_2
%128 = OpIEqual %bool %int_0 %127
%129 = OpSelect %int %128 %int_n1 %int_1
%131 = OpConvertSToF %float %129
%132 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_0
OpStore %132 %131
%134 = OpLoad %int %sk_VertexID
%135 = OpSDiv %int %134 %int_2
%136 = OpIEqual %bool %int_0 %135
%137 = OpSelect %int %136 %int_n1 %int_1
%138 = OpConvertSToF %float %137
%139 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_1
OpStore %139 %138
%140 = OpLoad %v2float %vertexpos
%141 = OpCompositeExtract %float %140 0
%142 = OpLoad %v2float %vertexpos
%143 = OpCompositeExtract %float %142 1
%146 = OpCompositeConstruct %v4float %141 %143 %float_0 %float_1
%147 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %147 %146
OpReturn
OpFunctionEnd
