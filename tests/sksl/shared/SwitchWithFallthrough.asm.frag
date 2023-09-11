               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"
               OpName %ok "ok"
               OpName %switch_fallthrough_groups_bi "switch_fallthrough_groups_bi"
               OpName %ok_0 "ok"
               OpName %main "main"
               OpName %x "x"
               OpName %_0_ok "_0_ok"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %41 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %27 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %58 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %22 = OpVariable %_ptr_Function_v2float Function
               OpStore %22 %21
         %24 = OpFunctionCall %v4float %main %22
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
%switch_fallthrough_twice_bi = OpFunction %bool None %27
         %28 = OpFunctionParameter %_ptr_Function_int
         %29 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
               OpStore %ok %false
         %33 = OpLoad %int %28
               OpSelectionMerge %34 None
               OpSwitch %33 %39 0 %35 1 %38 2 %38 3 %38
         %35 = OpLabel
               OpBranch %34
         %38 = OpLabel
               OpStore %ok %true
               OpBranch %34
         %39 = OpLabel
               OpBranch %34
         %34 = OpLabel
         %41 = OpLoad %bool %ok
               OpReturnValue %41
               OpFunctionEnd
%switch_fallthrough_groups_bi = OpFunction %bool None %27
         %42 = OpFunctionParameter %_ptr_Function_int
         %43 = OpLabel
       %ok_0 = OpVariable %_ptr_Function_bool Function
               OpStore %ok_0 %false
         %45 = OpLoad %int %42
               OpSelectionMerge %46 None
               OpSwitch %45 %56 -1 %47 0 %48 1 %49 2 %51 3 %51 4 %52 5 %56 6 %56 7 %56
         %47 = OpLabel
               OpStore %ok_0 %false
               OpBranch %48
         %48 = OpLabel
               OpReturnValue %false
         %49 = OpLabel
               OpStore %ok_0 %true
               OpBranch %51
         %51 = OpLabel
               OpBranch %46
         %52 = OpLabel
               OpStore %ok_0 %false
               OpBranch %56
         %56 = OpLabel
               OpBranch %46
         %46 = OpLabel
         %57 = OpLoad %bool %ok_0
               OpReturnValue %57
               OpFunctionEnd
       %main = OpFunction %v4float None %58
         %59 = OpFunctionParameter %_ptr_Function_v2float
         %60 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
      %_0_ok = OpVariable %_ptr_Function_bool Function
         %77 = OpVariable %_ptr_Function_int Function
         %82 = OpVariable %_ptr_Function_int Function
         %85 = OpVariable %_ptr_Function_v4float Function
         %62 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %65 = OpLoad %v4float %62
         %66 = OpCompositeExtract %float %65 1
         %67 = OpConvertFToS %int %66
               OpStore %x %67
               OpStore %_0_ok %false
               OpSelectionMerge %69 None
               OpSwitch %67 %73 2 %70 1 %72 0 %72
         %70 = OpLabel
               OpBranch %69
         %72 = OpLabel
               OpStore %_0_ok %true
               OpBranch %69
         %73 = OpLabel
               OpBranch %69
         %69 = OpLabel
         %74 = OpLoad %bool %_0_ok
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
               OpStore %77 %67
         %78 = OpFunctionCall %bool %switch_fallthrough_twice_bi %77
               OpBranch %76
         %76 = OpLabel
         %79 = OpPhi %bool %false %69 %78 %75
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
               OpStore %82 %67
         %83 = OpFunctionCall %bool %switch_fallthrough_groups_bi %82
               OpBranch %81
         %81 = OpLabel
         %84 = OpPhi %bool %false %76 %83 %80
               OpSelectionMerge %89 None
               OpBranchConditional %84 %87 %88
         %87 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %91 = OpLoad %v4float %90
               OpStore %85 %91
               OpBranch %89
         %88 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %94 = OpLoad %v4float %92
               OpStore %85 %94
               OpBranch %89
         %89 = OpLabel
         %95 = OpLoad %v4float %85
               OpReturnValue %95
               OpFunctionEnd
