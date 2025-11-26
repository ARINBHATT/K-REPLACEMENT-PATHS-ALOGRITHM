import subprocess
import os

# --- User configurable variables --- #
cpp_file_path = "/workspaces/K-REPLACEMENT-PATHS-ALOGRITHM/benchmark.cpp" # Full path to your C++ file
executable_name = "benchmark"                                  # Name for the compiled executable
num_runs = 5                                                   # Number of times to run the executable
log_file_name = "benchmark_run_log.txt"                        # Name of the log file
# ---------------------------------- #

def run_cpp_program_n_times_with_logging(cpp_file, exec_name, n, log_output_file):
    # Ensure the directory of the C++ file is the working directory for compilation
    cpp_dir = os.path.dirname(cpp_file)
    cpp_filename_only = os.path.basename(cpp_file)

    # Change to the C++ file's directory for compilation and execution
    original_cwd = os.getcwd()
    if cpp_dir and os.path.exists(cpp_dir):
        os.chdir(cpp_dir)
        print(f"Changed current directory to: {os.getcwd()}")
    else:
        print(f"Warning: C++ file directory '{cpp_dir}' not found or empty. Using current directory for compilation/execution.")

    print(f"Attempting to compile {cpp_filename_only}...")
    compile_command = [
        "g++", "-std=c++17", "-O2", "-o", exec_name,
        "benchmark.cpp",
        "ReplacementPaths.cpp",
        "NaiveReplacementPaths.cpp",
        "ShortDetour.cpp",
        "common/CommonGraph.cpp",
        "common/CommonAlgorithms.cpp"
    ]
    try:
        # Using shell=False (default) and a list of args is safer.
        # The command is run from cpp_dir, so relative paths are fine.
        subprocess.run(compile_command, check=True, text=True, capture_output=True)
        print(f"Compilation successful. Executable: ./{exec_name}")
    except subprocess.CalledProcessError as e:
        print(f"Compilation failed for {cpp_filename_only}:")
        print(f"STDOUT:\n{e.stdout}")
        print(f"STDERR:\n{e.stderr}")
        os.chdir(original_cwd) # Restore CWD before exiting
        return
    except FileNotFoundError:
        print("Error: g++ compiler not found. Make sure it's installed and in your PATH.")
        os.chdir(original_cwd) # Restore CWD before exiting
        return

    print(f"\nRunning ./{exec_name} {n} times and logging to {log_output_file}...")
    
    # Open the log file for writing (or appending)
    with open(os.path.join(original_cwd, log_output_file), "a") as log_f:
        for i in range(n):
            run_header = f"\n--- Start of Run {i+1}/{n} for {cpp_filename_only} ---\n"
            print(run_header, end='')
            log_f.write(run_header)

            try:
                run_command = [f"./{exec_name}"]
                # Execute the program, capturing its output
                result = subprocess.run(run_command, check=True, text=True, capture_output=True)
                
                stdout_content = result.stdout
                
                # Extract the benchmark summary table
                summary_start_index = stdout_content.find("BENCHMARK SUMMARY")
                summary_content = ""
                if summary_start_index != -1:
                    # Find the start of the table header after "BENCHMARK SUMMARY"
                    table_header_index = stdout_content.find("n ", summary_start_index)
                    if table_header_index != -1:
                        # Find the start of the '====' line before "BENCHMARK SUMMARY"
                        summary_block_start = stdout_content.rfind("===", 0, summary_start_index)
                        if summary_block_start != -1:
                            summary_content = stdout_content[summary_block_start:].strip()

                stderr_content = result.stderr.strip()

                if summary_content:
                    print(f"Benchmark Summary:\n{summary_content}")
                    log_f.write(f"{summary_content}\n\n")
                if stderr_content:
                    print(f"STDERR:\n{stderr_content}")
                    log_f.write(f"STDERR:\n{stderr_content}\n")

            except subprocess.CalledProcessError as e:
                error_msg = f"Execution failed for run {i+1}:\nSTDOUT:\n{e.stdout}\nSTDERR:\n{e.stderr}\n"
                print(error_msg)
                log_f.write(error_msg)
                break # Stop if an execution fails
            except FileNotFoundError:
                error_msg = f"Error: Executable '{exec_name}' not found after compilation in {os.getcwd()}.\n"
                print(error_msg)
                log_f.write(error_msg)
                break
            
            run_footer = f"--- End of Run {i+1}/{n} ---\n"
            print(run_footer, end='')
            log_f.write(run_footer)

    print(f"\nFinished running {cpp_filename_only} {num_runs} times. Log saved to {log_output_file}.")
    os.chdir(original_cwd) # Restore CWD

# Call the function with your specified file, executable name, run count, and log file
run_cpp_program_n_times_with_logging(cpp_file_path, executable_name, num_runs, log_file_name)
