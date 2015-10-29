# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Linux specific files and settings for SDL

{
 #TODO what is really necessary here
 'link_settings': {
   'libraries': [ 
     '-lm',
     '-ldl',
     '-lpthread',
     '-lrt' 
   ],
 },
 'include_dirs': [
    # TODO we currently disable dbus, is this okay?
    #'/usr/include/dbus-1.0',
    #'/usr/lib/x86_64-linux-gnu/dbus-1.0/include',
 ],
 'sources': [
   '<(src_dir)/src/video/x11/SDL_x11clipboard.c',
   '<(src_dir)/src/video/x11/SDL_x11dyn.c',
   '<(src_dir)/src/video/x11/SDL_x11events.c',
   '<(src_dir)/src/video/x11/SDL_x11framebuffer.c',
   '<(src_dir)/src/video/x11/SDL_x11keyboard.c',
   '<(src_dir)/src/video/x11/SDL_x11messagebox.c',
   '<(src_dir)/src/video/x11/SDL_x11modes.c',
   '<(src_dir)/src/video/x11/SDL_x11mouse.c',
   '<(src_dir)/src/video/x11/SDL_x11opengl.c',
   '<(src_dir)/src/video/x11/SDL_x11opengles.c',
   '<(src_dir)/src/video/x11/SDL_x11shape.c',
   '<(src_dir)/src/video/x11/SDL_x11touch.c',
   '<(src_dir)/src/video/x11/SDL_x11video.c',
   '<(src_dir)/src/video/x11/SDL_x11window.c',
   '<(src_dir)/src/video/x11/SDL_x11xinput2.c',
   '<(src_dir)/src/video/x11/edid-parse.c',
   '<(src_dir)/src/video/x11/imKStoUCS.c',
   '<(src_dir)/src/thread/pthread/SDL_systhread.c',
   '<(src_dir)/src/thread/pthread/SDL_syssem.c',
   '<(src_dir)/src/thread/pthread/SDL_sysmutex.c',
   '<(src_dir)/src/thread/pthread/SDL_syscond.c',
   '<(src_dir)/src/thread/pthread/SDL_systls.c',
   '<(src_dir)/src/joystick/linux/SDL_sysjoystick.c',
   '<(src_dir)/src/haptic/linux/SDL_syshaptic.c',
   '<(src_dir)/src/power/linux/SDL_syspower.c',
   '<(src_dir)/src/filesystem/unix/SDL_sysfilesystem.c',
   '<(src_dir)/src/timer/unix/SDL_systimer.c',
   '<(src_dir)/src/core/linux/SDL_udev.c',
   '<(src_dir)/src/core/linux/SDL_evdev.c',
   '<(src_dir)/src/loadso/dlopen/SDL_sysloadso.c',
 ],
 'defines': [ 
   '__LINUX__',
   '_REENTRANT'
 ],
 'cflags': [ '-fPIC' ],
}
