[bits 64]
vfmadd132ss xmm1, xmm2, xmm3
vfmadd132ss xmm1, xmm2, dword [rax]
vfmadd132ss xmm1, xmm2, [rax]
vfmadd231ss xmm1, xmm2, xmm3
vfmadd231ss xmm1, xmm2, dword [rax]
vfmadd231ss xmm1, xmm2, [rax]
vfmadd213ss xmm1, xmm2, xmm3
vfmadd213ss xmm1, xmm2, dword [rax]
vfmadd213ss xmm1, xmm2, [rax]
vfmadd132sd xmm1, xmm2, xmm3
vfmadd132sd xmm1, xmm2, qword [rax]
vfmadd132sd xmm1, xmm2, [rax]
vfmadd231sd xmm1, xmm2, xmm3
vfmadd231sd xmm1, xmm2, qword [rax]
vfmadd231sd xmm1, xmm2, [rax]
vfmadd213sd xmm1, xmm2, xmm3
vfmadd213sd xmm1, xmm2, qword [rax]
vfmadd213sd xmm1, xmm2, [rax]
vfmadd132ps xmm1, xmm2, xmm3
vfmadd132ps xmm1, xmm2, xmm3
vfmadd132ps xmm1, xmm2, [rax]
vfmadd231ps xmm1, xmm2, xmm3
vfmadd231ps xmm1, xmm2, xmm3
vfmadd231ps xmm1, xmm2, [rax]
vfmadd213ps xmm1, xmm2, xmm3
vfmadd213ps xmm1, xmm2, xmm3
vfmadd213ps xmm1, xmm2, [rax]
vfmadd132ps ymm1, ymm2, ymm3
vfmadd132ps ymm1, ymm2, yword [rax]
vfmadd132ps ymm1, ymm2, [rax]
vfmadd231ps ymm1, ymm2, ymm3
vfmadd231ps ymm1, ymm2, yword [rax]
vfmadd231ps ymm1, ymm2, [rax]
vfmadd213ps ymm1, ymm2, ymm3
vfmadd213ps ymm1, ymm2, yword [rax]
vfmadd213ps ymm1, ymm2, [rax]
vfmadd132pd xmm1, xmm2, xmm3
vfmadd132pd xmm1, xmm2, dqword [rax]
vfmadd132pd xmm1, xmm2, [rax]
vfmadd231pd xmm1, xmm2, xmm3
vfmadd231pd xmm1, xmm2, dqword [rax]
vfmadd231pd xmm1, xmm2, [rax]
vfmadd213pd xmm1, xmm2, xmm3
vfmadd213pd xmm1, xmm2, dqword [rax]
vfmadd213pd xmm1, xmm2, [rax]
vfmadd132pd ymm1, ymm2, ymm3
vfmadd132pd ymm1, ymm2, yword [rax]
vfmadd132pd ymm1, ymm2, [rax]
vfmadd231pd ymm1, ymm2, ymm3
vfmadd231pd ymm1, ymm2, yword [rax]
vfmadd231pd ymm1, ymm2, [rax]
vfmadd213pd ymm1, ymm2, ymm3
vfmadd213pd ymm1, ymm2, yword [rax]
vfmadd213pd ymm1, ymm2, [rax]
vfmsub132ss xmm1, xmm2, xmm3
vfmsub132ss xmm1, xmm2, dword [rax]
vfmsub132ss xmm1, xmm2, [rax]
vfmsub231ss xmm1, xmm2, xmm3
vfmsub231ss xmm1, xmm2, dword [rax]
vfmsub231ss xmm1, xmm2, [rax]
vfmsub213ss xmm1, xmm2, xmm3
vfmsub213ss xmm1, xmm2, dword [rax]
vfmsub213ss xmm1, xmm2, [rax]
vfmsub132sd xmm1, xmm2, xmm3
vfmsub132sd xmm1, xmm2, qword [rax]
vfmsub132sd xmm1, xmm2, [rax]
vfmsub231sd xmm1, xmm2, xmm3
vfmsub231sd xmm1, xmm2, qword [rax]
vfmsub231sd xmm1, xmm2, [rax]
vfmsub213sd xmm1, xmm2, xmm3
vfmsub213sd xmm1, xmm2, qword [rax]
vfmsub213sd xmm1, xmm2, [rax]
vfmsub132ps xmm1, xmm2, xmm3
vfmsub132ps xmm1, xmm2, xmm3
vfmsub132ps xmm1, xmm2, [rax]
vfmsub231ps xmm1, xmm2, xmm3
vfmsub231ps xmm1, xmm2, xmm3
vfmsub231ps xmm1, xmm2, [rax]
vfmsub213ps xmm1, xmm2, xmm3
vfmsub213ps xmm1, xmm2, xmm3
vfmsub213ps xmm1, xmm2, [rax]
vfmsub132ps ymm1, ymm2, ymm3
vfmsub132ps ymm1, ymm2, yword [rax]
vfmsub132ps ymm1, ymm2, [rax]
vfmsub231ps ymm1, ymm2, ymm3
vfmsub231ps ymm1, ymm2, yword [rax]
vfmsub231ps ymm1, ymm2, [rax]
vfmsub213ps ymm1, ymm2, ymm3
vfmsub213ps ymm1, ymm2, yword [rax]
vfmsub213ps ymm1, ymm2, [rax]
vfmsub132pd xmm1, xmm2, xmm3
vfmsub132pd xmm1, xmm2, dqword [rax]
vfmsub132pd xmm1, xmm2, [rax]
vfmsub231pd xmm1, xmm2, xmm3
vfmsub231pd xmm1, xmm2, dqword [rax]
vfmsub231pd xmm1, xmm2, [rax]
vfmsub213pd xmm1, xmm2, xmm3
vfmsub213pd xmm1, xmm2, dqword [rax]
vfmsub213pd xmm1, xmm2, [rax]
vfmsub132pd ymm1, ymm2, ymm3
vfmsub132pd ymm1, ymm2, yword [rax]
vfmsub132pd ymm1, ymm2, [rax]
vfmsub231pd ymm1, ymm2, ymm3
vfmsub231pd ymm1, ymm2, yword [rax]
vfmsub231pd ymm1, ymm2, [rax]
vfmsub213pd ymm1, ymm2, ymm3
vfmsub213pd ymm1, ymm2, yword [rax]
vfmsub213pd ymm1, ymm2, [rax]
vfnmadd132ss xmm1, xmm2, xmm3
vfnmadd132ss xmm1, xmm2, dword [rax]
vfnmadd132ss xmm1, xmm2, [rax]
vfnmadd231ss xmm1, xmm2, xmm3
vfnmadd231ss xmm1, xmm2, dword [rax]
vfnmadd231ss xmm1, xmm2, [rax]
vfnmadd213ss xmm1, xmm2, xmm3
vfnmadd213ss xmm1, xmm2, dword [rax]
vfnmadd213ss xmm1, xmm2, [rax]
vfnmadd132sd xmm1, xmm2, xmm3
vfnmadd132sd xmm1, xmm2, qword [rax]
vfnmadd132sd xmm1, xmm2, [rax]
vfnmadd231sd xmm1, xmm2, xmm3
vfnmadd231sd xmm1, xmm2, qword [rax]
vfnmadd231sd xmm1, xmm2, [rax]
vfnmadd213sd xmm1, xmm2, xmm3
vfnmadd213sd xmm1, xmm2, qword [rax]
vfnmadd213sd xmm1, xmm2, [rax]
vfnmadd132ps xmm1, xmm2, xmm3
vfnmadd132ps xmm1, xmm2, xmm3
vfnmadd132ps xmm1, xmm2, [rax]
vfnmadd231ps xmm1, xmm2, xmm3
vfnmadd231ps xmm1, xmm2, xmm3
vfnmadd231ps xmm1, xmm2, [rax]
vfnmadd213ps xmm1, xmm2, xmm3
vfnmadd213ps xmm1, xmm2, xmm3
vfnmadd213ps xmm1, xmm2, [rax]
vfnmadd132ps ymm1, ymm2, ymm3
vfnmadd132ps ymm1, ymm2, yword [rax]
vfnmadd132ps ymm1, ymm2, [rax]
vfnmadd231ps ymm1, ymm2, ymm3
vfnmadd231ps ymm1, ymm2, yword [rax]
vfnmadd231ps ymm1, ymm2, [rax]
vfnmadd213ps ymm1, ymm2, ymm3
vfnmadd213ps ymm1, ymm2, yword [rax]
vfnmadd213ps ymm1, ymm2, [rax]
vfnmadd132pd xmm1, xmm2, xmm3
vfnmadd132pd xmm1, xmm2, dqword [rax]
vfnmadd132pd xmm1, xmm2, [rax]
vfnmadd231pd xmm1, xmm2, xmm3
vfnmadd231pd xmm1, xmm2, dqword [rax]
vfnmadd231pd xmm1, xmm2, [rax]
vfnmadd213pd xmm1, xmm2, xmm3
vfnmadd213pd xmm1, xmm2, dqword [rax]
vfnmadd213pd xmm1, xmm2, [rax]
vfnmadd132pd ymm1, ymm2, ymm3
vfnmadd132pd ymm1, ymm2, yword [rax]
vfnmadd132pd ymm1, ymm2, [rax]
vfnmadd231pd ymm1, ymm2, ymm3
vfnmadd231pd ymm1, ymm2, yword [rax]
vfnmadd231pd ymm1, ymm2, [rax]
vfnmadd213pd ymm1, ymm2, ymm3
vfnmadd213pd ymm1, ymm2, yword [rax]
vfnmadd213pd ymm1, ymm2, [rax]
vfnmsub132ss xmm1, xmm2, xmm3
vfnmsub132ss xmm1, xmm2, dword [rax]
vfnmsub132ss xmm1, xmm2, [rax]
vfnmsub231ss xmm1, xmm2, xmm3
vfnmsub231ss xmm1, xmm2, dword [rax]
vfnmsub231ss xmm1, xmm2, [rax]
vfnmsub213ss xmm1, xmm2, xmm3
vfnmsub213ss xmm1, xmm2, dword [rax]
vfnmsub213ss xmm1, xmm2, [rax]
vfnmsub132sd xmm1, xmm2, xmm3
vfnmsub132sd xmm1, xmm2, qword [rax]
vfnmsub132sd xmm1, xmm2, [rax]
vfnmsub231sd xmm1, xmm2, xmm3
vfnmsub231sd xmm1, xmm2, qword [rax]
vfnmsub231sd xmm1, xmm2, [rax]
vfnmsub213sd xmm1, xmm2, xmm3
vfnmsub213sd xmm1, xmm2, qword [rax]
vfnmsub213sd xmm1, xmm2, [rax]
vfnmsub132ps xmm1, xmm2, xmm3
vfnmsub132ps xmm1, xmm2, xmm3
vfnmsub132ps xmm1, xmm2, [rax]
vfnmsub231ps xmm1, xmm2, xmm3
vfnmsub231ps xmm1, xmm2, xmm3
vfnmsub231ps xmm1, xmm2, [rax]
vfnmsub213ps xmm1, xmm2, xmm3
vfnmsub213ps xmm1, xmm2, xmm3
vfnmsub213ps xmm1, xmm2, [rax]
vfnmsub132ps ymm1, ymm2, ymm3
vfnmsub132ps ymm1, ymm2, yword [rax]
vfnmsub132ps ymm1, ymm2, [rax]
vfnmsub231ps ymm1, ymm2, ymm3
vfnmsub231ps ymm1, ymm2, yword [rax]
vfnmsub231ps ymm1, ymm2, [rax]
vfnmsub213ps ymm1, ymm2, ymm3
vfnmsub213ps ymm1, ymm2, yword [rax]
vfnmsub213ps ymm1, ymm2, [rax]
vfnmsub132pd xmm1, xmm2, xmm3
vfnmsub132pd xmm1, xmm2, dqword [rax]
vfnmsub132pd xmm1, xmm2, [rax]
vfnmsub231pd xmm1, xmm2, xmm3
vfnmsub231pd xmm1, xmm2, dqword [rax]
vfnmsub231pd xmm1, xmm2, [rax]
vfnmsub213pd xmm1, xmm2, xmm3
vfnmsub213pd xmm1, xmm2, dqword [rax]
vfnmsub213pd xmm1, xmm2, [rax]
vfnmsub132pd ymm1, ymm2, ymm3
vfnmsub132pd ymm1, ymm2, yword [rax]
vfnmsub132pd ymm1, ymm2, [rax]
vfnmsub231pd ymm1, ymm2, ymm3
vfnmsub231pd ymm1, ymm2, yword [rax]
vfnmsub231pd ymm1, ymm2, [rax]
vfnmsub213pd ymm1, ymm2, ymm3
vfnmsub213pd ymm1, ymm2, yword [rax]
vfnmsub213pd ymm1, ymm2, [rax]
vfmaddsub132ps xmm1, xmm2, xmm3
vfmaddsub132ps xmm1, xmm2, xmm3
vfmaddsub132ps xmm1, xmm2, [rax]
vfmaddsub231ps xmm1, xmm2, xmm3
vfmaddsub231ps xmm1, xmm2, xmm3
vfmaddsub231ps xmm1, xmm2, [rax]
vfmaddsub213ps xmm1, xmm2, xmm3
vfmaddsub213ps xmm1, xmm2, xmm3
vfmaddsub213ps xmm1, xmm2, [rax]
vfmaddsub132ps ymm1, ymm2, ymm3
vfmaddsub132ps ymm1, ymm2, yword [rax]
vfmaddsub132ps ymm1, ymm2, [rax]
vfmaddsub231ps ymm1, ymm2, ymm3
vfmaddsub231ps ymm1, ymm2, yword [rax]
vfmaddsub231ps ymm1, ymm2, [rax]
vfmaddsub213ps ymm1, ymm2, ymm3
vfmaddsub213ps ymm1, ymm2, yword [rax]
vfmaddsub213ps ymm1, ymm2, [rax]
vfmaddsub132pd xmm1, xmm2, xmm3
vfmaddsub132pd xmm1, xmm2, dqword [rax]
vfmaddsub132pd xmm1, xmm2, [rax]
vfmaddsub231pd xmm1, xmm2, xmm3
vfmaddsub231pd xmm1, xmm2, dqword [rax]
vfmaddsub231pd xmm1, xmm2, [rax]
vfmaddsub213pd xmm1, xmm2, xmm3
vfmaddsub213pd xmm1, xmm2, dqword [rax]
vfmaddsub213pd xmm1, xmm2, [rax]
vfmaddsub132pd ymm1, ymm2, ymm3
vfmaddsub132pd ymm1, ymm2, yword [rax]
vfmaddsub132pd ymm1, ymm2, [rax]
vfmaddsub231pd ymm1, ymm2, ymm3
vfmaddsub231pd ymm1, ymm2, yword [rax]
vfmaddsub231pd ymm1, ymm2, [rax]
vfmaddsub213pd ymm1, ymm2, ymm3
vfmaddsub213pd ymm1, ymm2, yword [rax]
vfmaddsub213pd ymm1, ymm2, [rax]
vfmsubadd132ps xmm1, xmm2, xmm3
vfmsubadd132ps xmm1, xmm2, xmm3
vfmsubadd132ps xmm1, xmm2, [rax]
vfmsubadd231ps xmm1, xmm2, xmm3
vfmsubadd231ps xmm1, xmm2, xmm3
vfmsubadd231ps xmm1, xmm2, [rax]
vfmsubadd213ps xmm1, xmm2, xmm3
vfmsubadd213ps xmm1, xmm2, xmm3
vfmsubadd213ps xmm1, xmm2, [rax]
vfmsubadd132ps ymm1, ymm2, ymm3
vfmsubadd132ps ymm1, ymm2, yword [rax]
vfmsubadd132ps ymm1, ymm2, [rax]
vfmsubadd231ps ymm1, ymm2, ymm3
vfmsubadd231ps ymm1, ymm2, yword [rax]
vfmsubadd231ps ymm1, ymm2, [rax]
vfmsubadd213ps ymm1, ymm2, ymm3
vfmsubadd213ps ymm1, ymm2, yword [rax]
vfmsubadd213ps ymm1, ymm2, [rax]
vfmsubadd132pd xmm1, xmm2, xmm3
vfmsubadd132pd xmm1, xmm2, dqword [rax]
vfmsubadd132pd xmm1, xmm2, [rax]
vfmsubadd231pd xmm1, xmm2, xmm3
vfmsubadd231pd xmm1, xmm2, dqword [rax]
vfmsubadd231pd xmm1, xmm2, [rax]
vfmsubadd213pd xmm1, xmm2, xmm3
vfmsubadd213pd xmm1, xmm2, dqword [rax]
vfmsubadd213pd xmm1, xmm2, [rax]
vfmsubadd132pd ymm1, ymm2, ymm3
vfmsubadd132pd ymm1, ymm2, yword [rax]
vfmsubadd132pd ymm1, ymm2, [rax]
vfmsubadd231pd ymm1, ymm2, ymm3
vfmsubadd231pd ymm1, ymm2, yword [rax]
vfmsubadd231pd ymm1, ymm2, [rax]
vfmsubadd213pd ymm1, ymm2, ymm3
vfmsubadd213pd ymm1, ymm2, yword [rax]
vfmsubadd213pd ymm1, ymm2, [rax]
