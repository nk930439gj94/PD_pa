Environment:
    edaU1 CentOS 6.5
    compile by c++11
compile:
    type make to compile executable FP
run:
    make run to run 5 testcases in the folder input_pa3/ with alpha=0.5
    # .out file would be in input_pa3/
    or type ./FP [alpha] [input.block] [input.net] [outputfile] { -v | -visualize }
    If flag -v or -visualize is added. A floorplanning graph would show.
check:
    make check to check 5 testcases with alpha=0.5
    # executable file checker should be in the folder checker/
remove executable and intput_pa3/*.out file:
    make clean
