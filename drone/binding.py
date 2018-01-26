from ctypes import *
import os


libc = cdll.LoadLibrary("./libcvlib.so")

class Letter(Structure):
    _fields_ = [
        ('letter', c_char),
        ('width', c_int),
        ('height', c_int),
        ('x', c_int),
        ('y', c_int),
        ('pos', c_int),
        ('avSize',c_int)]


class lib(object):

    global lastLetter

    def __init__(self, display=False):
        global lastLetter
        libc.py_camLib.argtypes = [c_bool]
        libc.py_camLib.restype = c_void_p

        libc.libtest.argtypes = []
        libc.libtest.restype = c_void_p

        libc.py_getImg.argtypes = [c_void_p]
        libc.py_getImg.restype = c_void_p

        libc.py_findLetter.argtypes = [c_void_p, c_int]
        libc.py_findLetter.restype = c_void_p

        libc.py_mostOccouring.argtypes = [c_void_p]
        libc.py_mostOccouring.restype = Letter

        lastLetter = Letter
        self.obj = libc.py_camLib(display)

    def libtest(self):
        print libc.libtest(self.obj)

    def getImg(self):
        return libc.py_getImg(self.obj)

    def findLetters(self):
        return libc.py_findLetter(self.obj, 75)

    def mostOccouring(self):
        global lastLetter
        lastLetter = libc.py_mostOccouring(self.obj)
        return lastLetter

    def getLastLetter(self):
        global lastLetter
        return lastLetter

