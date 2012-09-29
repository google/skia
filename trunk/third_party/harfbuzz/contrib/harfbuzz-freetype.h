#ifndef HB_FREETYPE_H_
#define HB_FREETYPE_H_

extern const HB_FontClass hb_freetype_class;

HB_Error hb_freetype_table_sfnt_get(void *voidface, const HB_Tag tag,
                                    HB_Byte *buffer, HB_UInt *len);

#endif  // HB_FREETYPE_H_
