#ifndef SERIAL_H
#define SERIAL_H

#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>

/* Serial flags */
enum {
    SERIAL_FLAG_MASTER  = 1,  /* The node is a master */
    SERIAL_FLAG_VIRTUAL = 2,  /* The node is a virtual */
    SERIAL_FLAG_WRITER  = 4,  /* The node is a writer */
};

#define nodeIsMaster(n) ((n)->flags & SERIAL_FLAG_MASTER)
#define nodeIsVirtual(n) ((n)->flags & SERIAL_FLAG_VIRTUAL)
#define nodeIsWriter(n) ((n)->flags & SERIAL_FLAG_WRITER)

struct serialNode;

typedef struct serialLink {
    int fd;                          /* Serial file descriptor */
    int sfd;                         /* Slave serial file descriptor */
    char recvbuf[BUFSIZ];            /* Receive buffer */
    int recvbuflen;                  /* Number of bytes received */
    struct serialNode *node;         /* Node related to this link if any, or NULL */
} serialLink;

typedef struct serialNode {
    char name[PATH_MAX];             /* Path to device (/dev/ttyS1) */
    uint32_t flags;
    struct serialNode *virtual_head; /* Pointers to virtuals (if node is master) */
    struct serialNode *virtualof;    /* Pointer to master (if node is virtual) */
    int baudrate;                    /* Baudrate of device */
    serialLink *link;                /* rs232 link with this node */
    struct serialNode *next;         /* Pointer to next master in list (if any) */
} serialNode;

typedef struct serialState {
    struct serialNode *master_head;  /* Pointer to masters */
} serialState;

/**
 * @brief Allocate memory and initialize a new serial node.
 *
 * @param[in] nodename - Serial device path
 * @param[in] flags - Serial device responsibility and state
 *
 * @return Pointer to newly allocated serialNode object
 */
serialNode *serialCreateNode(const char *nodename, uint32_t flags);

/**
 * @brief Add serialNode to list.
 *
 * @param[in] node - serialNode to add
 */
void serialAddNode(serialNode *node);

/**
 * @brief Remove serialNode from list.
 *
 * @param[in] node - serialNode to remove
 */
void serialDelNode(serialNode *node);

/**
 * @brief Associate a serialNode as a virtual of a master serialNode.
 *
 * @param[in] master - serialNode master
 * @param[in] virtual - serialNode virtual to associate with master
 */
void serialAddVirtualNode(serialNode *master, serialNode *virtual);

/**
 * @brief Remove an associated virtual from a master serialNode.
 *
 * @param[in] master - serialNode master
 * @param[in] virtual - serialNode virtual to remove
 */
void serialRemoveVirtualNode(serialNode *master, serialNode *virtual);

/**
 * @brief Given an initialized serialNode, it will attempt to open
 *        and configure the connection. This function will also create file
 *        events for polling file descriptor events.
 *
 * @param[in] node - serialNode to configure for communication
 */
int serialConnectNode(serialNode *node);

/**
 * @brief Return the node by given node name.
 *
 * @param[in] nodename - Name of the node (device name)
 *
 * @return Pointer to node if found or NULL if not found
 */
serialNode *serialGetNode(const char *nodename);

/**
 * @brief Return the virtual node of master by given node name.
 *
 * @param[in] master - Master node
 * @param[in] nodename - Name of the virtual node (device name)
 *
 * @return Pointer to node if found or NULL if not found
 */
serialNode *serialGetVirtualNode(serialNode *master, const char *nodename);

/**
 * @brief Return the virtual writer in the masters virtual set.
 *
 * @param[in] master - Master node
 *
 * @return Pointer to the virtual writer node, or NULL if no writers found
 */
serialNode *serialGetVirtualWriterNode(serialNode *master);

/**
 * @brief Create a virtual name from a device path and suffix.
 *
 * @param[in] device - Device path (master), ie. /dev/ttyS3
 * @param[in] suffix - Virtual suffix name, ie. myapp
 * @param[out] name - Buffer to store virtual name
 * @param[in] name_size - Size of name buffer
 *
 * @return 0 if successful, -1 otherwise
 */
int serialVirtualName(const char *device, const char *suffix,
                      char *name, size_t name_size);

#endif
