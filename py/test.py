import numpy as np

mask1 = np.array([True, False, False, True, False, True, True, True, False, False])
mask2 = np.array([True, False, False, True, False])

test = np.where(mask1)

print(mask1)
mask1[np.where(mask1)] = np.logical_and(mask1[np.where(mask1)], mask2)


print(mask1)

print(-2 % -3)
