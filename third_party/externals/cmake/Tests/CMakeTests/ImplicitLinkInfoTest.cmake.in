# This is not supposed to be included by user code, but we need to
# test it.
include(${CMAKE_ROOT}/Modules/CMakeParseImplicitLinkInfo.cmake)

#-----------------------------------------------------------------------------
# Linux

# gcc dummy.c -v
set(linux64_gcc_text " /usr/lib/gcc/x86_64-linux-gnu/4.3.3/collect2 --eh-frame-hdr -m elf_x86_64 --hash-style=both -dynamic-linker /lib64/ld-linux-x86-64.so.2 /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crt1.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/crtbegin.o -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3 -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3 -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib -L/lib/../lib -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../.. -L/usr/lib/x86_64-linux-gnu /tmp/ccEO9iux.o -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-linux-gnu/4.3.3/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crtn.o")
set(linux64_gcc_libs "c")
set(linux64_gcc_dirs "/usr/lib/gcc/x86_64-linux-gnu/4.3.3;/usr/lib;/lib;/usr/lib/x86_64-linux-gnu")
list(APPEND platforms linux64_gcc)

# g++ dummy.cxx -v
set(linux64_g++_text " /usr/lib/gcc/x86_64-linux-gnu/4.3.3/collect2 --eh-frame-hdr -m elf_x86_64 --hash-style=both -dynamic-linker /lib64/ld-linux-x86-64.so.2 /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crt1.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/crtbegin.o -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3 -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3 -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib -L/lib/../lib -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../.. -L/usr/lib/x86_64-linux-gnu /tmp/ccalRBlq.o -lstdc++ -lm -lgcc_s -lgcc -lc -lgcc_s -lgcc /usr/lib/gcc/x86_64-linux-gnu/4.3.3/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crtn.o")
set(linux64_g++_libs "stdc++;m;c")
set(linux64_g++_dirs "/usr/lib/gcc/x86_64-linux-gnu/4.3.3;/usr/lib;/lib;/usr/lib/x86_64-linux-gnu")
list(APPEND platforms linux64_g++)

# f95 dummy.f -v
set(linux64_f95_text " /usr/lib/gcc/x86_64-linux-gnu/4.3.3/collect2 --eh-frame-hdr -m elf_x86_64 --hash-style=both -dynamic-linker /lib64/ld-linux-x86-64.so.2 /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crt1.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/crtbegin.o -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3 -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3 -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib -L/lib/../lib -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../.. -L/usr/lib/x86_64-linux-gnu /tmp/ccAVcN7N.o -lgfortranbegin -lgfortran -lm -lgcc_s -lgcc -lc -lgcc_s -lgcc /usr/lib/gcc/x86_64-linux-gnu/4.3.3/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.3.3/../../../../lib/crtn.o")
set(linux64_f95_libs "gfortranbegin;gfortran;m;c")
set(linux64_f95_dirs "/usr/lib/gcc/x86_64-linux-gnu/4.3.3;/usr/lib;/lib;/usr/lib/x86_64-linux-gnu")
list(APPEND platforms linux64_f95)

# suncc dummy.c '-#'
set(linux64_suncc_text "/usr/bin/ld --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 /opt/sun/sunstudio12/prod/lib/amd64/crti.o /opt/sun/sunstudio12/prod/lib/amd64/crt1x.o /opt/sun/sunstudio12/prod/lib/amd64/values-xa.o dummy.o -Y \"/opt/sun/sunstudio12/prod/lib/amd64:/lib64:/usr/lib64\" -Qy -lc /opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a /opt/sun/sunstudio12/prod/lib/amd64/crtn.o")
set(linux64_suncc_libs "c;/opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a")
set(linux64_suncc_dirs "/opt/sun/sunstudio12/prod/lib/amd64;/lib64;/usr/lib64")
list(APPEND platforms linux64_suncc)

# sunCC dummy.cxx -v
set(linux64_sunCC_text "/opt/sun/sunstudio12/prod/lib/amd64/ld -u __1cH__CimplKcplus_init6F_v_ --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -R/opt/sun/sunstudio12/lib/rw7/amd64:/opt/sun/sunstudio12/lib/amd64:/opt/sun/sunstudio12/rtlibs/amd64:/opt/sun/lib/rtlibs/amd64:/opt/SUNWspro/lib/amd64:/lib64:/usr/lib64 -o a.out /opt/sun/sunstudio12/prod/lib/amd64/crti.o /opt/sun/sunstudio12/prod/lib/amd64/CCrti.o /opt/sun/sunstudio12/prod/lib/amd64/crt1x.o -Y P,/opt/sun/sunstudio12/lib/rw7/amd64:/opt/sun/sunstudio12/lib/amd64:/opt/sun/sunstudio12/rtlibs/amd64:/opt/sun/sunstudio12/prod/lib/rw7/amd64:/opt/sun/sunstudio12/prod/lib/amd64:/lib64:/usr/lib64 dummy.o -lCstd -lCrun -lm -lc /opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a /opt/sun/sunstudio12/prod/lib/amd64/CCrtn.o /opt/sun/sunstudio12/prod/lib/amd64/crtn.o >&/tmp/ld.04973.2.err")
set(linux64_sunCC_libs "Cstd;Crun;m;c;/opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a")
set(linux64_sunCC_dirs "/opt/sun/sunstudio12/lib/rw7/amd64;/opt/sun/sunstudio12/lib/amd64;/opt/sun/sunstudio12/rtlibs/amd64;/opt/sun/sunstudio12/prod/lib/rw7/amd64;/opt/sun/sunstudio12/prod/lib/amd64;/lib64;/usr/lib64")
list(APPEND platforms linux64_sunCC)

# sunf90 dummy.f -v
set(linux64_sunf90_text "/usr/bin/ld --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -R/opt/sun/sunstudio12/lib/amd64:/opt/sun/sunstudio12/rtlibs/amd64:/opt/sun/lib/rtlibs/amd64 -o a.out /opt/sun/sunstudio12/prod/lib/amd64/crti.o /opt/sun/sunstudio12/prod/lib/amd64/crt1x.o /opt/sun/sunstudio12/prod/lib/amd64/values-xi.o -Y P,/opt/sun/sunstudio12/lib/amd64:/opt/sun/sunstudio12/rtlibs/amd64:/opt/sun/sunstudio12/prod/lib/amd64:/lib64:/usr/lib64 dummy.o -lfui -lfai -lfsu -Bdynamic -lmtsk -lpthread -lm -lc /opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a /opt/sun/sunstudio12/prod/lib/amd64/crtn.o")
set(linux64_sunf90_libs "fui;fai;fsu;mtsk;pthread;m;c;/opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a")
set(linux64_sunf90_dirs "/opt/sun/sunstudio12/lib/amd64;/opt/sun/sunstudio12/rtlibs/amd64;/opt/sun/sunstudio12/prod/lib/amd64;/lib64;/usr/lib64")
list(APPEND platforms linux64_sunf90)

# icc dummy.c -v
set(linux64_icc_text "ld    /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crt1.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crti.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtbegin.o --eh-frame-hdr -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o a.out /tmp/iccBP8OfN.o -L/opt/compiler/intel/compiler/11.0/lib/intel64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../x86_64-suse-linux/lib -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../.. -L/lib64 -L/lib -L/usr/lib64 -L/usr/lib -Bstatic -limf -lsvml -Bdynamic -lm -Bstatic -lipgo -ldecimal -lirc -Bdynamic -lgcc_s -lgcc -Bstatic -lirc -Bdynamic -lc -lgcc_s -lgcc -Bstatic -lirc_s -Bdynamic -ldl -lc /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtend.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crtn.o")
set(linux64_icc_libs "imf;svml;m;ipgo;decimal;irc;irc;c;irc_s;dl;c")
set(linux64_icc_dirs "/opt/compiler/intel/compiler/11.0/lib/intel64;/usr/lib64/gcc/x86_64-suse-linux/4.1.2;/usr/lib64;/lib64;/usr/x86_64-suse-linux/lib;/lib;/usr/lib")
list(APPEND platforms linux64_icc)

