import matplotlib.pyplot as plt

replacement_rates = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]

fitness =   [0.98, 0.98, 0.96, 0.96, 0.96, 0.98, 0.96, 0.96, 0.98]
tournament = [0.96, 0.96, 0.96, 0.96, 0.96, 0.98, 0.98, 0.96, 0.96]
rank =       [0.96, 0.96, 0.96, 0.96, 0.96, 0.96, 0.96, 0.96, 0.98]

plt.figure()

plt.plot(replacement_rates, fitness, color='orange', marker='o',  linewidth=5, markersize=10, label='Fitness-Proportional')
plt.plot(replacement_rates, tournament, color='blue', marker='s', linewidth=2.5, markersize=7, label='Tournament')
plt.plot(replacement_rates, rank, color='red', marker='^',  linewidth=1, markersize=6, label='Rank')

plt.xlabel('Replacement Rate (r)')
plt.ylabel('Test Accuracy')
plt.title('Test Accuracy vs Replacement Rate (Iris)')
plt.ylim(0.95, 0.985)
plt.grid(True)
plt.legend()
plt.tight_layout()

plt.savefig("iris_replacement_plot.png", dpi=300)
print("Saved: iris_replacement_plot.png")