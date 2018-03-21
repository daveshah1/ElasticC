import tester, sys

res = tester.run_test(input_file="rgb_greyscale.ecc", uut_name="rgb_greyscale",
        inputs=[("input", 24)], outputs=[("output", 24)], is_clocked=False,
        input_vectors=  [[0xFF0000], [0x040404], [0x800080], [0x808000], [0xFFFFFF]],
        output_results= [[0x3F3F3F], [0x040404], [0x404040], [0x606060], [0xFDFDFD]])
sys.exit(res)
