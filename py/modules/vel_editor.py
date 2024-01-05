import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import TextBox, Button
from matplotlib.patches import FancyBboxPatch

"""
Edit the velocity of specific points and rotate them in one direction by userinput (GUI)
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
        self.v_data = np.max(data[:, 2]) - data[:, 2] / np.max(data[:, 2])

        self.cmap = plt.colormaps.get_cmap(color_scheme)
        self.color_original = self.cmap(self.v_data)
        self.color_adjust = np.copy(self.color_original)

        self.points = np.column_stack([self.x_data, self.y_data])
        self.v_adjust = np.copy(self.v_data)

        self.indices_picked_points = np.array([])
        self.rotB = 0

    def adjust(self):
        """
        Start GUI for editing.
        """
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
            color=self.color_original,
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
        self.__initPlotVelSubmitBttn()
        self.__initPlotVelResetBttn()
        self.__initPlotRotBSubmitBttn()
        self.__initPlotRotResetBttn()
        self.__initPlotOkBttn()
        self.__initPlotTxtbxVel()
        self.__initPlotTxtbxRotB()

    def __initPlotPointsSelection(self):
        self.cid = self.fig.canvas.mpl_connect("pick_event", self.__on_pick_points)

    def __initPlotVelSubmitBttn(self):
        self.bttn_Velsubmit_ax = self.fig.add_axes([0.815, 0.83, 0.1, 0.03])
        self.submit_Vel_button = Button(
            self.bttn_Velsubmit_ax,
            "Submit",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.submit_Vel_button.on_clicked(self.__on_click_bttn_vel_submit)

    def __initPlotRotBSubmitBttn(self):
        self.bttn_Rotsubmit_ax = self.fig.add_axes([0.815, 0.715, 0.1, 0.03])
        self.submit_rot_button = Button(
            self.bttn_Rotsubmit_ax,
            "Submit",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.submit_rot_button.on_clicked(self.__on_click_bttn_rotB_submit)

    def __initPlotVelResetBttn(self):
        self.bttn_Velreset_ax = self.fig.add_axes([0.815, 0.79, 0.1, 0.03])
        self.reset_vel_button = Button(
            self.bttn_Velreset_ax,
            "Reset",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.reset_vel_button.on_clicked(self.__on_click_bttn_vel_reset)

    def __initPlotRotResetBttn(self):
        self.bttn_Rotreset_ax = self.fig.add_axes([0.815, 0.675, 0.1, 0.03])
        self.reset_Rot_button = Button(
            self.bttn_Rotreset_ax,
            "Reset",
            color=(0.95, 0.95, 0.95, 0.8),
            hovercolor=(0.85, 0.85, 0.85, 0.8),
        )
        self.reset_Rot_button.on_clicked(self.__on_click_bttn_rot_reset)

    def __initPlotOkBttn(self):
        self.bttn_ok_ax = self.fig.add_axes([0.815, 0.59, 0.1, 0.03])
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

    def __initPlotTxtbxRotB(self):
        self.txtbx_rotB_ax = self.fig.add_axes([0.67, 0.715, 0.12, 0.03])
        self.rotB_txtbx = TextBox(
            self.txtbx_rotB_ax,
            "",
            textalignment="right",
            color=(0.99, 0.99, 0.99, 1),
            hovercolor=(0.87, 1.0, 1.0, 0.8),
        )

    def __initPlotTxt(self):
        self.__plot_text_info()
        self.__plot_text_adjust()
        self.__plot_text_velocity()
        self.__plot_text_Rotate()

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

    def __plot_text_adjust(self):
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

    def __plot_text_velocity(self):
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

    def __plot_text_Rotate(self):
        text = "Rotate:"
        props = dict(boxstyle="round", alpha=0.0)  # bbox features
        self.base_ax.text(
            1.21,
            0.8,
            text,
            transform=self.base_ax.transAxes,
            fontsize=10,
            verticalalignment="top",
            bbox=props,
        )

    def __initPlotBgLayout(self):
        self.__plot_BgBox_1()
        self.__plot_BgBox_2()
        self.__plot_BgBox_3()

    def __plot_BgBox_1(self):
        self.base_ax.add_patch(
            FancyBboxPatch(
                (1.175, 0.61),
                0.95,
                0.38,
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

    def __plot_BgBox_3(self):
        self.base_ax.add_patch(
            FancyBboxPatch(
                (1.2, 0.71),
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
        self.plotProp._facecolors[event.ind, :] = (1, 0, 1, 1)
        self.plotProp._edgecolors[event.ind, :] = (1, 0, 1, 1)
        self.indices_picked_points = np.append(self.indices_picked_points, event.ind)
        self.fig.canvas.draw()

    def __on_click_bttn_vel_submit(self, event):
        if not self.vel_txtbx.text.replace(".", "", 1).isdigit():
            return
        if not self.vel_txtbx.text:
            return
        vel = float(self.vel_txtbx.text)
        if vel < 0:
            return
        if vel > 1:
            return
        self.vel_txtbx.set_val("")
        self.__updateVelocity(self.indices_picked_points, vel)
        self.indices_picked_points = np.array([])

    def __on_click_bttn_rotB_submit(self, event):
        if not self.rotB_txtbx.text.replace("-", "", 1).isdigit():
            return
        if not self.rotB_txtbx.text:
            return
        self.rotB = self.rotB + int(self.rotB_txtbx.text)
        self.rotB_txtbx.set_val("")
        self.__updateRotationB()

    def __on_click_bttn_vel_reset(self, event):
        self.color_adjust = np.copy(self.color_original)
        self.plotProp._facecolors[:] = self.color_adjust
        self.plotProp._edgecolors[:] = self.color_adjust
        self.v_adjust = np.copy(self.v_data)
        self.fig.canvas.draw()

    def __on_click_bttn_rot_reset(self, event):
        self.rotB = -self.rotB
        self.__updateRotationB()
        self.rotB = 0

    def __on_click_bttn_ok(self, event):
        plt.close("all")

    def __updateVelocity(self, idx, vel):
        # ☻idxA = (idx.astype(int) + self.rotB) % self.v_adjust.shape[0]
        self.color_adjust[idx.astype(int)] = self.cmap(vel)
        self.plotProp._facecolors[:] = self.color_adjust
        self.plotProp._edgecolors[:] = self.color_adjust
        self.v_adjust[idx.astype(int)] = vel
        self.fig.canvas.draw()

    def __updateRotationB(self):
        localColors = np.zeros_like(self.plotProp._facecolors[:])
        localVelocity = np.zeros_like(self.v_adjust)
        for i in range(localColors.shape[0]):
            idx = (i - self.rotB) % localColors.shape[0]
            localColors[idx] = self.plotProp._facecolors[i]
            localVelocity[idx] = self.v_adjust[i]
        self.plotProp._facecolors[:] = localColors
        self.plotProp._edgecolors[:] = localColors
        self.v_adjust[:] = localVelocity
        self.fig.canvas.draw()


if __name__ == "__main__":
    data = np.loadtxt(
        "C:/Users/joshu/OneDrive/Dokumente/FhGr/Faecher/Labor/5. Semester/Software/PHO_Project_EsaveGo_J_Stutz/data/splinedata.csv",
        delimiter=",",
        dtype=float,
    )
    print(data)
    vEditor = VelocityEditor(data, "jet")
    vEditor.adjust()
