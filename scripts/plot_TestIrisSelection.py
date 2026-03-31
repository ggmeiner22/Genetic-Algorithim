import matplotlib.pyplot as plt

generations = [25, 50, 75, 100, 150, 200, 250, 300]

fitness_train = [0.97, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98]
fitness_test  = [0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98]

tournament_train = [0.97, 0.97, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98]
tournament_test  = [0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98]

rank_train = [0.97, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98, 0.98]
rank_test  = [0.98, 0.98, 0.98, 0.96, 0.96, 0.96, 0.96, 0.96]

# ---- TRAINING PLOT ----
plt.figure()
plt.plot(generations, fitness_train, color='orange', marker='o', label='Fitness-Proportional', linewidth=4.5, markersize=10)
plt.plot(generations, tournament_train, color='blue', marker='s', label='Tournament', linewidth=2.25, markersize=7)
plt.plot(generations, rank_train, color='red', marker='^', label='Rank', linewidth=1, markersize=6)

plt.xlabel('Generations')
plt.ylabel('Training Accuracy')
plt.title('Training Accuracy vs Generations (Iris)')
plt.ylim(0.95, 1.01)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("iris_training_plot.png", dpi=300)

# ---- TEST PLOT ----
plt.figure()
plt.plot(generations, fitness_test, color='orange', marker='o', label='Fitness-Proportional', linewidth=4.5, markersize=10)
plt.plot(generations, tournament_test, color='blue', marker='s', label='Tournament', linewidth=2.25, markersize=7)
plt.plot(generations, rank_test, color='red', marker='^', label='Rank', linewidth=1, markersize=6)

plt.xlabel('Generations')
plt.ylabel('Test Accuracy')
plt.title('Test Accuracy vs Generations (Iris)')
plt.ylim(0.95, 1.01)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("iris_test_plot.png", dpi=300)

print("Saved: iris_training_plot.png and iris_test_plot.png")