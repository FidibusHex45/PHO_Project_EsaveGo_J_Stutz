import numpy as np
import shapely
from sklearn.decomposition import PCA

"""
Applies sequential PCA on a list of 2d coordinate points
"""


class SequentialPCA:
    def __init__(self, points):
        """
        Constructor
        Initalize kd Tree with points.

        Parameters
        ----------
        points : coordinate points.
        """
        self.points = points
        self.pcaS = np.zeros((self.points.shape[0], self.points.shape[1]))
        self.pcaS_diff = np.array([])
        self.pcaS_diff_avg = []

    def seqPCA(self, nPCA, nAvg):
        """
        Perform sequential pca on a set of 2d points.

        Parameters
        ----------
        nPCA : sequenc size for pca.
        nAvg : sequence size for avaraging pca.
        """
        self.nPCA = nPCA
        self.nAvg = nAvg
        self.__pcaSlide()
        self.__pcaDiff()
        self.__avarage()
        self.pcaS_diff_avg_norm = np.array(self.pcaS_diff_avg) / np.max(
            np.array(self.pcaS_diff_avg)
        )
        return np.column_stack([self.points, self.pcaS_diff_avg_norm])

    def __pcaSlide(self):
        self.nPCA += self.nPCA % 2
        for i in range(self.points.shape[0]):
            indices = np.arange(i - int(self.nPCA / 2), i + int(self.nPCA / 2)) % (
                self.points.shape[0]
            )
            comp = self.__pca(self.points[indices])
            self.pcaS[i] = np.abs(comp)

    def __dist2Points(self, p1, p2):
        return np.linalg.norm(p1 - p2)

    def __pcaSlideLen(self):
        for i in range(self.points.shape[0]):
            length = 600
            currentLen = 0
            indices = np.array([i], dtype=int)
            p = 0
            while currentLen < length:
                idx = np.array([i - (p + 1), i + (p + 1)]) % (self.points.shape[0])
                indices = np.append(indices, idx)
                currentLen += self.__dist2Points(
                    self.points[idx[0] - 1], self.points[idx[0]]
                ) + self.__dist2Points(self.points[idx[1] - 1], self.points[1])
                p += 1
            comp = self.__pca(self.points[indices])
            self.pcaS[i] = np.abs(comp)

    def __pca(self, points, nComp=1):
        pca = PCA(n_components=nComp)
        pca.fit(points)
        return pca.components_

    def __pcaDiff(self):
        for i in range(self.pcaS.shape[0]):
            indices = np.array([i, i + 1]) % (self.pcaS.shape[0])
            diff = np.linalg.norm(self.pcaS[indices[0]] - self.pcaS[indices[1]])
            self.pcaS_diff = np.append(self.pcaS_diff, diff)

    def __avarage(self):
        self.nAvg += self.nAvg % 2
        for i in range(self.pcaS_diff.shape[0]):
            indices = np.arange(i - self.nAvg / 2, i + self.nAvg / 2, dtype=int) % (
                self.pcaS_diff.shape[0]
            )
            self.pcaS_diff_avg.append(np.mean(self.pcaS_diff[indices]))


import matplotlib.pyplot as plt

if __name__ == "__main__":
    data = np.loadtxt(
        "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/splineData.csv",
        delimiter=",",
        dtype=float,
    )

    coords = data[:, :2]
    print(coords)

    poly = shapely.LineString(coords)

    coords_new = poly.interpolate(np.linspace(0, 1, num=201), normalized=True)

    x_new = [pt.x for pt in coords_new]
    y_new = [pt.y for pt in coords_new]

    plt.plot(x_new, y_new)
    plt.scatter(x_new, y_new)
    plt.show()
    """ points = np.column_stack([data[:, 0], data[:, 1]])
    sequentialPCA = SequentialPCA(points)
    pointsPCA = sequentialPCA.seqPCA(8, 10)
 """
