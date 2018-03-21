"""Main VHDL test framework entry point"""
import os, sys, subprocess

import make_tb

dirname = os.path.dirname(__file__)
eccexe = os.path.join(dirname, '../../bin/elasticc')


def run_test(input_file, uut_name, inputs, outputs, is_clocked, input_vectors, output_results):
    """
    Build input_file using ElasticC into VHDL
    Build a testbench in VHDL and run it using ghdl.
    Inputs and outputs are list of (name, width) tuples.
    input_vectors and output_results are both an array of integers
    An entry in output_results can also be None for a don't care
    Return 0 on success or 1 on failure
    """
    print(" -- Testing module {} --".format(uut_name))
    input_dir = os.path.dirname(input_file)
    input_bn = os.path.basename(input_file)
    tempdir = os.path.join(input_dir, "temp_run")
    try:
        os.makedirs(tempdir)
    except OSError:
        pass
    
    uutpath = os.path.join(tempdir, "uut.vhd")
    try:
        # Run ElasticC
        subprocess.run([eccexe, "-o", "uut.vhd", os.path.join("..", input_file)],
                       cwd=tempdir, check=True)
    except subprocess.CalledProcessError:
        print("Test failure: GHDL analysis exited with non-zero return code")
        return 1

    tbpath = os.path.join(tempdir, "testbench.vhd")
    make_tb.generate_vhdl(uut_name, inputs, outputs, is_clocked, tbpath)

    ipvpath = os.path.join(tempdir, "input.txt")
    with open(ipvpath, 'w') as f:
        for vector in input_vectors:
            text_vectors = " ".join([format(vector[i], "0" + str(inputs[i][1]) + "b") for i in range(len(vector))])
            f.write(text_vectors + '\n')
    try:
        # Analyse UUT
        subprocess.run(["ghdl", "-a", "uut.vhd"],
                       cwd=tempdir, check=True)
    except subprocess.CalledProcessError:
        print("Test failure: ElasticC exited with non-zero return code")
        return 1

    try:
        # Analyse TB
        subprocess.run(["ghdl", "-a", "--ieee=synopsys", "testbench.vhd"],
                       cwd=tempdir, check=True)
    except subprocess.CalledProcessError:
        print("Test failure: GHDL analysis exited with non-zero return code")
        return 1
    try:
        # Elaborate
        subprocess.run(["ghdl", "-e", "--ieee=synopsys", "ecc_default_tb"], cwd=tempdir, check=True)
    except subprocess.CalledProcessError:
        print("Test failure: GHDL elaboration exited with non-zero return code")
        return 1
    try:
        # Run
        subprocess.run(["ghdl", "-r", "--ieee=synopsys", "ecc_default_tb", "--ieee-asserts=disable-at-0"], cwd=tempdir, check=True)
    except subprocess.CalledProcessError:
        print("Test failure: GHDL run exited with non-zero return code")
        return 1
    
    ovpath = os.path.join(tempdir, "output.txt")
    with open(ovpath, 'r') as f:
        current_idx = 0
        for line in f:
            splitLine = line.split(" ")
            splitLine = [x.strip() for x in splitLine]
            for i in range(len(output_results[current_idx])):
                expt = output_results[current_idx][i]
                if expt is None:
                    continue
                try:
                    res = int(splitLine[i], 2)
                except ValueError:
                    print("Test failure for result set {}. Bad result '{}' for output {}.".format(current_idx, splitLine[i], outputs[i]))
                    return 1
                if res != expt:
                    print("Test failure for result set {}. Expected {} for output {}, but got {}.".format(current_idx, expt, outputs[i], res))
                    return 1
            current_idx+=1
    print(" -- All tests for module {} passed --".format(uut_name))
    return 0
