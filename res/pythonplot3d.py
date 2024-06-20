import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Load data from CSV file
data = pd.read_csv("outp_.csv")

# Create a 3D plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Assuming the CSV has columns 'x', 'y', and 'z'
ax.plot(data["x"], data["y"], data["z"], label="3D Line")

ax.set_xlabel("x")
ax.set_ylabel("y")
ax.set_zlabel("z")
ax.set_title("3D Plot")

plt.legend()
plt.show()