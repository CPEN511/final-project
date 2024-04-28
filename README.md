# final-project
Final Project repo for CPEN511 <br />
Gagan Kanisetty and Felix Zhao

## Downloading Traces
The traces can be downloaded off the [2nd Cache Replacement championship website](https://crc2.ece.tamu.edu/)

## Setting up the workspace
1. Once the traces are downloaded save them in any directory of your choice.
2. Create a traces/ directory in the root directory of the project
   * mkdir traces/
3. Symlink the downloaded traces from your downloaded directory
   * ln -s /home/[user_name]/[path_to_downloaded_traces]/* traces/
4. Create a results/ directory in the root of the project. This directory saves all the results from the benchmarks
   * mkdir results/ 
5. Workspace setup!

## Running Code
### On Linux (Ubuntu)
1. Run ./run_sim.sh and provide the benchmark file to run
   * ./run_sim.sh
2. The run_sim.sh makes all the C++ code and configures the simulator first before prompting to enter a benchmark to run
3. Run the benchmarks
4. The results of the benchmarks are saved in the results/ directory
