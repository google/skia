               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %userfunc_ff "userfunc_ff"
               OpName %main "main"
               OpName %b "b"
               OpName %c "c"
               OpName %x "x"
               OpName %d "d"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %14 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %19 = OpTypeFunction %float %_ptr_Function_float
    %float_1 = OpConstant %float 1
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
   %float_77 = OpConstant %float 77
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %float_5 = OpConstant %float 5
    %float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %10
         %11 = OpLabel
         %15 = OpVariable %_ptr_Function_v2float Function
               OpStore %15 %14
         %17 = OpFunctionCall %v4float %main %15
               OpStore %sk_FragColor %17
               OpReturn
               OpFunctionEnd
%userfunc_ff = OpFunction %float None %19
         %20 = OpFunctionParameter %_ptr_Function_float
         %21 = OpLabel
         %22 = OpLoad %float %20
         %24 = OpFAdd %float %22 %float_1
               OpReturnValue %24
               OpFunctionEnd
       %main = OpFunction %v4float None %25
         %26 = OpFunctionParameter %_ptr_Function_v2float
         %27 = OpLabel
          %b = OpVariable %_ptr_Function_float Function
          %c = OpVariable %_ptr_Function_float Function
         %37 = OpVariable %_ptr_Function_float Function
         %40 = OpVariable %_ptr_Function_float Function
          %x = OpVariable %_ptr_Function_int Function
          %d = OpVariable %_ptr_Function_float Function
               OpStore %b %float_2
               OpStore %c %float_3
               OpStore %b %float_2
         %33 = OpFAdd %float %float_3 %float_77
               OpStore %b %33
         %35 = OpFAdd %float %float_3 %float_77
         %34 = OpExtInst %float %1 Sin %35
               OpStore %b %34
         %36 = OpFAdd %float %float_3 %float_77
               OpStore %37 %36
         %38 = OpFunctionCall %float %userfunc_ff %37
         %39 = OpFAdd %float %float_3 %float_77
               OpStore %40 %39
         %41 = OpFunctionCall %float %userfunc_ff %40
               OpStore %b %41
         %42 = OpExtInst %float %1 Cos %float_3
               OpStore %b %42
               OpStore %b %42
               OpStore %x %int_0
               OpBranch %47
         %47 = OpLabel
               OpLoopMerge %51 %50 None
               OpBranch %48
         %48 = OpLabel
         %52 = OpLoad %int %x
         %54 = OpSLessThan %bool %52 %int_1
               OpBranchConditional %54 %49 %51
         %49 = OpLabel
               OpBranch %50
         %50 = OpLabel
         %56 = OpLoad %int %x
         %57 = OpIAdd %int %56 %int_1
               OpStore %x %57
               OpBranch %47
         %51 = OpLabel
         %59 = OpLoad %float %c
               OpStore %d %59
               OpStore %b %float_3
         %60 = OpFAdd %float %59 %float_1
               OpStore %d %60
         %61 = OpFOrdEqual %bool %float_3 %float_2
         %62 = OpSelect %float %61 %float_1 %float_0
         %64 = OpSelect %float %true %float_1 %float_0
         %66 = OpFOrdEqual %bool %60 %float_5
         %67 = OpSelect %float %66 %float_1 %float_0
         %69 = OpFOrdEqual %bool %60 %float_4
         %70 = OpSelect %float %69 %float_1 %float_0
         %71 = OpCompositeConstruct %v4float %62 %64 %67 %70
               OpReturnValue %71
               OpFunctionEnd
