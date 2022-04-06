//--------------------------test IPC JS -------------------------
// var ipc = require('node-ipc').default;

// var socketId = 'icp-test';
// ipc.config.id = 'hello';
// ipc.config.socketRoot = '/tmp/';
// ipc.config.appspace = '';

// ipc.config.retry = 1500;
// ipc.connectTo(
//     socketId,
//     function() {
//         ipc.of[socketId].on(
//             'connect',
//             function() {
//                 console.log("Connected!!");
//                 ipc.log('## connected to world ##'.rainbow, ipc.config.delay);
//                 ipc.of[socketId].emit(
//                     'message', //any event or message type your server listens for
//                     'hello'
//                 )
//             }
//         );
//         ipc.of[socketId].on(
//             'disconnect',
//             function() {
//                 console.log("Disconnected!!");
//                 ipc.log('disconnected from world'.notice);
//             }
//         );
//         ipc.of[socketId].on(
//             'message', //any event or message type your server listens for
//             function(data) {
//                 console.log("Got a message!!");
//                 ipc.log('got a message from world : '.debug, data);
//             }
//         );
//     }
// );

// https: //stackoverflow.com/questions/39841942/communicating-between-nodejs-and-c-using-node-ipc-and-unix-sockets
//--------------------------test IPC CPP -------------------------

// #include <stdio.h>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <sys/un.h>
// #include <stdlib.h>

// char *socket_path = "/tmp/icp-test";

// int main(int argc, char *argv[]) {
//   struct sockaddr_un addr;
//   char buf[100];
//   int fd,cl,rc;

//   if (argc > 1) socket_path=argv[1];

//   if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
//     perror("socket error");
//     exit(-1);
//   }

//   memset(&addr, 0, sizeof(addr));
//   addr.sun_family = AF_UNIX;
//   if (*socket_path == '\0') {
//     *addr.sun_path = '\0';
//     strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
//   } else {
//     strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
//     unlink(socket_path);
//   }

//   if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//     perror("bind error");
//     exit(-1);
//   }

//   if (listen(fd, 5) == -1) {
//     perror("listen error");
//     exit(-1);
//   }

//   while (1) {
//     if ( (cl = accept(fd, NULL, NULL)) == -1) {
//       perror("accept error");
//       continue;
//     }

//     while ( (rc=read(cl,buf,sizeof(buf))) > 0) {
//       printf("read %u bytes: %.*s\n", rc, rc, buf);
//     }
//     if (rc == -1) {
//       perror("read");
//       exit(-1);
//     }
//     else if (rc == 0) {
//       printf("EOF\n");
//       close(cl);
//     }
//   }
//   return 0;
// }