{
  'targets': [
    {
      'target_name': 'libtess',
      'type': 'static_library',
      'include_dirs': [
        '../third_party/glu',
      ],
      'sources': [
        '../third_party/glu/sk_glu.h',
        '../third_party/glu/gluos.h',
        '../third_party/glu/libtess/dict-list.h',
        '../third_party/glu/libtess/dict.c',
        '../third_party/glu/libtess/dict.h',
        '../third_party/glu/libtess/geom.c',
        '../third_party/glu/libtess/geom.h',
        '../third_party/glu/libtess/memalloc.c',
        '../third_party/glu/libtess/memalloc.h',
        '../third_party/glu/libtess/mesh.c',
        '../third_party/glu/libtess/mesh.h',
        '../third_party/glu/libtess/normal.c',
        '../third_party/glu/libtess/normal.h',
        '../third_party/glu/libtess/priorityq-heap.h',
        '../third_party/glu/libtess/priorityq-sort.h',
        '../third_party/glu/libtess/priorityq.c',
        '../third_party/glu/libtess/priorityq.h',
        '../third_party/glu/libtess/render.c',
        '../third_party/glu/libtess/render.h',
        '../third_party/glu/libtess/sweep.c',
        '../third_party/glu/libtess/sweep.h',
        '../third_party/glu/libtess/tess.c',
        '../third_party/glu/libtess/tess.h',
        '../third_party/glu/libtess/tessmono.c',
        '../third_party/glu/libtess/tessmono.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/glu',
        ],
      },
      'conditions': [
        [ 'skia_os == "android"', {
          'cflags!': [
            '-fno-rtti', # supresses warnings about invalid option of non-C++ code
          ],
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
