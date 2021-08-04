#!/usr/bin/env lucicfg

# Enable LUCI Realms support.
lucicfg.enable_experiment("crbug.com/1085650")

# Keep only realms-dev.cfg.
lucicfg.config(tracked_files = ["realms-dev.cfg"])

luci.project(
    name = "Skia, 2D graphics library (dev configs)",
    dev = True,
)

# Contains ACLs for all dev Swarming resources. See chromium-swarm-dev/pools.cfg
# for where they are associated with this realm.
luci.realm(
    name = "skia.dev",
    bindings = [
        luci.binding(
            roles = "role/swarming.poolOwner",
            groups = "project-skia-admins",
        ),
        luci.binding(
            roles = "role/swarming.poolViewer",
            groups = "chromium-swarm-dev-view-all-bots",
        ),
        luci.binding(
            roles = "role/swarming.taskViewer",
            groups = "chromium-swarm-dev-view-all-tasks",
        ),
        luci.binding(
            roles = [
                "role/swarming.poolUser",
                "role/swarming.taskTriggerer",
            ],
            groups = [
                "project-skia-admins",
                "project-skia-external-task-schedulers",
            ],
        ),
        luci.binding(
            roles = "role/swarming.taskServiceAccount",
            users = [
                # All dev tasks use "none" as a service account they run as.
            ],
        )
    ],
)
