all:
	g++ main.cpp -o FP -O3 -std=c++11
clean:
	rm -f FP input_pa3/*.out
run:
	./FP 0.5 input_pa3/xerox.block input_pa3/xerox.nets input_pa3/xerox.out
	./FP 0.5 input_pa3/hp.block input_pa3/hp.nets input_pa3/hp.out
	./FP 0.5 input_pa3/apte.block input_pa3/apte.nets input_pa3/apte.out
	./FP 0.5 input_pa3/ami33.block input_pa3/ami33.nets input_pa3/ami33.out
	./FP 0.5 input_pa3/ami49.block input_pa3/ami49.nets input_pa3/ami49.out
check:
	./checker/checker input_pa3/xerox.block input_pa3/xerox.nets input_pa3/xerox.out 0.5
	./checker/checker input_pa3/hp.block input_pa3/hp.nets input_pa3/hp.out 0.5
	./checker/checker input_pa3/apte.block input_pa3/apte.nets input_pa3/apte.out 0.5
	./checker/checker input_pa3/ami33.block input_pa3/ami33.nets input_pa3/ami33.out 0.5
	./checker/checker input_pa3/ami49.block input_pa3/ami49.nets input_pa3/ami49.out 0.5