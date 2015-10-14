.text
foo:
movl %eax, foo@PLT
movl %eax, foo@GOTOFF
#movl %eax, foo@GOTPC
movl %eax, bar@TLSGD
movl %eax, bar@TLSLDM
movl %eax, bar@GOTTPOFF
movl %eax, bar@TPOFF
movl %eax, bar@NTPOFF
movl %eax, bar@DTPOFF
movl %eax, bar@GOTNTPOFF
movl %eax, bar@INDNTPOFF
movl %eax, foo@GOT
