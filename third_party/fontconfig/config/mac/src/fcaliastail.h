#if HAVE_GNUC_ATTRIBUTE
#ifdef __fcblanks__
# undef FcBlanksCreate
extern __typeof (FcBlanksCreate) FcBlanksCreate __attribute((alias("IA__FcBlanksCreate"), visibility("default")));
# undef FcBlanksDestroy
extern __typeof (FcBlanksDestroy) FcBlanksDestroy __attribute((alias("IA__FcBlanksDestroy"), visibility("default")));
# undef FcBlanksAdd
extern __typeof (FcBlanksAdd) FcBlanksAdd __attribute((alias("IA__FcBlanksAdd"), visibility("default")));
# undef FcBlanksIsMember
extern __typeof (FcBlanksIsMember) FcBlanksIsMember __attribute((alias("IA__FcBlanksIsMember"), visibility("default")));
#endif /* __fcblanks__ */
#ifdef __fccache__
# undef FcCacheCopySet
extern __typeof (FcCacheCopySet) FcCacheCopySet __attribute((alias("IA__FcCacheCopySet"), visibility("default")));
# undef FcCacheNumSubdir
extern __typeof (FcCacheNumSubdir) FcCacheNumSubdir __attribute((alias("IA__FcCacheNumSubdir"), visibility("default")));
# undef FcCacheNumFont
extern __typeof (FcCacheNumFont) FcCacheNumFont __attribute((alias("IA__FcCacheNumFont"), visibility("default")));
# undef FcDirCacheUnlink
extern __typeof (FcDirCacheUnlink) FcDirCacheUnlink __attribute((alias("IA__FcDirCacheUnlink"), visibility("default")));
# undef FcDirCacheValid
extern __typeof (FcDirCacheValid) FcDirCacheValid __attribute((alias("IA__FcDirCacheValid"), visibility("default")));
# undef FcDirCacheClean
extern __typeof (FcDirCacheClean) FcDirCacheClean __attribute((alias("IA__FcDirCacheClean"), visibility("default")));
# undef FcCacheCreateTagFile
extern __typeof (FcCacheCreateTagFile) FcCacheCreateTagFile __attribute((alias("IA__FcCacheCreateTagFile"), visibility("default")));
#endif /* __fccache__ */
#ifdef __fccfg__
# undef FcConfigHome
extern __typeof (FcConfigHome) FcConfigHome __attribute((alias("IA__FcConfigHome"), visibility("default")));
# undef FcConfigEnableHome
extern __typeof (FcConfigEnableHome) FcConfigEnableHome __attribute((alias("IA__FcConfigEnableHome"), visibility("default")));
# undef FcConfigFilename
extern __typeof (FcConfigFilename) FcConfigFilename __attribute((alias("IA__FcConfigFilename"), visibility("default")));
# undef FcConfigCreate
extern __typeof (FcConfigCreate) FcConfigCreate __attribute((alias("IA__FcConfigCreate"), visibility("default")));
# undef FcConfigReference
extern __typeof (FcConfigReference) FcConfigReference __attribute((alias("IA__FcConfigReference"), visibility("default")));
# undef FcConfigDestroy
extern __typeof (FcConfigDestroy) FcConfigDestroy __attribute((alias("IA__FcConfigDestroy"), visibility("default")));
# undef FcConfigSetCurrent
extern __typeof (FcConfigSetCurrent) FcConfigSetCurrent __attribute((alias("IA__FcConfigSetCurrent"), visibility("default")));
# undef FcConfigGetCurrent
extern __typeof (FcConfigGetCurrent) FcConfigGetCurrent __attribute((alias("IA__FcConfigGetCurrent"), visibility("default")));
# undef FcConfigUptoDate
extern __typeof (FcConfigUptoDate) FcConfigUptoDate __attribute((alias("IA__FcConfigUptoDate"), visibility("default")));
# undef FcConfigBuildFonts
extern __typeof (FcConfigBuildFonts) FcConfigBuildFonts __attribute((alias("IA__FcConfigBuildFonts"), visibility("default")));
# undef FcConfigGetFontDirs
extern __typeof (FcConfigGetFontDirs) FcConfigGetFontDirs __attribute((alias("IA__FcConfigGetFontDirs"), visibility("default")));
# undef FcConfigGetConfigDirs
extern __typeof (FcConfigGetConfigDirs) FcConfigGetConfigDirs __attribute((alias("IA__FcConfigGetConfigDirs"), visibility("default")));
# undef FcConfigGetConfigFiles
extern __typeof (FcConfigGetConfigFiles) FcConfigGetConfigFiles __attribute((alias("IA__FcConfigGetConfigFiles"), visibility("default")));
# undef FcConfigGetCache
extern __typeof (FcConfigGetCache) FcConfigGetCache __attribute((alias("IA__FcConfigGetCache"), visibility("default")));
# undef FcConfigGetBlanks
extern __typeof (FcConfigGetBlanks) FcConfigGetBlanks __attribute((alias("IA__FcConfigGetBlanks"), visibility("default")));
# undef FcConfigGetCacheDirs
extern __typeof (FcConfigGetCacheDirs) FcConfigGetCacheDirs __attribute((alias("IA__FcConfigGetCacheDirs"), visibility("default")));
# undef FcConfigGetRescanInterval
extern __typeof (FcConfigGetRescanInterval) FcConfigGetRescanInterval __attribute((alias("IA__FcConfigGetRescanInterval"), visibility("default")));
# undef FcConfigSetRescanInterval
extern __typeof (FcConfigSetRescanInterval) FcConfigSetRescanInterval __attribute((alias("IA__FcConfigSetRescanInterval"), visibility("default")));
# undef FcConfigGetFonts
extern __typeof (FcConfigGetFonts) FcConfigGetFonts __attribute((alias("IA__FcConfigGetFonts"), visibility("default")));
# undef FcConfigAppFontAddFile
extern __typeof (FcConfigAppFontAddFile) FcConfigAppFontAddFile __attribute((alias("IA__FcConfigAppFontAddFile"), visibility("default")));
# undef FcConfigAppFontAddDir
extern __typeof (FcConfigAppFontAddDir) FcConfigAppFontAddDir __attribute((alias("IA__FcConfigAppFontAddDir"), visibility("default")));
# undef FcConfigAppFontClear
extern __typeof (FcConfigAppFontClear) FcConfigAppFontClear __attribute((alias("IA__FcConfigAppFontClear"), visibility("default")));
# undef FcConfigSubstituteWithPat
extern __typeof (FcConfigSubstituteWithPat) FcConfigSubstituteWithPat __attribute((alias("IA__FcConfigSubstituteWithPat"), visibility("default")));
# undef FcConfigSubstitute
extern __typeof (FcConfigSubstitute) FcConfigSubstitute __attribute((alias("IA__FcConfigSubstitute"), visibility("default")));
# undef FcConfigGetSysRoot
extern __typeof (FcConfigGetSysRoot) FcConfigGetSysRoot __attribute((alias("IA__FcConfigGetSysRoot"), visibility("default")));
# undef FcConfigSetSysRoot
extern __typeof (FcConfigSetSysRoot) FcConfigSetSysRoot __attribute((alias("IA__FcConfigSetSysRoot"), visibility("default")));
#endif /* __fccfg__ */
#ifdef __fccharset__
# undef FcCharSetCreate
extern __typeof (FcCharSetCreate) FcCharSetCreate __attribute((alias("IA__FcCharSetCreate"), visibility("default")));
# undef FcCharSetNew
extern __typeof (FcCharSetNew) FcCharSetNew __attribute((alias("IA__FcCharSetNew"), visibility("default")));
# undef FcCharSetDestroy
extern __typeof (FcCharSetDestroy) FcCharSetDestroy __attribute((alias("IA__FcCharSetDestroy"), visibility("default")));
# undef FcCharSetAddChar
extern __typeof (FcCharSetAddChar) FcCharSetAddChar __attribute((alias("IA__FcCharSetAddChar"), visibility("default")));
# undef FcCharSetDelChar
extern __typeof (FcCharSetDelChar) FcCharSetDelChar __attribute((alias("IA__FcCharSetDelChar"), visibility("default")));
# undef FcCharSetCopy
extern __typeof (FcCharSetCopy) FcCharSetCopy __attribute((alias("IA__FcCharSetCopy"), visibility("default")));
# undef FcCharSetEqual
extern __typeof (FcCharSetEqual) FcCharSetEqual __attribute((alias("IA__FcCharSetEqual"), visibility("default")));
# undef FcCharSetIntersect
extern __typeof (FcCharSetIntersect) FcCharSetIntersect __attribute((alias("IA__FcCharSetIntersect"), visibility("default")));
# undef FcCharSetUnion
extern __typeof (FcCharSetUnion) FcCharSetUnion __attribute((alias("IA__FcCharSetUnion"), visibility("default")));
# undef FcCharSetSubtract
extern __typeof (FcCharSetSubtract) FcCharSetSubtract __attribute((alias("IA__FcCharSetSubtract"), visibility("default")));
# undef FcCharSetMerge
extern __typeof (FcCharSetMerge) FcCharSetMerge __attribute((alias("IA__FcCharSetMerge"), visibility("default")));
# undef FcCharSetHasChar
extern __typeof (FcCharSetHasChar) FcCharSetHasChar __attribute((alias("IA__FcCharSetHasChar"), visibility("default")));
# undef FcCharSetCount
extern __typeof (FcCharSetCount) FcCharSetCount __attribute((alias("IA__FcCharSetCount"), visibility("default")));
# undef FcCharSetIntersectCount
extern __typeof (FcCharSetIntersectCount) FcCharSetIntersectCount __attribute((alias("IA__FcCharSetIntersectCount"), visibility("default")));
# undef FcCharSetSubtractCount
extern __typeof (FcCharSetSubtractCount) FcCharSetSubtractCount __attribute((alias("IA__FcCharSetSubtractCount"), visibility("default")));
# undef FcCharSetIsSubset
extern __typeof (FcCharSetIsSubset) FcCharSetIsSubset __attribute((alias("IA__FcCharSetIsSubset"), visibility("default")));
# undef FcCharSetFirstPage
extern __typeof (FcCharSetFirstPage) FcCharSetFirstPage __attribute((alias("IA__FcCharSetFirstPage"), visibility("default")));
# undef FcCharSetNextPage
extern __typeof (FcCharSetNextPage) FcCharSetNextPage __attribute((alias("IA__FcCharSetNextPage"), visibility("default")));
# undef FcCharSetCoverage
extern __typeof (FcCharSetCoverage) FcCharSetCoverage __attribute((alias("IA__FcCharSetCoverage"), visibility("default")));
#endif /* __fccharset__ */
#ifdef __fcdbg__
# undef FcValuePrint
extern __typeof (FcValuePrint) FcValuePrint __attribute((alias("IA__FcValuePrint"), visibility("default")));
# undef FcPatternPrint
extern __typeof (FcPatternPrint) FcPatternPrint __attribute((alias("IA__FcPatternPrint"), visibility("default")));
# undef FcFontSetPrint
extern __typeof (FcFontSetPrint) FcFontSetPrint __attribute((alias("IA__FcFontSetPrint"), visibility("default")));
#endif /* __fcdbg__ */
#ifdef __fcdefault__
# undef FcGetDefaultLangs
extern __typeof (FcGetDefaultLangs) FcGetDefaultLangs __attribute((alias("IA__FcGetDefaultLangs"), visibility("default")));
# undef FcDefaultSubstitute
extern __typeof (FcDefaultSubstitute) FcDefaultSubstitute __attribute((alias("IA__FcDefaultSubstitute"), visibility("default")));
#endif /* __fcdefault__ */
#ifdef __fcdir__
# undef FcFileIsDir
extern __typeof (FcFileIsDir) FcFileIsDir __attribute((alias("IA__FcFileIsDir"), visibility("default")));
# undef FcFileScan
extern __typeof (FcFileScan) FcFileScan __attribute((alias("IA__FcFileScan"), visibility("default")));
# undef FcDirScan
extern __typeof (FcDirScan) FcDirScan __attribute((alias("IA__FcDirScan"), visibility("default")));
# undef FcDirSave
extern __typeof (FcDirSave) FcDirSave __attribute((alias("IA__FcDirSave"), visibility("default")));
#endif /* __fcdir__ */
#ifdef __fccache__
# undef FcDirCacheLoad
extern __typeof (FcDirCacheLoad) FcDirCacheLoad __attribute((alias("IA__FcDirCacheLoad"), visibility("default")));
#endif /* __fccache__ */
#ifdef __fcdir__
# undef FcDirCacheRead
extern __typeof (FcDirCacheRead) FcDirCacheRead __attribute((alias("IA__FcDirCacheRead"), visibility("default")));
#endif /* __fcdir__ */
#ifdef __fccache__
# undef FcDirCacheLoadFile
extern __typeof (FcDirCacheLoadFile) FcDirCacheLoadFile __attribute((alias("IA__FcDirCacheLoadFile"), visibility("default")));
# undef FcDirCacheUnload
extern __typeof (FcDirCacheUnload) FcDirCacheUnload __attribute((alias("IA__FcDirCacheUnload"), visibility("default")));
#endif /* __fccache__ */
#ifdef __fcfreetype__
# undef FcFreeTypeQuery
extern __typeof (FcFreeTypeQuery) FcFreeTypeQuery __attribute((alias("IA__FcFreeTypeQuery"), visibility("default")));
#endif /* __fcfreetype__ */
#ifdef __fcfs__
# undef FcFontSetCreate
extern __typeof (FcFontSetCreate) FcFontSetCreate __attribute((alias("IA__FcFontSetCreate"), visibility("default")));
# undef FcFontSetDestroy
extern __typeof (FcFontSetDestroy) FcFontSetDestroy __attribute((alias("IA__FcFontSetDestroy"), visibility("default")));
# undef FcFontSetAdd
extern __typeof (FcFontSetAdd) FcFontSetAdd __attribute((alias("IA__FcFontSetAdd"), visibility("default")));
#endif /* __fcfs__ */
#ifdef __fcinit__
# undef FcInitLoadConfig
extern __typeof (FcInitLoadConfig) FcInitLoadConfig __attribute((alias("IA__FcInitLoadConfig"), visibility("default")));
# undef FcInitLoadConfigAndFonts
extern __typeof (FcInitLoadConfigAndFonts) FcInitLoadConfigAndFonts __attribute((alias("IA__FcInitLoadConfigAndFonts"), visibility("default")));
# undef FcInit
extern __typeof (FcInit) FcInit __attribute((alias("IA__FcInit"), visibility("default")));
# undef FcFini
extern __typeof (FcFini) FcFini __attribute((alias("IA__FcFini"), visibility("default")));
# undef FcGetVersion
extern __typeof (FcGetVersion) FcGetVersion __attribute((alias("IA__FcGetVersion"), visibility("default")));
# undef FcInitReinitialize
extern __typeof (FcInitReinitialize) FcInitReinitialize __attribute((alias("IA__FcInitReinitialize"), visibility("default")));
# undef FcInitBringUptoDate
extern __typeof (FcInitBringUptoDate) FcInitBringUptoDate __attribute((alias("IA__FcInitBringUptoDate"), visibility("default")));
#endif /* __fcinit__ */
#ifdef __fclang__
# undef FcGetLangs
extern __typeof (FcGetLangs) FcGetLangs __attribute((alias("IA__FcGetLangs"), visibility("default")));
# undef FcLangNormalize
extern __typeof (FcLangNormalize) FcLangNormalize __attribute((alias("IA__FcLangNormalize"), visibility("default")));
# undef FcLangGetCharSet
extern __typeof (FcLangGetCharSet) FcLangGetCharSet __attribute((alias("IA__FcLangGetCharSet"), visibility("default")));
# undef FcLangSetCreate
extern __typeof (FcLangSetCreate) FcLangSetCreate __attribute((alias("IA__FcLangSetCreate"), visibility("default")));
# undef FcLangSetDestroy
extern __typeof (FcLangSetDestroy) FcLangSetDestroy __attribute((alias("IA__FcLangSetDestroy"), visibility("default")));
# undef FcLangSetCopy
extern __typeof (FcLangSetCopy) FcLangSetCopy __attribute((alias("IA__FcLangSetCopy"), visibility("default")));
# undef FcLangSetAdd
extern __typeof (FcLangSetAdd) FcLangSetAdd __attribute((alias("IA__FcLangSetAdd"), visibility("default")));
# undef FcLangSetDel
extern __typeof (FcLangSetDel) FcLangSetDel __attribute((alias("IA__FcLangSetDel"), visibility("default")));
# undef FcLangSetHasLang
extern __typeof (FcLangSetHasLang) FcLangSetHasLang __attribute((alias("IA__FcLangSetHasLang"), visibility("default")));
# undef FcLangSetCompare
extern __typeof (FcLangSetCompare) FcLangSetCompare __attribute((alias("IA__FcLangSetCompare"), visibility("default")));
# undef FcLangSetContains
extern __typeof (FcLangSetContains) FcLangSetContains __attribute((alias("IA__FcLangSetContains"), visibility("default")));
# undef FcLangSetEqual
extern __typeof (FcLangSetEqual) FcLangSetEqual __attribute((alias("IA__FcLangSetEqual"), visibility("default")));
# undef FcLangSetHash
extern __typeof (FcLangSetHash) FcLangSetHash __attribute((alias("IA__FcLangSetHash"), visibility("default")));
# undef FcLangSetGetLangs
extern __typeof (FcLangSetGetLangs) FcLangSetGetLangs __attribute((alias("IA__FcLangSetGetLangs"), visibility("default")));
# undef FcLangSetUnion
extern __typeof (FcLangSetUnion) FcLangSetUnion __attribute((alias("IA__FcLangSetUnion"), visibility("default")));
# undef FcLangSetSubtract
extern __typeof (FcLangSetSubtract) FcLangSetSubtract __attribute((alias("IA__FcLangSetSubtract"), visibility("default")));
#endif /* __fclang__ */
#ifdef __fclist__
# undef FcObjectSetCreate
extern __typeof (FcObjectSetCreate) FcObjectSetCreate __attribute((alias("IA__FcObjectSetCreate"), visibility("default")));
# undef FcObjectSetAdd
extern __typeof (FcObjectSetAdd) FcObjectSetAdd __attribute((alias("IA__FcObjectSetAdd"), visibility("default")));
# undef FcObjectSetDestroy
extern __typeof (FcObjectSetDestroy) FcObjectSetDestroy __attribute((alias("IA__FcObjectSetDestroy"), visibility("default")));
# undef FcObjectSetVaBuild
extern __typeof (FcObjectSetVaBuild) FcObjectSetVaBuild __attribute((alias("IA__FcObjectSetVaBuild"), visibility("default")));
# undef FcObjectSetBuild
extern __typeof (FcObjectSetBuild) FcObjectSetBuild __attribute((alias("IA__FcObjectSetBuild"), visibility("default")));
# undef FcFontSetList
extern __typeof (FcFontSetList) FcFontSetList __attribute((alias("IA__FcFontSetList"), visibility("default")));
# undef FcFontList
extern __typeof (FcFontList) FcFontList __attribute((alias("IA__FcFontList"), visibility("default")));
#endif /* __fclist__ */
#ifdef __fcatomic__
# undef FcAtomicCreate
extern __typeof (FcAtomicCreate) FcAtomicCreate __attribute((alias("IA__FcAtomicCreate"), visibility("default")));
# undef FcAtomicLock
extern __typeof (FcAtomicLock) FcAtomicLock __attribute((alias("IA__FcAtomicLock"), visibility("default")));
# undef FcAtomicNewFile
extern __typeof (FcAtomicNewFile) FcAtomicNewFile __attribute((alias("IA__FcAtomicNewFile"), visibility("default")));
# undef FcAtomicOrigFile
extern __typeof (FcAtomicOrigFile) FcAtomicOrigFile __attribute((alias("IA__FcAtomicOrigFile"), visibility("default")));
# undef FcAtomicReplaceOrig
extern __typeof (FcAtomicReplaceOrig) FcAtomicReplaceOrig __attribute((alias("IA__FcAtomicReplaceOrig"), visibility("default")));
# undef FcAtomicDeleteNew
extern __typeof (FcAtomicDeleteNew) FcAtomicDeleteNew __attribute((alias("IA__FcAtomicDeleteNew"), visibility("default")));
# undef FcAtomicUnlock
extern __typeof (FcAtomicUnlock) FcAtomicUnlock __attribute((alias("IA__FcAtomicUnlock"), visibility("default")));
# undef FcAtomicDestroy
extern __typeof (FcAtomicDestroy) FcAtomicDestroy __attribute((alias("IA__FcAtomicDestroy"), visibility("default")));
#endif /* __fcatomic__ */
#ifdef __fcmatch__
# undef FcFontSetMatch
extern __typeof (FcFontSetMatch) FcFontSetMatch __attribute((alias("IA__FcFontSetMatch"), visibility("default")));
# undef FcFontMatch
extern __typeof (FcFontMatch) FcFontMatch __attribute((alias("IA__FcFontMatch"), visibility("default")));
# undef FcFontRenderPrepare
extern __typeof (FcFontRenderPrepare) FcFontRenderPrepare __attribute((alias("IA__FcFontRenderPrepare"), visibility("default")));
# undef FcFontSetSort
extern __typeof (FcFontSetSort) FcFontSetSort __attribute((alias("IA__FcFontSetSort"), visibility("default")));
# undef FcFontSort
extern __typeof (FcFontSort) FcFontSort __attribute((alias("IA__FcFontSort"), visibility("default")));
# undef FcFontSetSortDestroy
extern __typeof (FcFontSetSortDestroy) FcFontSetSortDestroy __attribute((alias("IA__FcFontSetSortDestroy"), visibility("default")));
#endif /* __fcmatch__ */
#ifdef __fcmatrix__
# undef FcMatrixCopy
extern __typeof (FcMatrixCopy) FcMatrixCopy __attribute((alias("IA__FcMatrixCopy"), visibility("default")));
# undef FcMatrixEqual
extern __typeof (FcMatrixEqual) FcMatrixEqual __attribute((alias("IA__FcMatrixEqual"), visibility("default")));
# undef FcMatrixMultiply
extern __typeof (FcMatrixMultiply) FcMatrixMultiply __attribute((alias("IA__FcMatrixMultiply"), visibility("default")));
# undef FcMatrixRotate
extern __typeof (FcMatrixRotate) FcMatrixRotate __attribute((alias("IA__FcMatrixRotate"), visibility("default")));
# undef FcMatrixScale
extern __typeof (FcMatrixScale) FcMatrixScale __attribute((alias("IA__FcMatrixScale"), visibility("default")));
# undef FcMatrixShear
extern __typeof (FcMatrixShear) FcMatrixShear __attribute((alias("IA__FcMatrixShear"), visibility("default")));
#endif /* __fcmatrix__ */
#ifdef __fcname__
# undef FcNameRegisterObjectTypes
extern __typeof (FcNameRegisterObjectTypes) FcNameRegisterObjectTypes __attribute((alias("IA__FcNameRegisterObjectTypes"), visibility("default")));
# undef FcNameUnregisterObjectTypes
extern __typeof (FcNameUnregisterObjectTypes) FcNameUnregisterObjectTypes __attribute((alias("IA__FcNameUnregisterObjectTypes"), visibility("default")));
# undef FcNameGetObjectType
extern __typeof (FcNameGetObjectType) FcNameGetObjectType __attribute((alias("IA__FcNameGetObjectType"), visibility("default")));
# undef FcNameRegisterConstants
extern __typeof (FcNameRegisterConstants) FcNameRegisterConstants __attribute((alias("IA__FcNameRegisterConstants"), visibility("default")));
# undef FcNameUnregisterConstants
extern __typeof (FcNameUnregisterConstants) FcNameUnregisterConstants __attribute((alias("IA__FcNameUnregisterConstants"), visibility("default")));
# undef FcNameGetConstant
extern __typeof (FcNameGetConstant) FcNameGetConstant __attribute((alias("IA__FcNameGetConstant"), visibility("default")));
# undef FcNameConstant
extern __typeof (FcNameConstant) FcNameConstant __attribute((alias("IA__FcNameConstant"), visibility("default")));
# undef FcNameParse
extern __typeof (FcNameParse) FcNameParse __attribute((alias("IA__FcNameParse"), visibility("default")));
# undef FcNameUnparse
extern __typeof (FcNameUnparse) FcNameUnparse __attribute((alias("IA__FcNameUnparse"), visibility("default")));
#endif /* __fcname__ */
#ifdef __fcpat__
# undef FcPatternCreate
extern __typeof (FcPatternCreate) FcPatternCreate __attribute((alias("IA__FcPatternCreate"), visibility("default")));
# undef FcPatternDuplicate
extern __typeof (FcPatternDuplicate) FcPatternDuplicate __attribute((alias("IA__FcPatternDuplicate"), visibility("default")));
# undef FcPatternReference
extern __typeof (FcPatternReference) FcPatternReference __attribute((alias("IA__FcPatternReference"), visibility("default")));
# undef FcPatternFilter
extern __typeof (FcPatternFilter) FcPatternFilter __attribute((alias("IA__FcPatternFilter"), visibility("default")));
# undef FcValueDestroy
extern __typeof (FcValueDestroy) FcValueDestroy __attribute((alias("IA__FcValueDestroy"), visibility("default")));
# undef FcValueEqual
extern __typeof (FcValueEqual) FcValueEqual __attribute((alias("IA__FcValueEqual"), visibility("default")));
# undef FcValueSave
extern __typeof (FcValueSave) FcValueSave __attribute((alias("IA__FcValueSave"), visibility("default")));
# undef FcPatternDestroy
extern __typeof (FcPatternDestroy) FcPatternDestroy __attribute((alias("IA__FcPatternDestroy"), visibility("default")));
# undef FcPatternEqual
extern __typeof (FcPatternEqual) FcPatternEqual __attribute((alias("IA__FcPatternEqual"), visibility("default")));
# undef FcPatternEqualSubset
extern __typeof (FcPatternEqualSubset) FcPatternEqualSubset __attribute((alias("IA__FcPatternEqualSubset"), visibility("default")));
# undef FcPatternHash
extern __typeof (FcPatternHash) FcPatternHash __attribute((alias("IA__FcPatternHash"), visibility("default")));
# undef FcPatternAdd
extern __typeof (FcPatternAdd) FcPatternAdd __attribute((alias("IA__FcPatternAdd"), visibility("default")));
# undef FcPatternAddWeak
extern __typeof (FcPatternAddWeak) FcPatternAddWeak __attribute((alias("IA__FcPatternAddWeak"), visibility("default")));
# undef FcPatternGet
extern __typeof (FcPatternGet) FcPatternGet __attribute((alias("IA__FcPatternGet"), visibility("default")));
# undef FcPatternDel
extern __typeof (FcPatternDel) FcPatternDel __attribute((alias("IA__FcPatternDel"), visibility("default")));
# undef FcPatternRemove
extern __typeof (FcPatternRemove) FcPatternRemove __attribute((alias("IA__FcPatternRemove"), visibility("default")));
# undef FcPatternAddInteger
extern __typeof (FcPatternAddInteger) FcPatternAddInteger __attribute((alias("IA__FcPatternAddInteger"), visibility("default")));
# undef FcPatternAddDouble
extern __typeof (FcPatternAddDouble) FcPatternAddDouble __attribute((alias("IA__FcPatternAddDouble"), visibility("default")));
# undef FcPatternAddString
extern __typeof (FcPatternAddString) FcPatternAddString __attribute((alias("IA__FcPatternAddString"), visibility("default")));
# undef FcPatternAddMatrix
extern __typeof (FcPatternAddMatrix) FcPatternAddMatrix __attribute((alias("IA__FcPatternAddMatrix"), visibility("default")));
# undef FcPatternAddCharSet
extern __typeof (FcPatternAddCharSet) FcPatternAddCharSet __attribute((alias("IA__FcPatternAddCharSet"), visibility("default")));
# undef FcPatternAddBool
extern __typeof (FcPatternAddBool) FcPatternAddBool __attribute((alias("IA__FcPatternAddBool"), visibility("default")));
# undef FcPatternAddLangSet
extern __typeof (FcPatternAddLangSet) FcPatternAddLangSet __attribute((alias("IA__FcPatternAddLangSet"), visibility("default")));
# undef FcPatternGetInteger
extern __typeof (FcPatternGetInteger) FcPatternGetInteger __attribute((alias("IA__FcPatternGetInteger"), visibility("default")));
# undef FcPatternGetDouble
extern __typeof (FcPatternGetDouble) FcPatternGetDouble __attribute((alias("IA__FcPatternGetDouble"), visibility("default")));
# undef FcPatternGetString
extern __typeof (FcPatternGetString) FcPatternGetString __attribute((alias("IA__FcPatternGetString"), visibility("default")));
# undef FcPatternGetMatrix
extern __typeof (FcPatternGetMatrix) FcPatternGetMatrix __attribute((alias("IA__FcPatternGetMatrix"), visibility("default")));
# undef FcPatternGetCharSet
extern __typeof (FcPatternGetCharSet) FcPatternGetCharSet __attribute((alias("IA__FcPatternGetCharSet"), visibility("default")));
# undef FcPatternGetBool
extern __typeof (FcPatternGetBool) FcPatternGetBool __attribute((alias("IA__FcPatternGetBool"), visibility("default")));
# undef FcPatternGetLangSet
extern __typeof (FcPatternGetLangSet) FcPatternGetLangSet __attribute((alias("IA__FcPatternGetLangSet"), visibility("default")));
# undef FcPatternVaBuild
extern __typeof (FcPatternVaBuild) FcPatternVaBuild __attribute((alias("IA__FcPatternVaBuild"), visibility("default")));
# undef FcPatternBuild
extern __typeof (FcPatternBuild) FcPatternBuild __attribute((alias("IA__FcPatternBuild"), visibility("default")));
#endif /* __fcpat__ */
#ifdef __fcformat__
# undef FcPatternFormat
extern __typeof (FcPatternFormat) FcPatternFormat __attribute((alias("IA__FcPatternFormat"), visibility("default")));
#endif /* __fcformat__ */
#ifdef __fcstr__
# undef FcStrCopy
extern __typeof (FcStrCopy) FcStrCopy __attribute((alias("IA__FcStrCopy"), visibility("default")));
# undef FcStrCopyFilename
extern __typeof (FcStrCopyFilename) FcStrCopyFilename __attribute((alias("IA__FcStrCopyFilename"), visibility("default")));
# undef FcStrPlus
extern __typeof (FcStrPlus) FcStrPlus __attribute((alias("IA__FcStrPlus"), visibility("default")));
# undef FcStrFree
extern __typeof (FcStrFree) FcStrFree __attribute((alias("IA__FcStrFree"), visibility("default")));
# undef FcStrDowncase
extern __typeof (FcStrDowncase) FcStrDowncase __attribute((alias("IA__FcStrDowncase"), visibility("default")));
# undef FcStrCmpIgnoreCase
extern __typeof (FcStrCmpIgnoreCase) FcStrCmpIgnoreCase __attribute((alias("IA__FcStrCmpIgnoreCase"), visibility("default")));
# undef FcStrCmp
extern __typeof (FcStrCmp) FcStrCmp __attribute((alias("IA__FcStrCmp"), visibility("default")));
# undef FcStrStrIgnoreCase
extern __typeof (FcStrStrIgnoreCase) FcStrStrIgnoreCase __attribute((alias("IA__FcStrStrIgnoreCase"), visibility("default")));
# undef FcStrStr
extern __typeof (FcStrStr) FcStrStr __attribute((alias("IA__FcStrStr"), visibility("default")));
# undef FcUtf8ToUcs4
extern __typeof (FcUtf8ToUcs4) FcUtf8ToUcs4 __attribute((alias("IA__FcUtf8ToUcs4"), visibility("default")));
# undef FcUtf8Len
extern __typeof (FcUtf8Len) FcUtf8Len __attribute((alias("IA__FcUtf8Len"), visibility("default")));
# undef FcUcs4ToUtf8
extern __typeof (FcUcs4ToUtf8) FcUcs4ToUtf8 __attribute((alias("IA__FcUcs4ToUtf8"), visibility("default")));
# undef FcUtf16ToUcs4
extern __typeof (FcUtf16ToUcs4) FcUtf16ToUcs4 __attribute((alias("IA__FcUtf16ToUcs4"), visibility("default")));
# undef FcUtf16Len
extern __typeof (FcUtf16Len) FcUtf16Len __attribute((alias("IA__FcUtf16Len"), visibility("default")));
# undef FcStrDirname
extern __typeof (FcStrDirname) FcStrDirname __attribute((alias("IA__FcStrDirname"), visibility("default")));
# undef FcStrBasename
extern __typeof (FcStrBasename) FcStrBasename __attribute((alias("IA__FcStrBasename"), visibility("default")));
# undef FcStrSetCreate
extern __typeof (FcStrSetCreate) FcStrSetCreate __attribute((alias("IA__FcStrSetCreate"), visibility("default")));
# undef FcStrSetMember
extern __typeof (FcStrSetMember) FcStrSetMember __attribute((alias("IA__FcStrSetMember"), visibility("default")));
# undef FcStrSetEqual
extern __typeof (FcStrSetEqual) FcStrSetEqual __attribute((alias("IA__FcStrSetEqual"), visibility("default")));
# undef FcStrSetAdd
extern __typeof (FcStrSetAdd) FcStrSetAdd __attribute((alias("IA__FcStrSetAdd"), visibility("default")));
# undef FcStrSetAddFilename
extern __typeof (FcStrSetAddFilename) FcStrSetAddFilename __attribute((alias("IA__FcStrSetAddFilename"), visibility("default")));
# undef FcStrSetDel
extern __typeof (FcStrSetDel) FcStrSetDel __attribute((alias("IA__FcStrSetDel"), visibility("default")));
# undef FcStrSetDestroy
extern __typeof (FcStrSetDestroy) FcStrSetDestroy __attribute((alias("IA__FcStrSetDestroy"), visibility("default")));
# undef FcStrListCreate
extern __typeof (FcStrListCreate) FcStrListCreate __attribute((alias("IA__FcStrListCreate"), visibility("default")));
# undef FcStrListFirst
extern __typeof (FcStrListFirst) FcStrListFirst __attribute((alias("IA__FcStrListFirst"), visibility("default")));
# undef FcStrListNext
extern __typeof (FcStrListNext) FcStrListNext __attribute((alias("IA__FcStrListNext"), visibility("default")));
# undef FcStrListDone
extern __typeof (FcStrListDone) FcStrListDone __attribute((alias("IA__FcStrListDone"), visibility("default")));
#endif /* __fcstr__ */
#ifdef __fcxml__
# undef FcConfigParseAndLoad
extern __typeof (FcConfigParseAndLoad) FcConfigParseAndLoad __attribute((alias("IA__FcConfigParseAndLoad"), visibility("default")));
#endif /* __fcxml__ */
#ifdef __fccfg__
# undef FcConfigGetRescanInverval
extern __typeof (FcConfigGetRescanInverval) FcConfigGetRescanInverval __attribute((alias("IA__FcConfigGetRescanInverval"), visibility("default")));
# undef FcConfigSetRescanInverval
extern __typeof (FcConfigSetRescanInverval) FcConfigSetRescanInverval __attribute((alias("IA__FcConfigSetRescanInverval"), visibility("default")));
#endif /*  */
#endif /* HAVE_GNUC_ATTRIBUTE */
