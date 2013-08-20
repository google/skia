#if HAVE_GNUC_ATTRIBUTE
#ifdef __fcblanks__
#undef FcBlanksCreate
extern __typeof (FcBlanksCreate) FcBlanksCreate __attribute((alias("IA__FcBlanksCreate"), visibility("default")));
#endif
#ifdef __fcblanks__
#undef FcBlanksDestroy
extern __typeof (FcBlanksDestroy) FcBlanksDestroy __attribute((alias("IA__FcBlanksDestroy"), visibility("default")));
#endif
#ifdef __fcblanks__
#undef FcBlanksAdd
extern __typeof (FcBlanksAdd) FcBlanksAdd __attribute((alias("IA__FcBlanksAdd"), visibility("default")));
#endif
#ifdef __fcblanks__
#undef FcBlanksIsMember
extern __typeof (FcBlanksIsMember) FcBlanksIsMember __attribute((alias("IA__FcBlanksIsMember"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcCacheCopySet
extern __typeof (FcCacheCopySet) FcCacheCopySet __attribute((alias("IA__FcCacheCopySet"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcCacheNumSubdir
extern __typeof (FcCacheNumSubdir) FcCacheNumSubdir __attribute((alias("IA__FcCacheNumSubdir"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcCacheNumFont
extern __typeof (FcCacheNumFont) FcCacheNumFont __attribute((alias("IA__FcCacheNumFont"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcDirCacheUnlink
extern __typeof (FcDirCacheUnlink) FcDirCacheUnlink __attribute((alias("IA__FcDirCacheUnlink"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcDirCacheValid
extern __typeof (FcDirCacheValid) FcDirCacheValid __attribute((alias("IA__FcDirCacheValid"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigHome
extern __typeof (FcConfigHome) FcConfigHome __attribute((alias("IA__FcConfigHome"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigEnableHome
extern __typeof (FcConfigEnableHome) FcConfigEnableHome __attribute((alias("IA__FcConfigEnableHome"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigFilename
extern __typeof (FcConfigFilename) FcConfigFilename __attribute((alias("IA__FcConfigFilename"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigCreate
extern __typeof (FcConfigCreate) FcConfigCreate __attribute((alias("IA__FcConfigCreate"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigReference
extern __typeof (FcConfigReference) FcConfigReference __attribute((alias("IA__FcConfigReference"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigDestroy
extern __typeof (FcConfigDestroy) FcConfigDestroy __attribute((alias("IA__FcConfigDestroy"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigSetCurrent
extern __typeof (FcConfigSetCurrent) FcConfigSetCurrent __attribute((alias("IA__FcConfigSetCurrent"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetCurrent
extern __typeof (FcConfigGetCurrent) FcConfigGetCurrent __attribute((alias("IA__FcConfigGetCurrent"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigUptoDate
extern __typeof (FcConfigUptoDate) FcConfigUptoDate __attribute((alias("IA__FcConfigUptoDate"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigBuildFonts
extern __typeof (FcConfigBuildFonts) FcConfigBuildFonts __attribute((alias("IA__FcConfigBuildFonts"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetFontDirs
extern __typeof (FcConfigGetFontDirs) FcConfigGetFontDirs __attribute((alias("IA__FcConfigGetFontDirs"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetConfigDirs
extern __typeof (FcConfigGetConfigDirs) FcConfigGetConfigDirs __attribute((alias("IA__FcConfigGetConfigDirs"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetConfigFiles
extern __typeof (FcConfigGetConfigFiles) FcConfigGetConfigFiles __attribute((alias("IA__FcConfigGetConfigFiles"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetCache
extern __typeof (FcConfigGetCache) FcConfigGetCache __attribute((alias("IA__FcConfigGetCache"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetBlanks
extern __typeof (FcConfigGetBlanks) FcConfigGetBlanks __attribute((alias("IA__FcConfigGetBlanks"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetCacheDirs
extern __typeof (FcConfigGetCacheDirs) FcConfigGetCacheDirs __attribute((alias("IA__FcConfigGetCacheDirs"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetRescanInterval
extern __typeof (FcConfigGetRescanInterval) FcConfigGetRescanInterval __attribute((alias("IA__FcConfigGetRescanInterval"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigSetRescanInterval
extern __typeof (FcConfigSetRescanInterval) FcConfigSetRescanInterval __attribute((alias("IA__FcConfigSetRescanInterval"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetFonts
extern __typeof (FcConfigGetFonts) FcConfigGetFonts __attribute((alias("IA__FcConfigGetFonts"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigAppFontAddFile
extern __typeof (FcConfigAppFontAddFile) FcConfigAppFontAddFile __attribute((alias("IA__FcConfigAppFontAddFile"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigAppFontAddDir
extern __typeof (FcConfigAppFontAddDir) FcConfigAppFontAddDir __attribute((alias("IA__FcConfigAppFontAddDir"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigAppFontClear
extern __typeof (FcConfigAppFontClear) FcConfigAppFontClear __attribute((alias("IA__FcConfigAppFontClear"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigSubstituteWithPat
extern __typeof (FcConfigSubstituteWithPat) FcConfigSubstituteWithPat __attribute((alias("IA__FcConfigSubstituteWithPat"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigSubstitute
extern __typeof (FcConfigSubstitute) FcConfigSubstitute __attribute((alias("IA__FcConfigSubstitute"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetCreate
extern __typeof (FcCharSetCreate) FcCharSetCreate __attribute((alias("IA__FcCharSetCreate"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetNew
extern __typeof (FcCharSetNew) FcCharSetNew __attribute((alias("IA__FcCharSetNew"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetDestroy
extern __typeof (FcCharSetDestroy) FcCharSetDestroy __attribute((alias("IA__FcCharSetDestroy"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetAddChar
extern __typeof (FcCharSetAddChar) FcCharSetAddChar __attribute((alias("IA__FcCharSetAddChar"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetCopy
extern __typeof (FcCharSetCopy) FcCharSetCopy __attribute((alias("IA__FcCharSetCopy"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetEqual
extern __typeof (FcCharSetEqual) FcCharSetEqual __attribute((alias("IA__FcCharSetEqual"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetIntersect
extern __typeof (FcCharSetIntersect) FcCharSetIntersect __attribute((alias("IA__FcCharSetIntersect"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetUnion
extern __typeof (FcCharSetUnion) FcCharSetUnion __attribute((alias("IA__FcCharSetUnion"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetSubtract
extern __typeof (FcCharSetSubtract) FcCharSetSubtract __attribute((alias("IA__FcCharSetSubtract"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetMerge
extern __typeof (FcCharSetMerge) FcCharSetMerge __attribute((alias("IA__FcCharSetMerge"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetHasChar
extern __typeof (FcCharSetHasChar) FcCharSetHasChar __attribute((alias("IA__FcCharSetHasChar"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetCount
extern __typeof (FcCharSetCount) FcCharSetCount __attribute((alias("IA__FcCharSetCount"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetIntersectCount
extern __typeof (FcCharSetIntersectCount) FcCharSetIntersectCount __attribute((alias("IA__FcCharSetIntersectCount"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetSubtractCount
extern __typeof (FcCharSetSubtractCount) FcCharSetSubtractCount __attribute((alias("IA__FcCharSetSubtractCount"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetIsSubset
extern __typeof (FcCharSetIsSubset) FcCharSetIsSubset __attribute((alias("IA__FcCharSetIsSubset"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetFirstPage
extern __typeof (FcCharSetFirstPage) FcCharSetFirstPage __attribute((alias("IA__FcCharSetFirstPage"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetNextPage
extern __typeof (FcCharSetNextPage) FcCharSetNextPage __attribute((alias("IA__FcCharSetNextPage"), visibility("default")));
#endif
#ifdef __fccharset__
#undef FcCharSetCoverage
extern __typeof (FcCharSetCoverage) FcCharSetCoverage __attribute((alias("IA__FcCharSetCoverage"), visibility("default")));
#endif
#ifdef __fcdbg__
#undef FcValuePrint
extern __typeof (FcValuePrint) FcValuePrint __attribute((alias("IA__FcValuePrint"), visibility("default")));
#endif
#ifdef __fcdbg__
#undef FcPatternPrint
extern __typeof (FcPatternPrint) FcPatternPrint __attribute((alias("IA__FcPatternPrint"), visibility("default")));
#endif
#ifdef __fcdbg__
#undef FcFontSetPrint
extern __typeof (FcFontSetPrint) FcFontSetPrint __attribute((alias("IA__FcFontSetPrint"), visibility("default")));
#endif
#ifdef __fcdefault__
#undef FcDefaultSubstitute
extern __typeof (FcDefaultSubstitute) FcDefaultSubstitute __attribute((alias("IA__FcDefaultSubstitute"), visibility("default")));
#endif
#ifdef __fcdir__
#undef FcFileIsDir
extern __typeof (FcFileIsDir) FcFileIsDir __attribute((alias("IA__FcFileIsDir"), visibility("default")));
#endif
#ifdef __fcdir__
#undef FcFileScan
extern __typeof (FcFileScan) FcFileScan __attribute((alias("IA__FcFileScan"), visibility("default")));
#endif
#ifdef __fcdir__
#undef FcDirScan
extern __typeof (FcDirScan) FcDirScan __attribute((alias("IA__FcDirScan"), visibility("default")));
#endif
#ifdef __fcdir__
#undef FcDirSave
extern __typeof (FcDirSave) FcDirSave __attribute((alias("IA__FcDirSave"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcDirCacheLoad
extern __typeof (FcDirCacheLoad) FcDirCacheLoad __attribute((alias("IA__FcDirCacheLoad"), visibility("default")));
#endif
#ifdef __fcdir__
#undef FcDirCacheRead
extern __typeof (FcDirCacheRead) FcDirCacheRead __attribute((alias("IA__FcDirCacheRead"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcDirCacheLoadFile
extern __typeof (FcDirCacheLoadFile) FcDirCacheLoadFile __attribute((alias("IA__FcDirCacheLoadFile"), visibility("default")));
#endif
#ifdef __fccache__
#undef FcDirCacheUnload
extern __typeof (FcDirCacheUnload) FcDirCacheUnload __attribute((alias("IA__FcDirCacheUnload"), visibility("default")));
#endif
#ifdef __fcfreetype__
#undef FcFreeTypeQuery
extern __typeof (FcFreeTypeQuery) FcFreeTypeQuery __attribute((alias("IA__FcFreeTypeQuery"), visibility("default")));
#endif
#ifdef __fcfs__
#undef FcFontSetCreate
extern __typeof (FcFontSetCreate) FcFontSetCreate __attribute((alias("IA__FcFontSetCreate"), visibility("default")));
#endif
#ifdef __fcfs__
#undef FcFontSetDestroy
extern __typeof (FcFontSetDestroy) FcFontSetDestroy __attribute((alias("IA__FcFontSetDestroy"), visibility("default")));
#endif
#ifdef __fcfs__
#undef FcFontSetAdd
extern __typeof (FcFontSetAdd) FcFontSetAdd __attribute((alias("IA__FcFontSetAdd"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcInitLoadConfig
extern __typeof (FcInitLoadConfig) FcInitLoadConfig __attribute((alias("IA__FcInitLoadConfig"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcInitLoadConfigAndFonts
extern __typeof (FcInitLoadConfigAndFonts) FcInitLoadConfigAndFonts __attribute((alias("IA__FcInitLoadConfigAndFonts"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcInit
extern __typeof (FcInit) FcInit __attribute((alias("IA__FcInit"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcFini
extern __typeof (FcFini) FcFini __attribute((alias("IA__FcFini"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcGetVersion
extern __typeof (FcGetVersion) FcGetVersion __attribute((alias("IA__FcGetVersion"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcInitReinitialize
extern __typeof (FcInitReinitialize) FcInitReinitialize __attribute((alias("IA__FcInitReinitialize"), visibility("default")));
#endif
#ifdef __fcinit__
#undef FcInitBringUptoDate
extern __typeof (FcInitBringUptoDate) FcInitBringUptoDate __attribute((alias("IA__FcInitBringUptoDate"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcGetLangs
extern __typeof (FcGetLangs) FcGetLangs __attribute((alias("IA__FcGetLangs"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangGetCharSet
extern __typeof (FcLangGetCharSet) FcLangGetCharSet __attribute((alias("IA__FcLangGetCharSet"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetCreate
extern __typeof (FcLangSetCreate) FcLangSetCreate __attribute((alias("IA__FcLangSetCreate"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetDestroy
extern __typeof (FcLangSetDestroy) FcLangSetDestroy __attribute((alias("IA__FcLangSetDestroy"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetCopy
extern __typeof (FcLangSetCopy) FcLangSetCopy __attribute((alias("IA__FcLangSetCopy"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetAdd
extern __typeof (FcLangSetAdd) FcLangSetAdd __attribute((alias("IA__FcLangSetAdd"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetHasLang
extern __typeof (FcLangSetHasLang) FcLangSetHasLang __attribute((alias("IA__FcLangSetHasLang"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetCompare
extern __typeof (FcLangSetCompare) FcLangSetCompare __attribute((alias("IA__FcLangSetCompare"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetContains
extern __typeof (FcLangSetContains) FcLangSetContains __attribute((alias("IA__FcLangSetContains"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetEqual
extern __typeof (FcLangSetEqual) FcLangSetEqual __attribute((alias("IA__FcLangSetEqual"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetHash
extern __typeof (FcLangSetHash) FcLangSetHash __attribute((alias("IA__FcLangSetHash"), visibility("default")));
#endif
#ifdef __fclang__
#undef FcLangSetGetLangs
extern __typeof (FcLangSetGetLangs) FcLangSetGetLangs __attribute((alias("IA__FcLangSetGetLangs"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcObjectSetCreate
extern __typeof (FcObjectSetCreate) FcObjectSetCreate __attribute((alias("IA__FcObjectSetCreate"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcObjectSetAdd
extern __typeof (FcObjectSetAdd) FcObjectSetAdd __attribute((alias("IA__FcObjectSetAdd"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcObjectSetDestroy
extern __typeof (FcObjectSetDestroy) FcObjectSetDestroy __attribute((alias("IA__FcObjectSetDestroy"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcObjectSetVaBuild
extern __typeof (FcObjectSetVaBuild) FcObjectSetVaBuild __attribute((alias("IA__FcObjectSetVaBuild"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcObjectSetBuild
extern __typeof (FcObjectSetBuild) FcObjectSetBuild __attribute((alias("IA__FcObjectSetBuild"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcFontSetList
extern __typeof (FcFontSetList) FcFontSetList __attribute((alias("IA__FcFontSetList"), visibility("default")));
#endif
#ifdef __fclist__
#undef FcFontList
extern __typeof (FcFontList) FcFontList __attribute((alias("IA__FcFontList"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicCreate
extern __typeof (FcAtomicCreate) FcAtomicCreate __attribute((alias("IA__FcAtomicCreate"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicLock
extern __typeof (FcAtomicLock) FcAtomicLock __attribute((alias("IA__FcAtomicLock"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicNewFile
extern __typeof (FcAtomicNewFile) FcAtomicNewFile __attribute((alias("IA__FcAtomicNewFile"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicOrigFile
extern __typeof (FcAtomicOrigFile) FcAtomicOrigFile __attribute((alias("IA__FcAtomicOrigFile"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicReplaceOrig
extern __typeof (FcAtomicReplaceOrig) FcAtomicReplaceOrig __attribute((alias("IA__FcAtomicReplaceOrig"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicDeleteNew
extern __typeof (FcAtomicDeleteNew) FcAtomicDeleteNew __attribute((alias("IA__FcAtomicDeleteNew"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicUnlock
extern __typeof (FcAtomicUnlock) FcAtomicUnlock __attribute((alias("IA__FcAtomicUnlock"), visibility("default")));
#endif
#ifdef __fcatomic__
#undef FcAtomicDestroy
extern __typeof (FcAtomicDestroy) FcAtomicDestroy __attribute((alias("IA__FcAtomicDestroy"), visibility("default")));
#endif
#ifdef __fcmatch__
#undef FcFontSetMatch
extern __typeof (FcFontSetMatch) FcFontSetMatch __attribute((alias("IA__FcFontSetMatch"), visibility("default")));
#endif
#ifdef __fcmatch__
#undef FcFontMatch
extern __typeof (FcFontMatch) FcFontMatch __attribute((alias("IA__FcFontMatch"), visibility("default")));
#endif
#ifdef __fcmatch__
#undef FcFontRenderPrepare
extern __typeof (FcFontRenderPrepare) FcFontRenderPrepare __attribute((alias("IA__FcFontRenderPrepare"), visibility("default")));
#endif
#ifdef __fcmatch__
#undef FcFontSetSort
extern __typeof (FcFontSetSort) FcFontSetSort __attribute((alias("IA__FcFontSetSort"), visibility("default")));
#endif
#ifdef __fcmatch__
#undef FcFontSort
extern __typeof (FcFontSort) FcFontSort __attribute((alias("IA__FcFontSort"), visibility("default")));
#endif
#ifdef __fcmatch__
#undef FcFontSetSortDestroy
extern __typeof (FcFontSetSortDestroy) FcFontSetSortDestroy __attribute((alias("IA__FcFontSetSortDestroy"), visibility("default")));
#endif
#ifdef __fcmatrix__
#undef FcMatrixCopy
extern __typeof (FcMatrixCopy) FcMatrixCopy __attribute((alias("IA__FcMatrixCopy"), visibility("default")));
#endif
#ifdef __fcmatrix__
#undef FcMatrixEqual
extern __typeof (FcMatrixEqual) FcMatrixEqual __attribute((alias("IA__FcMatrixEqual"), visibility("default")));
#endif
#ifdef __fcmatrix__
#undef FcMatrixMultiply
extern __typeof (FcMatrixMultiply) FcMatrixMultiply __attribute((alias("IA__FcMatrixMultiply"), visibility("default")));
#endif
#ifdef __fcmatrix__
#undef FcMatrixRotate
extern __typeof (FcMatrixRotate) FcMatrixRotate __attribute((alias("IA__FcMatrixRotate"), visibility("default")));
#endif
#ifdef __fcmatrix__
#undef FcMatrixScale
extern __typeof (FcMatrixScale) FcMatrixScale __attribute((alias("IA__FcMatrixScale"), visibility("default")));
#endif
#ifdef __fcmatrix__
#undef FcMatrixShear
extern __typeof (FcMatrixShear) FcMatrixShear __attribute((alias("IA__FcMatrixShear"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameRegisterObjectTypes
extern __typeof (FcNameRegisterObjectTypes) FcNameRegisterObjectTypes __attribute((alias("IA__FcNameRegisterObjectTypes"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameUnregisterObjectTypes
extern __typeof (FcNameUnregisterObjectTypes) FcNameUnregisterObjectTypes __attribute((alias("IA__FcNameUnregisterObjectTypes"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameGetObjectType
extern __typeof (FcNameGetObjectType) FcNameGetObjectType __attribute((alias("IA__FcNameGetObjectType"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameRegisterConstants
extern __typeof (FcNameRegisterConstants) FcNameRegisterConstants __attribute((alias("IA__FcNameRegisterConstants"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameUnregisterConstants
extern __typeof (FcNameUnregisterConstants) FcNameUnregisterConstants __attribute((alias("IA__FcNameUnregisterConstants"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameGetConstant
extern __typeof (FcNameGetConstant) FcNameGetConstant __attribute((alias("IA__FcNameGetConstant"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameConstant
extern __typeof (FcNameConstant) FcNameConstant __attribute((alias("IA__FcNameConstant"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameParse
extern __typeof (FcNameParse) FcNameParse __attribute((alias("IA__FcNameParse"), visibility("default")));
#endif
#ifdef __fcname__
#undef FcNameUnparse
extern __typeof (FcNameUnparse) FcNameUnparse __attribute((alias("IA__FcNameUnparse"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternCreate
extern __typeof (FcPatternCreate) FcPatternCreate __attribute((alias("IA__FcPatternCreate"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternDuplicate
extern __typeof (FcPatternDuplicate) FcPatternDuplicate __attribute((alias("IA__FcPatternDuplicate"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternReference
extern __typeof (FcPatternReference) FcPatternReference __attribute((alias("IA__FcPatternReference"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternFilter
extern __typeof (FcPatternFilter) FcPatternFilter __attribute((alias("IA__FcPatternFilter"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcValueDestroy
extern __typeof (FcValueDestroy) FcValueDestroy __attribute((alias("IA__FcValueDestroy"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcValueEqual
extern __typeof (FcValueEqual) FcValueEqual __attribute((alias("IA__FcValueEqual"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcValueSave
extern __typeof (FcValueSave) FcValueSave __attribute((alias("IA__FcValueSave"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternDestroy
extern __typeof (FcPatternDestroy) FcPatternDestroy __attribute((alias("IA__FcPatternDestroy"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternEqual
extern __typeof (FcPatternEqual) FcPatternEqual __attribute((alias("IA__FcPatternEqual"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternEqualSubset
extern __typeof (FcPatternEqualSubset) FcPatternEqualSubset __attribute((alias("IA__FcPatternEqualSubset"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternHash
extern __typeof (FcPatternHash) FcPatternHash __attribute((alias("IA__FcPatternHash"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAdd
extern __typeof (FcPatternAdd) FcPatternAdd __attribute((alias("IA__FcPatternAdd"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddWeak
extern __typeof (FcPatternAddWeak) FcPatternAddWeak __attribute((alias("IA__FcPatternAddWeak"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGet
extern __typeof (FcPatternGet) FcPatternGet __attribute((alias("IA__FcPatternGet"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternDel
extern __typeof (FcPatternDel) FcPatternDel __attribute((alias("IA__FcPatternDel"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternRemove
extern __typeof (FcPatternRemove) FcPatternRemove __attribute((alias("IA__FcPatternRemove"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddInteger
extern __typeof (FcPatternAddInteger) FcPatternAddInteger __attribute((alias("IA__FcPatternAddInteger"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddDouble
extern __typeof (FcPatternAddDouble) FcPatternAddDouble __attribute((alias("IA__FcPatternAddDouble"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddString
extern __typeof (FcPatternAddString) FcPatternAddString __attribute((alias("IA__FcPatternAddString"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddMatrix
extern __typeof (FcPatternAddMatrix) FcPatternAddMatrix __attribute((alias("IA__FcPatternAddMatrix"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddCharSet
extern __typeof (FcPatternAddCharSet) FcPatternAddCharSet __attribute((alias("IA__FcPatternAddCharSet"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddBool
extern __typeof (FcPatternAddBool) FcPatternAddBool __attribute((alias("IA__FcPatternAddBool"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternAddLangSet
extern __typeof (FcPatternAddLangSet) FcPatternAddLangSet __attribute((alias("IA__FcPatternAddLangSet"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetInteger
extern __typeof (FcPatternGetInteger) FcPatternGetInteger __attribute((alias("IA__FcPatternGetInteger"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetDouble
extern __typeof (FcPatternGetDouble) FcPatternGetDouble __attribute((alias("IA__FcPatternGetDouble"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetString
extern __typeof (FcPatternGetString) FcPatternGetString __attribute((alias("IA__FcPatternGetString"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetMatrix
extern __typeof (FcPatternGetMatrix) FcPatternGetMatrix __attribute((alias("IA__FcPatternGetMatrix"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetCharSet
extern __typeof (FcPatternGetCharSet) FcPatternGetCharSet __attribute((alias("IA__FcPatternGetCharSet"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetBool
extern __typeof (FcPatternGetBool) FcPatternGetBool __attribute((alias("IA__FcPatternGetBool"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternGetLangSet
extern __typeof (FcPatternGetLangSet) FcPatternGetLangSet __attribute((alias("IA__FcPatternGetLangSet"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternVaBuild
extern __typeof (FcPatternVaBuild) FcPatternVaBuild __attribute((alias("IA__FcPatternVaBuild"), visibility("default")));
#endif
#ifdef __fcpat__
#undef FcPatternBuild
extern __typeof (FcPatternBuild) FcPatternBuild __attribute((alias("IA__FcPatternBuild"), visibility("default")));
#endif
#ifdef __fcformat__
#undef FcPatternFormat
extern __typeof (FcPatternFormat) FcPatternFormat __attribute((alias("IA__FcPatternFormat"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrCopy
extern __typeof (FcStrCopy) FcStrCopy __attribute((alias("IA__FcStrCopy"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrCopyFilename
extern __typeof (FcStrCopyFilename) FcStrCopyFilename __attribute((alias("IA__FcStrCopyFilename"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrPlus
extern __typeof (FcStrPlus) FcStrPlus __attribute((alias("IA__FcStrPlus"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrFree
extern __typeof (FcStrFree) FcStrFree __attribute((alias("IA__FcStrFree"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrDowncase
extern __typeof (FcStrDowncase) FcStrDowncase __attribute((alias("IA__FcStrDowncase"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrCmpIgnoreCase
extern __typeof (FcStrCmpIgnoreCase) FcStrCmpIgnoreCase __attribute((alias("IA__FcStrCmpIgnoreCase"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrCmp
extern __typeof (FcStrCmp) FcStrCmp __attribute((alias("IA__FcStrCmp"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrStrIgnoreCase
extern __typeof (FcStrStrIgnoreCase) FcStrStrIgnoreCase __attribute((alias("IA__FcStrStrIgnoreCase"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrStr
extern __typeof (FcStrStr) FcStrStr __attribute((alias("IA__FcStrStr"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcUtf8ToUcs4
extern __typeof (FcUtf8ToUcs4) FcUtf8ToUcs4 __attribute((alias("IA__FcUtf8ToUcs4"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcUtf8Len
extern __typeof (FcUtf8Len) FcUtf8Len __attribute((alias("IA__FcUtf8Len"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcUcs4ToUtf8
extern __typeof (FcUcs4ToUtf8) FcUcs4ToUtf8 __attribute((alias("IA__FcUcs4ToUtf8"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcUtf16ToUcs4
extern __typeof (FcUtf16ToUcs4) FcUtf16ToUcs4 __attribute((alias("IA__FcUtf16ToUcs4"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcUtf16Len
extern __typeof (FcUtf16Len) FcUtf16Len __attribute((alias("IA__FcUtf16Len"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrDirname
extern __typeof (FcStrDirname) FcStrDirname __attribute((alias("IA__FcStrDirname"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrBasename
extern __typeof (FcStrBasename) FcStrBasename __attribute((alias("IA__FcStrBasename"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetCreate
extern __typeof (FcStrSetCreate) FcStrSetCreate __attribute((alias("IA__FcStrSetCreate"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetMember
extern __typeof (FcStrSetMember) FcStrSetMember __attribute((alias("IA__FcStrSetMember"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetEqual
extern __typeof (FcStrSetEqual) FcStrSetEqual __attribute((alias("IA__FcStrSetEqual"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetAdd
extern __typeof (FcStrSetAdd) FcStrSetAdd __attribute((alias("IA__FcStrSetAdd"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetAddFilename
extern __typeof (FcStrSetAddFilename) FcStrSetAddFilename __attribute((alias("IA__FcStrSetAddFilename"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetDel
extern __typeof (FcStrSetDel) FcStrSetDel __attribute((alias("IA__FcStrSetDel"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrSetDestroy
extern __typeof (FcStrSetDestroy) FcStrSetDestroy __attribute((alias("IA__FcStrSetDestroy"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrListCreate
extern __typeof (FcStrListCreate) FcStrListCreate __attribute((alias("IA__FcStrListCreate"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrListNext
extern __typeof (FcStrListNext) FcStrListNext __attribute((alias("IA__FcStrListNext"), visibility("default")));
#endif
#ifdef __fcstr__
#undef FcStrListDone
extern __typeof (FcStrListDone) FcStrListDone __attribute((alias("IA__FcStrListDone"), visibility("default")));
#endif
#ifdef __fcxml__
#undef FcConfigParseAndLoad
extern __typeof (FcConfigParseAndLoad) FcConfigParseAndLoad __attribute((alias("IA__FcConfigParseAndLoad"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigGetRescanInverval
extern __typeof (FcConfigGetRescanInverval) FcConfigGetRescanInverval __attribute((alias("IA__FcConfigGetRescanInverval"), visibility("default")));
#endif
#ifdef __fccfg__
#undef FcConfigSetRescanInverval
extern __typeof (FcConfigSetRescanInverval) FcConfigSetRescanInverval __attribute((alias("IA__FcConfigSetRescanInverval"), visibility("default")));
#endif
#endif
