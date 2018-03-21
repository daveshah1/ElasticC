import tester, sys

res = tester.run_test(input_file="cond.ecc", uut_name="conditional",
        inputs=[("x", 8)], outputs=[("y", 8)], is_clocked=False,
        input_vectors=[[5], [127], [255]],
        output_results= [[5], [127], [1]])
sys.exit(res)
