import numpy as np
from sklearn.neighbors import KDTree

"""
Sorts a list of coordinate points
"""


class PointSorter:
    def __init__(self, points):
        """
        Constructor
        Initalize kd Tree with points.

        Parameters
        ----------
        points : points and velocity data.
        """
        self.points = points
        self.tree = KDTree(self.points, leaf_size=2)

    def sortPoints(self, refPoint):
        """
        sorts the point, beginning with the point nearest to the `refpoint`

        Parameters
        ----------
        refpoint : starting point.
        """
        _, indx = self.tree.query(refPoint, k=2)
        pIndx = indx[0, 0]
        sequence = np.array([], dtype=int)
        k = self.points.shape[0]

        for i in range(self.points.shape[0]):
            _, indx = self.tree.query(np.array([self.points[pIndx, :]]), k=k)
            for j in range(k):
                if not np.sum(np.isin(sequence, indx[:, j][0])):
                    pIndx = indx[:, j][0]
                    sequence = np.append(sequence, pIndx)
                    break
        return self.points[sequence[: sequence.shape[0] - 2]]


if __name__ == "__main__":
    data = np.loadtxt(
        "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/dataPoints.csv",
        delimiter=",",
        dtype=int,
    )
    refPoint = np.array([[564, 1026]])
    pSorter = PointSorter(data)
    pSorter.sortPoints(refPoint)
