"""
 * @Projectname:        PHO_PROJECT_ESAVEGO_J_STUTZ
 *
 * @Description:        Do carrera go track analysis.
 *                      
 * @IDE:                VSCode
 * @Language:           python
 * @Interpreter:        Python 3.9.18
 * @Platform            Windows 11 Pro
 *
 * @Creation date:      08.12.2023
 * @Creator:            Joshua Stutz             
"""

import numpy as np

from modules.point_sorter import PointSorter
from modules.sequential_PCA import SequentialPCA
from modules.vel_editor import VelocityEditor
from modules.spline_interpolation import SplineInterpolation
from modules.csv_handler import CSVHandler


class CarreraTrackAnalysisApp:
    def __init__(self, path_load, path_write, refPoint):
        self.path_write = path_write
        self.refPoint = refPoint
        self.path_load = path_load
        self.csvHandler = CSVHandler()

    def run(self):
        points = self.csvHandler.loadCSV_Points(self.path_load)
        uniquePoints = np.unique(points, axis=0)

        pointSorter = PointSorter(uniquePoints)
        sortedPoints = pointSorter.sortPoints(self.refPoint)

        splineInterpolation = SplineInterpolation(
            sortedPoints, numKnots=201, smoothing=0.001
        )
        splinePoints = splineInterpolation.interpolate()

        sequentialPCA = SequentialPCA(splinePoints)
        data = sequentialPCA.seqPCA(15, 10)

        velocityEditor = VelocityEditor(data, "jet")
        adjusted_Data = velocityEditor.adjust()

        self.csvHandler.saveData_2CSV(adjusted_Data, self.path_write)


if __name__ == "__main__":
    path_load = "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/dataPoints.csv"
    path_write = "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/splineData.csv"
    refPoint = np.array([[564, 1026]])

    app = CarreraTrackAnalysisApp(path_load, path_write, refPoint)
    app.run()
