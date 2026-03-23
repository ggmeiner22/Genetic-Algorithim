#!/usr/bin/env bash
set -euo pipefail

make

mkdir -p results

BIN="./ga_rules"

DATASET="${1:-iris}"

if [[ "$DATASET" == "tennis" ]]; then
    MODE="testTennis"
    POPS=(40 60 80 100)
    REPS=(0.3 0.5 0.6 0.7)
    MUTS=(0.005 0.01 0.03 0.05)
    GENS=(50 100 150 200)
    SELS=("fitness-proportional" "tournament" "rank")
    MAXRULES=(2 3 4 5 6)
    SEEDS=(1 7 21 42 99)
elif [[ "$DATASET" == "iris" ]]; then
    MODE="testIris"
    POPS=(80 100 150 200)
    REPS=(0.3 0.5 0.6 0.7 0.9)
    MUTS=(0.01 0.03 0.05 0.08)
    GENS=(100 200 300)
    SELS=("fitness-proportional" "tournament" "rank")
    MAXRULES=(4 6 8 10)
    SEEDS=(1 7 21 42 99)
else
    echo "Usage: $0 [tennis|iris]"
    exit 1
fi

OUTCSV="results/grid_${DATASET}.csv"
BESTTXT="results/grid_${DATASET}_best.txt"

echo "dataset,population,replacement,mutation,generations,selection,max_rules,seed,train_acc,test_acc" > "$OUTCSV"

extract_accuracy() {
    local label="$1"
    awk -v lbl="$label" '
        index($0, lbl) > 0 {
            gsub(/^[[:space:]]+|[[:space:]]+$/, "", $0)
            split($0, a, ":")
            gsub(/^[[:space:]]+|[[:space:]]+$/, "", a[2])
            print a[2]
            exit
        }
    '
}

for p in "${POPS[@]}"; do
    for r in "${REPS[@]}"; do
        for m in "${MUTS[@]}"; do
            for g in "${GENS[@]}"; do
                for s in "${SELS[@]}"; do
                    for mr in "${MAXRULES[@]}"; do
                        for seed in "${SEEDS[@]}"; do
                            echo "Running: dataset=$DATASET p=$p r=$r m=$m gens=$g selection=$s max_rules=$mr seed=$seed"

                            output=$("$BIN" "$MODE" \
                                --p "$p" \
                                --r "$r" \
                                --m "$m" \
                                --gens "$g" \
                                --selection "$s" \
                                --max-rules "$mr" \
                                --seed "$seed")

                            train=$(echo "$output" | extract_accuracy "Training accuracy:")
                            test=$(echo "$output"  | extract_accuracy "Test accuracy:")

                            if [[ -z "${train:-}" || -z "${test:-}" ]]; then
                                echo "ERROR: Could not parse accuracies for this run:"
                                echo "$output"
                                exit 1
                            fi

                            echo "$DATASET,$p,$r,$m,$g,$s,$mr,$seed,$train,$test" >> "$OUTCSV"
                        done
                    done
                done
            done
        done
    done
done

DATASET_NAME="$DATASET" python3 - << 'PY'
import csv
import os
from collections import defaultdict

dataset = os.environ["DATASET_NAME"]
input_file = f"results/grid_{dataset}.csv"
best_file = f"results/grid_{dataset}_best.txt"

groups = defaultdict(lambda: {"train": [], "test": []})

with open(input_file, newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        key = (
            int(row["population"]),
            float(row["replacement"]),
            float(row["mutation"]),
            int(row["generations"]),
            row["selection"],
            int(row["max_rules"]),
        )
        groups[key]["train"].append(float(row["train_acc"]))
        groups[key]["test"].append(float(row["test_acc"]))

rows = []
for key, vals in groups.items():
    avg_train = sum(vals["train"]) / len(vals["train"])
    avg_test = sum(vals["test"]) / len(vals["test"])
    gap = avg_train - avg_test
    rows.append((avg_test, -gap, avg_train, gap, key))

rows.sort(reverse=True)

with open(best_file, "w") as f:
    f.write(f"Top parameter combinations for {dataset} by average test accuracy\n")
    f.write("(ties broken by smaller train-test gap)\n\n")

    for i, (avg_test, neg_gap, avg_train, gap, key) in enumerate(rows[:20], start=1):
        p, r, m, g, s, mr = key
        f.write(
            f"{i:2d}. avg_test={avg_test:.4f} avg_train={avg_train:.4f} gap={gap:.4f} "
            f"p={p} r={r:.3f} m={m:.3f} gens={g} selection={s} max_rules={mr}\n"
        )

print(f"Wrote summary to {best_file}")
print("\nTop 10:")
for i, (avg_test, neg_gap, avg_train, gap, key) in enumerate(rows[:10], start=1):
    p, r, m, g, s, mr = key
    print(
        f"{i:2d}. avg_test={avg_test:.4f} avg_train={avg_train:.4f} gap={gap:.4f} "
        f"p={p} r={r:.3f} m={m:.3f} gens={g} selection={s} max_rules={mr}"
    )
PY

echo
echo "Done."
echo "CSV results:   $OUTCSV"
echo "Best summary:  $BESTTXT"