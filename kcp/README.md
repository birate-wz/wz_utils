单独编译：
client：gcc -o client client.c ikcp.c delay.c  -lpthread
server: gcc -o server server.c ikcp.c 

cmake编译：
mkdir build
cd build
cmake ..
make
编译后的文件在build目录 


// 特别需要注意，这里的服务器端也只能一次使用，即是等客户端退出后，服务端也要停止掉再启动
// 之所以是这样，主要是因为sn的问题，比如客户端第一次启动 sn 0~5， 第二次启动发送的sn还是0 ~5 如果服务器端不停止则自己以为0~5已经收到过了就不会回复。

// 在真正使用的时候，还需要另外的通道让客户端和服务器端之前重新创建ikcpcb，以匹配ikcpcb的conv
