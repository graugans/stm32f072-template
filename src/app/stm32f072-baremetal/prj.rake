# -*- ruby -*-


# Project type, possible values are APP for applications, LIB for static libraries,
# SOLIB for shared objects and DISABLED if this project should be excluded from building.
PRJ_TYPE = 'APP'

# Additional white space separated list of sub directories this project uses for finding source files.
# By default only sources in the projects top directory will be used for compilation.
ADD_SOURCE_DIRS = 'Src'

# White space separated list of ignored source files. These will be excluded from compilation.
IGNORED_SOURCES = ''

# Additional white space separated list of sub directories this project uses for finding includes.
# By default the subdirectory 'include/' is always supposed.
ADD_INC_DIRS = 'Inc'

# White space separated list of test source directories.
TEST_SOURCE_DIRS = ''

# Specific Test Framework to use instead of the default test framework defined in rake/defaults.rb
TEST_FRAMEWORK = ''

# Additional white space separated list of CFLAGS. Used for all platforms.
# E.g. '-O3 -Wextra'
ADD_CFLAGS = ''

# Additional white space separated list of CXXFLAGS. Used for all platforms.
# E.g. '-O3 -Wextra'
ADD_CFLAGS = ''

# Additional white space separated list of libraries this project depends on. These can be either libraries provided
# from other subprojects or external libraries. In case of the former the include/ directory of that library
# is used for compilation as well. Used for all platforms.
# e.g. 'pthread rt m'
ADD_LIBS = 'printf stm32hal m c gcc nosys'

# Additional white space separated list of linker flags. Used for all platforms.
ADD_LDFLAGS = ''

# Set to 1 if you need Qt support. If enabled, all header files will be parsed for the
# declaration of the keyword Q_OBJECT and if found used as input for the moc compiler.
# By default QtCore and QtNetwork libs are enabled. If you need more Qt libraries,
# place them in ADD_LIBS variable.
USE_QT = 0

# White space separated list of ignored platforms, i.e. platforms this project will _not_ be compiled for.
# Possible values depend on your toolchain.
# E.g. 'arm-linux-gnueabi i686-linux-gnu'
IGNORED_PLATFORMS = ''

