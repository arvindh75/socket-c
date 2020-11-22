#Sockets in C
##Instructions
- Un-tar the downloaded folder 
- Switch to the directory using
```C
cd 2019111010
```
- Run make

```
make
```

-  Clients files are present inside ``client_dir`` and Server files are present under ``server_dir``
- Server can be turned on by running 
```
./server_dir/server
```
- Client can be turned on by running 
```
./client_dir/client
```
- Client can get data using
```
get <filename1> <filename2>
```
- Client can get exit using
```
exit
```
- Multiple clients can obtain data from a single server but one by one.
- The code flow is pretty much straightforward.

### File with random ASCII characters can be created using gen.sh (size in mb and name are arguments).
```
bash gen.sh 2 2mb.txt
```
generates a file of size 2 mb with name `2mb.txt` in ./server_dir
