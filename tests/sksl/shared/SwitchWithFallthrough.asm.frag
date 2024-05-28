               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"
               OpName %ok "ok"
               OpName %main "main"
               OpName %x "x"
               OpName %_0_ok "_0_ok"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %38 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %24 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %39 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%switch_fallthrough_twice_bi = OpFunction %bool None %24
         %25 = OpFunctionParameter %_ptr_Function_int
         %26 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
               OpStore %ok %false
         %30 = OpLoad %int %25
               OpSelectionMerge %31 None
               OpSwitch %30 %36 0 %32 1 %35 2 %35 3 %35
         %32 = OpLabel
               OpBranch %31
         %35 = OpLabel
               OpStore %ok %true
               OpBranch %31
         %36 = OpLabel
               OpBranch %31
         %31 = OpLabel
         %38 = OpLoad %bool %ok
               OpReturnValue %38
               OpFunctionEnd
       %main = OpFunction %v4float None %39
         %40 = OpFunctionParameter %_ptr_Function_v2float
         %41 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
      %_0_ok = OpVariable %_ptr_Function_bool Function
         %58 = OpVariable %_ptr_Function_int Function
         %61 = OpVariable %_ptr_Function_v4float Function
         %43 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %46 = OpLoad %v4float %43
         %47 = OpCompositeExtract %float %46 1
         %48 = OpConvertFToS %int %47
               OpStore %x %48
               OpStore %_0_ok %false
               OpSelectionMerge %50 None
               OpSwitch %48 %54 2 %51 1 %53 0 %53
         %51 = OpLabel
               OpBranch %50
         %53 = OpLabel
               OpStore %_0_ok %true
               OpBranch %50
         %54 = OpLabel
               OpBranch %50
         %50 = OpLabel
         %55 = OpLoad %bool %_0_ok
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
               OpStore %58 %48
         %59 = OpFunctionCall %bool %switch_fallthrough_twice_bi %58
               OpBranch %57
         %57 = OpLabel
         %60 = OpPhi %bool %false %50 %59 %56
               OpSelectionMerge %65 None
               OpBranchConditional %60 %63 %64
         %63 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %67 = OpLoad %v4float %66
               OpStore %61 %67
               OpBranch %65
         %64 = OpLabel
         %68 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %70 = OpLoad %v4float %68
               OpStore %61 %70
               OpBranch %65
         %65 = OpLabel
         %71 = OpLoad %v4float %61
               OpReturnValue %71
               OpFunctionEnd
