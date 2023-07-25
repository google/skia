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
%float_0_0625 = OpConstant %float 0.0625
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
         %63 = OpVariable %_ptr_Function_float Function
          %l = OpVariable %_ptr_Function_float Function
          %r = OpVariable %_ptr_Function_float Function
          %t = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_float Function
  %vertexpos = OpVariable %_ptr_Function_v2float Function
         %92 = OpVariable %_ptr_Function_float Function
        %104 = OpVariable %_ptr_Function_float Function
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
         %34 = OpIAdd %int %31 %int_1
         %35 = OpLoad %int %sk_InstanceID
         %37 = OpIMul %int %35 %int_1637
         %38 = OpISub %int %int_17 %31
         %39 = OpSMod %int %37 %38
         %40 = OpIAdd %int %34 %39
               OpStore %iright %40
         %42 = OpLoad %int %sk_InstanceID
         %44 = OpIMul %int %42 %int_313
         %45 = OpSMod %int %44 %int_17
               OpStore %itop %45
         %47 = OpIAdd %int %45 %int_1
         %48 = OpLoad %int %sk_InstanceID
         %50 = OpIMul %int %48 %int_1901
         %51 = OpISub %int %int_17 %45
         %52 = OpSMod %int %50 %51
         %53 = OpIAdd %int %47 %52
               OpStore %ibot %53
               OpStore %outset %float_0_03125
         %58 = OpIAdd %int %22 %25
         %60 = OpSMod %int %58 %int_2
         %61 = OpIEqual %bool %int_0 %60
               OpSelectionMerge %66 None
               OpBranchConditional %61 %64 %65
         %64 = OpLabel
         %67 = OpFNegate %float %float_0_03125
               OpStore %63 %67
               OpBranch %66
         %65 = OpLabel
               OpStore %63 %float_0_03125
               OpBranch %66
         %66 = OpLabel
         %68 = OpLoad %float %63
               OpStore %outset %68
         %70 = OpConvertSToF %float %31
         %72 = OpFMul %float %70 %float_0_0625
         %73 = OpFSub %float %72 %68
               OpStore %l %73
         %75 = OpConvertSToF %float %40
         %76 = OpFMul %float %75 %float_0_0625
         %77 = OpFAdd %float %76 %68
               OpStore %r %77
         %79 = OpConvertSToF %float %45
         %80 = OpFMul %float %79 %float_0_0625
         %81 = OpFSub %float %80 %68
               OpStore %t %81
         %83 = OpConvertSToF %float %53
         %84 = OpFMul %float %83 %float_0_0625
         %85 = OpFAdd %float %84 %68
               OpStore %b %85
         %88 = OpConvertSToF %float %22
         %89 = OpLoad %int %sk_VertexID
         %90 = OpSMod %int %89 %int_2
         %91 = OpIEqual %bool %int_0 %90
               OpSelectionMerge %95 None
               OpBranchConditional %91 %93 %94
         %93 = OpLabel
               OpStore %92 %73
               OpBranch %95
         %94 = OpLabel
               OpStore %92 %77
               OpBranch %95
         %95 = OpLabel
         %96 = OpLoad %float %92
         %97 = OpFAdd %float %88 %96
         %98 = OpAccessChain %_ptr_Function_float %vertexpos %int_0
               OpStore %98 %97
         %99 = OpLoad %int %y
        %100 = OpConvertSToF %float %99
        %101 = OpLoad %int %sk_VertexID
        %102 = OpSDiv %int %101 %int_2
        %103 = OpIEqual %bool %int_0 %102
               OpSelectionMerge %107 None
               OpBranchConditional %103 %105 %106
        %105 = OpLabel
        %108 = OpLoad %float %t
               OpStore %104 %108
               OpBranch %107
        %106 = OpLabel
        %109 = OpLoad %float %b
               OpStore %104 %109
               OpBranch %107
        %107 = OpLabel
        %110 = OpLoad %float %104
        %111 = OpFAdd %float %100 %110
        %112 = OpAccessChain %_ptr_Function_float %vertexpos %int_1
               OpStore %112 %111
        %113 = OpLoad %int %sk_VertexID
        %114 = OpSMod %int %113 %int_2
        %115 = OpIEqual %bool %int_0 %114
        %116 = OpSelect %int %115 %int_n1 %int_1
        %118 = OpConvertSToF %float %116
        %119 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_0
               OpStore %119 %118
        %121 = OpLoad %int %sk_VertexID
        %122 = OpSDiv %int %121 %int_2
        %123 = OpIEqual %bool %int_0 %122
        %124 = OpSelect %int %123 %int_n1 %int_1
        %125 = OpConvertSToF %float %124
        %126 = OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_1
               OpStore %126 %125
        %127 = OpLoad %v2float %vertexpos
        %128 = OpCompositeExtract %float %127 0
        %129 = OpLoad %v2float %vertexpos
        %130 = OpCompositeExtract %float %129 1
        %133 = OpCompositeConstruct %v4float %128 %130 %float_0 %float_1
        %134 = OpAccessChain %_ptr_Output_v4float %3 %int_0
               OpStore %134 %133
               OpReturn
               OpFunctionEnd
