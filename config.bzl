
def platform_config():
    native.config_setting(
        name = "windows",
        constraint_values = ["@bazel_tools//platforms:windows"],
        visibility = ["//visibility:public"]
    )

package_copt = select({":windows" : ["/std:c++17"],
    "//conditions:default" : ["-std=c++17"],})