               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %colorR "colorR"
               OpName %colorGreen "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %_0_ok "_0_ok"
               OpName %_1_d "_1_d"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %colorR RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Private_v4float = OpTypePointer Private %v4float
     %colorR = OpVariable %_ptr_Private_v4float Private
 %colorGreen = OpVariable %_ptr_Private_v4float Private
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
         %18 = OpTypeFunction %v4float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %17 = OpFunctionCall %v4float %main
               OpStore %sk_FragColor %17
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %18
         %19 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_int Function
       %_1_d = OpVariable %_ptr_Function__arr_float_int_2 Function
         %44 = OpVariable %_ptr_Function_v4float Function
         %29 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_1
               OpStore %_1_d %29
               OpBranch %30
         %30 = OpLabel
               OpLoopMerge %34 %33 None
               OpBranch %31
         %31 = OpLabel
         %35 = OpNot %int %int_1
         %37 = OpAccessChain %_ptr_Function_float %_1_d %35
         %39 = OpLoad %float %37
         %40 = OpFOrdLessThan %bool %float_0 %39
               OpBranchConditional %40 %32 %34
         %32 = OpLabel
               OpBranch %33
         %33 = OpLabel
               OpBranch %30
         %34 = OpLabel
         %41 = OpLoad %int %_0_ok
         %43 = OpIEqual %bool %41 %int_0
               OpSelectionMerge %48 None
               OpBranchConditional %43 %46 %47
         %46 = OpLabel
         %49 = OpLoad %v4float %colorGreen
               OpStore %44 %49
               OpBranch %48
         %47 = OpLabel
         %50 = OpLoad %v4float %colorR
               OpStore %44 %50
               OpBranch %48
         %48 = OpLabel
         %51 = OpLoad %v4float %44
               OpReturnValue %51
               OpFunctionEnd
