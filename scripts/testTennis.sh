make | exit 1
echo ""

./ga_rules testTennis --p 40 --r 0.3 --m 0.005 --gens 50 --selection fitness-proportional --max-rules 3 --seed 42
