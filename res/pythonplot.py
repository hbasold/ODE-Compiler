import pandas as pd
import matplotlib.pyplot as plt

# Load data from CSV file
data = pd.read_csv("simple.csv")

# Plot x, y, and z variables over time
plt.plot(data["time"], data["x"], label="")

plt.xlabel("x")
plt.ylabel("y")
plt.title("Lorenz System")

plt.show()
