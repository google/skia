               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testArray"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %array "array"                    ; id %29
               OpName %S "S"                            ; id %39
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "y"
               OpMemberName %S 2 "m"
               OpMemberName %S 3 "a"
               OpName %s1 "s1"                      ; id %37
               OpName %s2 "s2"                      ; id %47
               OpName %s3 "s3"                      ; id %52

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpMemberDecorate %S 2 Offset 16
               OpMemberDecorate %S 2 ColMajor
               OpMemberDecorate %S 2 MatrixStride 16
               OpMemberDecorate %S 2 RelaxedPrecision
               OpMemberDecorate %S 3 Offset 48
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5       ; ArrayStride 16
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_5 = OpTypePointer Function %_arr_float_int_5
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
%mat2v2float = OpTypeMatrix %v2float 2
          %S = OpTypeStruct %int %int %mat2v2float %_arr_float_int_5
%_ptr_Function_S = OpTypePointer Function %S
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
         %43 = OpConstantComposite %v2float %float_1 %float_0
         %44 = OpConstantComposite %v2float %float_0 %float_1
         %45 = OpConstantComposite %mat2v2float %43 %44
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
         %53 = OpConstantComposite %v2float %float_2 %float_0
         %54 = OpConstantComposite %v2float %float_0 %float_2
         %55 = OpConstantComposite %mat2v2float %53 %54
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %26         ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v2float

         %28 = OpLabel
      %array =   OpVariable %_ptr_Function__arr_float_int_5 Function
         %s1 =   OpVariable %_ptr_Function_S Function
         %s2 =   OpVariable %_ptr_Function_S Function
         %s3 =   OpVariable %_ptr_Function_S Function
         %98 =   OpVariable %_ptr_Function_v4float Function
         %36 =   OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_4 %float_5
                 OpStore %array %36
         %46 =   OpCompositeConstruct %S %int_1 %int_2 %45 %36
                 OpStore %s1 %46
         %48 =   OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
         %50 =   OpLoad %_arr_float_int_5 %48
         %51 =   OpCompositeConstruct %S %int_1 %int_2 %45 %50
                 OpStore %s2 %51
         %56 =   OpCompositeConstruct %S %int_1 %int_2 %55 %36
                 OpStore %s3 %56
         %60 =   OpLogicalAnd %bool %true %true
         %62 =   OpFOrdEqual %v2bool %43 %43        ; RelaxedPrecision
         %63 =   OpAll %bool %62
         %64 =   OpFOrdEqual %v2bool %44 %44        ; RelaxedPrecision
         %65 =   OpAll %bool %64
         %66 =   OpLogicalAnd %bool %63 %65
         %67 =   OpLogicalAnd %bool %66 %60
         %68 =   OpCompositeExtract %float %50 0
         %69 =   OpFOrdEqual %bool %float_1 %68
         %70 =   OpCompositeExtract %float %50 1
         %71 =   OpFOrdEqual %bool %float_2 %70
         %72 =   OpLogicalAnd %bool %71 %69
         %73 =   OpCompositeExtract %float %50 2
         %74 =   OpFOrdEqual %bool %float_3 %73
         %75 =   OpLogicalAnd %bool %74 %72
         %76 =   OpCompositeExtract %float %50 3
         %77 =   OpFOrdEqual %bool %float_4 %76
         %78 =   OpLogicalAnd %bool %77 %75
         %79 =   OpCompositeExtract %float %50 4
         %80 =   OpFOrdEqual %bool %float_5 %79
         %81 =   OpLogicalAnd %bool %80 %78
         %82 =   OpLogicalAnd %bool %81 %67
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %85 =       OpLogicalOr %bool %false %false
         %86 =       OpFUnordNotEqual %v2bool %43 %53   ; RelaxedPrecision
         %87 =       OpAny %bool %86
         %88 =       OpFUnordNotEqual %v2bool %44 %54   ; RelaxedPrecision
         %89 =       OpAny %bool %88
         %90 =       OpLogicalOr %bool %87 %89
         %91 =       OpLogicalOr %bool %90 %85
         %92 =       OpLogicalOr %bool %false %false
         %93 =       OpLogicalOr %bool %false %92
         %94 =       OpLogicalOr %bool %false %93
         %95 =       OpLogicalOr %bool %false %94
         %96 =       OpLogicalOr %bool %95 %91
                     OpBranch %84

         %84 = OpLabel
         %97 =   OpPhi %bool %false %28 %96 %83
                 OpSelectionMerge %102 None
                 OpBranchConditional %97 %100 %101

        %100 =     OpLabel
        %103 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %106 =       OpLoad %v4float %103           ; RelaxedPrecision
                     OpStore %98 %106
                     OpBranch %102

        %101 =     OpLabel
        %107 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %108 =       OpLoad %v4float %107           ; RelaxedPrecision
                     OpStore %98 %108
                     OpBranch %102

        %102 = OpLabel
        %109 =   OpLoad %v4float %98                ; RelaxedPrecision
                 OpReturnValue %109
               OpFunctionEnd