# icxx dummy.cxx -v
set(linux64_icxx_text "ld    /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crt1.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crti.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtbegin.o --eh-frame-hdr -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o a.out /tmp/icpc270GoT.o -L/opt/compiler/intel/compiler/11.0/lib/intel64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../x86_64-suse-linux/lib -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../.. -L/lib64 -L/lib -L/usr/lib64 -L/usr/lib -Bstatic -limf -lsvml -Bdynamic -lm -Bstatic -lipgo -ldecimal -Bdynamic -lstdc++ -Bstatic -lirc -Bdynamic -lgcc_s -lgcc -Bstatic -lirc -Bdynamic -lc -lgcc_s -lgcc -Bstatic -lirc_s -Bdynamic -ldl -lc /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtend.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crtn.o")
set(linux64_icxx_libs "imf;svml;m;ipgo;decimal;stdc++;irc;irc;c;irc_s;dl;c")
set(linux64_icxx_dirs "/opt/compiler/intel/compiler/11.0/lib/intel64;/usr/lib64/gcc/x86_64-suse-linux/4.1.2;/usr/lib64;/lib64;/usr/x86_64-suse-linux/lib;/lib;/usr/lib")
list(APPEND platforms linux64_icxx)

# ifort dummy.f -v
set(linux64_ifort_text "ld    --eh-frame-hdr -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o a.out -L/opt/compiler/intel/compiler/11.0/lib/intel64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../x86_64-suse-linux/lib -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../.. -L/lib64 -L/lib -L/usr/lib64 -L/usr/lib /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crt1.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crti.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtbegin.o /opt/compiler/intel/compiler/11.0/lib/intel64/for_main.o dum.cxx -Bstatic -lifport -lifcore -limf -lsvml -Bdynamic -lm -Bstatic -lipgo -lirc -Bdynamic -lpthread -lc -lgcc_s -lgcc -Bstatic -lirc_s -Bdynamic -ldl -lc /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtend.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/../../../../lib64/crtn.o")
set(linux64_ifort_libs "ifport;ifcore;imf;svml;m;ipgo;irc;pthread;c;irc_s;dl;c")
set(linux64_ifort_dirs "/opt/compiler/intel/compiler/11.0/lib/intel64;/usr/lib64/gcc/x86_64-suse-linux/4.1.2;/usr/lib64;/lib64;/usr/x86_64-suse-linux/lib;/lib;/usr/lib")
list(APPEND platforms linux64_ifort)

# pgcc dummy.c -v
set(linux64_pgcc_text "/usr/bin/ld /usr/lib64/crt1.o /usr/lib64/crti.o /opt/compiler/pgi/linux86-64/8.0-3/lib/trace_init.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtbegin.o -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 /opt/compiler/pgi/linux86-64/8.0-3/lib/pgi.ld -L/opt/compiler/pgi/linux86-64/8.0-3/lib -L/usr/lib64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2 /tmp/pgcc7OscXa5ur7Zk.o -rpath /opt/compiler/pgi/linux86-64/8.0-3/lib -lnspgc -lpgc -lm -lgcc -lc -lgcc /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtend.o /usr/lib64/crtn.o")
set(linux64_pgcc_libs "nspgc;pgc;m;c")
set(linux64_pgcc_dirs "/opt/compiler/pgi/linux86-64/8.0-3/lib;/usr/lib64;/usr/lib64/gcc/x86_64-suse-linux/4.1.2")
list(APPEND platforms linux64_pgcc)

# pgCC dummy.cxx -v
set(linux64_pgCC_text "/usr/bin/ld /usr/lib64/crt1.o /usr/lib64/crti.o /opt/compiler/pgi/linux86-64/8.0-3/lib/trace_init.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtbegin.o -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 /opt/compiler/pgi/linux86-64/8.0-3/lib/pgi.ld -L/opt/compiler/pgi/linux86-64/8.0-3/lib -L/usr/lib64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2 /tmp/pgCCFhjcDt1fs1Ki.o -rpath /opt/compiler/pgi/linux86-64/8.0-3/lib -lstd -lC -lnspgc -lpgc -lm -lgcc -lc -lgcc /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtend.o /usr/lib64/crtn.o")
set(linux64_pgCC_libs "std;C;nspgc;pgc;m;c")
set(linux64_pgCC_dirs "/opt/compiler/pgi/linux86-64/8.0-3/lib;/usr/lib64;/usr/lib64/gcc/x86_64-suse-linux/4.1.2")
list(APPEND platforms linux64_pgCC)

# pgf90 dummy.f -v
set(linux64_pgf90_text "/usr/bin/ld /usr/lib64/crt1.o /usr/lib64/crti.o /opt/compiler/pgi/linux86-64/8.0-3/lib/trace_init.o /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtbegin.o /opt/compiler/pgi/linux86-64/8.0-3/lib/f90main.o -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 /opt/compiler/pgi/linux86-64/8.0-3/lib/pgi.ld -L/opt/compiler/pgi/linux86-64/8.0-3/lib -L/usr/lib64 -L/usr/lib64/gcc/x86_64-suse-linux/4.1.2 /tmp/pgf90QOIc_eB9xY5h.o -rpath /opt/compiler/pgi/linux86-64/8.0-3/lib -lpgf90 -lpgf90_rpm1 -lpgf902 -lpgf90rtl -lpgftnrtl -lnspgc -lpgc -lrt -lpthread -lm -lgcc -lc -lgcc /usr/lib64/gcc/x86_64-suse-linux/4.1.2/crtend.o /usr/lib64/crtn.o")
set(linux64_pgf90_libs "pgf90;pgf90_rpm1;pgf902;pgf90rtl;pgftnrtl;nspgc;pgc;rt;pthread;m;c")
set(linux64_pgf90_dirs "/opt/compiler/pgi/linux86-64/8.0-3/lib;/usr/lib64;/usr/lib64/gcc/x86_64-suse-linux/4.1.2")
list(APPEND platforms linux64_pgf90)

# nagfor dummy.f -Wl,-v
set(linux64_nagfor_text " /usr/libexec/gcc/x86_64-redhat-linux/4.4.5/collect2 --no-add-needed --eh-frame-hdr --build-id -m elf_x86_64 --hash-style=gnu -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o a.out /usr/lib/gcc/x86_64-redhat-linux/4.4.5/../../../../lib64/crt1.o /usr/lib/gcc/x86_64-redhat-linux/4.4.5/../../../../lib64/crti.o /usr/lib/gcc/x86_64-redhat-linux/4.4.5/crtbegin.o -L/usr/lib/gcc/x86_64-redhat-linux/4.4.5 -L/usr/lib/gcc/x86_64-redhat-linux/4.4.5 -L/usr/lib/gcc/x86_64-redhat-linux/4.4.5/../../../../lib64 -L/lib/../lib64 -L/usr/lib/../lib64 -L/usr/lib/gcc/x86_64-redhat-linux/4.4.5/../../.. /usr/local/NAG/lib/f90_init.o /usr/local/NAG/lib/quickfit.o dummy.o -rpath /usr/local/NAG/lib /usr/local/NAG/lib/libf53.so /usr/local/NAG/lib/libf53.a -lm -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-redhat-linux/4.4.5/crtend.o /usr/lib/gcc/x86_64-redhat-linux/4.4.5/../../../../lib64/crtn.o")
set(linux64_nagfor_libs "/usr/local/NAG/lib/f90_init.o;/usr/local/NAG/lib/quickfit.o;/usr/local/NAG/lib/libf53.a;m;c")
set(linux64_nagfor_dirs "/usr/lib/gcc/x86_64-redhat-linux/4.4.5;/usr/lib64;/lib64;/usr/lib")
set(linux64_nagfor_obj_regex "^/usr/local/NAG/lib")
list(APPEND platforms linux64_nagfor)

