import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import splprep, splev



def spl_curvature(t, tck):
    """ Returns signed curvature
    For curves, the canonical example is that of a circle, which
    has a curvature equal to the reciprocal of its radius.
    Smaller circles bend more sharply, and hence have higher
    curvature. The curvature at a point of a differentiable
    curve is the curvature of its osculating circle, that is the
    circle that best approximates the curve near this point. The
    curvature of a straight line is zero. In contrast to the
    tangent, which is a vector quantity, the curvature at a
    point is typically a scalar quantity, that is, it is
    expressed by a single real number. (Wikipedia)

    :param t: Sample points, shape (n, )
    :param tck: Knots, coefficients, and degree of the spline
    :return: Curvature for sample points, shape (n, )
    """
    xdot, ydot = splev(t, tck, der=1)
    xdot = np.asarray(xdot)
    ydot = np.asarray(ydot)
    x2dot, y2dot = splev(t, tck, der=2)
    x2dot = np.asarray(x2dot)
    y2dot = np.asarray(y2dot)
    return (xdot * y2dot - ydot * x2dot) / np.sqrt((xdot**2 + ydot**2)**3)



if __name__ == '__main__':
    N = 1001
    amplitudes = np.linspace(200.0, 60.0, N)
    omega = np.linspace(0.05, 0.15, N)
    points = np.zeros((N, 2))
    points[:, 0] = 50.0 * np.linspace(0, 0.5 * np.pi, N)
    points[:, 1] = amplitudes * np.sin(omega*points[:, 0])

    # s=0, no smoothing; careful if choosing other values than 0
    tck, u = splprep([points[:, 0], points[:, 1]], k=3, s=0)

    # Spline interpolation
    if False:
        # t is uniformly spaced on the interval 0 to 1
        t = np.linspace(0, 1, N)
    else:
        # t is spaced proportional to edge lengths
        # see https://scicomp.stackexchange.com/questions/26825/parametrized-spline-oscilating-second-derivative
        t = np.zeros(N)
        for i in range(1, N):
            t[i] = np.linalg.norm(points[i, :] - points[i-1, :])
        t = np.cumsum(t) / np.sum(t)
    x, y = splev(t, tck)

    # Check how well spline fits sample points
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.plot(points[:, 0], points[:, 1], 'ob', label='sample points')
    ax.plot(x, y, '-r', label='spline interpolation')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.grid()
    ax.legend()

    # Calculate and plot curvature
    curvature = np.abs(spl_curvature(t, tck))
    # Calculate max speed
    mass_kg = 1500
    F_max_N = 3500
    v_max_car = 30
    v_max = np.zeros(curvature.size)
    mask_valid = ~np.isclose(curvature, 0.0)
    v_max[~mask_valid] = v_max_car
    v_max[mask_valid] = np.sqrt(F_max_N / (mass_kg * curvature[mask_valid]))
    v_max = np.clip(v_max, 0.0, v_max_car)

    fig = plt.figure()
    ax = fig.add_subplot(311)
    ax.plot(x, y, '-r')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.grid()
    ax = fig.add_subplot(312)
    ax.plot(x, curvature, '-g')
    ax.set_xlabel('x')
    ax.set_ylabel('curvature')
    ax.grid()
    ax = fig.add_subplot(313)
    ax.plot(x, v_max, '-g')
    ax.set_xlabel('x')
    ax.set_ylabel('v_max')
    ax.grid()

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.scatter(x, y, c=v_max, cmap='viridis_r')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.grid()

    plt.show()