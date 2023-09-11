               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %colorR "colorR"
               OpName %colorGreen "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %_0_ok "_0_ok"
               OpName %_1_d "_1_d"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %colorR RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_v4float = OpTypePointer Private %v4float
     %colorR = OpVariable %_ptr_Private_v4float Private
 %colorGreen = OpVariable %_ptr_Private_v4float Private
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
         %15 = OpTypeFunction %v4float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %14 = OpFunctionCall %v4float %main
               OpStore %sk_FragColor %14
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %15
         %16 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_int Function
       %_1_d = OpVariable %_ptr_Function__arr_float_int_2 Function
         %42 = OpVariable %_ptr_Function_v4float Function
         %26 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_1
               OpStore %_1_d %26
               OpBranch %27
         %27 = OpLabel
               OpLoopMerge %31 %30 None
               OpBranch %28
         %28 = OpLabel
         %32 = OpNot %int %int_1
         %34 = OpAccessChain %_ptr_Function_float %_1_d %32
         %36 = OpLoad %float %34
         %37 = OpFOrdLessThan %bool %float_0 %36
               OpBranchConditional %37 %29 %31
         %29 = OpLabel
               OpBranch %30
         %30 = OpLabel
               OpBranch %27
         %31 = OpLabel
         %39 = OpLoad %int %_0_ok
         %41 = OpIEqual %bool %39 %int_0
               OpSelectionMerge %46 None
               OpBranchConditional %41 %44 %45
         %44 = OpLabel
         %47 = OpLoad %v4float %colorGreen
               OpStore %42 %47
               OpBranch %46
         %45 = OpLabel
         %48 = OpLoad %v4float %colorR
               OpStore %42 %48
               OpBranch %46
         %46 = OpLabel
         %49 = OpLoad %v4float %42
               OpReturnValue %49
               OpFunctionEnd
