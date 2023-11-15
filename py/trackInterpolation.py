import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from sklearn.neighbors import KDTree
from scipy.interpolate import splprep, splev


def kdTreeSKlearn(points):
    tree = KDTree(points, leaf_size=2)
    return tree


points = np.loadtxt(
    "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/prop/dataPoints.csv",
    delimiter=",",
    dtype=int,
)

# Delete duplicate points
pointsUnique = np.unique(points, axis=0)

tree = kdTreeSKlearn(pointsUnique)

# Get point with biggest distance to each other
refPoint = np.array([[564, 1026]])
_, indx = tree.query(refPoint, k=2)


# Evaluate correct sequence
pIndx = indx[0, 0]
sequence = np.array([], dtype=int)
k = pointsUnique.shape[0]

for i in range(pointsUnique.shape[0]):
    _, indx = tree.query(np.array([pointsUnique[pIndx, :]]), k=k)

    for j in range(k):
        if not np.sum(np.isin(sequence, indx[:, j][0])):
            pIndx = indx[:, j][0]
            sequence = np.append(sequence, pIndx)
            break
    # print(pIndx)

orderedPoints = pointsUnique[sequence[: sequence.shape[0] - 2]]
max_x = np.max(orderedPoints[:, 0])
max_y = np.max(orderedPoints[:, 1])
orderedPoints = orderedPoints / np.array([max_x, max_y])


k = 3
# The user can use s to control the trade-off between
# closeness and smoothness of fit. Larger s means more
# smoothing while smaller values of s indicate less
# smoothing.
# s = None
tck, t = splprep([orderedPoints[:, 0], orderedPoints[:, 1]], k=3, s=0.005)
# (t,c,k) a tuple containing the vector of knots, the B-spline coefficients, and the degree of the spline

# Spline interpolation
# Spline 0 degree derivative
t = np.linspace(0, 1, 101)
splinePoints = splev(t, tck)
splinePoints = np.array(splinePoints).T
splinePoints[:, 0] = splinePoints[:, 0] * max_x
splinePoints[:, 1] = splinePoints[:, 1] * max_y
# print(splinePoints)

# Spline 1 degree derivative
splineDerivitivePoints = splev(t, tck, der=1)
splineDerivitivePoints = np.array(splineDerivitivePoints).T
# Slope
splineDerivitivePoints = splineDerivitivePoints[:, 0] / splineDerivitivePoints[:, 1]
# print(splineDerivitivePoints)

# Combine 0,1 degree derivative in one array
x = splinePoints[:, 0].round().astype(int)
y = splinePoints[:, 1].round().astype(int)
d = (np.around(splineDerivitivePoints, decimals=4) * 1000).round().astype(int)
dataSet = np.block(
    [
        [x],
        [y],
        [d],
    ]
)
# print(dataSet)

# Convert to pandas dataframe
df = pd.DataFrame(dataSet.T)

# Save as .csv
df.to_csv(
    "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/prop/splineData.csv",
    index=False,
    header=False,
)

orderedPoints[:, 0] = orderedPoints[:, 0] * max_x
orderedPoints[:, 1] = orderedPoints[:, 1] * max_y


def dist2Points(p1, p2):
    dist = np.linalg.norm(p2 - p1)
    # print(dist)
    return dist


def cosLaw(a, b, c):
    angle = np.rad2deg(np.arccos((c**2 - a**2 - b**2) / (-2 * a * b)))
    # print(angle)
    return angle


# Callculate angels
numAnglePoints = 10
angles = np.array([])
sumAngles = np.array([])

for i in range(splinePoints.shape[0]):
    indices = np.array([i - 1, i, i + 1])
    for k, idx in enumerate(indices):
        if idx > splinePoints.shape[0] - 1:
            idx = idx - (splinePoints.shape[0])
        elif idx < 0:
            idx = idx + (splinePoints.shape[0])
        indices[k] = idx
    angle = cosLaw(
        dist2Points(splinePoints[indices[1]], splinePoints[indices[2]]),
        dist2Points(splinePoints[indices[0]], splinePoints[indices[1]]),
        dist2Points(splinePoints[indices[0]], splinePoints[indices[2]]),
    )
    angles = np.append(angles, np.around(180 - angle, 3))

print(angles)


# Plot
fig = plt.figure(figsize=(7, 7))
ax = fig.add_subplot(111)
ax.plot(orderedPoints[:, 0], orderedPoints[:, 1], "ob", label="sample points")
ax.plot(splinePoints[:, 0], splinePoints[:, 1], "-xr", label="spline interpolation")
ax.plot(
    orderedPoints[0, 0],
    orderedPoints[0, 1],
    "og",
    label="first point",
)
ax.plot(
    orderedPoints[orderedPoints.shape[0] - 1, 0],
    orderedPoints[orderedPoints.shape[0] - 1, 1],
    "or",
    label="last point",
)
ax.plot(refPoint[0, 0], refPoint[0, 1], "oy", label="Reference point")
ax.set_title("b-Spline fit")
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.invert_yaxis()
ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
ax.set_box_aspect(np.max(orderedPoints[:, 1]) / np.max(orderedPoints[:, 0]))
ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))
plt.tight_layout()
plt.show()

""" # Plot
fig, ax = plt.subplots(subplot_kw=dict(projection="3d"))
ax.stem(splinePoints[:, 0], splinePoints[:, 1], splineDerivitivePoints)
ax.set_title("b-Spline, slope")
ax.set(xlabel="x", ylabel="y", zlabel="slope")
ax.legend()
plt.show() """
