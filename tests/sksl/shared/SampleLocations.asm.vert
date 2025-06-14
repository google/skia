               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %7 %sk_InstanceID %sk_VertexID %vcoord_Stage0

               ; Debug Information
               OpName %sk_PerVertex "sk_PerVertex"  ; id %10
               OpMemberName %sk_PerVertex 0 "sk_Position"
               OpMemberName %sk_PerVertex 1 "sk_PointSize"
               OpName %sk_InstanceID "sk_InstanceID"    ; id %12
               OpName %sk_VertexID "sk_VertexID"        ; id %13
               OpName %vcoord_Stage0 "vcoord_Stage0"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %x "x"                            ; id %20
               OpName %y "y"                            ; id %25
               OpName %ileft "ileft"                    ; id %28
               OpName %iright "iright"                  ; id %34
               OpName %itop "itop"                      ; id %43
               OpName %ibot "ibot"                      ; id %48
               OpName %outset "outset"                  ; id %56
               OpName %l "l"                            ; id %71
               OpName %r "r"                            ; id %76
               OpName %t "t"                            ; id %80
               OpName %b "b"                            ; id %84
               OpName %vertexpos "vertexpos"            ; id %88

               ; Annotations
               OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
               OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
               OpDecorate %sk_PerVertex Block
               OpDecorate %sk_InstanceID BuiltIn InstanceIndex
               OpDecorate %sk_VertexID BuiltIn VertexIndex
               OpDecorate %vcoord_Stage0 Location 1
               OpDecorate %vcoord_Stage0 NoPerspective

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float        ; Block
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
          %7 = OpVariable %_ptr_Output_sk_PerVertex Output
%sk_InstanceID = OpVariable %_ptr_Input_int Input   ; BuiltIn InstanceIndex
%sk_VertexID = OpVariable %_ptr_Input_int Input     ; BuiltIn VertexIndex
    %v2float = OpTypeVector %float 2
