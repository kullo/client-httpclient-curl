win32 {
    LIBS += -ladvapi32 -luser32
    QMAKE_CXXFLAGS_RELEASE += /Zi  # compiler: put debug info into PDB
    QMAKE_LFLAGS_RELEASE += /debug # linker: put debug info into PDB
    QMAKE_LFLAGS_RELEASE += /opt:ref  # linker: re-enable unreferenced code optimization (disabled by /debug)
}
else {
    LIBS += -lpthread
    LIBS += -ldl
    QMAKE_LFLAGS += -rdynamic
}

macx-clang {
    # http://petereisentraut.blogspot.de/2011/05/ccache-and-clang.html
    QMAKE_CXXFLAGS += -Qunused-arguments
}
