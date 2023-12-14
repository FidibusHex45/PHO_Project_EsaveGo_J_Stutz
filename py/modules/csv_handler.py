import numpy as np
import pandas as pd

"""
Description, ToDo
"""


class CSVHandler:
    def __init__(self):
        """
        Constructor
        """
        pass

    def loadCSV_Points(self, path):
        points = np.loadtxt(
            path,
            delimiter=",",
            dtype=int,
        )
        return points

    def saveData_2CSV(self, data, path):
        assert data.ndim == 2
        assert data[0].shape[0] == 3
        x_data = data[:, 0].round().astype(int)
        y_data = data[:, 1].round().astype(int)
        v_data = np.around(data[:, 2], 3)
        processed_data = np.array([x_data, y_data, v_data], dtype=object).T
        df = pd.DataFrame(processed_data)
        df.to_csv(
            path,
            index=False,
            header=False,
        )


if __name__ == "__main__":
    path_load = "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/dataPoints.csv"
    csvHandler = CSVHandler()
    a = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]]).T
    print(a)
