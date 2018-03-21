import tester, sys

res = tester.run_test(input_file="template.ecc", uut_name="template_test",
        inputs=[("a", 24),("b", 24)], outputs=[("q", 16)], is_clocked=False,
        input_vectors=[[0x010203, 0x040506], [0x01FF00, 0x010100], [0x7F7F7F, 0x020304]],
        output_results= [[32], [0], [1143]])
sys.exit(res)
