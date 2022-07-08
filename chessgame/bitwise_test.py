import numpy as np
import time

a = np.int64(834758)
st = time.time()
for i in range(100000):
    a = np.left_shift(a, 5)
et = time.time() - st
print('execution time:', et);