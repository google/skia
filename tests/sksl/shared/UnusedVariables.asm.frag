               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %userfunc_ff "userfunc_ff"
               OpName %main "main"
               OpName %b "b"
               OpName %c "c"
               OpName %x "x"
               OpName %d "d"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %22 = OpTypeFunction %float %_ptr_Function_float
    %float_1 = OpConstant %float 1
         %28 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
   %float_77 = OpConstant %float 77
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %true = OpConstantTrue %bool
    %float_5 = OpConstant %float 5
    %float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%userfunc_ff = OpFunction %float None %22
         %23 = OpFunctionParameter %_ptr_Function_float
         %24 = OpLabel
         %25 = OpLoad %float %23
         %27 = OpFAdd %float %25 %float_1
               OpReturnValue %27
               OpFunctionEnd
       %main = OpFunction %v4float None %28
         %29 = OpFunctionParameter %_ptr_Function_v2float
         %30 = OpLabel
          %b = OpVariable %_ptr_Function_float Function
          %c = OpVariable %_ptr_Function_float Function
         %40 = OpVariable %_ptr_Function_float Function
         %43 = OpVariable %_ptr_Function_float Function
          %x = OpVariable %_ptr_Function_int Function
          %d = OpVariable %_ptr_Function_float Function
               OpStore %b %float_2
               OpStore %c %float_3
               OpStore %b %float_2
         %36 = OpFAdd %float %float_3 %float_77
               OpStore %b %36
         %38 = OpFAdd %float %float_3 %float_77
         %37 = OpExtInst %float %1 Sin %38
               OpStore %b %37
         %39 = OpFAdd %float %float_3 %float_77
               OpStore %40 %39
         %41 = OpFunctionCall %float %userfunc_ff %40
         %42 = OpFAdd %float %float_3 %float_77
               OpStore %43 %42
         %44 = OpFunctionCall %float %userfunc_ff %43
               OpStore %b %44
         %45 = OpExtInst %float %1 Cos %float_3
               OpStore %b %45
               OpStore %b %45
               OpStore %x %int_0
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %54 %53 None
               OpBranch %51
         %51 = OpLabel
         %55 = OpLoad %int %x
         %57 = OpSLessThan %bool %55 %int_1
               OpBranchConditional %57 %52 %54
         %52 = OpLabel
               OpBranch %53
         %53 = OpLabel
         %58 = OpLoad %int %x
         %59 = OpIAdd %int %58 %int_1
               OpStore %x %59
               OpBranch %50
         %54 = OpLabel
         %61 = OpLoad %float %c
               OpStore %d %61
               OpStore %b %float_3
         %62 = OpFAdd %float %61 %float_1
               OpStore %d %62
         %63 = OpFOrdEqual %bool %float_3 %float_2
         %64 = OpSelect %float %63 %float_1 %float_0
         %66 = OpSelect %float %true %float_1 %float_0
         %68 = OpFOrdEqual %bool %62 %float_5
         %69 = OpSelect %float %68 %float_1 %float_0
         %71 = OpFOrdEqual %bool %62 %float_4
         %72 = OpSelect %float %71 %float_1 %float_0
         %73 = OpCompositeConstruct %v4float %64 %66 %69 %72
               OpReturnValue %73
               OpFunctionEnd
