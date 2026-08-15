find_package(PkgConfig)
pkg_check_modules(Wayland wayland-client wayland-egl wayland-cursor)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Wayland DEFAULT_MSG Wayland_LIBRARIES)
