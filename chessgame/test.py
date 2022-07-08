import ctypes
import time

testlib = ctypes.CDLL('./test.so')
testlib.num_test.restype = ctypes.c_int

testlib.is_even.restype = ctypes.c_bool
testlib.is_even.argtypes = [ctypes.c_int]

testlib.get_list.restype = ctypes.POINTER(ctypes.c_int * 64)

testlib.hello_world()
time.sleep(1)
print(testlib.num_test())
time.sleep(1)
print(testlib.is_even(5))

a = testlib.get_list()
print([i for i in a.contents])
a = testlib.get_list()
print([i for i in a.contents])
