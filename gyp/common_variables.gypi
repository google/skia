# variables used in both common.gypi and skia.gyp in chromium
#
{
  # Define all variables, allowing for override in GYP_DEFINES.
  #
  # One such variable is 'skia_os', which we use instead of 'OS' throughout
  # our gyp files.  We set it automatically based on 'OS', but allow the
  # user to override it via GYP_DEFINES if they like.
  'variables': {
    'skia_scalar%': 'float',
    'skia_os%': '<(OS)',
    'skia_mesa%': 0,
    'skia_target_arch%': '',
  },
  'skia_scalar%': '<(skia_scalar)',
  'skia_os': '<(skia_os)',
  'skia_mesa': '<(skia_mesa)',
  'skia_target_arch': '<(skia_target_arch)',
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
