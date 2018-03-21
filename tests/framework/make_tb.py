"""
Load the testbench template and customise it for a given design
"""
import os, sys

dirname = os.path.dirname(__file__)
filename = os.path.join(dirname, 'tb_template.vhd')

def get_type(width):
    return "std_logic_vector({} downto 0)".format(width-1) if width > 1 else "std_logic"

def generate_vhdl(uut_name, inputs, outputs, is_clocked, outfile):
    """
    Build a testbench in VHDL. Inputs and outputs a list of (name, width) tuples
    """
    uut_signals = ""
    assignments = []
    if is_clocked:
        assignments.append(("clock", "clock"))
    
    for sig in inputs + outputs:
        name, width = sig
        sig_type = get_type(width)
        uut_signals += "signal {} : {};".format(name, sig_type)
        assignments.append((name, name))
    
    uut_map = ", \n".join(["{} => {}".format(a[0], a[1]) for a in assignments])
    
    input_vars = ""
    read_vars = ""
    assign_ip = ""
    first_inp = True
    for sig in inputs:
        name, width = sig
        sig_type = get_type(width)
        input_vars += "    variable v_{} : {};\n".format(name, sig_type)
        if not first_inp:
            read_vars += "    read(iline, space);\n"
        first_inp = False
        read_vars += "    read(iline, v_{});\n".format(name)
        assign_ip += "    {} <= v_{};\n".format(name, name)
   
    write_out = ""
    for sig in outputs:
       name, width = sig
       write_out += "    write(oline, {});\n".format(name)
       write_out += "    write(oline, string'(\" \"));\n"

    with open(filename, 'r') as template_file:
      testbench = template_file.read()
    testbench = testbench.replace("${UUT_NAME}", uut_name)
    testbench = testbench.replace("${UUT_SIGNALS}", uut_signals)
    testbench = testbench.replace("${UUT_MAP}", uut_map)
    testbench = testbench.replace("${INPUT_VARS}", input_vars)
    testbench = testbench.replace("${READ_VARS}", read_vars)
    testbench = testbench.replace("${ASSIGN_IP}", assign_ip)
    testbench = testbench.replace("${WRITE_OUT}", write_out)

    with open(outfile, 'w') as outf:
       outf.write(testbench)
