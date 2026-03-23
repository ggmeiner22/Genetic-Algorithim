make | exit 1
echo ""

./ga_rules testIris --p 150 --r 0.6 --m 0.03 --gens 300 --selection tournament --max-rules 6 --seed 42 --bins 3
