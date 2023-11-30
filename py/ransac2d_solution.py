import numpy as np

np.random.seed(0)
from matplotlib import pyplot as plt
from sklearn import linear_model


def ransac_fit(x, y, threshold=10, samples=2, trials=1000):
    best_inliers = np.zeros(x.shape[0], dtype=bool)
    for i in range(trials):
        indices = np.random.choice(x.shape[0], samples, replace=False)
        fit = np.polyfit(x[indices, 0], y[indices, 0], 1)
        residuals = np.abs(x[:, 0] * fit[0] + fit[1] - y[:, 0])
        inliers = residuals <= threshold
        if np.sum(inliers) > np.sum(best_inliers):
            best_inliers = inliers
    best_fit = np.polyfit(x[best_inliers, 0], y[best_inliers, 0], 1)
    return best_fit, best_inliers


# Data generation: Linear model with some noise
n_inliers = 500
inliers_x = np.random.normal(0.0, 10.0, (n_inliers, 1))
inliers_y = 3.0 * inliers_x + 5.0
inliers_y += np.random.normal(0.0, 2, (n_inliers, 1))

# Data generation: Other linear model with some noise
n_outliers = 50
outliers_x = np.random.normal(24.0, 4.0, (n_outliers, 1))
outliers_y = 0.0 * outliers_x - 1.0
outliers_y += np.random.normal(0.0, 2, (n_outliers, 1))

# Join both
data_x = np.vstack((inliers_x, outliers_x))
data_y = np.vstack((inliers_y, outliers_y))

# And shuffle
indices = np.arange(n_inliers + n_outliers)
np.random.shuffle(indices)
data_x = data_x[indices]
data_y = data_y[indices]

print(data_x)


# Do linear regression
lr = linear_model.LinearRegression()
lr.fit(data_x, data_y)
lr_line_x = np.array([[data_x.min()], [data_x.max()]])
lr_line_y = lr.predict(lr_line_x)
print(f"Linear fit: y={lr.coef_.item():.2f}x{lr.intercept_.item():+.2f}")

# Do RANSAC regression
rs = linear_model.RANSACRegressor(max_trials=1000)
rs.fit(data_x, data_y)
rs_inlier_mask = rs.inlier_mask_
rs_outlier_mask = np.logical_not(rs_inlier_mask)
rs_line_x = lr_line_x
rs_line_y = rs.predict(rs_line_x)
print("RANSAC fit, sklearn implementation")
print(
    f"    fit: y={rs.estimator_.coef_.item():.2f}x{rs.estimator_.intercept_.item():+.2f}"
)
print(
    f"    inliers: {np.sum(~rs_outlier_mask)}, RANSAC outliers: {np.sum(rs_outlier_mask)}"
)

# Own RANSAC implementation
fit, inliers_mask = ransac_fit(data_x, data_y)
print("RANSAC fit, own implementation")
print(f"    fit: y={fit[0]:.2f}x{fit[1]:+.2f}")
print(f"    inliers: {np.sum(inliers_mask)}, RANSAC outliers: {np.sum(~inliers_mask)}")

# Plot results
fig = plt.figure()
ax = fig.add_subplot(111)
ax.plot(
    data_x[rs_inlier_mask],
    data_y[rs_inlier_mask],
    "o",
    color="green",
    label="RANSAC inliers",
)
ax.plot(
    data_x[rs_outlier_mask],
    data_y[rs_outlier_mask],
    "o",
    color="red",
    label="RANSAC outliers",
)
ax.plot(rs_line_x, rs_line_y, "-", color="yellowgreen", label="RANSAC fit")
ax.plot(lr_line_x, lr_line_y, "-", color="orange", label="Linear fit")
ax.grid()
ax.legend(loc="best", fancybox=True, framealpha=0.5)
plt.show()
