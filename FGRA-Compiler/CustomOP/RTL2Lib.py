import os
import subprocess
import sys

def run_yosys(RTL_directory_path, dest_directory_path):
    """
        Run Yosys to transform each custom OP's RTL to RTLIL
    Args:
        RTL_directory_path (str): the RTL directory path
        dest_directory_path (str): the generated RTLIL directory path
    """
    if not os.path.exists(RTL_directory_path):
        print(f"Error: directory '{RTL_directory_path}' doesn't exist")
        return False
    
    # find all the RTL files
    verilog_files = [f for f in os.listdir(RTL_directory_path) if f.endswith('.v')]
    
    if not verilog_files:
        print(f"There is no RTL file in '{RTL_directory_path}' !")
        return False
    verilog_names = [os.path.splitext(f)[0] for f in verilog_files]    
    print(f"There are {len(verilog_files)} RTL files, including: {verilog_files}")
    print(f"The Custom OPs: {verilog_names}")

    #check if Yosys is available
    try:
        subprocess.run(['yosys', '-V'], check=True, capture_output=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Error: Please install Yosys first~")
        return False
    print("Yosys is available, begin to transform!")
    #for each OP
    for i, verilog_file in enumerate(verilog_files):
        file_path = os.path.join(RTL_directory_path, verilog_file)
        file_name = verilog_names[i].upper()
        print(f"Transforming {file_name}!")
        yosys_command = f"tcl ./Syn.tcl ~i {file_path} ~o {dest_directory_path}/{file_name}.rtlil ~v true"
        
        try:
            # run Yosys
            # subprocess.run(
            #     ['yosys', '-p', yosys_command],
            #     check=True
            # )
            subprocess.run(
                ['yosys', '-p', yosys_command],
                check=True, capture_output=True
            )
        except subprocess.CalledProcessError as e:
            print(f"Error!")


    

if __name__ == "__main__":
    # 设置要处理的目录
    RTL_directory = "./OpRTL"
    RTLIL_directory = "./Lib"
    
    # 运行处理函数
    run_yosys(RTL_directory, RTLIL_directory)
    print("\nTransforming Done!")