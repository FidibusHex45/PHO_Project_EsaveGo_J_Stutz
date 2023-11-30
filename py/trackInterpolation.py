import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from sklearn.neighbors import KDTree
from scipy.interpolate import splprep, splev
from sklearn import linear_model


def kdTreeSKlearn(points):
    tree = KDTree(points, leaf_size=2)
    return tree


points = np.loadtxt(
    "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/prop/dataPoints.csv",
    # "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/prop/dataPoints8.csv",
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
""" increment = 0.5

startDir = np.array([-np.sqrt(2), -np.sqrt(2)])
halfDir = np.array([-np.sqrt(2), +np.sqrt(2)])

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
        ) """


# Ransac Plausch
x_data = np.linspace(np.min(orderedPoints[:, 0]), np.max(orderedPoints[:, 0]), 101)


def ransac_fit_line(x, y, threshold=10, samples=2, trials=500):
    best_inliers = np.zeros(x.shape[0], dtype=bool)
    for i in range(trials):
        indices = np.random.choice(x.shape[0], samples, replace=False)
        fit = np.polyfit(x[indices], y[indices], 1)
        residuals = np.abs(x * fit[0] + fit[1] - y)
        inliers = residuals <= threshold
        if np.sum(inliers) > np.sum(best_inliers):
            best_inliers = inliers
    best_fit = np.polyfit(x[best_inliers], y[best_inliers], 1)
    return best_fit, best_inliers


def ransac_fit_line_alt(x, y, threshold=10, samples=10):
    best_inliers = np.zeros(x.shape[0], dtype=bool)
    for i in range(x.shape[0] - 1):
        indices = np.arange(i, i + samples) % (x.shape[0])
        fit = np.polyfit(x[indices], y[indices], 1)
        residuals = np.abs(x * fit[0] + fit[1] - y)
        inliers = residuals <= threshold
        if np.sum(inliers) > np.sum(best_inliers):
            best_inliers = inliers
    best_fit = np.polyfit(x[best_inliers], y[best_inliers], 1)
    return best_fit, best_inliers


def circlefit(x, y):
    A = np.block([[-2 * x], [-2 * y], [np.ones_like(x)]]).T
    if np.linalg.det(A.T @ A):
        x0, y0, b = np.linalg.solve(A.T @ A, A.T @ (-(x**2) - (y**2)))
        r = np.sqrt(x0**2 + y0**2 - b)
        return x0, y0, r
    else:
        return 0, 0, 0


def ransac_fit_circle(x, y, threshold=10, samples=15, maxRadius=1000):
    best_inliers = np.zeros(x.shape[0], dtype=bool)
    for i in range(x.shape[0] - 1):
        indices = np.arange(i, i + samples) % (x.shape[0])
        x0, y0, r = circlefit(x[indices], y[indices])
        residuals = np.abs(
            np.linalg.norm(np.block([[x], [y]]).T - np.block([x0, y0]), axis=1) - r
        )
        inliers = residuals <= threshold
        if r < maxRadius:
            if np.sum(inliers) > np.sum(best_inliers):
                best_inliers = inliers
    x0, y0, r = circlefit(x[best_inliers], y[best_inliers])
    return [x0, y0, r], best_inliers


splinePoints_copy = orderedPoints.copy()
inlier_linePoints_list = []
line_list = []
inlier_mask = np.zeros_like(splinePoints_copy[:, 0], dtype=bool)

while True:
    fit, inlier_mask = ransac_fit_line_alt(
        splinePoints_copy[:, 0],
        splinePoints_copy[:, 1],
        threshold=48,
    )
    if np.sum(inlier_mask) < 20:
        break
    inlier_linePoints_list.append(splinePoints_copy[inlier_mask])
    x_data = np.linspace(
        np.min(splinePoints_copy[inlier_mask][:, 0]),
        np.max(splinePoints_copy[inlier_mask][:, 0]),
        101,
    )
    y_data = fit[0] * x_data + fit[1]
    line_list.append(np.block([[x_data], [y_data]]).T)

    outlier_mask = np.logical_not(inlier_mask)
    splinePoints_copy = splinePoints_copy[outlier_mask]

inlier_CirclePoints_list = []
circle_list = []
theta = np.linspace(0, 2 * np.pi, 360)
inlier_mask = np.zeros_like(splinePoints_copy[:, 0], dtype=bool)

while True:
    fit, inlier_mask = ransac_fit_circle(
        splinePoints_copy[:, 0], splinePoints_copy[:, 1], threshold=15, samples=10
    )
    if np.sum(inlier_mask) < 5:
        break
    inlier_CirclePoints_list.append(splinePoints_copy[inlier_mask])

    x_data = fit[2] * np.cos(theta) + fit[0]
    y_data = fit[2] * np.sin(theta) + fit[1]

    circle_list.append(np.block([[x_data], [y_data]]).T)

    outlier_mask = np.logical_not(inlier_mask)
    splinePoints_copy = splinePoints_copy[outlier_mask]


fig = plt.figure(figsize=(7, 7))
ax = fig.add_subplot(111)
for circle in circle_list:
    ax.plot(
        circle[:, 0],
        circle[:, 1],
        "-",
    )
for inlier_CirclePoints in inlier_CirclePoints_list:
    ax.plot(
        inlier_CirclePoints[:, 0],
        inlier_CirclePoints[:, 1],
        "o",
    )
ax.set_title("ransac")
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.set_xlim((0, 950))
ax.set_ylim((0, 2000))
ax.invert_yaxis()
ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
ax.set_box_aspect(np.max(splinePoints[:, 1]) / np.max(splinePoints[:, 0]))
plt.tight_layout()

fig = plt.figure(figsize=(7, 7))
ax = fig.add_subplot(111)
for line in line_list:
    ax.plot(
        line[:, 0],
        line[:, 1],
        "-",
    )
for inlier_linePoint in inlier_linePoints_list:
    ax.plot(
        inlier_linePoint[:, 0],
        inlier_linePoint[:, 1],
        "o",
    )
ax.set_title("ransac")
ax.set_xlabel("x")
ax.set_ylabel("y")
ax.set_xlim((0, 950))
ax.set_ylim((0, 2000))
ax.invert_yaxis()
ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
ax.set_box_aspect(np.max(splinePoints[:, 1]) / np.max(splinePoints[:, 0]))
plt.tight_layout()
plt.show()


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
