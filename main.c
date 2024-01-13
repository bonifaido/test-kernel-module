#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <linux/net.h>
#include <linux/in.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Me");
MODULE_DESCRIPTION("Simple kernel module for sending a HTTP request over TCP");

#define MAX_BUFFER_SIZE 4096

u32 create_address(u8 *ip)
{
    u32 addr = 0;
    int i;

    for (i = 0; i < 4; i++)
    {
        addr += ip[i];
        if (i == 3)
            break;
        addr <<= 8;
    }
    return addr;
}

// System call function to perform HTTP request
asmlinkage long sys_http_request(char __user destip[5], char __user *buffer, size_t size)
{
    struct socket *sock;
    struct sockaddr_in server_addr;
    int error, bytes_received;

    // Create a socket
    error = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (error < 0)
    {
        printk(KERN_ERR "Error creating socket\n");
        return error;
    }

    sock->ops->setsockopt(sock, SOL_TCP, TCP_ULP, KERNEL_SOCKPTR("nasp"), sizeof("nasp"));

    // Set server address
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(443); // HTTPS port
    server_addr.sin_addr.s_addr = htonl(create_address(destip));

    // Connect to the server
    error = sock->ops->connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr), 0);
    if (error < 0)
    {
        printk(KERN_ERR "Error connecting to server\n");
        sock_release(sock);
        return error;
    }

    struct msghdr msg = {0};
    msg.msg_flags = MSG_DONTWAIT;
    struct kvec vec = {0};
    int len, written = 0, left = strlen(buffer);

    vec.iov_len = left;
    vec.iov_base = (char *)buffer + written;

    // Send HTTP GET request
    error = kernel_sendmsg(sock, &msg, &vec, left, left);
    if (error < 0)
    {
        printk(KERN_ERR "Error sending HTTP request\n");
        sock_release(sock);
        return error;
    }

    // // Receive and print the response
    // error = kernel_recvmsg(sock, buffer, size, &bytes_received, 0);
    // if (error < 0) {
    //     printk(KERN_ERR "Error receiving HTTP response\n");
    //     sock_release(sock);
    //     return error;
    // }

    // printk(KERN_INFO "HTTP Response:\n%s\n", buffer);

    // Release the socket
    sock_release(sock);

    return bytes_received;
}

// Module initialization
static int __init init_syscall_module(void)
{
    printk(KERN_INFO "TCP Request Kernel Module Loaded\n");
    // unsigned char destip[5] = {127, 0, 0, 1, '\0'};
    unsigned char destip[5] = {104, 16, 123, 96, '\0'};
    sys_http_request(destip, "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n", MAX_BUFFER_SIZE);
    return 0;
}

// Module cleanup
static void __exit exit_syscall_module(void)
{
    printk(KERN_INFO "TCP Request Kernel Module Unloaded\n");
}

// Register module initialization and cleanup functions
module_init(init_syscall_module);
module_exit(exit_syscall_module);
