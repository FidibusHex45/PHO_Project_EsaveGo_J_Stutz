import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from sklearn.neighbors import KDTree
from scipy.interpolate import splprep, splev
from sklearn.decomposition import PCA
from matplotlib.collections import LineCollection


def circlefit(x, y):
    A = np.block([[-2 * x], [-2 * y], [np.ones_like(x)]]).T
    if np.linalg.det(A.T @ A):
        x0, y0, b = np.linalg.solve(A.T @ A, A.T @ (-(x**2) - (y**2)))
        r = np.sqrt(x0**2 + y0**2 - b)
        return x0, y0, r
    else:
        return 0, 0, 0


def ransac_fit_circle(x, y, threshold=10, samples=15, maxRadius=500):
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


def findCircles(points, thres=36, sampl=10, minSize=6, maxR=320):
    inlier_CirclePoints_list = []
    circle_list = []
    theta = np.linspace(0, 2 * np.pi, 360)
    inlier_mask = np.zeros_like(points[:, 0], dtype=bool)

    while True:
        fit, inlier_mask = ransac_fit_circle(
            points[:, 0], points[:, 1], threshold=thres, samples=sampl, maxRadius=maxR
        )
        if np.sum(inlier_mask) < minSize:
            break
        inlier_CirclePoints_list.append(points[inlier_mask])

        x_data = fit[2] * np.cos(theta) + fit[0]
        y_data = fit[2] * np.sin(theta) + fit[1]

        circle_list.append(np.block([[x_data], [y_data]]).T)

        outlier_mask = np.logical_not(inlier_mask)
        points = points[outlier_mask]
    return circle_list, inlier_CirclePoints_list


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


def ransac_fit_line_alt(x, y, threshold=10, samples=5):
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


def lineDist(points, mask=None):
    length = np.linalg.norm(
        np.array(
            [
                np.min(points[mask, 0]),
                np.min(points[mask, 1]),
            ]
        )
        - np.array(
            [
                np.max(points[mask, 0]),
                np.max(points[mask, 1]),
            ]
        )
    )
    return length


def findLines(points, thres=36, sampl=10, minSize=6, minLength=320):
    inlier_list = []
    line_list = []
    inlier_mask = np.zeros_like(points[:, 0], dtype=bool)
    indices = np.arange(0, points.shape[0])
    faultyIndices = np.array([], dtype=int)
    outliers = points

    while True:
        fit, inlier_mask = ransac_fit_line(
            points[:, 0], points[:, 1], threshold=thres, samples=sampl, trials=2000
        )
        if np.sum(inlier_mask) < minSize:
            break

        length = lineDist(points, inlier_mask)
        if length > minLength:
            inlier_list.append(points[inlier_mask])
            x_data = np.linspace(
                np.min(points[inlier_mask][:, 0]),
                np.max(points[inlier_mask][:, 0]),
                101,
            )
            y_data = fit[0] * x_data + fit[1]
            line_list.append(np.block([[x_data], [y_data]]).T)
        else:
            faultyIndices = np.append(faultyIndices, indices[inlier_mask])

        outlier_mask = np.logical_not(inlier_mask)
        points = points[outlier_mask]
        indices = indices[outlier_mask]
    indices = np.append(indices, faultyIndices)
    return line_list, inlier_list, indices


def pca(points, nComp):
    pca = PCA(n_components=nComp)
    pca.fit(points)
    return pca.components_


def pcaSlide(points, samples):
    pcaArray = np.zeros((points.shape[0], points.shape[1]))
    samples += samples % 2
    for i in range(points.shape[0]):
        indices = np.arange(i - int(samples / 2), i + int(samples / 2)) % (
            points.shape[0]
        )
        comp = pca(points[indices], 1)
        pcaArray[i] = np.abs(comp)
    return pcaArray


def dirDiff(pcaArray):
    diffArray = np.array([])
    for i in range(pcaArray.shape[0]):
        indices = np.array([i, i + 1]) % (pcaArray.shape[0])
        diff = np.linalg.norm(pcaArray[indices[0]] - pcaArray[indices[1]])
        diffArray = np.append(diffArray, diff)
    return diffArray


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


