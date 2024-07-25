import pandas as pd
import matplotlib.pyplot as plt

# Load data from CSV file
data = pd.read_csv("lorenz.csv")

# Plot x, y, and z variables over time
plt.plot(data["y"], data["z"], label="")

plt.xlabel("y")
plt.ylabel("x")
plt.title("Lorenz System")

plt.show()
