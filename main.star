#!/usr/bin/env lucicfg

luci.project(
    name = "Skia, 2D graphics library",
    buildbucket = "cr-buildbucket.appspot.com",
    swarming = "chromium-swarm.appspot.com",
    acls = [
        acl.entry(acl.PROJECT_CONFIGS_READER, groups = ["all"]),
        acl.entry(acl.LOGDOG_READER, groups = ["all"]),
        acl.entry(acl.LOGDOG_WRITER, groups = ["luci-logdog-skia-writers"]),
        acl.entry(acl.CQ_COMMITTER, groups = ["project-skia-committers"]),
        acl.entry(acl.CQ_DRY_RUNNER, groups = ["project-skia-tryjob-access"]),
    ],
    logdog = "luci-logdog",
)

luci.logdog(
    gs_bucket = "skia-logdog",
)

luci.bucket(
    name = "skia.primary",
    acls = [
        acl.entry(acl.BUILDBUCKET_READER, groups = ["all"]),
        acl.entry(acl.BUILDBUCKET_OWNER, groups = [
            "project-skia-external-buildbucket-writers",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, groups = [
            "project-skia-tryjob-access",
            "service-account-cq",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, projects = [
            "skiabuildbot",
            "skia-skcms",
        ]),
    ],
)

luci.bucket(
    name = "skia.testing",
    acls = [
        acl.entry(acl.BUILDBUCKET_READER, groups = ["all"]),
        acl.entry(acl.BUILDBUCKET_OWNER, groups = [
            "project-skia-external-buildbucket-writers",
            "project-skia-committers",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, groups = [
            "project-skia-tryjob-access",
            "service-account-cq",
        ]),
        acl.entry(acl.BUILDBUCKET_TRIGGERER, projects = [
            "skiabot-test",
        ]),
    ],
)

luci.cq(
    status_host = "chromium-cq-status.appspot.com",
    submit_burst_delay = time.duration(300000),
    submit_max_burst = 2,
)

luci.cq_group(
    name = "android/next",
    watch = cq.refset(
        repo = "https://skia.googlesource.com/skia",
        refs = ["refs/heads/android/next-release"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 2,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Release-Android_API26",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm64-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Tidy",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Wuffs",
            location_regexp = [".+/[+]/DEPS", ".+/[+]/src/codec/SkWuffs.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Debug-NoGPU_Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Release-Docker",
        ),
        #luci.cq_tryjob_verifier(
        #    builder = "skia:skia.primary/Build-Mac-Clang-arm64-Debug-iOS",
        #),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Mac-Clang-x86_64-Release",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-arm64-Release-ANGLE",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-OnDemand-Presubmit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-InfraTests_Linux",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Release-All",
        ),
    ],
)

luci.cq_group(
    name = "chrome/m90",
    watch = cq.refset(
        repo = "https://skia.googlesource.com/skia",
        refs = ["refs/heads/chrome/m90"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 2,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Release-Android_API26",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm64-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Tidy",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Wuffs",
            location_regexp = [".+/[+]/DEPS", ".+/[+]/src/codec/SkWuffs.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Debug-NoGPU_Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Release-Docker",
        ),
        #luci.cq_tryjob_verifier(
        #    builder = "skia:skia.primary/Build-Mac-Clang-arm64-Debug-iOS",
        #),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Mac-Clang-x86_64-Release",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-OnDemand-Presubmit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-CheckGeneratedFiles",
            location_regexp = [".+/[+]/src/gpu/effects/generated/.*", ".+/[+]/src/sksl/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-InfraTests_Linux",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-RunGnToBp",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Mac10.15.1-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All-Metal",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Release-All",
        ),
    ],
)

luci.cq_group(
    name = "chrome/m91",
    watch = cq.refset(
        repo = "https://skia.googlesource.com/skia",
        refs = ["refs/heads/chrome/m91"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 2,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Release-Android_API26",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm64-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Tidy",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Wuffs",
            location_regexp = [".+/[+]/DEPS", ".+/[+]/src/codec/SkWuffs.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Debug-NoGPU_Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Release-Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Mac-Clang-x86_64-Release",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-arm64-Release-ANGLE",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-OnDemand-Presubmit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-CheckGeneratedFiles",
            location_regexp = [".+/[+]/src/gpu/effects/generated/.*", ".+/[+]/src/sksl/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-InfraTests_Linux",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-RunGnToBp",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Mac10.15.1-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All-Metal",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Release-All",
        ),
    ],
)

luci.cq_group(
    name = "main",
    watch = cq.refset(
        repo = "https://skia.googlesource.com/skia",
        refs = ["refs/heads/main"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 2,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    tree_status_host = "tree-status.skia.org",
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "chromium:try/linux-blink-rel",
            includable_only = True,
        ),
        luci.cq_tryjob_verifier(
            builder = "chromium:try/linux_chromium_compile_dbg_ng",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Release-Android_API26",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm64-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Tidy",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Wuffs",
            location_regexp = [".+/[+]/DEPS", ".+/[+]/src/codec/SkWuffs.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-EMCC-wasm-Release-CanvasKit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-EMCC-wasm-Release-CanvasKit_CPU",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Debug-NoGPU_Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Release-Docker",
        ),
        #luci.cq_tryjob_verifier(
        #    builder = "skia:skia.primary/Build-Mac-Clang-arm64-Debug-iOS",
        #),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Mac-Clang-x86_64-Release",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-arm64-Release-ANGLE",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/BuildStats-Debian10-EMCC-wasm-Release-CanvasKit",
            location_regexp = [".+/[+]/infra/canvaskit/.*", ".+/[+]/modules/canvaskit/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/BuildStats-Debian9-Clang-arm-Release-Flutter_Android_Docker",
            experiment_percentage = 100,
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-OnDemand-Presubmit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-CheckGeneratedFiles",
            location_regexp = [".+/[+]/src/gpu/effects/generated/.*", ".+/[+]/src/sksl/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-InfraTests_Linux",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-RunGnToBp",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-EMCC-GCE-CPU-AVX2-asmjs-Release-All-PathKit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-EMCC-GCE-CPU-AVX2-wasm-Release-All-CanvasKit",
            location_regexp = [".+/[+]/modules/canvaskit/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-EMCC-GCE-CPU-AVX2-wasm-Release-All-PathKit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-EMCC-GCE-GPU-AVX2-wasm-Release-All-CanvasKit",
            location_regexp = [".+/[+]/modules/canvaskit/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Mac10.15.1-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All-Metal",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Release-All",
        ),
    ],
)

luci.cq_group(
    name = "chrome/m92",
    watch = cq.refset(
        repo = "https://skia.googlesource.com/skia",
        refs = ["refs/heads/chrome/m92"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 2,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Release-Android_API26",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm64-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Tidy",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Wuffs",
            location_regexp = [".+/[+]/DEPS", ".+/[+]/src/codec/SkWuffs.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Debug-NoGPU_Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Release-Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Mac-Clang-x86_64-Release",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-arm64-Release-ANGLE",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-OnDemand-Presubmit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-CheckGeneratedFiles",
            location_regexp = [".+/[+]/src/gpu/effects/generated/.*", ".+/[+]/src/sksl/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-InfraTests_Linux",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-RunGnToBp",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Mac10.15.1-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All-Metal",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Release-All",
        ),
    ],
)

luci.cq_group(
    name = "chrome/m93",
    watch = cq.refset(
        repo = "https://skia.googlesource.com/skia",
        refs = ["refs/heads/chrome/m93"],
    ),
    retry_config = cq.retry_config(
        single_quota = 1,
        global_quota = 2,
        failure_weight = 2,
        transient_failure_weight = 1,
        timeout_weight = 2,
    ),
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm-Release-Android_API26",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-arm64-Debug-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Tidy",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-Clang-x86_64-Debug-Wuffs",
            location_regexp = [".+/[+]/DEPS", ".+/[+]/src/codec/SkWuffs.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Debug-NoGPU_Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Debian10-GCC-x86_64-Release-Docker",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Mac-Clang-x86_64-Release",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86-Debug",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-Clang-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-arm64-Release-ANGLE",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Build-Win-MSVC-x86_64-Release-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-OnDemand-Presubmit",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-CheckGeneratedFiles",
            location_regexp = [".+/[+]/src/gpu/effects/generated/.*", ".+/[+]/src/sksl/.*"],
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-InfraTests_Linux",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Housekeeper-PerCommit-RunGnToBp",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All-Android",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Mac10.15.1-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All-Metal",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-ASAN",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3_Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Dawn",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-All",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Direct3D",
        ),
        luci.cq_tryjob_verifier(
            builder = "skia:skia.primary/Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Release-All",
        ),
    ],
)