%_ptr_Output_v2float = OpTypePointer Output %v2float
%vcoord_Stage0 = OpVariable %_ptr_Output_v2float Output     ; Location 1, NoPerspective
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
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


               ; Function main
       %main = OpFunction %void None %18

         %19 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
          %y =   OpVariable %_ptr_Function_int Function
      %ileft =   OpVariable %_ptr_Function_int Function
     %iright =   OpVariable %_ptr_Function_int Function
       %itop =   OpVariable %_ptr_Function_int Function
       %ibot =   OpVariable %_ptr_Function_int Function
     %outset =   OpVariable %_ptr_Function_float Function
         %65 =   OpVariable %_ptr_Function_float Function
          %l =   OpVariable %_ptr_Function_float Function
          %r =   OpVariable %_ptr_Function_float Function
          %t =   OpVariable %_ptr_Function_float Function
          %b =   OpVariable %_ptr_Function_float Function
  %vertexpos =   OpVariable %_ptr_Function_v2float Function
         %94 =   OpVariable %_ptr_Function_float Function
        %106 =   OpVariable %_ptr_Function_float Function
         %22 =   OpLoad %int %sk_InstanceID
         %24 =   OpSMod %int %22 %int_200
                 OpStore %x %24
         %26 =   OpLoad %int %sk_InstanceID
         %27 =   OpSDiv %int %26 %int_200
                 OpStore %y %27
         %29 =   OpLoad %int %sk_InstanceID
         %31 =   OpIMul %int %29 %int_929
         %33 =   OpSMod %int %31 %int_17
                 OpStore %ileft %33
         %36 =   OpIAdd %int %33 %int_1
         %37 =   OpLoad %int %sk_InstanceID
         %39 =   OpIMul %int %37 %int_1637
         %40 =   OpISub %int %int_17 %33
         %41 =   OpSMod %int %39 %40
         %42 =   OpIAdd %int %36 %41
                 OpStore %iright %42
         %44 =   OpLoad %int %sk_InstanceID
         %46 =   OpIMul %int %44 %int_313
         %47 =   OpSMod %int %46 %int_17
                 OpStore %itop %47
         %49 =   OpIAdd %int %47 %int_1
         %50 =   OpLoad %int %sk_InstanceID
         %52 =   OpIMul %int %50 %int_1901
         %53 =   OpISub %int %int_17 %47
         %54 =   OpSMod %int %52 %53
         %55 =   OpIAdd %int %49 %54
                 OpStore %ibot %55
                 OpStore %outset %float_0_03125
         %60 =   OpIAdd %int %24 %27
         %62 =   OpSMod %int %60 %int_2
         %63 =   OpIEqual %bool %int_0 %62
                 OpSelectionMerge %68 None
                 OpBranchConditional %63 %66 %67

         %66 =     OpLabel
         %69 =       OpFNegate %float %float_0_03125
                     OpStore %65 %69
                     OpBranch %68

         %67 =     OpLabel
                     OpStore %65 %float_0_03125
                     OpBranch %68

         %68 = OpLabel
         %70 =   OpLoad %float %65
                 OpStore %outset %70
         %72 =   OpConvertSToF %float %33
         %74 =   OpFMul %float %72 %float_0_0625
         %75 =   OpFSub %float %74 %70
                 OpStore %l %75
         %77 =   OpConvertSToF %float %42
         %78 =   OpFMul %float %77 %float_0_0625
         %79 =   OpFAdd %float %78 %70
                 OpStore %r %79
         %81 =   OpConvertSToF %float %47
         %82 =   OpFMul %float %81 %float_0_0625
         %83 =   OpFSub %float %82 %70
                 OpStore %t %83
         %85 =   OpConvertSToF %float %55
         %86 =   OpFMul %float %85 %float_0_0625
         %87 =   OpFAdd %float %86 %70
                 OpStore %b %87
         %90 =   OpConvertSToF %float %24
         %91 =   OpLoad %int %sk_VertexID
         %92 =   OpSMod %int %91 %int_2
         %93 =   OpIEqual %bool %int_0 %92
                 OpSelectionMerge %97 None
                 OpBranchConditional %93 %95 %96

         %95 =     OpLabel
                     OpStore %94 %75
                     OpBranch %97

         %96 =     OpLabel
                     OpStore %94 %79
                     OpBranch %97

         %97 = OpLabel
         %98 =   OpLoad %float %94
         %99 =   OpFAdd %float %90 %98
        %100 =   OpAccessChain %_ptr_Function_float %vertexpos %int_0
                 OpStore %100 %99
        %101 =   OpLoad %int %y
        %102 =   OpConvertSToF %float %101
        %103 =   OpLoad %int %sk_VertexID
        %104 =   OpSDiv %int %103 %int_2
        %105 =   OpIEqual %bool %int_0 %104
                 OpSelectionMerge %109 None
                 OpBranchConditional %105 %107 %108

        %107 =     OpLabel
        %110 =       OpLoad %float %t
                     OpStore %106 %110
                     OpBranch %109

        %108 =     OpLabel
        %111 =       OpLoad %float %b
                     OpStore %106 %111
                     OpBranch %109

        %109 = OpLabel
        %112 =   OpLoad %float %106
        %113 =   OpFAdd %float %102 %112
        %114 =   OpAccessChain %_ptr_Function_float %vertexpos %int_1
                 OpStore %114 %113
        %115 =   OpLoad %int %sk_VertexID
        %116 =   OpSMod %int %115 %int_2
        %117 =   OpIEqual %bool %int_0 %116
        %118 =   OpSelect %int %117 %int_n1 %int_1
        %120 =   OpConvertSToF %float %118
        %121 =   OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_0
                 OpStore %121 %120
        %123 =   OpLoad %int %sk_VertexID
        %124 =   OpSDiv %int %123 %int_2
        %125 =   OpIEqual %bool %int_0 %124
        %126 =   OpSelect %int %125 %int_n1 %int_1
        %127 =   OpConvertSToF %float %126
        %128 =   OpAccessChain %_ptr_Output_float %vcoord_Stage0 %int_1
                 OpStore %128 %127
        %129 =   OpLoad %v2float %vertexpos
        %130 =   OpCompositeExtract %float %129 0
        %131 =   OpLoad %v2float %vertexpos
        %132 =   OpCompositeExtract %float %131 1
        %135 =   OpCompositeConstruct %v4float %130 %132 %float_0 %float_1
        %136 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %136 %135
                 OpReturn
               OpFunctionEnd
