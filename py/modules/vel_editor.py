import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import TextBox, Button
from matplotlib.patches import FancyBboxPatch

"""
Description, ToDo
"""


class VelocityEditor:
    def __init__(self, data, color_scheme):
        """
        Constructor
        Initalize and split `data` to points and velocity.

        Parameters
        ----------
        data : Points and velocity data.
        color_scheme : cmap for matplotlib.
        """
        assert data.ndim == 2

        self.x_data = data[:, 0]
        self.y_data = data[:, 1]
        self.v_data = data[:, 2] / np.max(data[:, 2])

        self.cmap = plt.colormaps.get_cmap(color_scheme)
        self.color_original = self.cmap(self.v_data)
        self.color_adjust = np.copy(self.color_original)

        self.points = np.column_stack([self.x_data, self.y_data])
        self.v_adjust = np.copy(self.v_data)

        self.indices_picked_points = np.array([])

    def adjust(self):
        self.__initPlotBase()
        self.__initPlotWidgets()
        self.__initPlotTxt()
        self.__initPlotBgLayout()
        plt.show()
        return np.column_stack([self.x_data, self.y_data, self.v_adjust])

    def __initPlotBase(self):
        self.fig, self.base_ax = plt.subplots(figsize=(7, 7))
        self.fig.subplots_adjust(bottom=0.2)
        self.plotProp = self.base_ax.scatter(
            self.points[:, 0],
            self.points[:, 1],
            color=self.color_adjust,
            s=[50] * len(self.points),
            picker=3,
        )
        self.base_ax.set_title("Angle directions")
        self.base_ax.grid()
        self.base_ax.set_xlabel("x")
        self.base_ax.set_ylabel("y")
        self.base_ax.invert_yaxis()
        self.base_ax.tick_params(
            top=True, labeltop=True, bottom=False, labelbottom=False
        )
        self.base_ax.set_box_aspect(
            np.max(self.points[:, 1]) / np.max(self.points[:, 0])
        )
        l, b, w, h = self.base_ax.get_position().bounds
        self.base_ax.set_position([1, b, w, h])
        plt.tight_layout()

    def __initPlotWidgets(self):
        self.__initPlotPointsSelection()
        self.__initPlotSubmitBttn()
        self.__initPlotResetBttn()
        self.__initPlotOkBttn()
        self.__initPlotTxtbxVel()

    def __initPlotPointsSelection(self):
        self.cid = self.fig.canvas.mpl_connect("pick_event", self.__on_pick_points)

    def __initPlotSubmitBttn(self):
        self.bttn_submit_ax = self.fig.add_axes([0.815, 0.83, 0.1, 0.03])
        self.submit_button = Button(
            self.bttn_submit_ax,
            "Submit",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.submit_button.on_clicked(self.__on_click_bttn_submit)

    def __initPlotResetBttn(self):
        self.bttn_reset_ax = self.fig.add_axes([0.815, 0.79, 0.1, 0.03])
        self.reset_button = Button(
            self.bttn_reset_ax,
            "Reset",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.reset_button.on_clicked(self.__on_click_bttn_reset)

    def __initPlotOkBttn(self):
        self.bttn_ok_ax = self.fig.add_axes([0.815, 0.72, 0.1, 0.03])
        self.ok_button = Button(
            self.bttn_ok_ax,
            "Ok",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.ok_button.on_clicked(self.__on_click_bttn_ok)

    def __initPlotTxtbxVel(self):
        self.txtbx_vel_ax = self.fig.add_axes([0.67, 0.83, 0.12, 0.03])
        self.vel_txtbx = TextBox(
            self.txtbx_vel_ax,
            "",
            textalignment="right",
            color=(0.99, 0.99, 0.99, 1),
            hovercolor=(0.87, 1.0, 1.0, 0.8),
        )

    def __initPlotTxt(self):
        self.__plot_text_info()
        self._plot_text_adjust()
        self._plot_text_velocity()

    def __plot_text_info(self):
        text = "Velocity Adjustment" + "\n"
        text += "Select points by clicking on them." + "\n"
        text += "The selected points will change color." + "\n"
        text += "Write the new value in the input box." + "\n"
        text += "Confirm by pressing the submit button."
        props = dict(boxstyle="round", facecolor="grey", alpha=0.15)  # bbox features
        self.base_ax.text(
            1.15,
            0.15,
            text,
            transform=self.base_ax.transAxes,
            fontsize=10,
            verticalalignment="top",
            bbox=props,
        )

    def _plot_text_adjust(self):
        text = "Adjust"
        props = dict(boxstyle="round", alpha=0.0)  # bbox features
        self.base_ax.text(
            1.21,
            0.985,
            text,
            transform=self.base_ax.transAxes,
            fontsize=10,
            verticalalignment="top",
            bbox=props,
        )

    def _plot_text_velocity(self):
        text = "Velocity:"
        props = dict(boxstyle="round", alpha=0.0)  # bbox features
        self.base_ax.text(
            1.21,
            0.93,
            text,
            transform=self.base_ax.transAxes,
            fontsize=10,
            verticalalignment="top",
            bbox=props,
        )

    def __initPlotBgLayout(self):
        self.__plot_BgBox_1()
        self.__plot_BgBox_2()

    def __plot_BgBox_1(self):
        self.base_ax.add_patch(
            FancyBboxPatch(
                (1.175, 0.77),
                0.95,
                0.22,
                alpha=1,
                boxstyle="round, pad=0.01",
                clip_on=False,
                edgecolor=(0.81, 0.81, 0.81, 0.8),
                facecolor=(0.89, 0.89, 0.89, 0.8),
                fill=True,
                lw=1,
                transform=self.base_ax.transAxes,
            )
        )

    def __plot_BgBox_2(self):
        self.base_ax.add_patch(
            FancyBboxPatch(
                (1.2, 0.845),
                0.9,
                0.1,
                alpha=1,
                boxstyle="round, pad=0.01",
                clip_on=False,
                edgecolor=(0.81, 0.81, 0.81, 0.8),
                facecolor=(0.95, 0.95, 0.95, 0.8),
                fill=True,
                lw=1,
                transform=self.base_ax.transAxes,
            )
        )

    def __on_pick_points(self, event):
        self.plotProp._facecolors[event.ind, :] = (1, 0, 0, 1)
        self.plotProp._edgecolors[event.ind, :] = (1, 0, 0, 1)
        self.indices_picked_points = np.append(self.indices_picked_points, event.ind)
        self.fig.canvas.draw()

    def __on_click_bttn_submit(self, event):
        if not self.vel_txtbx.text.replace(".", "", 1).isdigit():
            return
        if not self.vel_txtbx.text:
            return
        vel = float(self.vel_txtbx.text)
        if vel < 0:
            return
        if vel > 1:
            return
        print(f"Velocity: {vel}")
        self.vel_txtbx.set_val("")
        self.__updateVelocity(self.indices_picked_points, vel)
        self.indices_picked_points = np.array([])

    def __on_click_bttn_reset(self, event):
        color_adjust = np.copy(self.color_original)
        self.plotProp._facecolors[:] = color_adjust
        self.plotProp._edgecolors[:] = color_adjust
        self.v_adjust = np.copy(self.v_data)
        self.fig.canvas.draw()

    def __on_click_bttn_ok(self, event):
        plt.close("all")
        # ToDo: return velocities

    def __updateVelocity(self, idx, vel):
        self.color_adjust[idx.astype(int)] = self.cmap(vel)
        self.plotProp._facecolors[:] = self.color_adjust
        self.plotProp._edgecolors[:] = self.color_adjust
        self.v_adjust[idx.astype(int)] = vel
        self.fig.canvas.draw()


if __name__ == "__main__":
    data = np.loadtxt(
        "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/splinedata.csv",
        delimiter=",",
        dtype=int,
    )
    print(data)
    vEditor = VelocityEditor(data, "jet")
    vEditor.adjust()
