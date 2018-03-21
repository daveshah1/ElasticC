import tester, sys

res = tester.run_test(input_file="adder.ecc.vhd", uut_name="adder",
        inputs=[("a", 8), ("b", 8)], outputs=[("q", 8)], is_clocked=False,
        input_vectors=  [[1, 1], [12, 15],   [255, 1] ],
        output_results= [[2],    [27],       [0]])
sys.exit(res)
