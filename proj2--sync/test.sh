for ((j=1;j<=32;j++))
do
for ((i=1;i<=10;i++))
do
./lock -t $j -i 10000 >> ./out.txt
done
done