def avarageArray(array, nAvg):
    avgAngles = []
    nAvg += nAvg % 2
    for i in range(array.shape[0]):
        indices = np.arange(i - nAvg / 2, i + nAvg / 2, dtype=int) % (array.shape[0])
        avgAngles.append(np.mean(array[indices]))
    return np.array(avgAngles)


def thresArray(array, thres):
    inliers = array <= thres
    outliers = np.logical_not(inliers)
    return inliers, outliers


def calcAngles(points):
    angles = np.array([])
    directions = np.array([])
    for i in range(points.shape[0]):
        indices = np.array([i - 1, i, i + 1]) % (points.shape[0])
        angle = cosLaw(
            dist2Points(points[indices[1]], points[indices[2]]),
            dist2Points(points[indices[0]], points[indices[1]]),
            dist2Points(points[indices[0]], points[indices[2]]),
        )
        directions = np.append(
            directions,
            direction(points[indices[0]], points[indices[1]], points[indices[2]]),
        )
        angles = np.append(angles, 180 - angle)

    directions = directions.reshape((int(directions.shape[0] / 2), 2))
    return angles, directions


def removeDuplicats(points):
    return np.unique(points, axis=0)


def kdTreeSKlearn(points):
    tree = KDTree(points, leaf_size=2)
    return tree


def orderPoints(points, refPoint):
    # Initialize kd Tree
    tree = kdTreeSKlearn(points)
    _, indx = tree.query(refPoint, k=2)
    # Initial values
    pIndx = indx[0, 0]
    sequence = np.array([], dtype=int)
    k = points.shape[0]
    # Evaluate correct sequence
    for i in range(points.shape[0]):
        _, indx = tree.query(np.array([points[pIndx, :]]), k=k)
        for j in range(k):
            if not np.sum(np.isin(sequence, indx[:, j][0])):
                pIndx = indx[:, j][0]
                sequence = np.append(sequence, pIndx)
                break
    return points[sequence[: sequence.shape[0] - 2]]


def plotPoints(points, linestyle="ob", beginEnd=False, title="Points"):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    ax.plot(points[:, 0], points[:, 1], linestyle)
    if beginEnd:
        ax.plot(points[0, 0], points[0, 1], "og")
        ax.plot(points[-1, 0], points[-1, 1], "or")
    ax.set_title(title)
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.set_xlim((0, 950))
    ax.set_ylim((0, 2000))
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(points[:, 1]) / np.max(points[:, 0]))
    plt.tight_layout()


def plotPointsList(pointList, title):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    for points in pointList:
        ax.plot(points[:, 0], points[:, 1], "o")
    ax.set_title(title)
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.set_xlim((0, 950))
    ax.set_ylim((0, 2000))
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(points[:, 1]) / np.max(points[:, 0]))
    plt.tight_layout()


def plotSamplePoints(points, spline):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    ax.plot(points[:, 0], points[:, 1], "ob", label="sample points")
    ax.plot(spline[:, 0], spline[:, 1], "-xr", label="spline interpolation")
    ax.plot(
        points[0, 0],
        points[0, 1],
        "og",
        label="first point",
    )
    ax.plot(
        points[points.shape[0] - 1, 0],
        points[points.shape[0] - 1, 1],
        "or",
        label="last point",
    )
    ax.plot(refPoint[0, 0], refPoint[0, 1], "oy", label="Reference point")
    ax.set_title("b-Spline fit")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(points[:, 1]) / np.max(points[:, 0]))
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))
    plt.tight_layout()


def plotLines(inliers, lines):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    for i, line in enumerate(lines):
        ax.plot(line[:, 0], line[:, 1], "-", label="found line idx:" + str(i + 1))
    for i, inlier in enumerate(inliers):
        ax.plot(inlier[:, 0], inlier[:, 1], "x", label="inliers" + str(i + 1))
    ax.set_title("Lines")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.set_xlim((0, 950))
    ax.set_ylim((0, 2000))
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(splinePoints[:, 1]) / np.max(splinePoints[:, 0]))
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))
    plt.tight_layout()


