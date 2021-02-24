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
%69 = OpVariable %_ptr_Function_float Function
%l = OpVariable %_ptr_Function_float Function
%r = OpVariable %_ptr_Function_float Function
%t = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%vertexpos = OpVariable %_ptr_Function_v2float Function
%109 = OpVariable %_ptr_Function_float Function
%123 = OpVariable %_ptr_Function_float Function
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
OpSelectionMerge %72 None
OpBranchConditional %67 %70 %71
%70 = OpLabel
%74 = OpLoad %float %outset
%73 = OpFNegate %float %74
OpStore %69 %73
OpBranch %72
%71 = OpLabel
%75 = OpLoad %float %outset
OpStore %69 %75
OpBranch %72
%72 = OpLabel
%76 = OpLoad %float %69
OpStore %outset %76
%78 = OpLoad %int %ileft
%79 = OpConvertSToF %float %78
%81 = OpFDiv %float %79 %float_16
%82 = OpLoad %float %outset
%83 = OpFSub %float %81 %82
OpStore %l %83
%85 = OpLoad %int %iright
%86 = OpConvertSToF %float %85
%87 = OpFDiv %float %86 %float_16
%88 = OpLoad %float %outset
%89 = OpFAdd %float %87 %88
OpStore %r %89
%91 = OpLoad %int %itop
%92 = OpConvertSToF %float %91
%93 = OpFDiv %float %92 %float_16
%94 = OpLoad %float %outset
%95 = OpFSub %float %93 %94
OpStore %t %95
%97 = OpLoad %int %ibot
%98 = OpConvertSToF %float %97
%99 = OpFDiv %float %98 %float_16
%100 = OpLoad %float %outset
%101 = OpFAdd %float %99 %100
OpStore %b %101
%104 = OpLoad %int %x
%105 = OpConvertSToF %float %104
%106 = OpLoad %int %sk_VertexID
%107 = OpSMod %int %106 %int_2
%108 = OpIEqual %bool %int_0 %107
OpSelectionMerge %112 None
OpBranchConditional %108 %110 %111
%110 = OpLabel
%113 = OpLoad %float %l
OpStore %109 %113
OpBranch %112
%111 = OpLabel
%114 = OpLoad %float %r
OpStore %109 %114
OpBranch %112
%112 = OpLabel
%115 = OpLoad %float %109
%116 = OpFAdd %float %105 %115
%117 = OpAccessChain %_ptr_Function_float %vertexpos %int_0
OpStore %117 %116
%118 = OpLoad %int %y
%119 = OpConvertSToF %float %118
%120 = OpLoad %int %sk_VertexID
%121 = OpSDiv %int %120 %int_2
%122 = OpIEqual %bool %int_0 %121
OpSelectionMerge %126 None
OpBranchConditional %122 %124 %125
%124 = OpLabel
%127 = OpLoad %float %t
OpStore %123 %127
OpBranch %126
%125 = OpLabel
%128 = OpLoad %float %b
OpStore %123 %128
OpBranch %126
%126 = OpLabel
%129 = OpLoad %float %123
%130 = OpFAdd %float %119 %129
%131 = OpAccessChain %_ptr_Function_float %vertexpos %int_1
OpStore %131 %130
%132 = OpLoad %int %sk_VertexID
%133 = OpSMod %int %132 %int_2
%134 = OpIEqual %bool %int_0 %133
%135 = OpSelect %int %134 %int_n1 %int_1
%137 = OpConvertSToF %float %135
%138 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_0
OpStore %138 %137
%140 = OpLoad %int %sk_VertexID
%141 = OpSDiv %int %140 %int_2
%142 = OpIEqual %bool %int_0 %141
%143 = OpSelect %int %142 %int_n1 %int_1
%144 = OpConvertSToF %float %143
%145 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_1
OpStore %145 %144
%146 = OpLoad %v2float %vertexpos
%147 = OpCompositeExtract %float %146 0
%148 = OpLoad %v2float %vertexpos
%149 = OpCompositeExtract %float %148 1
%152 = OpCompositeConstruct %v4float %147 %149 %float_0 %float_1
%153 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %153 %152
OpReturn
OpFunctionEnd
