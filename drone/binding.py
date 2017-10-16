from ctypes import *
import os

class lib:

    libc = None

    def __init__(self):
        global libc
        libc = cdll.LoadLibrary("../cvlib/lib/libcvlib.so")

    def libtest(self):
        global libc
        call = libc.libtest
        call.restype = c_void_p
        print call()
