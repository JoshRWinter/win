find_package(PkgConfig)
pkg_check_modules(EGL egl)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(EGL DEFAULT_MSG EGL_LIBRARIES)
