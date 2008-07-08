# This script runs the specified coloring algorithm over the complete
# set of instances, generating an output file with the results.
if [ $# -ne 1 ]; then echo 1>&2 Usage: $0 ALGORITHM_NAME; exit; fi
cd ../instances/
for file in *.col
do ../debug/coloring ${1} ${file}
done > ../reports/${1}_output.txt