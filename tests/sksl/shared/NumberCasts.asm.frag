               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %B "B"
               OpName %F "F"
               OpName %I "I"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %58 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %13 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
       %true = OpConstantTrue %bool
%_ptr_Function_bool = OpTypePointer Function %bool
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1_23000002 = OpConstant %float 1.23000002
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_int = OpTypePointer Function %int
      %false = OpConstantFalse %bool
%_entrypoint_v = OpFunction %void None %9
         %10 = OpLabel
         %14 = OpVariable %_ptr_Function_v2float Function
               OpStore %14 %13
         %16 = OpFunctionCall %v4float %main %14
               OpStore %sk_FragColor %16
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %17
         %18 = OpFunctionParameter %_ptr_Function_v2float
         %19 = OpLabel
          %B = OpVariable %_ptr_Function_v3bool Function
          %F = OpVariable %_ptr_Function_v3float Function
          %I = OpVariable %_ptr_Function_v3int Function
         %25 = OpAccessChain %_ptr_Function_bool %B %int_0
               OpStore %25 %true
         %29 = OpAccessChain %_ptr_Function_bool %B %int_1
               OpStore %29 %true
         %31 = OpAccessChain %_ptr_Function_bool %B %int_2
               OpStore %31 %true
         %37 = OpAccessChain %_ptr_Function_float %F %int_0
               OpStore %37 %float_1_23000002
         %39 = OpAccessChain %_ptr_Function_float %F %int_1
               OpStore %39 %float_0
         %41 = OpAccessChain %_ptr_Function_float %F %int_2
               OpStore %41 %float_1
         %45 = OpAccessChain %_ptr_Function_int %I %int_0
               OpStore %45 %int_1
         %47 = OpAccessChain %_ptr_Function_int %I %int_1
               OpStore %47 %int_1
         %48 = OpAccessChain %_ptr_Function_int %I %int_2
               OpStore %48 %int_1
         %49 = OpLoad %v3float %F
         %50 = OpCompositeExtract %float %49 0
         %51 = OpLoad %v3float %F
         %52 = OpCompositeExtract %float %51 1
         %53 = OpFMul %float %50 %52
         %54 = OpLoad %v3float %F
         %55 = OpCompositeExtract %float %54 2
         %56 = OpFMul %float %53 %55
         %58 = OpLoad %v3bool %B
         %59 = OpCompositeExtract %bool %58 0
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
         %62 = OpLoad %v3bool %B
         %63 = OpCompositeExtract %bool %62 1
               OpBranch %61
         %61 = OpLabel
         %64 = OpPhi %bool %false %19 %63 %60
               OpSelectionMerge %66 None
               OpBranchConditional %64 %65 %66
         %65 = OpLabel
         %67 = OpLoad %v3bool %B
         %68 = OpCompositeExtract %bool %67 2
               OpBranch %66
         %66 = OpLabel
         %69 = OpPhi %bool %false %61 %68 %65
         %70 = OpSelect %float %69 %float_1 %float_0
         %71 = OpLoad %v3int %I
         %72 = OpCompositeExtract %int %71 0
         %73 = OpLoad %v3int %I
         %74 = OpCompositeExtract %int %73 1
         %75 = OpIMul %int %72 %74
         %76 = OpLoad %v3int %I
         %77 = OpCompositeExtract %int %76 2
         %78 = OpIMul %int %75 %77
         %79 = OpConvertSToF %float %78
         %80 = OpCompositeConstruct %v4float %56 %70 %float_0 %79
               OpReturnValue %80
               OpFunctionEnd
