#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include <iostream>

int main()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr, my_addr; 

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("1.1.1.1");
    server_addr.sin_port = htons(0);

    connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    memset(&my_addr, 0, sizeof(my_addr));
    int len = sizeof(my_addr);
    getsockname(fd, (struct sockaddr *) &my_addr, (socklen_t*)&len);

    char myIP[16] = {0};
    inet_ntop(AF_INET, &my_addr.sin_addr, myIP, sizeof(myIP));

    std::cerr << "Local ip address: " << myIP << std::endl;
    return 0;
}
