struc PPC_CPU_State
	dummy:
        gpr:
	fpr:
	cr:
	fpscr:
	xer:
	xer_ca:
	lr:
	ctr:

	msr:
	pvr:
	
	ibatu:
	ibatl:
	ibat_bl17:
	
	dbatu:
	dbatl:
	dbat_bl17:
	
	sdr1:
	
	sr:

	dar:
	dsisr:
	sprg:
	srr0:
	srr1:

	decr:
	ear:
	pir:
	tb:

	hid:

	pc:
	npc:
	current_opc:
	
	exception_pending:
	dec_exception:
	ext_exception:
	stop_exception:
	singlestep_ignore:
	align1:
	align2:
	align3:
	
	pagetable_base:
	pagetable_hashmask:
	reserve:
	have_reservation:
	
	tlb_last:
	tlb_pa:
	tlb_va:
	
	effective_code_page:
	physical_code_page:
	pdec:
	ptb:

	temp:
	temp2:
	x87cw:
	pc_ofs:
	current_code_base:
endstruc

struc JITC
	clientPages
	
	tlb_code_0_eff
	tlb_data_0_eff
	tlb_data_8_eff
	tlb_code_0_phys
	tlb_data_0_phys
	tlb_data_8_phys
	tlb_code_0_hits
	tlb_data_0_hits
	tlb_data_8_hits
	tlb_code_0_misses
	tlb_data_0_misses
	tlb_data_8_misses

	nativeReg
	
	nativeRegState
	
	nativeFlags

	nativeFlagsState
	nativeCarryState
	
	clientReg
	
	nativeRegsList
		 
	LRUreg
	MRUreg

	LRUpage
	MRUpage

	freeFragmentsList

	freeClientPages
	
	translationCache
endstruc

extern gCPU, gJITC, gMemory, gMemorySize, 
extern jitc_error, ppc_isi_exception_asm, ppc_dsi_exception_asm
extern jitcDestroyAndFreeClientPage
extern io_mem_read_glue
extern io_mem_write_glue
extern io_mem_read64_glue
extern io_mem_write64_glue
extern io_mem_read128_glue
extern io_mem_write128_glue
extern io_mem_read128_native_glue
extern io_mem_write128_native_glue
global ppc_effective_to_physical_code, ppc_effective_to_physical_data
global ppc_write_effective_byte_asm
global ppc_write_effective_half_asm
global ppc_write_effective_word_asm
global ppc_write_effective_dword_asm
global ppc_write_effective_qword_asm
global ppc_write_effective_qword_sse_asm
global ppc_read_effective_byte_asm
global ppc_read_effective_half_z_asm
global ppc_read_effective_half_s_asm
global ppc_read_effective_word_asm
global ppc_read_effective_dword_asm
global ppc_read_effective_qword_asm
global ppc_read_effective_qword_sse_asm
global ppc_mmu_tlb_invalidate_all_asm
global ppc_mmu_tlb_invalidate_entry_asm
global ppc_opc_lswi_asm
global ppc_opc_stswi_asm
global ppc_opc_icbi_asm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
ppc_mmu_tlb_invalidate_all_asm:
	mov	edi, gJITC+tlb_code_0_eff

ppc_mmu_tlb_invalidate_entry_asm:
	
ppc_pte_protection:

%macro bat_lookup 4
	%%npr:
	%%ok:
%%bat_lookup_failed:
%endmacro

%macro pg_table_lookup 3
%%invalid:
%endmacro

protection_fault_0_code:
protection_fault_0_data:
protection_fault_8_data:

%macro tlb_lookup 2
%%tlb_lookup_failed:
%endmacro

ppc_effective_to_physical_code_ret:
ppc_effective_to_physical_code:
	tlb_lookup 0, code

	bat_lookup i, 0, 0, code
	bat_lookup i, 1, 0, code
	bat_lookup i, 2, 0, code
	bat_lookup i, 3, 0, code

	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	pg_table_lookup 0, 0, code
	
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code
	pg_table_lookup (1<<6), 0, code