def plotCircles(inliers, circles):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    for i, circle in enumerate(circles):
        ax.plot(
            circle[:, 0], circle[:, 1], "-", label="found circle idx: " + str(i + 1)
        )
    for i, inlier in enumerate(inliers):
        ax.plot(inlier[:, 0], inlier[:, 1], "x", label="inliers idx: " + str(i + 1))
    ax.set_title("Lines")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.set_xlim((0, 950))
    ax.set_ylim((0, 2000))
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(splinePoints[:, 1]) / np.max(splinePoints[:, 0]))
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))
    plt.tight_layout()


def plotLinesNCircles(lines, circles):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    for i, circle in enumerate(circles):
        ax.plot(
            circle[:, 0], circle[:, 1], "or", label="Circle cluster idx: " + str(i + 1)
        )
    for i, line in enumerate(lines):
        ax.plot(line[:, 0], line[:, 1], "og", label="line cluster idx: " + str(i + 1))
    ax.set_title("Lines & circles")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.set_xlim((0, 950))
    ax.set_ylim((0, 2000))
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(splinePoints[:, 1]) / np.max(splinePoints[:, 0]))
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))
    plt.tight_layout()


def plotPCA(points, pcaArray):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    ax.plot(
        points[:, 0],
        points[:, 1],
        "xr",
        label="spline points",
    )
    for i in range(points.shape[0]):
        x1 = splinePoints[i, 0]
        x2 = 150 * pcaArray[i, 0] + points[i, 0]
        y1 = splinePoints[i, 1]
        y2 = 150 * pcaArray[i, 1] + points[i, 1]
        ax.annotate("", xytext=(x1, y1), xy=(x2, y2), arrowprops=dict(arrowstyle="->"))
    ax.set_title("Directions")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.set_xlim((0, 1000))
    ax.set_ylim((0, 2100))
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(orderedPoints[:, 1]) / np.max(orderedPoints[:, 0]))
    ax.legend(loc="center left", bbox_to_anchor=(1, 0.5))


def PlotcoloredLine(points, dirDiff, title):
    plotPoints = points.reshape(-1, 1, 2)
    segments = np.concatenate([plotPoints[:-1], plotPoints[1:]], axis=1)

    fig, ax = plt.subplots(1, 1, figsize=(4, 7))

    # Create a continuous norm to map from data points to colors
    norm = plt.Normalize(dirDiff.min(), dirDiff.max())
    lc = LineCollection(segments, cmap="jet", norm=norm)

    # Set the values used for colormapping
    lc.set_array(dirDiff)
    lc.set_linewidth(5)
    line = ax.add_collection(lc)
    fig.colorbar(line, ax=ax)
    ax.set_box_aspect(np.max(points[:, 1]) / np.max(points[:, 0]))
    ax.set_xlim(points[:, 0].min() - 100, points[:, 0].max() + 100)
    ax.set_ylim(points[:, 1].min() - 100, points[:, 1].max() + 100)
    ax.set_title(title)
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.grid()
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    plt.tight_layout()


def plotAngleDirections(points, directions):
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111)
    ax.plot(
        points[:, 0],
        points[:, 1],
        "or",
    )
    for i in range(points.shape[0]):
        x1 = points[i, 0]
        x2 = 100 * directions[i, 0] + points[i, 0]
        y1 = points[i, 1]
        y2 = 100 * directions[i, 1] + points[i, 1]
        # ax.arrow(x1, y1, xl, yl, width=1)
        ax.annotate("", xytext=(x1, y1), xy=(x2, y2), arrowprops=dict(arrowstyle="->"))
    ax.set_title("Angle directions")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(points[:, 1]) / np.max(points[:, 0]))


def plotDerSpline(spline, derivative):
    der = derivative[:, 0] / derivative[:, 1]
    fig, ax = plt.subplots(subplot_kw=dict(projection="3d"))
    ax.stem(spline[:, 0], spline[:, 1], der)
    # ax.plot(der[:, 0], der[:, 1], "-r", label="Derivative")
    ax.set_title("Spline der")
    ax.set_xlabel("x")
    ax.set_ylabel("y")
    """ ax.invert_yaxis()
    ax.tick_params(top=True, labeltop=True, bottom=False, labelbottom=False)
    ax.set_box_aspect(np.max(spline[:, 1]) / np.max(spline[:, 0])) """


