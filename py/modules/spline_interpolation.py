import numpy as np
import shapely
from scipy.interpolate import splprep, splev

"""
Perform a splineInterpolation on a set of 2d coordinate points
"""


class SplineInterpolation:
    def __init__(
        self, points, numKnots=201, smoothing=0.001, periodic=False, derivative=0
    ):
        """
        Constructor
        Perform spline interpolation on a set of 2d points.

        Parameters
        ----------
        points : data points.
        numKnots : amount of spline points returned.
        smoothing : spline smoothing condition.
        periodic : data points concidered periodic or not.
        derivative: derivative degree of callculated spline
        """
        self.points = points
        self.numKnots = numKnots
        self.smoothing = smoothing
        self.periodic = periodic
        self.derivative = derivative
        self.__splineInterpol()
        self.__basicInterpolation()

    def interpolate(self):
        """
        Perform interpolation
        """
        self.__splineInterpol()
        self.__basicInterpolation()
        return self.bInt_SplinePoints

    def __splineInterpol(self):
        # Norm points to 1
        max_x = np.max(self.points[:, 0])
        max_y = np.max(self.points[:, 1])
        normPoints = self.points / np.array([max_x, max_y])
        # Get Knots
        tck, t = splprep(
            [normPoints[:, 0], normPoints[:, 1]],
            k=3,
            s=self.smoothing,
            per=self.periodic,
        )
        # (t,c,k) a tuple containing the vector of knots, the B-spline coefficients,
        # and the degree of the spline.
        # Get splinepoints
        t = np.linspace(0, 1, self.numKnots)
        self.splinePoints = splev(t, tck, der=self.derivative)
        self.splinePoints = np.array(self.splinePoints).T
        self.splinePoints[:, 0] = self.splinePoints[:, 0] * max_x
        self.splinePoints[:, 1] = self.splinePoints[:, 1] * max_y

    def __basicInterpolation(self):
        poly = shapely.LineString(self.splinePoints)
        coords_new = poly.interpolate(
            np.linspace(0, 1, num=self.numKnots), normalized=True
        )
        x_new = [pt.x for pt in coords_new]
        y_new = [pt.y for pt in coords_new]
        self.bInt_SplinePoints = np.column_stack([x_new, y_new])
