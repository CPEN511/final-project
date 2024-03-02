#!/bin/bash
config_file="champsim_config.json"

if [ ! -f "$config_file" ]; then
  echo "Error: Configuration file champsim_config.json not found."
  exit 1
fi

make
trace_files=($(ls traces/))

if [ ${#trace_files[@]} -eq 0 ]; then
  echo "Error: No trace files found in the traces directory."
  exit 1
fi

echo "Available Traces:"
for i in "${!trace_files[@]}"; do 
  echo "$((i + 1)) - ${trace_files[$i]}"
done

read -p "Enter the number of the trace file you want to use: " trace_choice
if [[ ! $trace_choice =~ ^[0-9]+$ ]] || (( trace_choice <= 0 || trace_choice > ${#trace_files[@]} )); then
  echo "Error: Invalid selection."
  exit 1
fi

trace_choice=$((trace_choice - 1))
trace_filename="${trace_files[$trace_choice]}"
folder_name="${trace_filename%.trace.xz}"
folder_name="${folder_name%.trace}"
mkdir -p "results/$folder_name"
bin/champsim --warmup_instructions 1000000 --simulation_instructions 4000000 "traces/$trace_filename" > "results/$folder_name/$folder_name.txt"
echo "Simulation complete!" 
