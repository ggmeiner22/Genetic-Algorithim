make | exit 1
echo ""

./ga_rules testIrisSelection --p 150 --r 0.6 --m 0.03 --max-rules 8 --seed 42
