config_setting(
    name = "windows",
    constraint_values = ["@bazel_tools//platforms:windows"],
)

cc_library(
    name = "timer",
    hdrs = [
        "include/timer.h"
    ],
    srcs = [
        "src/timer.cpp",
    ],
    strip_include_prefix = "include",
    copts = select({":windows" : ["/std:c++17"],
    "//conditions:default" : ["-std=c++17"],}),
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test_timer",
    srcs = ["test/test_timer.cpp"],
    copts = select({":windows" : ["/std:c++17"],
                    "//conditions:default" : ["-std=c++17"],
                }
    ),
    tags = ["unit"],
    deps = [
        ":timer",
        "@gtest//:gtest",
    ],
    visibility = ["//visibility:private"]
)
