import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

# Load data from CSV file
data = pd.read_csv("outp_.csv")

# Extract the columns for time and each variable
time = data["t"].to_numpy()
a1 = data["a1"].to_numpy()
a2 = data["a2"].to_numpy()
a3 = data["a3"].to_numpy()
a4 = data["a4"].to_numpy()
a5 = data["a5"].to_numpy()

# Create a 2D grid for time and position
time_grid = np.repeat(time[:, np.newaxis], 5, axis=1)
position_grid = np.tile(np.arange(1, 6), (len(time), 1))

# Create the 3D plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Plot each variable as a separate line
ax.plot(time, np.ones_like(time), a1, label="a1")
ax.plot(time, 2*np.ones_like(time), a2, label="a2")
ax.plot(time, 3*np.ones_like(time), a3, label="a3")
ax.plot(time, 4*np.ones_like(time), a4, label="a4")
ax.plot(time, 5*np.ones_like(time), a5, label="a5")

# Optionally, plot a surface
# Combine all variables into a single 2D array
values = np.array([a1, a2, a3, a4, a5]).T

# Plot the surface
ax.plot_surface(time_grid, position_grid, values, cmap="viridis", alpha=0.7)

ax.set_xlabel("Time")
ax.set_ylabel("Position")
ax.set_zlabel("Value")
ax.set_title("1D Heat Equation Approximation")

plt.legend()
plt.show()