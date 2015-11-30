CONFIG += c++11

CONFIG(release, debug|release): DEFINES += NDEBUG

CONFIG(release, debug|release):    LIBKULLO_BIN_DIR = $$PWD/../bin-libkullo
else:CONFIG(debug, debug|release): LIBKULLO_BIN_DIR = $$PWD/../bin-libkullo-debug

# Add farcaster repo root and libkullo includes to include path
INCLUDEPATH += $$PWD
INCLUDEPATH += $$LIBKULLO_BIN_DIR/include
DEFINES += BOOST_ALL_NO_LIB

defineTest(osx) {
    macx: return(true)
    return(false)
}

defineTest(osxDebug) {
    osx():CONFIG(debug, debug|release): return(true)
    return(false)
}

defineTest(osxRelease) {
    osx():CONFIG(release, debug|release): return(true)
    return(false)
}

defineTest(windows) {
    win32: return(true)
    return(false)
}

defineTest(windowsDebug) {
    windows():CONFIG(debug, debug|release): return(true)
    return(false)
}

defineTest(windowsRelease) {
    windows():CONFIG(release, debug|release): return(true)
    return(false)
}

defineTest(linux) {
    unix:!mac: return(true)
    return(false)
}

defineTest(linuxDebug) {
    linux():CONFIG(debug, debug|release): return(true)
    return(false)
}

defineTest(linuxRelease) {
    linux():CONFIG(release, debug|release): return(true)
    return(false)
}

macx {
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

# Hardening
*-g++:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -fstack-protector -fPIE -Wformat -Wformat-security -D_FORTIFY_SOURCE=2 -O2
    QMAKE_LFLAGS += -pie -z relro -z now
}

# Breakpad
*-g++:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -g
}

# Fix make's $(DEL_DIR)
#
# rmdir:
# /s : Removes the specified directory and all subdirectories including any files. Use /s to remove a tree.
# /q : Runs rmdir in quiet mode. Deletes directories without confirmation.
#
# rm:
#  -f    ignore nonexistent files and arguments, never prompt
#  -r    remove directories and their contents recursively
win32:QMAKE_DEL_DIR = rmdir /s /q
unix:QMAKE_DEL_DIR = rm -rf

