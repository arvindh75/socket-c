all:
	gcc -o ./client_dir/client ./client_dir/client.c
	gcc -o ./server_dir/server ./server_dir/server.c
clean:
	-rm -f ./server_dir/server ./client_dir/client
