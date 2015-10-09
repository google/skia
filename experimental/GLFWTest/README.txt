This test is Mac-only for now. Adding Linux or Win support shouldn’t be that difficult — it just needs a project file.

To run this demo, you’ll need to do three things:
1) Get GLFW 3.1.1 or later, and install it at third_party/externals/glfw. You’ll need to run ‘cmake’ in that directory to build the makefiles, and then ‘make’ to build the libglfw.a library.
2) Build the skia libraries via the command line (by running ‘ninja -C out/<config> dm’, for example.
3) Get a ship.png file. Place it in this directory. 
