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
  * No preprocessor use is allowed (no lines can begin with #includes).


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
     |  |  +––––––––––+||    
     |  +––––––––––––––+|    
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

Database
--------

Code submitted is stored in an SQL database so that it can be referenced
later, i.e. we can let users bookmark their SkFiddles.

The storage layer will be Cloud SQL (a cloud version of MySQL). Back of the
envelope estimates of traffic come out to a price of a about $1/month.

All passwords for MySQL are stored in valentine.

To connect to the database from the skia-webtry-b server:

    $ mysql --host=173.194.83.52 --user=root --password

Initial setup of the database, the user, and the only table:

    CREATE DATABASE webtry;
    USE webtry;
    CREATE USER 'webtry'@'%' IDENTIFIED BY '<password is in valentine>';
    GRANT SELECT, INSERT, UPDATE ON webtry.webtry TO 'webtry'@'%';

    // If this gets changed also update the sqlite create statement in webtry.go.

    CREATE TABLE webtry (
      code      TEXT      DEFAULT ''                 NOT NULL,
      create_ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
      hash      CHAR(64)  DEFAULT ''                 NOT NULL,
      PRIMARY KEY(hash)
    );

Common queries webtry.go will use:

    INSERT INTO webtry (code, hash) VALUES('int i = 0;...', 'abcdef...');

    SELECT code, create_ts, hash FROM webtry WHERE hash='abcdef...';

    SELECT code, create_ts, hash FROM webtry ORDER BY create_ts DESC LIMIT 2;

    // To change the password for the webtry sql client:
    SET PASSWORD for 'webtry'@'%' = PASSWORD('<password is in valentine>');

    // Run before and after to confirm the password changed:
    SELECT Host, User, Password FROM mysql.user;

Password for the database will be stored in the metadata instance, if the
metadata server can't be found, i.e. running locally, then data will not be
stored.  To see the current password stored in metadata and the fingerprint:

    gcutil  --project=google.com:skia-buildbots    getinstance skia-webtry-b

To set the mysql password that webtry is to use:

    gcutil  --project=google.com:skia-buildbots   setinstancemetadata skia-webtry-b --metadata=password:'[mysql client webtry password]' --fingerprint=[some fingerprint]

To retrieve the password from the running instance just GET the right URL from
the metadata server:

    curl "http://metadata/computeMetadata/v1/instance/attributes/password" -H "X-Google-Metadata-Request: True"

N.B. If you need to change the MySQL password that webtry uses, you must change
it both in MySQL and the value stored in the metadata server.

Installation
------------
See the README file.


