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
               OpName %switch_fallthrough_groups_bi "switch_fallthrough_groups_bi"
               OpName %ok_0 "ok"
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
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %39 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %25 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %56 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%switch_fallthrough_twice_bi = OpFunction %bool None %25
         %26 = OpFunctionParameter %_ptr_Function_int
         %27 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
               OpStore %ok %false
         %31 = OpLoad %int %26
               OpSelectionMerge %32 None
               OpSwitch %31 %37 0 %33 1 %36 2 %36 3 %36
         %33 = OpLabel
               OpBranch %32
         %36 = OpLabel
               OpStore %ok %true
               OpBranch %32
         %37 = OpLabel
               OpBranch %32
         %32 = OpLabel
         %39 = OpLoad %bool %ok
               OpReturnValue %39
               OpFunctionEnd
%switch_fallthrough_groups_bi = OpFunction %bool None %25
         %40 = OpFunctionParameter %_ptr_Function_int
         %41 = OpLabel
       %ok_0 = OpVariable %_ptr_Function_bool Function
               OpStore %ok_0 %false
         %43 = OpLoad %int %40
               OpSelectionMerge %44 None
               OpSwitch %43 %54 -1 %45 0 %46 1 %47 2 %49 3 %49 4 %50 5 %54 6 %54 7 %54
         %45 = OpLabel
               OpStore %ok_0 %false
               OpBranch %46
         %46 = OpLabel
               OpReturnValue %false
         %47 = OpLabel
               OpStore %ok_0 %true
               OpBranch %49
         %49 = OpLabel
               OpBranch %44
         %50 = OpLabel
               OpStore %ok_0 %false
               OpBranch %54
         %54 = OpLabel
               OpBranch %44
         %44 = OpLabel
         %55 = OpLoad %bool %ok_0
               OpReturnValue %55
               OpFunctionEnd
       %main = OpFunction %v4float None %56
         %57 = OpFunctionParameter %_ptr_Function_v2float
         %58 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
      %_0_ok = OpVariable %_ptr_Function_bool Function
         %75 = OpVariable %_ptr_Function_int Function
         %80 = OpVariable %_ptr_Function_int Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %60 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %63 = OpLoad %v4float %60
         %64 = OpCompositeExtract %float %63 1
         %65 = OpConvertFToS %int %64
               OpStore %x %65
               OpStore %_0_ok %false
               OpSelectionMerge %67 None
               OpSwitch %65 %71 2 %68 1 %70 0 %70
         %68 = OpLabel
               OpBranch %67
         %70 = OpLabel
               OpStore %_0_ok %true
               OpBranch %67
         %71 = OpLabel
               OpBranch %67
         %67 = OpLabel
         %72 = OpLoad %bool %_0_ok
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
               OpStore %75 %65
         %76 = OpFunctionCall %bool %switch_fallthrough_twice_bi %75
               OpBranch %74
         %74 = OpLabel
         %77 = OpPhi %bool %false %67 %76 %73
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
               OpStore %80 %65
         %81 = OpFunctionCall %bool %switch_fallthrough_groups_bi %80
               OpBranch %79
         %79 = OpLabel
         %82 = OpPhi %bool %false %74 %81 %78
               OpSelectionMerge %87 None
               OpBranchConditional %82 %85 %86
         %85 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %89 = OpLoad %v4float %88
               OpStore %83 %89
               OpBranch %87
         %86 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %92 = OpLoad %v4float %90
               OpStore %83 %92
               OpBranch %87
         %87 = OpLabel
         %93 = OpLoad %v4float %83
               OpReturnValue %93
               OpFunctionEnd
