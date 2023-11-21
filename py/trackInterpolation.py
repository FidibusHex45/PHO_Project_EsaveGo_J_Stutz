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
# orderedPoints = np.vstack([orderedPoints, orderedPoints[0]])
# print(orderedPoints)
max_x = np.max(orderedPoints[:, 0])
max_y = np.max(orderedPoints[:, 1])
orderedPoints = orderedPoints / np.array([max_x, max_y])


k = 3
# The user can use s to control the trade-off between
# closeness and smoothness of fit. Larger s means more
# smoothing while smaller values of s indicate less
# smoothing.
# s = None
tck, t = splprep([orderedPoints[:, 0], orderedPoints[:, 1]], k=3, s=0.005, per=0)
# (t,c,k) a tuple containing the vector of knots, the B-spline coefficients, and the degree of the spline

# Spline interpolation
# Spline 0 degree derivative
t = np.linspace(0, 1, 101)
splinePoints = splev(t, tck)
splinePoints = np.array(splinePoints).T
splinePoints[:, 0] = splinePoints[:, 0] * max_x
splinePoints[:, 1] = splinePoints[:, 1] * max_y

orderedPoints[:, 0] = orderedPoints[:, 0] * max_x
orderedPoints[:, 1] = orderedPoints[:, 1] * max_y


def dist2Points(p1, p2):
    dist = np.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)

    # print(dist)
    return dist


def cosLaw(a, b, c):
    angle = np.rad2deg(np.arccos((c**2 - a**2 - b**2) / (-2 * a * b)))
    # print(angle)
    return angle


def direction(p1, p2, p3):
    # solve (P_11+t*(P_21-P_11))*(P_21-P_11)+(P_12+t*(P_22-P_12))*(P_22-P_12) for t
    t = (
        p1[0] ** 2
        - (p2[0] + p3[0]) * p1[0]
        + p1[1] ** 2
        + p2[0] * p3[0]
        + p2[1] * p3[1]
        - p1[1] * (p2[1] + p3[1])
    ) / (
        p1[0] ** 2
        - 2 * p3[0] * p1[0]
        + p1[1] ** 2
        + p3[0] ** 2
        + p3[1] ** 2
        - 2 * p1[1] * p3[1]
    )

    p_dir = np.array([(p1[0] + t * (p3[0] - p1[0])), (p1[1] + t * (p3[1] - p1[1]))])
    direction = p_dir - p2
    return direction / np.linalg.norm(direction)


# Callculate angels
angles = np.array([])
directions = np.array([])

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
    directions = np.append(
        directions,
        direction(
            splinePoints[indices[0]], splinePoints[indices[1]], splinePoints[indices[2]]
        ),
    )
    angles = np.append(angles, 180 - angle)

directions = directions.reshape((int(directions.shape[0] / 2), 2))

# spline correction
ratio = 1.8

startDir = np.array([-np.sqrt(2), -np.sqrt(2)])
halfDir = np.array([-np.sqrt(2), +np.sqrt(2)])

increment = 0.5

for i in range(directions.shape[0] - 1):
    diffdir = 0

    if i == 0:
        diffdir = np.linalg.norm(startDir + directions[i])

    elif i == int(directions.shape[0] / ratio) + 1:
        diffdir = np.linalg.norm(halfDir + directions[i])

    else:
        diffdir = np.linalg.norm(directions[i - 1] + directions[i])

    if diffdir < 1:
        while diffdir < 1.5:
            splinePoints[i] += directions[i] * increment
            directions[i] = direction(
                splinePoints[i - 1], splinePoints[i], splinePoints[i + 1]
            )
            diffdir = np.linalg.norm(directions[i - 1] + directions[i])

        angles[i] = 180 - cosLaw(
            dist2Points(splinePoints[i], splinePoints[i + 1]),
            dist2Points(splinePoints[i - 1], splinePoints[i]),
            dist2Points(splinePoints[i - 1], splinePoints[i + 1]),
        )
    print(f"Angel {i}: {angles[i]}")


# Calculate sum angles
sumAngles = np.array([])
numAnglePoints = 5
futureOffset = 5

for i in range(angles.shape[0]):
    indices = np.arange(i, numAnglePoints + i, 1) + futureOffset
    for j, idx in enumerate(indices):
        if idx > angles.shape[0] - 1:
            indices[j] = idx - angles.shape[0]
    sumAngles = np.append(sumAngles, np.mean(angles[indices]))
# print(sumAngles)


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

# Velocety plot
from matplotlib.collections import LineCollection
from matplotlib.colors import BoundaryNorm, ListedColormap

points = splinePoints.reshape(-1, 1, 2)
segments = np.concatenate([points[:-1], points[1:]], axis=1)
# print(segments)

fig, ax = plt.subplots(1, 1, figsize=(4, 7))

# Create a continuous norm to map from data points to colors
norm = plt.Normalize(sumAngles.min(), sumAngles.max())
lc = LineCollection(segments, cmap="jet", norm=norm)

# Set the values used for colormapping
lc.set_array(sumAngles)
lc.set_linewidth(5)
line = ax.add_collection(lc)
fig.colorbar(line, ax=ax)
ax.set_box_aspect(np.max(splinePoints[:, 1]) / np.max(splinePoints[:, 0]))
ax.set_xlim(splinePoints[:, 0].min() - 100, splinePoints[:, 0].max() + 100)
ax.set_ylim(splinePoints[:, 1].min() - 100, splinePoints[:, 1].max() + 100)
ax.set_title("Velocity")
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.invert_yaxis()
ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
plt.tight_layout()


# Direction plot
fig = plt.figure(figsize=(7, 7))
ax = fig.add_subplot(111)
ax.plot(
    splinePoints[0 : int(splinePoints.shape[0] / ratio), 0],
    splinePoints[0 : int(splinePoints.shape[0] / ratio), 1],
    "or",
    label="spline points",
)
ax.plot(
    splinePoints[int(splinePoints.shape[0] / ratio) :, 0],
    splinePoints[int(splinePoints.shape[0] / ratio) :, 1],
    "og",
    label="spline points",
)

for i in range(splinePoints.shape[0]):
    x1 = splinePoints[i, 0]
    x2 = 100 * directions[i, 0] + splinePoints[i, 0]
    y1 = splinePoints[i, 1]
    y2 = 100 * directions[i, 1] + splinePoints[i, 1]
    # ax.arrow(x1, y1, xl, yl, width=1)
    ax.annotate("", xytext=(x1, y1), xy=(x2, y2), arrowprops=dict(arrowstyle="->"))
ax.set_title("Directions")
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.invert_yaxis()
ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
ax.set_box_aspect(np.max(orderedPoints[:, 1]) / np.max(orderedPoints[:, 0]))
ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))

plt.show()


# Combine 0,1 degree derivative in one array
x = splinePoints[:, 0].round().astype(int)
y = splinePoints[:, 1].round().astype(int)
d = (np.around(sumAngles, decimals=4) * 1000).round().astype(int)
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
