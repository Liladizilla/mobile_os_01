/*
 * Adizilla OS - Kernel Header
 * Main header file for all kernel components
 * Built by Lila, Nairobi 🇰🇪
 */

#ifndef ADZ_H
#define ADZ_H

#include <stdint.h>
#include <stddef.h>

/* ==================== CONSTANTS ==================== */
#define MAX_PROCESSES      256
#define MAX_VMAS           128
#define MAX_INTERFACES     8
#define MAX_SOCKETS        64
#define MAX_NAME_LEN       64
#define MAX_PATH_LEN       256

/* Process states */
#define PROC_DEAD          0
#define PROC_READY         1
#define PROC_RUNNING       2
#define PROC_SLEEPING      3
#define PROC_WAITING       4
#define PROC_ZOMBIE        5

/* Socket states */
#define SOCKET_CREATED     0
#define SOCKET_BOUND       1
#define SOCKET_CONNECTED   2
#define SOCKET_LISTENING   3
#define SOCKET_CLOSED      4

/* Network interface flags */
#define IFF_UP            0x1
#define IFF_RUNNING       0x2
#define IFF_LOOPBACK      0x4
#define IFF_PROMISC       0x8

/* ==================== DATA STRUCTURES ==================== */

// Kernel state
typedef struct {
    const char *version;
    int booted;
    uint64_t uptime;
    int cpu_count;
    uint64_t memory_total;
    uint64_t memory_free;
} kernel_state_t;

// Process structure
typedef struct process {
    int pid;
    int parent;
    char name[MAX_NAME_LEN];
    void *entry;
    int priority;
    int state;
    uint64_t cpu_time;
    uint64_t memory;
    int sleep_ticks;
} process_t;

// Process info
typedef struct {
    int count;
    int running;
    int sleeping;
    int zombie;
} process_info_t;

// Virtual memory area
typedef struct vma {
    void *start;
    void *end;
    uint64_t flags;
    struct vma *next;
} vma_t;

// Memory statistics
typedef struct {
    uint64_t total;
    uint64_t used;
    uint64_t free;
    uint64_t largest_block;
} mem_stats_t;

// Network interface
typedef struct {
    char name[16];
    char ip[16];
    char netmask[16];
    int mtu;
    int flags;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} net_interface_t;

// Socket
typedef struct {
    int fd;
    int domain;
    int type;
    int protocol;
    int state;
    char local_ip[16];
    unsigned short local_port;
    char remote_ip[16];
    unsigned short remote_port;
} socket_t;

// Network statistics
typedef struct {
    uint64_t rx_total;
    uint64_t tx_total;
    int errors;
} net_stats_t;

// Kernel info
typedef struct {
    const char *version;
    uint64_t uptime;
    int cpu_count;
    uint64_t memory_total;
    uint64_t memory_free;
} kernel_info_t;

/* ==================== FUNCTION DECLARATIONS ==================== */

/* Core functions */
void kernel_init(void);
void kernel_main(void);
void kernel_info(kernel_info_t *info);
void kernel_shutdown(void);
void kernel_panic(const char *message);

/* Memory management */
void mem_init(void);
void *mem_alloc_page(void);
void mem_free_page(void *addr);
unsigned int mem_get_free(void);
void *mem_vmalloc(size_t size);
void mem_vfree(void *addr, size_t size);
void *mem_copy(void *dest, const void *src, size_t n);
void *mem_set(void *s, int c, size_t n);
void mem_stats(mem_stats_t *stats);

/* Process management */
void proc_init(void);
process_t *proc_create(const char *name, void *entry, int priority);
process_t *proc_find(int pid);
void proc_terminate(int pid);
void proc_kill_all(void);
void scheduler_tick(void);
void proc_info(process_info_t *info);
void proc_sleep(int ticks);
void proc_wake(int pid);
process_t *proc_current(void);
int proc_list(process_t **list, int max);

/* Network */
void net_init(void);
int net_add_interface(const char *name, const char *ip, const char *netmask);
net_interface_t *net_find_interface(const char *name);
int net_socket(int domain, int type, int protocol);
int net_bind(int fd, const char *ip, unsigned short port);
int net_connect(int fd, const char *ip, unsigned short port);
int net_send(int fd, const void *data, size_t len);
int net_recv(int fd, void *buffer, size_t len);
void net_close(int fd);
void net_ifaces(net_interface_t *list, int *count);
int net_ping(const char *host);
void net_stats(net_stats_t *stats);

/* Interrupts */
void interrupt_init(void);
void interrupt_handle(void);

/* Virtual File System */
void vfs_init(void);
void vfs_sync(void);

#endif /* ADZ_H */