def splineInterpol(points, numKnots=201, smoothing=0.001, periodic=0, derivative=0):
    # Norm points to 1
    max_x = np.max(points[:, 0])
    max_y = np.max(points[:, 1])
    normPoints = points / np.array([max_x, max_y])
    # Get Knots
    tck, t = splprep(
        [normPoints[:, 0], normPoints[:, 1]], k=3, s=smoothing, per=periodic
    )
    # (t,c,k) a tuple containing the vector of knots, the B-spline coefficients, and the degree of the spline
    # Get splinepoints
    t = np.linspace(0, 1, numKnots)
    splinePoints = splev(t, tck, der=derivative)
    splinePoints = np.array(splinePoints).T
    splinePoints[:, 0] = splinePoints[:, 0] * max_x
    splinePoints[:, 1] = splinePoints[:, 1] * max_y
    return splinePoints


if __name__ == "__main__":
    points = np.loadtxt(
        "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/dataPoints.csv",
        delimiter=",",
        dtype=int,
    )

    uniquePoints = removeDuplicats(points)

    refPoint = np.array([[564, 1026]])
    orderedPoints = orderPoints(uniquePoints, refPoint)

    splinePoints = splineInterpol(orderedPoints, smoothing=0.001, numKnots=201)
    splinePoints_der1 = splineInterpol(
        orderedPoints, smoothing=0.001, numKnots=201, derivative=1
    )
    splinePoints_der2 = splineInterpol(
        orderedPoints, smoothing=0.001, numKnots=201, derivative=2
    )

    # PCA
    pcaS = pcaSlide(splinePoints, 8)
    diffS = dirDiff(pcaS)
    avgdiffs = avarageArray(diffS, 10)
    pca_thres_inliers, pca_thres_outliers = thresArray(avgdiffs, 0.05)

    # Lines & circles
    lines, inliers_l, outliers_l = findLines(
        splinePoints, thres=30, sampl=2, minSize=10, minLength=370
    )

    circles, inliers_c = findCircles(
        splinePoints[outliers_l],
        thres=10,
        sampl=5,
        minSize=5,
        maxR=400,
    )

    # Angles
    angles, angleDirections = calcAngles(splinePoints)
    avgAngles = avarageArray(angles, 10)
    angles_thresh_inliers, angles_thresh_outliers = thresArray(avgAngles, 2.7)

    plotDerSpline(splinePoints, splinePoints_der1)
    plotDerSpline(splinePoints, splinePoints_der2)
    """ plotSamplePoints(orderedPoints, splinePoints)
    plotPoints(splinePoints)

    plotLines(inliers_l, lines)
    plotCircles(inliers_c, circles)
    plotLinesNCircles(inliers_l, inliers_c)

    plotPCA(splinePoints, pcaS)
    PlotcoloredLine(splinePoints, diffS, "Unavaraged pca")
    PlotcoloredLine(splinePoints, avgdiffs, "Avaraged pca")
    plotPointsList(
        [splinePoints[pca_thres_inliers], splinePoints[pca_thres_outliers]],
        "Thresholded avg pca",
    )

    PlotcoloredLine(splinePoints, angles, "Unavaraged angles")
    PlotcoloredLine(splinePoints, avgAngles, "Avaraged angles")
    plotPointsList(
        [splinePoints[angles_thresh_inliers], splinePoints[angles_thresh_outliers]],
        "Thresholded avg angles",
    )
    plotAngleDirections(splinePoints, angleDirections) """

    plt.show()

    # %% CSV Shit
# Combine 0,1 degree derivative in one array
x = splinePoints[:, 0].round().astype(int)
y = splinePoints[:, 1].round().astype(int)
d = np.zeros(splinePoints.shape[0])
dataSet = np.block(
    [
        [x],
        [y],
        [d],
    ]
)

# Convert to pandas dataframe
df = pd.DataFrame(dataSet.T)


# Save as .csv
df.to_csv(
    "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/splineData.csv",
    index=False,
    header=False,
)
