Import("env")
import sys

# Winsock is a system library on MinGW and is not needed on POSIX hosts.
if sys.platform == "win32":
    env.Append(LIBS=["ws2_32", "iphlpapi"])
    env.Append(LINKFLAGS=["-mconsole"])
