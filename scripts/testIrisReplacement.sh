make | exit 1
echo ""

./ga_rules testIrisReplacement --p 150 --m 0.03 --gens 300 --max-rules 8 --seed 42
