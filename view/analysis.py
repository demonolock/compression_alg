import matplotlib.pyplot as plt
import pandas as pd

data = pd.read_csv('results.csv')

# Set up graph size
plt.figure(figsize=(10, 13))

# Graph for compare compression speed
plt.subplot(2, 1, 1)
colors = plt.cm.viridis_r(data['Time']/max(data['Time']))
plt.bar(data['Type'], data['Time'], color=colors)
plt.ylabel('Time')
plt.title('Compression Speed')

# Graph for compare compression ratio
plt.subplot(2, 1, 2)
colors = plt.cm.viridis_r(data['Ratio']/max(data['Ratio']))
plt.bar(data['Type'], data['Ratio'], color=colors)
plt.ylabel('Ratio')
plt.title('Compression Ratio')

# Size between columns
plt.tight_layout()

plt.show()