.noexec:
ppc_effective_to_physical_data_read_ret:
ppc_effective_to_physical_data_read:
	tlb_lookup 0, data

	bat_lookup d, 0, 0, data
	bat_lookup d, 1, 0, data
	bat_lookup d, 2, 0, data
	bat_lookup d, 3, 0, data

	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	pg_table_lookup 0, 0, data
	
	pg_table_lookup (1<<6), 0, data
	pg_table_lookup (1<<6), 0, data
	pg_table_lookup (1<<6), 0, data 
	pg_table_lookup (1<<6), 0, data
	pg_table_lookup (1<<6), 0, data
	pg_table_lookup (1<<6), 0, data
	pg_table_lookup (1<<6), 0, data
	pg_table_lookup (1<<6), 0, data

ppc_effective_to_physical_data_write_ret:
ppc_effective_to_physical_data_write:
	tlb_lookup 8, data

	bat_lookup d, 0, 8, data
	bat_lookup d, 1, 8, data
	bat_lookup d, 2, 8, data
	bat_lookup d, 3, 8, data
	
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	pg_table_lookup 0, 8, data
	
	pg_table_lookup (1<<6), 8, data
	pg_table_lookup (1<<6), 8, data
	pg_table_lookup (1<<6), 8, data 
	pg_table_lookup (1<<6), 8, data
	pg_table_lookup (1<<6), 8, data
	pg_table_lookup (1<<6), 8, data
	pg_table_lookup (1<<6), 8, data
	pg_table_lookup (1<<6), 8, data

ppc_write_effective_byte_asm:
.mmio:
ppc_write_effective_half_asm:
.mmio:
.overlap:
	.overlapped_mmio_1_back:
.overlapped_mmio_1:
.overlapped_mmio_2:
ppc_write_effective_word_asm:
.mmio:
.overlap:
	.loop1:
	.overlapped_mmio_1_back:
	.loop2:
.overlapped_mmio_1:
	.overlapped_mmio_1_loop:
.overlapped_mmio_2:
	.overlapped_mmio_2_loop:
ppc_write_effective_dword_asm:
.mmio:

.overlap:
	.loop1:
	.overlapped_mmio_1_back:
	.loop2:
.overlapped_mmio_1:
	.overlapped_mmio_1_loop:
.overlapped_mmio_2:
	.overlapped_mmio_2_loop:
ppc_write_effective_qword_asm:
.mmio:

ppc_write_effective_qword_sse_asm:
.mmio:
ppc_read_effective_byte_asm:
.mmio:
ppc_read_effective_half_z_asm:
.mmio:
.overlap:
.loop1:
.mmio1:
.mmio2:
ppc_read_effective_half_s_asm:
.mmio:
.overlap:
.loop1:

.mmio1:
.mmio2:
ppc_read_effective_word_asm:
.mmio:
.overlap:
	.loop1:
	.overlapped_mmio_1_back:
	.loop2:

.overlapped_mmio_1:
	.overlapped_mmio_1_loop:
.overlapped_mmio_2:
	.overlapped_mmio_2_loop:
ppc_read_effective_dword_asm:
.mmio:
.overlap:
	.loop1:
	.overlapped_mmio_1_back:
	.loop2:

.overlapped_mmio_1:
	.overlapped_mmio_1_loop:
.overlapped_mmio_2:
	.overlapped_mmio_2_loop:
ppc_read_effective_qword_asm:
.mmio:

ppc_read_effective_qword_sse_asm:
.mmio:
ppc_opc_stswi_asm:
.loop:
	.ok1:
.back:
.mmio:
ppc_opc_lswi_asm:
.loop:
	.ok1:
.back:
	.loop2:
.ret:
.mmio:
ppc_opc_icbi_asm:
.destroy:
.ok:
