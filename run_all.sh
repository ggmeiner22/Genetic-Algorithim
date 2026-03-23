make | exit 1
echo ""

echo ""
echo "----------testTennis----------"
./ga_rules testTennis --p 80 --r 0.6 --m 0.05 --gens 200 --selection tournament --max-rules 6 --seed 42

echo ""
echo "----------testIris----------"
./ga_rules testIris --p 150 --r 0.6 --m 0.03 --gens 300 --selection tournament --max-rules 8 --seed 42 --bins 3

echo ""
echo "----------testIrisSelection----------"
./ga_rules testIrisSelection --p 150 --r 0.6 --m 0.03 --max-rules 8 --seed 42

echo ""
echo "----------testIrisReplacement----------"
./ga_rules testIrisReplacement --p 150 --m 0.03 --gens 300 --max-rules 8 --seed 42
