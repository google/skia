Design
======


Overview
--------
Allows trying out Skia code in the browser.


Security
--------
We're putting a C++ compiler on the web, and promising to run the results of
user submitted code, so security is a large concern. Security is handled in a
layered approach, using a combination of seccomp-bpf, chroot jail and rlimits.

*seccomp-bpf* - Used to limit the types of system calls that the user code can
make. Any attempts to make a system call that isn't allowed causes the
application to terminate immediately.

*chroot jail* - The code is run in a chroot jail, making the rest of the
operating system files unreachable from the running code.

*rlimits* - Used to limit the resources the running code can get access to,
for example runtime is limited to 5s of CPU.

User submitted code is also restricted in the following ways:
  * Limited to 10K of code total.
  * No preprocessor use is allowed (no lines can begin with \s*#).


Architecture
------------

The server runs on GCE, and consists of a Go Web Server that calls out to the
c++ compiler and executes code in a chroot jail. See the diagram below:

                           
   +–––––––––––––+         
   |             |         
   |  Browser    |         
   |             |         
   +––––––+––––––+         
          |                
   +––––––+––––––+         
   |             |         
   |             |         
   | Web Server  |         
   |             |         
   |   (Go)      |         
   |             |         
   |             |         
   +–––––––+–––––+         
           |               
   +–––––––+––––––––––+    
   | chroot jail      |    
   |  +––––––––––––––+|    
   |  | seccomp      ||    
   |  |  +––––––––––+||    
   |  |  |User code |||    
   |  |  |          |||    
   |  |  +----------+||    
   |  +––------------+|    
   |                  |    
   +––––––––––––––––––+    
                           
                           
The user code is expanded into a simple template and linked against libskia
and a couple other .o files that contain main() and the code that sets up the
seccomp and rlimit restrictions. This code also sets up the SkCanvas that is
handed to the user code. Any code the user submits is restricted to running in
a single function that looks like this:


    void draw(SkCanvas* canvas) {
      // User code goes here.
    }

The user code is tracked by taking an MD5 hash of the code The template is
expanded out into <hash>.cpp, which is compiled into <hash>.o, which is then
linked together with all the other libs and object files to create an
executable named <hash>.  That executable is copied into a directory
/home/webtry/inout, that is accessible to both the web server and the schroot
jail. The application is then run in the schroot jail, writing its response,
<hash>.png, out into the same directory, /home/webtry/inout/, where is it read
by the web server and returned to the user.

Startup and config
------------------
The server is started and stopped via:

    sudo /etc/init.d/webtry [start|stop|restart]

By sysv init only handles starting and stopping a program once, so we use
Monit to monitor the application and restart it if it crashes. The config
is in:

    /etc/monit/conf.d/webtry

The chroot jail is implemented using schroot, its configuration
file is found in:

    /etc/schroot/chroot.d/webtry

The seccomp configuration is in main.cpp and only allows the following system
calls:

    exit_group
    exit
    fstat
    read
    write
    close
    mmap
    munmap
    brk

Installation
------------
See the README file.


