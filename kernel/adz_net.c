/*
 * Adizilla OS - Network Stack
 * Basic networking functions
 * Built by Lila, Nairobi 🇰🇪
 */

#include <stdio.h>
#include <string.h>
#include "include/adz.h"

// Network interfaces
net_interface_t interfaces[MAX_INTERFACES];
int interface_count = 0;

// Socket table
socket_t sockets[MAX_SOCKETS];
int socket_count = 0;

// Initialize network stack
void net_init(void) {
    memset(interfaces, 0, sizeof(interfaces));
    memset(sockets, 0, sizeof(sockets));
    
    // Create loopback interface
    net_add_interface("lo", "127.0.0.1", "255.0.0.0");
    
    // Create ethernet interface (simulated)
    net_add_interface("eth0", "192.168.1.100", "255.255.255.0");
    
    printf("[NET] Network stack initialized\n");
}

// Add a network interface
int net_add_interface(const char *name, const char *ip, const char *netmask) {
    if (interface_count >= MAX_INTERFACES) {
        printf("[NET] ERROR: Maximum interfaces reached\n");
        return -1;
    }
    
    net_interface_t *iface = &interfaces[interface_count++];
    strncpy(iface->name, name, 15);
    strncpy(iface->ip, ip, 15);
    strncpy(iface->netmask, netmask, 15);
    iface->mtu = 1500;
    iface->flags = IFF_UP | IFF_RUNNING;
    iface->rx_bytes = 0;
    iface->tx_bytes = 0;
    
    printf("[NET] Added interface: %s (%s)\n", name, ip);
    
    return 0;
}

// Get interface by name
net_interface_t *net_find_interface(const char *name) {
    for (int i = 0; i < interface_count; i++) {
        if (strcmp(interfaces[i].name, name) == 0) {
            return &interfaces[i];
        }
    }
    return NULL;
}

// Create a socket
int net_socket(int domain, int type, int protocol) {
    if (socket_count >= MAX_SOCKETS) {
        printf("[NET] ERROR: Maximum sockets reached\n");
        return -1;
    }
    
    socket_t *sock = &sockets[socket_count++];
    sock->fd = socket_count - 1;
    sock->domain = domain;
    sock->type = type;
    sock->protocol = protocol;
    sock->state = SOCKET_CREATED;
    sock->local_port = 0;
    sock->remote_port = 0;
    
    return sock->fd;
}

// Bind socket to port
int net_bind(int fd, const char *ip, unsigned short port) {
    socket_t *sock = &sockets[fd];
    if (!sock || sock->state != SOCKET_CREATED) {
        return -1;
    }
    
    strncpy(sock->local_ip, ip, 15);
    sock->local_port = port;
    sock->state = SOCKET_BOUND;
    
    return 0;
}

// Connect to remote host
int net_connect(int fd, const char *ip, unsigned short port) {
    socket_t *sock = &sockets[fd];
    if (!sock || sock->state != SOCKET_BOUND) {
        return -1;
    }
    
    strncpy(sock->remote_ip, ip, 15);
    sock->remote_port = port;
    sock->state = SOCKET_CONNECTED;
    
    return 0;
}

// Send data
int net_send(int fd, const void *data, size_t len) {
    socket_t *sock = &sockets[fd];
    if (!sock || sock->state != SOCKET_CONNECTED) {
        return -1;
    }
    
    // Simulate sending
    net_interface_t *iface = net_find_interface("eth0");
    if (iface) {
        iface->tx_bytes += len;
    }
    
    return len;
}

// Receive data
int net_recv(int fd, void *buffer, size_t len) {
    socket_t *sock = &sockets[fd];
    if (!sock || sock->state != SOCKET_CONNECTED) {
        return -1;
    }
    
    // Simulate receiving
    return 0;
}

// Close socket
void net_close(int fd) {
    if (fd >= 0 && fd < socket_count) {
        sockets[fd].state = SOCKET_CLOSED;
    }
}

// Get interface list
void net_ifaces(net_interface_t *list, int *count) {
    *count = interface_count;
    for (int i = 0; i < interface_count; i++) {
        list[i] = interfaces[i];
    }
}

// Ping (ICMP echo request)
int net_ping(const char *host) {
    printf("[NET] Pinging %s...\n", host);
    
    // Simulate ping
    return 0;  // Success
}

// Network statistics
void net_stats(net_stats_t *stats) {
    if (!stats) return;
    
    stats->rx_total = 0;
    stats->tx_total = 0;
    stats->errors = 0;
    
    for (int i = 0; i < interface_count; i++) {
        stats->rx_total += interfaces[i].rx_bytes;
        stats->tx_total += interfaces[i].tx_bytes;
    }
}
