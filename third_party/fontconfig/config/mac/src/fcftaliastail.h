#if HAVE_GNUC_ATTRIBUTE
#ifdef __fcfreetype__
# undef FcFreeTypeCharIndex
extern __typeof (FcFreeTypeCharIndex) FcFreeTypeCharIndex __attribute((alias("IA__FcFreeTypeCharIndex"), visibility("default")));
# undef FcFreeTypeCharSetAndSpacing
extern __typeof (FcFreeTypeCharSetAndSpacing) FcFreeTypeCharSetAndSpacing __attribute((alias("IA__FcFreeTypeCharSetAndSpacing"), visibility("default")));
# undef FcFreeTypeCharSet
extern __typeof (FcFreeTypeCharSet) FcFreeTypeCharSet __attribute((alias("IA__FcFreeTypeCharSet"), visibility("default")));
#endif /* __fcfreetype__ */
#ifdef __fcpat__
# undef FcPatternGetFTFace
extern __typeof (FcPatternGetFTFace) FcPatternGetFTFace __attribute((alias("IA__FcPatternGetFTFace"), visibility("default")));
# undef FcPatternAddFTFace
extern __typeof (FcPatternAddFTFace) FcPatternAddFTFace __attribute((alias("IA__FcPatternAddFTFace"), visibility("default")));
#endif /* __fcpat__ */
#ifdef __fcfreetype__
# undef FcFreeTypeQueryFace
extern __typeof (FcFreeTypeQueryFace) FcFreeTypeQueryFace __attribute((alias("IA__FcFreeTypeQueryFace"), visibility("default")));
#endif /*  */
#endif /* HAVE_GNUC_ATTRIBUTE */
