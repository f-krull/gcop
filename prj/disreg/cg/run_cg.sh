#!/usr/bin/env bash

trap 'exit' ERR

declare -r scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"


declare -r input_path="$1"
declare -r output_path="$2"

docker build -t clusgram "$scriptdir"

cat "$input_path" \
  | sed "s/\xc3\|\xb6\|\xc2\|\xa0//g" \
  | docker run --rm -i clusgram bash -c "
cat > /tmp/input_mat
cat > /tmp/pscript << EOF
from clustergrammer import Network
net = Network()
net.load_file('/tmp/input_mat')
net.normalize(axis='row', norm_type='zscore', keep_orig=True)
net.cluster()
net.write_json_to_file('viz', '/tmp/mult_view.json')
EOF
python /tmp/pscript
cat /tmp/mult_view.json
" > "$output_path"
