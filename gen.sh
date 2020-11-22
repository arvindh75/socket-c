base64 /dev/urandom | head -c $(($1 * 1000000)) > ./server_dir/$2