# absoft dummy.f -X -v
set(linux64_absoft_text "collect2 version 4.4.5 (x86-64 Linux/ELF)
/usr/bin/ld --build-id --eh-frame-hdr -m elf_x86_64 --hash-style=both -dynamic-linker /lib64/ld-linux-x86-64.so.2 /usr/lib/gcc/x86_64-linux-gnu/4.4.5/../../../../lib/crt1.o /usr/lib/gcc/x86_64-linux-gnu/4.4.5/../../../../lib/crti.o /usr/lib/gcc/x86_64-linux-gnu/4.4.5/crtbegin.o -L/opt/absoft11.1/lib64 -L/usr/lib/gcc/x86_64-linux-gnu/4.4.5 -L/usr/lib/gcc/x86_64-linux-gnu/4.4.5 -L/usr/lib/gcc/x86_64-linux-gnu/4.4.5/../../../../lib -L/lib/../lib -L/usr/lib/../lib -L/usr/lib/gcc/x86_64-linux-gnu/4.4.5/../../.. /tmp/E3Bii1/dummy.o -v -laf90math -lafio -lamisc -labsoftmain -laf77math -lm -lmv -lpthread -lgcc --as-needed -lgcc_s --no-as-needed -lc -lgcc --as-needed -lgcc_s --no-as-needed /usr/lib/gcc/x86_64-linux-gnu/4.4.5/crtend.o /usr/lib/gcc/x86_64-linux-gnu/4.4.5/../../../../lib/crtn.o")
set(linux64_absoft_libs "af90math;afio;amisc;absoftmain;af77math;m;mv;pthread;c")
set(linux64_absoft_dirs "/opt/absoft11.1/lib64;/usr/lib/gcc/x86_64-linux-gnu/4.4.5;/usr/lib;/lib")
list(APPEND platforms linux64_absoft)

# gcc dummy.c -v # in strange path
set(linux64_test1_text "
/this/might/match/as/a/linker/ld/but/it/is/not because the ld is not the last path component
${linux64_gcc_text}")
set(linux64_test1_libs "${linux64_gcc_libs}")
set(linux64_test1_dirs "${linux64_gcc_dirs}")
list(APPEND platforms linux64_test1)

# sunCC dummy.cxx -v # extra slashes
set(linux64_test2_text "/usr/bin/ld --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 /opt/sun/sunstudio12/prod/lib/amd64//crti.o /opt/sun/sunstudio12/prod/lib/amd64/crt1x.o /opt/sun/sunstudio12/prod/lib/amd64/values-xa.o dummy.o -Y \"/opt/sun/sunstudio12/prod/lib//amd64:/lib64:/usr//lib64\" -Qy -lc /opt/sun/sunstudio12/prod/lib/amd64//libc_supp.a /opt/sun/sunstudio12/prod/lib/amd64/crtn.o")
set(linux64_test2_libs "c;/opt/sun/sunstudio12/prod/lib/amd64/libc_supp.a")
set(linux64_test2_dirs "/opt/sun/sunstudio12/prod/lib/amd64;/lib64;/usr/lib64")
list(APPEND platforms linux64_test2)

#-----------------------------------------------------------------------------
# Mac

# gcc -arch i686 dummy.c -v -Wl,-v
set(mac_i686_gcc_Wlv_text " /usr/libexec/gcc/i686-apple-darwin10/4.2.1/collect2 -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/tmp//ccnhXAGL.o -lSystem -lgcc -lSystem
collect2 version 4.2.1 (Apple Inc. build 5646) (i686 Darwin)
/usr/libexec/gcc/i686-apple-darwin10/4.2.1/ld -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/tmp//ccSiuUhD.o -v -lSystem -lgcc -lSystem
@(#)PROGRAM:ld  PROJECT:ld64-95.2.12
Library search paths:
	/usr/lib/i686-apple-darwin10/4.2.1
	/usr/lib/gcc/i686-apple-darwin10/4.2.1
	/usr/lib/gcc/i686-apple-darwin10/4.2.1
	/usr/lib/i686-apple-darwin10/4.2.1
	/usr/lib
	/usr/lib
	/usr/local/lib
Framework search paths:
	/Library/Frameworks/
	/System/Library/Frameworks/")
set(mac_i686_gcc_Wlv_libs "")
set(mac_i686_gcc_Wlv_dirs "/usr/lib/i686-apple-darwin10/4.2.1;/usr/lib/gcc/i686-apple-darwin10/4.2.1;/usr/lib;/usr/local/lib")
set(mac_i686_gcc_Wlv_fwks "/Library/Frameworks;/System/Library/Frameworks")
list(APPEND platforms mac_i686_gcc_Wlv)

# gcc -arch i686 dummy.c -v
set(mac_i686_gcc_text " /usr/libexec/gcc/i686-apple-darwin10/4.2.1/collect2 -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/tmp//ccnhXAGL.o -lSystem -lgcc -lSystem")
set(mac_i686_gcc_libs "")
set(mac_i686_gcc_dirs "/usr/lib/i686-apple-darwin10/4.2.1;/usr/lib/gcc/i686-apple-darwin10/4.2.1;/usr/lib")
list(APPEND platforms mac_i686_gcc)

# g++ -arch i686 dummy.cxx -v -Wl,-v
set(mac_i686_g++_Wlv_text " /usr/libexec/gcc/i686-apple-darwin10/4.2.1/collect2 -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/tmp//ccEXXICh.o -lstdc++ -lSystem -lgcc -lSystem
collect2 version 4.2.1 (Apple Inc. build 5646) (i686 Darwin)
/usr/libexec/gcc/i686-apple-darwin10/4.2.1/ld -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/tmp//ccWrBoVl.o -v -lstdc++ -lSystem -lgcc -lSystem
@(#)PROGRAM:ld  PROJECT:ld64-95.2.12
Library search paths:
	/usr/lib/i686-apple-darwin10/4.2.1
	/usr/lib/gcc/i686-apple-darwin10/4.2.1
	/usr/lib/gcc/i686-apple-darwin10/4.2.1
	/usr/lib/i686-apple-darwin10/4.2.1
	/usr/lib
	/usr/lib
	/usr/local/lib
Framework search paths:
	/Library/Frameworks/
	/System/Library/Frameworks/")
set(mac_i686_g++_Wlv_libs "stdc++")
set(mac_i686_g++_Wlv_dirs "/usr/lib/i686-apple-darwin10/4.2.1;/usr/lib/gcc/i686-apple-darwin10/4.2.1;/usr/lib;/usr/local/lib")
set(mac_i686_g++_Wlv_fwks "/Library/Frameworks;/System/Library/Frameworks")
list(APPEND platforms mac_i686_g++_Wlv)

# g++ -arch i686 dummy.cxx -v
set(mac_i686_g++_text " /usr/libexec/gcc/i686-apple-darwin10/4.2.1/collect2 -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/tmp//ccEXXICh.o -lstdc++ -lSystem -lgcc -lSystem")
set(mac_i686_g++_libs "stdc++")
set(mac_i686_g++_dirs "/usr/lib/i686-apple-darwin10/4.2.1;/usr/lib/gcc/i686-apple-darwin10/4.2.1;/usr/lib")
list(APPEND platforms mac_i686_g++)

# gfortran dummy.f -v -Wl,-v
set(mac_i686_gfortran_Wlv_text " /usr/local/libexec/gcc/i386-apple-darwin9.7.0/4.4.1/collect2 -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/local/lib/gcc/i386-apple-darwin9.7.0/4.4.1 -L/usr/local/lib/gcc/i386-apple-darwin9.7.0/4.4.1/../../.. /var/tmp//cc9zXNax.o -v -lgfortranbegin -lgfortran -lgcc_s.10.5 -lgcc -lSystem
collect2 version 4.4.1 20090623 (prerelease) (i686 Darwin)
/usr/bin/ld -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/local/lib/gcc/i386-apple-darwin9.7.0/4.4.1 -L/usr/local/lib/gcc/i386-apple-darwin9.7.0/4.4.1/../../.. /var/tmp//cc9zXNax.o -v -lgfortranbegin -lgfortran -lgcc_s.10.5 -lgcc -lSystem
@(#)PROGRAM:ld  PROJECT:ld64-95.2.12
Library search paths:
	/usr/local/lib/gcc/i386-apple-darwin9.7.0/4.4.1
	/usr/local/lib
	/usr/lib
	/usr/local/lib
Framework search paths:
	/Library/Frameworks/
	/System/Library/Frameworks/")
set(mac_i686_gfortran_Wlv_libs "gfortranbegin;gfortran")
set(mac_i686_gfortran_Wlv_dirs "/usr/local/lib/gcc/i386-apple-darwin9.7.0/4.4.1;/usr/local/lib;/usr/lib")
set(mac_i686_gfortran_Wlv_fwks "/Library/Frameworks;/System/Library/Frameworks")
list(APPEND platforms mac_i686_gfortran_Wlv)

# gfortran dummy.f -v
set(mac_i686_gfortran_text " /usr/libexec/gcc/i386-apple-darwin9.7.0/4.4.1/collect2 -dynamic -arch i386 -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/gcc/i386-apple-darwin9.7.0/4.4.1 -L/usr/lib/gcc -L/usr/lib/gcc/i386-apple-darwin9.7.0/4.4.1/../../.. /var/tmp//ccgqbX5P.o -lgfortranbegin -lgfortran -lgcc_s.10.5 -lgcc -lSystem")
set(mac_i686_gfortran_libs "gfortranbegin;gfortran")
set(mac_i686_gfortran_dirs "/usr/lib/gcc/i386-apple-darwin9.7.0/4.4.1;/usr/lib/gcc;/usr/lib")
list(APPEND platforms mac_i686_gfortran)

# gcc -arch ppc dummy.c -v -Wl,-v
set(mac_ppc_gcc_Wlv_text " /usr/libexec/gcc/powerpc-apple-darwin10/4.2.1/collect2 -dynamic -arch ppc -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../../powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../.. /var/tmp//cclziQY4.o -v -lgcc -lSystemStubs -lSystem
collect2 version 4.2.1 (Apple Inc. build 5646) (Darwin/PowerPC)
/usr/libexec/gcc/powerpc-apple-darwin10/4.2.1/ld -dynamic -arch ppc -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../../powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../.. /var/tmp//cclziQY4.o -v -lgcc -lSystemStubs -lSystem
@(#)PROGRAM:ld  PROJECT:ld64-95.2.12
Library search paths:
	/usr/lib/powerpc-apple-darwin10/4.2.1
	/usr/lib/gcc/powerpc-apple-darwin10/4.2.1
	/usr/lib/gcc/powerpc-apple-darwin10/4.2.1
	/usr/lib/powerpc-apple-darwin10/4.2.1
	/usr/lib
	/usr/lib
	/usr/local/lib
Framework search paths:
	/Library/Frameworks/
	/System/Library/Frameworks/")
set(mac_ppc_gcc_Wlv_libs "")
set(mac_ppc_gcc_Wlv_dirs "/usr/lib/powerpc-apple-darwin10/4.2.1;/usr/lib/gcc/powerpc-apple-darwin10/4.2.1;/usr/lib;/usr/local/lib")
set(mac_ppc_gcc_Wlv_fwks "/Library/Frameworks;/System/Library/Frameworks")
list(APPEND platforms mac_ppc_gcc_Wlv)

# gcc -arch ppc dummy.c -v
set(mac_ppc_gcc_text " /usr/libexec/gcc/powerpc-apple-darwin10/4.2.1/collect2 -dynamic -arch ppc -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../../powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../.. /var/tmp//ccdcolsP.o -lgcc -lSystemStubs -lSystem")
set(mac_ppc_gcc_libs "")
set(mac_ppc_gcc_dirs "/usr/lib/powerpc-apple-darwin10/4.2.1;/usr/lib/gcc/powerpc-apple-darwin10/4.2.1;/usr/lib")
list(APPEND platforms mac_ppc_gcc)

# g++ -arch ppc dummy.cxx -v -Wl,-v
set(mac_ppc_g++_Wlv_text " /usr/libexec/gcc/powerpc-apple-darwin10/4.2.1/collect2 -dynamic -arch ppc -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../../powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../.. /var/tmp//ccaFTkwq.o -v -lstdc++ -lgcc -lSystemStubs -lSystem
collect2 version 4.2.1 (Apple Inc. build 5646) (Darwin/PowerPC)
/usr/libexec/gcc/powerpc-apple-darwin10/4.2.1/ld -dynamic -arch ppc -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../../powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../.. /var/tmp//ccaFTkwq.o -v -lstdc++ -lgcc -lSystemStubs -lSystem
@(#)PROGRAM:ld  PROJECT:ld64-95.2.12
Library search paths:
	/usr/lib/powerpc-apple-darwin10/4.2.1
	/usr/lib/gcc/powerpc-apple-darwin10/4.2.1
	/usr/lib/gcc/powerpc-apple-darwin10/4.2.1
	/usr/lib/powerpc-apple-darwin10/4.2.1
	/usr/lib
	/usr/lib
	/usr/local/lib
Framework search paths:
	/Library/Frameworks/
	/System/Library/Frameworks/")
set(mac_ppc_g++_Wlv_libs "stdc++")
set(mac_ppc_g++_Wlv_dirs "/usr/lib/powerpc-apple-darwin10/4.2.1;/usr/lib/gcc/powerpc-apple-darwin10/4.2.1;/usr/lib;/usr/local/lib")
set(mac_ppc_g++_Wlv_fwks "/Library/Frameworks;/System/Library/Frameworks")
list(APPEND platforms mac_ppc_g++_Wlv)

# g++ -arch ppc dummy.cxx -v
set(mac_ppc_g++_text " /usr/libexec/gcc/powerpc-apple-darwin10/4.2.1/collect2 -dynamic -arch ppc -macosx_version_min 10.6.0 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.5.o -L/usr/lib/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../../powerpc-apple-darwin10/4.2.1 -L/usr/lib/gcc/powerpc-apple-darwin10/4.2.1/../../.. /var/tmp//ccbjB6Lj.o -lstdc++ -lgcc -lSystemStubs -lSystem")
set(mac_ppc_g++_libs "stdc++")
set(mac_ppc_g++_dirs "/usr/lib/powerpc-apple-darwin10/4.2.1;/usr/lib/gcc/powerpc-apple-darwin10/4.2.1;/usr/lib")
list(APPEND platforms mac_ppc_g++)

# absoft dummy.f -X -v
set(mac_absoft_text "collect2 version 4.2.1 (Apple Inc. build 5664) (i686 Darwin)
/usr/libexec/gcc/i686-apple-darwin10/4.2.1/ld -dynamic -arch i386 -macosx_version_min 10.6.6 -weak_reference_mismatches non-weak -o a.out -lcrt1.10.6.o -L/Applications/Absoft11.1/lib -L/usr/lib/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../../i686-apple-darwin10/4.2.1 -L/usr/lib/gcc/i686-apple-darwin10/4.2.1/../../.. /var/folders/04/04+Djjm8GZWBmuEdp2Gsw++++TM/-Tmp-//bTAoJc/dummy.o -v -Y 10 -laf90math -lafio -lamisc -labsoftmain -laf77math -lm -lmv -lSystem -lgcc -lSystem
")
set(mac_absoft_libs "af90math;afio;amisc;absoftmain;af77math;m;mv")
set(mac_absoft_dirs "/Applications/Absoft11.1/lib;/usr/lib/i686-apple-darwin10/4.2.1;/usr/lib/gcc/i686-apple-darwin10/4.2.1;/usr/lib")
list(APPEND platforms mac_absoft)

#-----------------------------------------------------------------------------
# Sun

# cc dummy.c '-#'/
set(sun_cc_text "/usr/ccs/bin/ld /opt/SUNWspro/prod/lib/crti.o /opt/SUNWspro/prod/lib/crt1.o /opt/SUNWspro/prod/lib/misalign.o /opt/SUNWspro/prod/lib/values-xa.o dummy.o -Y \"P,/opt/SUNWspro/prod/lib/v8plus:/opt/SUNWspro/prod/lib:/usr/ccs/lib:/lib:/usr/lib\" -Qy -lc /opt/SUNWspro/prod/lib/crtn.o")
set(sun_cc_libs "c")
set(sun_cc_dirs "/opt/SUNWspro/prod/lib/v8plus;/opt/SUNWspro/prod/lib;/usr/ccs/lib;/lib;/usr/lib")
list(APPEND platforms sun_cc)

# CC dummy.cxx -v
set(sun_CC_text "/usr/ccs/bin/ld -u __1cH__CimplKcplus_init6F_v_ -zld32=-S/opt/SUNWspro/prod/lib/libldstab_ws.so -zld64=-S/opt/SUNWspro/prod/lib/v9/libldstab_ws.so -zld32=-S/opt/SUNWspro/prod/lib/libCCexcept.so.1 -zld64=-S/opt/SUNWspro/prod/lib/v9/libCCexcept.so.1 -R/opt/SUNWspro/lib/rw7:/opt/SUNWspro/lib/v8plus:/opt/SUNWspro/lib:/usr/ccs/lib:/lib:/usr/lib -o a.out /opt/SUNWspro/prod/lib/crti.o /opt/SUNWspro/prod/lib/CCrti.o /opt/SUNWspro/prod/lib/crt1.o /opt/SUNWspro/prod/lib/misalign.o /opt/SUNWspro/prod/lib/values-xa.o -Y P,/opt/SUNWspro/lib/rw7:/opt/SUNWspro/lib/v8plus:/opt/SUNWspro/prod/lib/rw7:/opt/SUNWspro/prod/lib/v8plus:/opt/SUNWspro/lib:/opt/SUNWspro/prod/lib:/usr/ccs/lib:/lib:/usr/lib dummy.o -lCstd -lCrun -lm -lc /opt/SUNWspro/prod/lib/CCrtn.o /opt/SUNWspro/prod/lib/crtn.o >&/tmp/ld.14846.2.err")
set(sun_CC_libs "Cstd;Crun;m;c")
set(sun_CC_dirs "/opt/SUNWspro/lib/rw7;/opt/SUNWspro/lib/v8plus;/opt/SUNWspro/prod/lib/rw7;/opt/SUNWspro/prod/lib/v8plus;/opt/SUNWspro/lib;/opt/SUNWspro/prod/lib;/usr/ccs/lib;/lib;/usr/lib")
list(APPEND platforms sun_CC)

# f77 dummy.f -v
set(sun_f77_text "/usr/ccs/bin/ld -t -R/opt/SUNWspro/lib/v8plus:/opt/SUNWspro/lib -o a.out /opt/SUNWspro/prod/lib/crti.o /opt/SUNWspro/prod/lib/crt1.o /opt/SUNWspro/prod/lib/misalign.o /opt/SUNWspro/prod/lib/values-xi.o -Y P,/opt/SUNWspro/lib/v8plus:/opt/SUNWspro/prod/lib/v8plus:/opt/SUNWspro/lib:/opt/SUNWspro/prod/lib:/usr/ccs/lib:/lib:/usr/lib dummy.o -lf77compat -zallextract -lompstubs -zdefaultextract -lfui -lfai -lfai2 -lfsumai -lfprodai -lfminlai -lfmaxlai -lfminvai -lfmaxvai -lfsu -lsunmath -lm -lc /opt/SUNWspro/prod/lib/crtn.o")
set(sun_f77_libs "f77compat;-zallextract;ompstubs;-zdefaultextract;fui;fai;fai2;fsumai;fprodai;fminlai;fmaxlai;fminvai;fmaxvai;fsu;sunmath;m;c")
set(sun_f77_dirs "/opt/SUNWspro/lib/v8plus;/opt/SUNWspro/prod/lib/v8plus;/opt/SUNWspro/lib;/opt/SUNWspro/prod/lib;/usr/ccs/lib;/lib;/usr/lib")
list(APPEND platforms sun_f77)

# f90 dummy.f -v
set(sun_f90_text "/usr/ccs/bin/ld -t -R/opt/SUNWspro/lib/v8plus:/opt/SUNWspro/lib -o a.out /opt/SUNWspro/prod/lib/crti.o /opt/SUNWspro/prod/lib/crt1.o /opt/SUNWspro/prod/lib/misalign.o /opt/SUNWspro/prod/lib/values-xi.o -Y P,/opt/SUNWspro/lib/v8plus:/opt/SUNWspro/prod/lib/v8plus:/opt/SUNWspro/lib:/opt/SUNWspro/prod/lib:/usr/ccs/lib:/lib:/usr/lib dummy.o -zallextract -lompstubs -zdefaultextract -lfui -lfai -lfai2 -lfsumai -lfprodai -lfminlai -lfmaxlai -lfminvai -lfmaxvai -lfsu -lsunmath -lm -lc /opt/SUNWspro/prod/lib/crtn.o")
set(sun_f90_libs "-zallextract;ompstubs;-zdefaultextract;fui;fai;fai2;fsumai;fprodai;fminlai;fmaxlai;fminvai;fmaxvai;fsu;sunmath;m;c")
set(sun_f90_dirs "/opt/SUNWspro/lib/v8plus;/opt/SUNWspro/prod/lib/v8plus;/opt/SUNWspro/lib;/opt/SUNWspro/prod/lib;/usr/ccs/lib;/lib;/usr/lib")
list(APPEND platforms sun_f90)

#-----------------------------------------------------------------------------
# AIX

# xlc -q32 dummy.c -V
set(aix_xlc_32_text "/bin/ld -b32 /lib/crt0.o -bpT:0x10000000 -bpD:0x20000000 dummy.o -L/usr/vac/lib -lxlopt -lxl -lc")
set(aix_xlc_32_libs "xlopt;xl;c")
set(aix_xlc_32_dirs "/usr/vac/lib")
list(APPEND platforms aix_xlc_32)

# xlc -q64 dummy.c -V
set(aix_xlc_64_text "/bin/ld -b64 /lib/crt0_64.o -bpT:0x100000000 -bpD:0x110000000 dummy.o -L/usr/vac/lib -lxlopt -lxl -lc")
set(aix_xlc_64_libs "xlopt;xl;c")
set(aix_xlc_64_dirs "/usr/vac/lib")
list(APPEND platforms aix_xlc_64)

# xlC -q32 dummy.cxx -V
set(aix_xlC_32_text "
/bin/ld -b32 /lib/crt0.o -bpT:0x10000000 -bpD:0x20000000 dummy.o -L/usr/vac/lib -lxlopt -lxl -L/usr/vacpp/lib -lC -lm -lc -bnobind  >/tmp/xlcLDW0agyh
/bin/ld -b32 /lib/crt0.o -bpT:0x10000000 -bpD:0x20000000 dummy.o -L/usr/vac/lib -lxlopt -lxl -L/usr/vacpp/lib -lC -lm -lc  |")
set(aix_xlC_32_libs "xlopt;xl;C;m;c")
set(aix_xlC_32_dirs "/usr/vac/lib;/usr/vacpp/lib")
list(APPEND platforms aix_xlC_32)

# xlC -q64 dummy.cxx -V
set(aix_xlC_64_text "
/bin/ld -b64 /lib/crt0_64.o -bpT:0x100000000 -bpD:0x110000000 dummy.o -L/usr/vac/lib -lxlopt -lxl -L/usr/vacpp/lib -lC -lm -lc -bnobind  >/tmp/xlcLD5nUnah
/bin/ld -b64 /lib/crt0_64.o -bpT:0x100000000 -bpD:0x110000000 dummy.o -L/usr/vac/lib -lxlopt -lxl -L/usr/vacpp/lib -lC -lm -lc  |")
set(aix_xlC_64_libs "xlopt;xl;C;m;c")
set(aix_xlC_64_dirs "/usr/vac/lib;/usr/vacpp/lib")
list(APPEND platforms aix_xlC_64)

# xlf -q32 dummy.f -V
set(aix_xlf_32_text "/bin/ld -b32 /lib/crt0.o -bpT:0x10000000 -bpD:0x20000000 -bh:4 -bh:4 -bh:4 -bh:4 dummy.o -lxlf90 -L/usr/lpp/xlf/lib -lxlopt -lxlf -lxlomp_ser -lm -lc")
set(aix_xlf_32_libs "xlf90;xlopt;xlf;xlomp_ser;m;c")
set(aix_xlf_32_dirs "/usr/lpp/xlf/lib")
list(APPEND platforms aix_xlf_32)

# xlf -q64 dummy.f -V
set(aix_xlf_64_text "/bin/ld -b64 /lib/crt0_64.o -bpT:0x100000000 -bpD:0x110000000 -bh:4 -bh:4 -bh:4 -bh:4 dummy.o -lxlf90 -L/usr/lpp/xlf/lib -lxlopt -lxlf -lxlomp_ser -lm -lc")
set(aix_xlf_64_libs "xlf90;xlopt;xlf;xlomp_ser;m;c")
set(aix_xlf_64_dirs "/usr/lpp/xlf/lib")
list(APPEND platforms aix_xlf_64)

# xlf90 -q32 dummy.f -V
set(aix_xlf90_32_text "/bin/ld -b32 /lib/crt0.o -bpT:0x10000000 -bpD:0x20000000 -bh:4 dummy.o -lxlf90 -L/usr/lpp/xlf/lib -lxlopt -lxlf -lxlomp_ser -lm -lc")
set(aix_xlf90_32_libs "xlf90;xlopt;xlf;xlomp_ser;m;c")
set(aix_xlf90_32_dirs "/usr/lpp/xlf/lib")
list(APPEND platforms aix_xlf90_32)

# xlf90 -q64 dummy.f -V
set(aix_xlf90_64_text "/bin/ld -b64 /lib/crt0_64.o -bpT:0x100000000 -bpD:0x110000000 -bh:4 dummy.o -lxlf90 -L/usr/lpp/xlf/lib -lxlopt -lxlf -lxlomp_ser -lm -lc")
set(aix_xlf90_64_libs "xlf90;xlopt;xlf;xlomp_ser;m;c")
set(aix_xlf90_64_dirs "/usr/lpp/xlf/lib")
list(APPEND platforms aix_xlf90_64)

#-----------------------------------------------------------------------------
# HP

# cc dummy.c -v
set(hp_cc_old_text "cc: LPATH is /usr/lib/pa1.1:/usr/lib:/opt/langtools/lib:
/usr/ccs/bin/ld /opt/langtools/lib/crt0.o -u main dummy.o -lc")
set(hp_cc_old_libs "c")
set(hp_cc_old_dirs "/usr/lib/pa1.1;/usr/lib;/opt/langtools/lib")
list(APPEND platforms hp_cc_old)

# aCC dummy.cxx -v
set(hp_aCC_old_text "LPATH=/usr/lib/pa1.1:/usr/lib:/opt/langtools/lib
 /usr/ccs/bin/ld -o a.out /opt/aCC/lib/crt0.o -u ___exit -u main -L /opt/aCC/lib /opt/aCC/lib/cpprt0.o dummy.o -lstd -lstream -lCsup -lm -lcl -lc /usr/lib/libdld.sl >/var/tmp/AAAa27787 2>&1")
set(hp_aCC_old_libs "std;stream;Csup;m;cl;c")
set(hp_aCC_old_dirs "/usr/lib/pa1.1;/usr/lib;/opt/langtools/lib;/opt/aCC/lib")
list(APPEND platforms hp_aCC_old)

# cc dummy.c -v
set(hp_cc_32_text "LPATH=/usr/lib/hpux32:/opt/langtools/lib/hpux32
 /usr/ccs/bin/ld -o a.out -u___exit -umain dummy.o -lc")
set(hp_cc_32_libs "c")
set(hp_cc_32_dirs "/usr/lib/hpux32;/opt/langtools/lib/hpux32")
list(APPEND platforms hp_cc_32)

# cc +DD64 dummy.c -v
set(hp_cc_64_text "LPATH=/usr/lib/hpux64:/opt/langtools/lib/hpux64
 /usr/ccs/bin/ld -o a.out -u___exit -umain dummy.o -lc")
set(hp_cc_64_libs "c")
set(hp_cc_64_dirs "/usr/lib/hpux64;/opt/langtools/lib/hpux64")
list(APPEND platforms hp_cc_64)

# aCC dummy.cxx -v
set(hp_aCC_32_text "LPATH=/usr/lib/hpux32:/opt/langtools/lib/hpux32
 /usr/ccs/bin/ld -o a.out -u___exit -umain -L/opt/aCC/lib/hpux32 dummy.o -lstd_v2 -lCsup -lm -lunwind -lCsup -lc -ldl >/var/tmp/AAAa03601 2>&1")
set(hp_aCC_32_libs "std_v2;Csup;m;unwind;Csup;c;dl")
set(hp_aCC_32_dirs "/usr/lib/hpux32;/opt/langtools/lib/hpux32;/opt/aCC/lib/hpux32")
list(APPEND platforms hp_aCC_32)

# aCC +DD64 dummy.cxx -v
set(hp_aCC_64_text "LPATH=/usr/lib/hpux64:/opt/langtools/lib/hpux64
 /usr/ccs/bin/ld -o a.out -u___exit -umain -L/opt/aCC/lib/hpux64 dummy.o -lstd_v2 -lCsup -lm -lunwind -lCsup -lc -ldl >/var/tmp/AAAa03597 2>&1")
set(hp_aCC_64_libs "std_v2;Csup;m;unwind;Csup;c;dl")
set(hp_aCC_64_dirs "/usr/lib/hpux64;/opt/langtools/lib/hpux64;/opt/aCC/lib/hpux64")
list(APPEND platforms hp_aCC_64)

# f90 dummy.f -v
set(hp_f90_32_text "LPATH is: /usr/lib/hpux32/:/opt/langtools/lib/hpux32/
/usr/ccs/bin/ld -u f90\$sgemm -u f90\$sgemv -u f90\$dgemm -u f90\$dgemv dummy.c +vnoshlibunsats -l:libF90.a -l:libIO77.a -lm -lc -lunwind -luca")
set(hp_f90_32_libs "-l:libF90.a;-l:libIO77.a;m;c;unwind;uca")
set(hp_f90_32_dirs "/usr/lib/hpux32;/opt/langtools/lib/hpux32")
list(APPEND platforms hp_f90_32)

# f90 +DD64 dummy.f -v
set(hp_f90_64_text "LPATH is: /usr/lib/hpux64/:/opt/langtools/lib/hpux64/
/usr/ccs/bin/ld -u f90\$sgemm -u f90\$sgemv -u f90\$dgemm -u f90\$dgemv dummy.c +vnoshlibunsats -l:libF90.a -l:libIO77.a -lm -lc -lunwind -luca")
set(hp_f90_64_libs "-l:libF90.a;-l:libIO77.a;m;c;unwind;uca")
set(hp_f90_64_dirs "/usr/lib/hpux64;/opt/langtools/lib/hpux64")
list(APPEND platforms hp_f90_64)

#-----------------------------------------------------------------------------
# IRIX

# cc -o32 dummy.c -v
set(irix64_cc_o32_text "/usr/lib/ld -elf -_SYSTYPE_SVR4 -require_dynamic_link _rld_new_interface -no_unresolved -Wx,-G 0 -o32 -mips2 -call_shared -g0 -KPIC -L/usr/lib/ -nocount /usr/lib/crt1.o -count dummy.o -nocount -lc /usr/lib/crtn.o")
set(irix64_cc_o32_libs "c")
set(irix64_cc_o32_dirs "/usr/lib")
list(APPEND platforms irix64_cc_o32)

# cc -n32 dummy.c -v
set(irix64_cc_n32_text "/usr/lib32/cmplrs/ld32 -call_shared -no_unresolved -transitive_link -elf -_SYSTYPE_SVR4 -show -mips4 -n32 -L/usr/lib32/mips4/r10000 -L/usr/lib32/mips4 -L/usr/lib32 /usr/lib32/mips4/crt1.o dummy.o -dont_warn_unused -Bdynamic -lc /usr/lib32/mips4/crtn.o -warn_unused")
set(irix64_cc_n32_libs "c")
set(irix64_cc_n32_dirs "/usr/lib32/mips4/r10000;/usr/lib32/mips4;/usr/lib32")
list(APPEND platforms irix64_cc_n32)

# cc -64 dummy.c -v
set(irix64_cc_64_text "/usr/lib32/cmplrs/ld64 -call_shared -no_unresolved -transitive_link -elf -_SYSTYPE_SVR4 -show -mips4 -64 -L/usr/lib64/mips4/r10000 -L/usr/lib64/mips4 -L/usr/lib64 /usr/lib64/mips4/crt1.o dummy.o -dont_warn_unused -Bdynamic -lc /usr/lib64/mips4/crtn.o -warn_unused")
set(irix64_cc_64_libs "c")
set(irix64_cc_64_dirs "/usr/lib64/mips4/r10000;/usr/lib64/mips4;/usr/lib64")
list(APPEND platforms irix64_cc_64)

# CC -o32 dummy.cxx -v
set(irix64_CC_o32_text "/usr/lib/ld -elf -cxx -woff 134 -_SYSTYPE_SVR4 -require_dynamic_link _rld_new_interface -no_unresolved -Wx,-G 0 -o32 -mips2 -call_shared -g0 -KPIC -L/usr/lib/ -nocount /usr/lib/crt1.o /usr/lib/c++init.o -count dummy.o -nocount -dont_warn_unused -lC -warn_unused -lc /usr/lib/crtn.o")
set(irix64_CC_o32_libs "C;c")
set(irix64_CC_o32_dirs "/usr/lib")
list(APPEND platforms irix64_CC_o32)

# CC -n32 dummy.cxx -v
set(irix64_CC_n32_text "/usr/lib32/cmplrs/ld32 -call_shared -init _main -fini _fini -no_unresolved -transitive_link -demangle -elf -_SYSTYPE_SVR4 -LANG:std -show -mips4 -n32 -L/usr/lib32/mips4/r10000 -L/usr/lib32/mips4 -L/usr/lib32 -cxx -woff 134 /usr/lib32/mips4/crt1.o /usr/lib32/c++init.o dummy.o -dont_warn_unused -lCsup -lC -lCio -Bdynamic -lc /usr/lib32/mips4/crtn.o -warn_unused")
set(irix64_CC_n32_libs "Csup;C;Cio;c")
set(irix64_CC_n32_dirs "/usr/lib32/mips4/r10000;/usr/lib32/mips4;/usr/lib32")
list(APPEND platforms irix64_CC_n32)

# CC -64 dummy.cxx -v
set(irix64_CC_64_text "/usr/lib32/cmplrs/ld64 -call_shared -init _main -fini _fini -no_unresolved -transitive_link -demangle -elf -_SYSTYPE_SVR4 -LANG:std -show -mips4 -64 -L/usr/lib64/mips4/r10000 -L/usr/lib64/mips4 -L/usr/lib64 -cxx -woff 134 /usr/lib64/mips4/crt1.o /usr/lib64/c++init.o dummy.o -dont_warn_unused -lCsup -lC -lCio -Bdynamic -lc /usr/lib64/mips4/crtn.o -warn_unused")
set(irix64_CC_64_libs "Csup;C;Cio;c")
set(irix64_CC_64_dirs "/usr/lib64/mips4/r10000;/usr/lib64/mips4;/usr/lib64")
list(APPEND platforms irix64_CC_64)

# f77 -o32 dummy.f -v
set(irix64_f77_o32_text "/usr/lib/ld -elf -_SYSTYPE_SVR4 -require_dynamic_link _rld_new_interface -no_unresolved -Wx,-G 0 -o32 -mips2 -call_shared -g0 -KPIC -L/usr/lib/ -nocount /usr/lib/crt1.o -count dummy.o -nocount -lftn -lm -lc /usr/lib/crtn.o")
set(irix64_f77_o32_libs "ftn;m;c")
set(irix64_f77_o32_dirs "/usr/lib")
list(APPEND platforms irix64_f77_o32)

# f77 -n32 dummy.f -v
set(irix64_f77_n32_text "/usr/lib32/cmplrs/ld32 -call_shared -no_unresolved -transitive_link -elf -_SYSTYPE_SVR4 -show -mips4 -n32 -L/usr/lib32/mips4/r10000 -L/usr/lib32/mips4 -L/usr/lib32 /usr/lib32/mips4/crt1.o dummy.o -dont_warn_unused -lftn -lm -Bdynamic -lc /usr/lib32/mips4/crtn.o -warn_unused")
set(irix64_f77_n32_libs "ftn;m;c")
set(irix64_f77_n32_dirs "/usr/lib32/mips4/r10000;/usr/lib32/mips4;/usr/lib32")
list(APPEND platforms irix64_f77_n32)

# f77 -64 dummy.f -v
set(irix64_f77_64_text "/usr/lib32/cmplrs/ld64 -call_shared -no_unresolved -transitive_link -elf -_SYSTYPE_SVR4 -show -mips4 -64 -L/usr/lib64/mips4/r10000 -L/usr/lib64/mips4 -L/usr/lib64 /usr/lib64/mips4/crt1.o dummy.o -dont_warn_unused -lftn -lm -Bdynamic -lc /usr/lib64/mips4/crtn.o -warn_unused")
set(irix64_f77_64_libs "ftn;m;c")
set(irix64_f77_64_dirs "/usr/lib64/mips4/r10000;/usr/lib64/mips4;/usr/lib64")
list(APPEND platforms irix64_f77_64)

# f90 -o32 dummy.f -v
#f90 ERROR:  specified abi -o32 not supported.

# f90 -n32 dummy.f -v
set(irix64_f90_n32_text "/usr/lib32/cmplrs/ld32 -call_shared -no_unresolved -transitive_link -elf -_SYSTYPE_SVR4 -show -mips4 -n32 -L/usr/lib32/mips4/r10000 -L/usr/lib32/mips4 -L/usr/lib32 /usr/lib32/mips4/crt1.o dummy.o -dont_warn_unused -lfortran -lffio -lftn -lm -Bdynamic -lc /usr/lib32/mips4/crtn.o -warn_unused")
set(irix64_f90_n32_libs "fortran;ffio;ftn;m;c")
set(irix64_f90_n32_dirs "/usr/lib32/mips4/r10000;/usr/lib32/mips4;/usr/lib32")
list(APPEND platforms irix64_f90_n32)

# f90 -64 dummy.f -v
set(irix64_f90_64_text "/usr/lib32/cmplrs/ld64 -call_shared -no_unresolved -transitive_link -elf -_SYSTYPE_SVR4 -show -mips4 -64 -L/usr/lib64/mips4/r10000 -L/usr/lib64/mips4 -L/usr/lib64 /usr/lib64/mips4/crt1.o dummy.o -dont_warn_unused -lfortran -lffio -lftn -lm -Bdynamic -lc /usr/lib64/mips4/crtn.o -warn_unused")
set(irix64_f90_64_libs "fortran;ffio;ftn;m;c")
set(irix64_f90_64_dirs "/usr/lib64/mips4/r10000;/usr/lib64/mips4;/usr/lib64")
list(APPEND platforms irix64_f90_64)

#-----------------------------------------------------------------------------
# Cygwin

# gcc dummy.c -v
set(cygwin_gcc_text " /usr/lib/gcc/i686-pc-cygwin/3.4.4/collect2.exe -Bdynamic --dll-search-prefix=cyg /usr/lib/gcc/i686-pc-cygwin/3.4.4/../../../crt0.o -L/usr/lib/gcc/i686-pc-cygwin/3.4.4 -L/usr/lib/gcc/i686-pc-cygwin/3.4.4 -L/usr/lib/gcc/i686-pc-cygwin/3.4.4/../../.. /home/user/AppData/Local/Temp/cczg1Arh.o -lgcc -lcygwin -luser32 -lkernel32 -ladvapi32 -lshell32 -lgcc")
set(cygwin_gcc_libs "cygwin;user32;kernel32;advapi32;shell32")
set(cygwin_gcc_dirs "/usr/lib/gcc/i686-pc-cygwin/3.4.4;/usr/lib")
list(APPEND platforms cygwin_gcc)

# g++ dummy.cxx -v
set(cygwin_g++_text " /usr/lib/gcc/i686-pc-cygwin/3.4.4/collect2.exe -Bdynamic --dll-search-prefix=cyg /usr/lib/gcc/i686-pc-cygwin/3.4.4/../../../crt0.o -L/usr/lib/gcc/i686-pc-cygwin/3.4.4 -L/usr/lib/gcc/i686-pc-cygwin/3.4.4 -L/usr/lib/gcc/i686-pc-cygwin/3.4.4/../../.. /home/user/AppData/Local/Temp/ccsvcDO6.o -lstdc++ -lgcc -lcygwin -luser32 -lkernel32 -ladvapi32 -lshell32 -lgcc")
set(cygwin_g++_libs "stdc++;cygwin;user32;kernel32;advapi32;shell32")
set(cygwin_g++_dirs "/usr/lib/gcc/i686-pc-cygwin/3.4.4;/usr/lib")
list(APPEND platforms cygwin_g++)

# gfortran dummy.f -v
set(cygwin_gfortran_text "Configured with: ... LD=/opt/gcc-tools/bin/ld.exe
 /usr/lib/gcc/i686-pc-cygwin/4.3.2/collect2.exe -Bdynamic --dll-search-prefix=cyg -u ___register_frame_info -u ___deregister_frame_info /usr/lib/gcc/i686-pc-cygwin/4.3.2/../../../crt0.o /usr/lib/gcc/i686-pc-cygwin/4.3.2/crtbegin.o -L/usr/lib/gcc/i686-pc-cygwin/4.3.2 -L/usr/lib/gcc/i686-pc-cygwin/4.3.2 -L/usr/lib/gcc/i686-pc-cygwin/4.3.2/../../.. /home/user/AppData/Local/Temp/ccqRWKWg.o -lgfortranbegin -lgfortran -lgcc_s -lgcc_s -lgcc -lcygwin -luser32 -lkernel32 -ladvapi32 -lshell32 -lgcc_s -lgcc_s -lgcc /usr/lib/gcc/i686-pc-cygwin/4.3.2/crtend.o
")
set(cygwin_gfortran_libs "gfortranbegin;gfortran;cygwin;user32;kernel32;advapi32;shell32")
set(cygwin_gfortran_dirs "/usr/lib/gcc/i686-pc-cygwin/4.3.2;/usr/lib")
list(APPEND platforms cygwin_gfortran)

#-----------------------------------------------------------------------------
# MSYS

# gcc dummy.c -v
set(msys_gcc_text " C:/some-mingw/bin/../libexec/gcc/mingw32/3.4.5/collect2.exe -Bdynamic /some-mingw/lib/crt2.o C:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/crtbegin.o -LC:/some-mingw/bin/../lib/gcc/mingw32/3.4.5 -LC:/some-mingw/bin/../lib/gcc -L/some-mingw/lib -LC:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/../../.. C:/home/user/AppData/Local/Temp/cckQmvRt.o -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -luser32 -lkernel32 -ladvapi32 -lshell32 -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt C:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/crtend.o")
set(msys_gcc_libs "mingw32;moldname;mingwex;msvcrt;user32;kernel32;advapi32;shell32;mingw32;moldname;mingwex;msvcrt")
set(msys_gcc_dirs "C:/some-mingw/lib/gcc/mingw32/3.4.5;C:/some-mingw/lib/gcc;/some-mingw/lib;C:/some-mingw/lib")
list(APPEND platforms msys_gcc)

# g++ dummy.cxx -v
set(msys_g++_text " C:/some-mingw/bin/../libexec/gcc/mingw32/3.4.5/collect2.exe -Bdynamic /some-mingw/lib/crt2.o C:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/crtbegin.o -LC:/some-mingw/bin/../lib/gcc/mingw32/3.4.5 -LC:/some-mingw/bin/../lib/gcc -L/some-mingw/lib -LC:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/../../.. C:/home/user/AppData/Local/Temp/cci5hYPk.o -lstdc++ -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -luser32 -lkernel32 -ladvapi32 -lshell32 -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt C:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/crtend.o")
set(msys_g++_libs "stdc++;mingw32;moldname;mingwex;msvcrt;user32;kernel32;advapi32;shell32;mingw32;moldname;mingwex;msvcrt")
set(msys_g++_dirs "C:/some-mingw/lib/gcc/mingw32/3.4.5;C:/some-mingw/lib/gcc;/some-mingw/lib;C:/some-mingw/lib")
list(APPEND platforms msys_g++)

# g77 dummy.f -v
set(msys_g77_text " C:/some-mingw/bin/../libexec/gcc/mingw32/3.4.5/collect2.exe -Bdynamic /some-mingw/lib/crt2.o C:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/crtbegin.o -LC:/some-mingw/bin/../lib/gcc/mingw32/3.4.5 -LC:/some-mingw/bin/../lib/gcc -L/some-mingw/lib -LC:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/../../.. C:/home/user/AppData/Local/Temp/ccabRxQ1.o -lfrtbegin -lg2c -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -luser32 -lkernel32 -ladvapi32 -lshell32 -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt C:/some-mingw/bin/../lib/gcc/mingw32/3.4.5/crtend.o")
set(msys_g77_libs "frtbegin;g2c;mingw32;moldname;mingwex;msvcrt;user32;kernel32;advapi32;shell32;mingw32;moldname;mingwex;msvcrt")
set(msys_g77_dirs "C:/some-mingw/lib/gcc/mingw32/3.4.5;C:/some-mingw/lib/gcc;/some-mingw/lib;C:/some-mingw/lib")
list(APPEND platforms msys_g77)


#-----------------------------------------------------------------------------
# Test parsing for all above examples.
set(CMAKE_LINKER "not-a-linker[]().*+^$?")

foreach(p IN LISTS platforms)
  cmake_parse_implicit_link_info("${${p}_text}" libs dirs fwks log "${${p}_obj_regex}")

  foreach(v libs dirs fwks)
    if(DEFINED "${p}_${v}" AND NOT "${${v}}" STREQUAL "${${p}_${v}}")
      message(FATAL_ERROR
        "cmake_parse_implicit_link_info failed\n"
        "Expected '${p}' implicit ${v}\n"
        "  [${${p}_${v}}]\n"
        "but got\n"
        "  [${${v}}]\n"
        "Parse log was:\n"
        "${log}"
        )
    endif()
  endforeach()
endforeach()
