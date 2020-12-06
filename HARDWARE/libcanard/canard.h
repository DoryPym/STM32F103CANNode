/*
 * Copyright (c) 2016-2017 UAVCAN Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Contributors: https://github.com/UAVCAN/libcanard/contributors
 *
 * Documentation: http://uavcan.org/Implementations/Libcanard
 */

#ifndef CANARD_H
#define CANARD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Libcanard version. API will be backwards compatible within the same major version.
/// Libcanard版本。 API将在同一主要版本中向后兼容。
#define CANARD_VERSION_MAJOR                        0
#define CANARD_VERSION_MINOR                        2

/// By default this macro resolves to the standard assert(). The user can redefine this if necessary.
/// 默认情况下，此宏解析为标准的assert（）。 用户可以根据需要重新定义。
#ifndef CANARD_ASSERT
# define CANARD_ASSERT(x)   assert(x)
#endif

#define CANARD_GLUE(a, b)           CANARD_GLUE_IMPL_(a, b)
#define CANARD_GLUE_IMPL_(a, b)     a##b

/// By default this macro expands to static_assert if supported by the language (C11, C++11, or newer).
/// The user can redefine this if necessary.
/// 默认情况下，如果语言（C11，C ++ 11或更高版本）支持，此宏将扩展为static_assert。用户可以根据需要重新定义该宏。
#ifndef CANARD_STATIC_ASSERT
# if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) ||\
     (defined(__cplusplus) && (__cplusplus >= 201103L))
#  define CANARD_STATIC_ASSERT(...) static_assert(__VA_ARGS__)
# else
#  define CANARD_STATIC_ASSERT(x, ...) typedef char CANARD_GLUE(_static_assertion_, __LINE__)[(x) ? 1 : -1]
# endif
#endif

/// Error code definitions; inverse of these values may be returned from API calls.
/// 错误代码定义； 这些值的倒数可以从API调用中返回。
#define CANARD_OK                                   0
// Value 1 is omitted intentionally, since -1 is often used in 3rd party code
// 故意省略值1，因为在第三方代码中经常使用-1
#define CANARD_ERROR_INVALID_ARGUMENT               2
#define CANARD_ERROR_OUT_OF_MEMORY                  3
#define CANARD_ERROR_NODE_ID_NOT_SET                4
#define CANARD_ERROR_INTERNAL                       9

/// The size of a memory block in bytes.
/// 内存块的大小（以字节为单位）。
#define CANARD_MEM_BLOCK_SIZE                       32U

/// This will be changed when the support for CAN FD is added
/// 当添加对CAN FD的支持时，将更改此设置
#define CANARD_CAN_FRAME_MAX_DATA_LEN               8U

/// Node ID values. Refer to the specification for more info.
/// 节点ID值。 有关更多信息，请参考规范。
#define CANARD_BROADCAST_NODE_ID                    0
#define CANARD_MIN_NODE_ID                          1
#define CANARD_MAX_NODE_ID                          127

/// Refer to the type CanardRxTransfer
/// 引用类型CanardRxTransfer
#define CANARD_MULTIFRAME_RX_PAYLOAD_HEAD_SIZE      (CANARD_MEM_BLOCK_SIZE - offsetof(CanardRxState, buffer_head))

/// Refer to the type CanardBufferBlock
/// 引用类型CanardBufferBlock
#define CANARD_BUFFER_BLOCK_DATA_SIZE               (CANARD_MEM_BLOCK_SIZE - offsetof(CanardBufferBlock, data))

/// Refer to canardCleanupStaleTransfers() for details.
/// 有关详细信息，请参考canardCleanupStaleTransfers（）。
#define CANARD_RECOMMENDED_STALE_TRANSFER_CLEANUP_INTERVAL_USEC     1000000U

/// Transfer priority definitions
/// 转移优先级定义
#define CANARD_TRANSFER_PRIORITY_HIGHEST            0
#define CANARD_TRANSFER_PRIORITY_HIGH               8
#define CANARD_TRANSFER_PRIORITY_MEDIUM             16
#define CANARD_TRANSFER_PRIORITY_LOW                24
#define CANARD_TRANSFER_PRIORITY_LOWEST             31

/// Related to CanardCANFrame
/// 关于CanardCANFrame
#define CANARD_CAN_EXT_ID_MASK                      0x1FFFFFFFU
#define CANARD_CAN_STD_ID_MASK                      0x000007FFU
#define CANARD_CAN_FRAME_EFF                        (1UL << 31U)         ///< Extended frame format扩展帧格式
#define CANARD_CAN_FRAME_RTR                        (1UL << 30U)         ///< Remote transmission (not used by UAVCAN)远程传输（UAVCAN不使用）
#define CANARD_CAN_FRAME_ERR                        (1UL << 29U)         ///< Error frame (not used by UAVCAN)错误帧（UAVCAN不使用）

#define CANARD_TRANSFER_PAYLOAD_LEN_BITS            10U
#define CANARD_MAX_TRANSFER_PAYLOAD_LEN             ((1U << CANARD_TRANSFER_PAYLOAD_LEN_BITS) - 1U)


/**
 * This data type holds a standard CAN 2.0B data frame with 29-bit ID.
 * 该数据类型保存具有29位ID的标准CAN 2.0B数据帧。
 */
typedef struct
{
    /**
     * Refer to the following definitions:请参考以下定义：
     *  - CANARD_CAN_FRAME_EFF
     *  - CANARD_CAN_FRAME_RTR
     *  - CANARD_CAN_FRAME_ERR
     */
    uint32_t id;
    uint8_t data[CANARD_CAN_FRAME_MAX_DATA_LEN];
    uint8_t data_len;
} CanardCANFrame;

/**
 * Transfer types are defined by the UAVCAN specification.
 * 传输类型由UAVCAN规范定义。
 */
typedef enum
{
    CanardTransferTypeResponse  = 0,
    CanardTransferTypeRequest   = 1,
    CanardTransferTypeBroadcast = 2
} CanardTransferType;

/**
 * Types of service transfers. These are not applicable to message transfers.
 * 服务转移的类型。 这些不适用于邮件传输。
 */
typedef enum
{
    CanardResponse,
    CanardRequest
} CanardRequestResponse;

/*
 * Forward declarations.
 * 转发声明。
 */
typedef struct CanardInstance CanardInstance;
typedef struct CanardRxTransfer CanardRxTransfer;
typedef struct CanardRxState CanardRxState;
typedef struct CanardTxQueueItem CanardTxQueueItem;

/**
 * The application must implement this function and supply a pointer to it to the library during initialization.
 * The library calls this function to determine whether the transfer should be received.
 *
 * If the application returns true, the value pointed to by 'out_data_type_signature' must be initialized with the
 * correct data type signature, otherwise transfer reception will fail with CRC mismatch error. Please refer to the
 * specification for more details about data type signatures. Signature for any data type can be obtained in many
 * ways; for example, using the command line tool distributed with Libcanard (see the repository).
 * 
 * 应用程序必须实现此功能，并在初始化期间提供指向库的指针。库调用此函数以确定是否应接收传输。
 * 如果应用程序返回true，则必须使用来初始化“ out_data_type_signature”所指向的值。
 * 正确的数据类型签名，否则传输接收将因CRC不匹配错误而失败。 请参考规范，以获取有关数据类型签名的更多详细信息。任何数据类型的签名都可以通过许多方式获得
 * 方法; 例如，使用与Libcanard一起分发的命令行工具（请参见存储库）。
 */
typedef bool (* CanardShouldAcceptTransfer)(const CanardInstance* ins,          ///< Library instance 库实例
                                            uint64_t* out_data_type_signature,  ///< Must be set by the application!必须由应用程序设置！
                                            uint16_t data_type_id,              ///< Refer to the specification参考规格
                                            CanardTransferType transfer_type,   ///< Refer to CanardTransferType请参考CanardTransferType
                                            uint8_t source_node_id);            ///< Source node ID or Broadcast (0)源节点ID或广播（0）

/**
 * This function will be invoked by the library every time a transfer is successfully received.
 * If the application needs to send another transfer from this callback, it is highly recommended
 * to call canardReleaseRxTransferPayload() first, so that the memory that was used for the block
 * buffer can be released and re-used by the TX queue.
 * 每次成功接收传输时，库都会调用此函数。如果应用程序需要通过此回调发送其他传输，则强烈建议
 * 首先调用canardReleaseRxTransferPayload（），以便用于该块的内存缓冲区可以被TX队列释放并重新使用。
 */
typedef void (* CanardOnTransferReception)(CanardInstance* ins,                 ///< Library instance
                                           CanardRxTransfer* transfer);         ///< Ptr to temporary transfer object，PTR到临时转移对象

/**
 * INTERNAL DEFINITION, DO NOT USE DIRECTLY.
 * A memory block used in the memory block allocator.
 * 内部定义，请勿直接使用。在内存块分配器中使用的内存块。
 */
typedef union CanardPoolAllocatorBlock_u
{
    char bytes[CANARD_MEM_BLOCK_SIZE];
    union CanardPoolAllocatorBlock_u* next;
} CanardPoolAllocatorBlock;

/**
 * This structure provides usage statistics of the memory pool allocator.
 * This data helps to evaluate whether the allocated memory is sufficient for the application.
 * 此结构提供内存池分配器的使用情况统计信息。此数据有助于评估分配的内存是否足以满足应用程序的需求。
 */
typedef struct
{
    uint16_t capacity_blocks;               ///< Pool capacity in number of blocks，池容量（以块数计）
    uint16_t current_usage_blocks;          ///< Number of blocks that are currently allocated by the library，库当前分配的块数
    uint16_t peak_usage_blocks;             ///< Maximum number of blocks used since initialization，自初始化以来使用的最大块数
} CanardPoolAllocatorStatistics;

/**
 * INTERNAL DEFINITION, DO NOT USE DIRECTLY.
 * 内部使用，请勿直接使用
 */
typedef struct
{
    CanardPoolAllocatorBlock* free_list;
    CanardPoolAllocatorStatistics statistics;
} CanardPoolAllocator;

/**
 * INTERNAL DEFINITION, DO NOT USE DIRECTLY.
 * Buffer block for received data.
 * 接收数据的缓冲区。
 */
typedef struct CanardBufferBlock
{
    struct CanardBufferBlock* next;
    uint8_t data[];
} CanardBufferBlock;

/**
 * INTERNAL DEFINITION, DO NOT USE DIRECTLY.
 */
struct CanardRxState
{
    struct CanardRxState* next;

    CanardBufferBlock* buffer_blocks;

    uint64_t timestamp_usec;

    const uint32_t dtid_tt_snid_dnid;

    // We're using plain 'unsigned' here, because C99 doesn't permit explicit field type specification
    // 我们在这里使用普通的“无符号”，因为C99不允许显式字段类型规范
    unsigned calculated_crc : 16;
    unsigned payload_len    : CANARD_TRANSFER_PAYLOAD_LEN_BITS;
    unsigned transfer_id    : 5;
    unsigned next_toggle    : 1;    // 16+10+5+1 = 32, aligned.

    uint16_t payload_crc;

    uint8_t buffer_head[];
};
CANARD_STATIC_ASSERT(offsetof(CanardRxState, buffer_head) <= 28, "Invalid memory layout");
CANARD_STATIC_ASSERT(CANARD_MULTIFRAME_RX_PAYLOAD_HEAD_SIZE >= 4, "Invalid memory layout");

/**
 * This is the core structure that keeps all of the states and allocated resources of the library instance.
 * The application should never access any of the fields directly! Instead, API functions should be used.
 * 这是保留库实例的所有状态和分配的资源的核心结构。应用程序永远不要直接访问任何字段！ 而是应使用API函数。
 */
struct CanardInstance
{
    uint8_t node_id;                                ///< Local node ID; may be zero if the node is anonymous,本地节点ID； 如果节点是匿名的，则可能为零

    CanardShouldAcceptTransfer should_accept;       ///< Function to decide whether the application wants this transfer，决定应用程序是否要进行此转移的功能
    CanardOnTransferReception on_reception;         ///< Function the library calls after RX transfer is complete，RX传输完成后函数调用库

    CanardPoolAllocator allocator;                  ///< Pool allocator，池分配器

    CanardRxState* rx_states;                       ///< RX transfer states，RX传输状态
    CanardTxQueueItem* tx_queue;                    ///< TX frames awaiting transmission，TX帧等待传输

    void* user_reference;                           ///< User pointer that can link this instance with other objects，可以将此实例与其他对象链接的用户指针
};

/**
 * This structure represents a received transfer for the application.
 * An instance of it is passed to the application via callback when the library receives a new transfer.
 * Pointers to the structure and all its fields are invalidated after the callback returns.
 * 此结构表示该应用程序的已接收传输。当库收到新的传输时，它的一个实例通过回调传递给应用程序。
 * 回调返回后，指向该结构及其所有字段的指针无效。
 */
struct CanardRxTransfer
{
    /**
     * Timestamp at which the first frame of this transfer was received.
     * 接收到该传输的第一帧的时间戳。
     */
    uint64_t timestamp_usec;

    /**
     * Payload is scattered across three storages:
     *  - Head points to CanardRxState.buffer_head (length of which is up to CANARD_PAYLOAD_HEAD_SIZE), or to the
     *    payload field (possibly with offset) of the last received CAN frame.
     *
     *  - Middle is located in the linked list of dynamic blocks (only for multi-frame transfers).
     *
     *  - Tail points to the payload field (possibly with offset) of the last received CAN frame
     *    (only for multi-frame transfers).
     *
     * The tail offset depends on how much data of the last frame was accommodated in the last allocated block.
     *
     * For single-frame transfers, middle and tail will be NULL, and the head will point at first byte
     * of the payload of the CAN frame.
     *
     * In simple cases it should be possible to get data directly from the head and/or tail pointers.
     * Otherwise it is advised to use canardDecodeScalar().
     * 
     * 有效负载分散在三个存储中：
     * -Head指向CanardRxState.buffer_head（其长度最多为CANARD_PAYLOAD_HEAD_SIZE），或指向最后接收到的CAN帧的有效负载字段（可能带有偏移）。
     * -中间位于动态块的链接列表中（仅用于多帧传输）。
     *
     * -尾部指向最后接收到的CAN帧的有效载荷字段（可能有偏移）（仅适用于多帧传输）。
     *
     * 尾部偏移量取决于最后分配的块中容纳了最后一帧的数据量。
     * 对于单帧传输，中间和尾部将为NULL，并且头将指向第一个字节CAN帧的有效负载。
     *
     * 在简单的情况下，应该可以直接从头和/或尾指针获取数据。否则建议使用canardDecodeScalar（）。
     * 
     */
    const uint8_t* payload_head;            ///< Always valid, i.e. not NULL.
                                            ///< For multi frame transfers, the maximum size is defined in the constant
                                            ///< CANARD_MULTIFRAME_RX_PAYLOAD_HEAD_SIZE.
                                            ///< For single-frame transfers, the size is defined in the
                                            ///< field payload_len.
    CanardBufferBlock* payload_middle;      ///< May be NULL if the buffer was not needed. Always NULL for single-frame
                                            ///< transfers.
    const uint8_t* payload_tail;            ///< Last bytes of multi-frame transfers. Always NULL for single-frame
                                            ///< transfers.
    uint16_t payload_len;                   ///< Effective length of the payload in bytes.

    /**
     * These fields identify the transfer for the application.
     */
    uint16_t data_type_id;                  ///< 0 to 255 for services, 0 to 65535 for messages
    uint8_t transfer_type;                  ///< See CanardTransferType
    uint8_t transfer_id;                    ///< 0 to 31
    uint8_t priority;                       ///< 0 to 31
    uint8_t source_node_id;                 ///< 1 to 127, or 0 if the source is anonymous
};

/**
 * Initializes a library instance.
 * Local node ID will be set to zero, i.e. the node will be anonymous.
 *
 * Typically, size of the memory pool should not be less than 1K, although it depends on the application. The
 * recommended way to detect the required pool size is to measure the peak pool usage after a stress-test. Refer to
 * the function canardGetPoolAllocatorStatistics().
 * 初始化库实例。本地节点ID将设置为零，即该节点将是匿名的。
 * 通常，内存池的大小不应少于1K，尽管它取决于应用程序的检测所需池大小的推荐方法是在压力测试后测量峰值池使用量。参考函数canardGetPoolAllocatorStatistics（）。
 */
void canardInit(CanardInstance* out_ins,                    ///< Uninitialized library instance，未初始化的库实例
                void* mem_arena,                            ///< Raw memory chunk used for dynamic allocation，用于动态分配的原始内存块
                size_t mem_arena_size,                      ///< Size of the above, in bytes，上面的大小，以字节为单位
                CanardOnTransferReception on_reception,     ///< Callback, see CanardOnTransferReception，回调，请参阅CanardOnTransferReception
                CanardShouldAcceptTransfer should_accept,   ///< Callback, see CanardShouldAcceptTransfer
                void* user_reference);                      ///< Optional pointer for user's convenience, can be NULL，为方便用户使用的可选指针，可以为NULL

/**
 * Returns the value of the user pointer.
 * The user pointer is configured once during initialization.
 * It can be used to store references to any user-specific data, or to link the instance object with C++ objects.
 * 返回用户指针的值。用户指针在初始化期间配置一次。可用于存储对任何用户特定数据的引用，或将实例对象与C ++对象链接。
 */
void* canardGetUserReference(CanardInstance* ins);

/**
 * Assigns a new node ID value to the current node.
 * Node ID can be assigned only once.
 * 将新的节点ID值分配给当前节点。节点ID只能分配一次。
 */
void canardSetLocalNodeID(CanardInstance* ins,
                          uint8_t self_node_id);

/**
 * Returns node ID of the local node.
 * Returns zero (broadcast) if the node ID is not set, i.e. if the local node is anonymous.
 * 返回本地节点的节点ID。如果未设置节点ID，即本地节点为匿名，则返回零（广播）。
 */
uint8_t canardGetLocalNodeID(const CanardInstance* ins);

/**
 * Sends a broadcast transfer.
 * If the node is in passive mode, only single frame transfers will be allowed (they will be transmitted as anonymous).
 *
 * For anonymous transfers, maximum data type ID is limited to 3 (see specification for details).
 *
 * Please refer to the specification for more details about data type signatures. Signature for any data type can be
 * obtained in many ways; for example, using the command line tool distributed with Libcanard (see the repository).
 *
 * Pointer to the Transfer ID should point to a persistent variable (e.g. static or heap allocated, not on the stack);
 * it will be updated by the library after every transmission. The Transfer ID value cannot be shared between
 * transfers that have different descriptors! More on this in the transport layer specification.
 *
 * Returns the number of frames enqueued, or negative error code.
 * 
 * 发送广播传输。
 * 如果节点处于被动模式，则仅允许单帧传输（它们将以匿名方式传输）。
 *
 * 对于匿名传输，最大数据类型ID限制为3（有关详细信息，请参阅规范）。
 *
 * 有关数据类型签名的更多详细信息，请参考规范。 任何数据类型的签名都可以是以多种方式获得； 例如，使用与Libcanard一起分发的命令行工具（请参见存储库）。
 *
 * 指向传输ID的指针应指向一个持久变量（例如，静态变量或分配的堆，不在堆栈上）；它将在每次传输后由库进行更新。 传输ID值之间不能共享具有不同描述符的传输！ 
 * 有关传输层规范的更多信息。 返回排队的帧数或负错误代码。
 */
int16_t canardBroadcast(CanardInstance* ins,            ///< Library instance
                        uint64_t data_type_signature,   ///< See above
                        uint16_t data_type_id,          ///< Refer to the specification
                        uint8_t* inout_transfer_id,     ///< Pointer to a persistent variable containing the transfer ID
                        uint8_t priority,               ///< Refer to definitions CANARD_TRANSFER_PRIORITY_*
                        const void* payload,            ///< Transfer payload
                        uint16_t payload_len);          ///< Length of the above, in bytes

/**
 * Sends a request or a response transfer.
 * Fails if the node is in passive mode.
 *
 * Please refer to the specification for more details about data type signatures. Signature for any data type can be
 * obtained in many ways; for example, using the command line tool distributed with Libcanard (see the repository).
 *
 * For Request transfers, the pointer to the Transfer ID should point to a persistent variable (e.g. static or heap
 * allocated, not on the stack); it will be updated by the library after every request. The Transfer ID value
 * cannot be shared between requests that have different descriptors! More on this in the transport layer
 * specification.
 *
 * For Response transfers, the pointer to the Transfer ID will be treated as const (i.e. read-only), and normally it
 * should point to the transfer_id field of the structure CanardRxTransfer.
 *
 * Returns the number of frames enqueued, or negative error code.
 * 
 * *发送请求或响应传输。
  *如果节点处于被动模式，则失败。
  *
  *有关数据类型签名的更多详细信息，请参考规范。 任何数据类型的签名都可以是以多种方式获得； 例如，使用与Libcanard一起分发的命令行工具（请参见存储库）。
  *
  *对于请求传输，指向传输ID的指针应指向一个持久变量（例如静态变量或堆已分配，不在堆栈上）；每次请求后，库都会对其进行更新。传输ID值不能在具有不同描述符的请求之间共享！ 有关传输层的更多信息
  *规格。
  *
  *对于响应传输，指向传输ID的指针将被视为const（即只读），通常情况下应指向结构CanardRxTransfer的transfer_id字段。
  *
  *返回排队的帧数或负错误代码。
 * 
 */
int16_t canardRequestOrRespond(CanardInstance* ins,             ///< Library instance
                               uint8_t destination_node_id,     ///< Node ID of the server/client
                               uint64_t data_type_signature,    ///< See above
                               uint8_t data_type_id,            ///< Refer to the specification
                               uint8_t* inout_transfer_id,      ///< Pointer to a persistent variable with transfer ID
                               uint8_t priority,                ///< Refer to definitions CANARD_TRANSFER_PRIORITY_*
                               CanardRequestResponse kind,      ///< Refer to CanardRequestResponse
                               const void* payload,             ///< Transfer payload，转移有效载荷，就是有效数据部分
                               uint16_t payload_len);           ///< Length of the above, in bytes

/**
 * Returns a pointer to the top priority frame in the TX queue.
 * Returns NULL if the TX queue is empty.
 * The application will call this function after canardBroadcast() or canardRequestOrRespond() to transmit generated
 * frames over the CAN bus.
 * 
 * 返回指向TX队列中最高优先级帧的指针。
 * 如果TX队列为空，则返回NULL。
 * 应用程序将在canardBroadcast（）或canardRequestOrRespond（）发送生成的内容后调用此函数，通过CAN总线的帧。
 */
const CanardCANFrame* canardPeekTxQueue(const CanardInstance* ins);

/**
 * Removes the top priority frame from the TX queue.
 * The application will call this function after canardPeekTxQueue() once the obtained frame has been processed.
 * Calling canardBroadcast() or canardRequestOrRespond() between canardPeekTxQueue() and canardPopTxQueue()
 * is NOT allowed, because it may change the frame at the top of the TX queue.
 * 从TX队列中删除最高优先级帧。
 * 处理完获取的帧后，应用程序将在canardPeekTxQueue（）之后调用此函数。
 * 在canardPeekTxQueue（）和canardPopTxQueue（）之间调用canardBroadcast（）或canardRequestOrRespond（）不允许使用*，因为它可能会更改TX队列顶部的帧。
 * 
 */
void canardPopTxQueue(CanardInstance* ins);

/**
 * Processes a received CAN frame with a timestamp.
 * The application will call this function when it receives a new frame from the CAN bus.
 * 使用时间戳处理收到的CAN帧。
 * 当应用程序从CAN总线接收到新的帧时，它将调用此函数。
 */
void canardHandleRxFrame(CanardInstance* ins,
                         const CanardCANFrame* frame,
                         uint64_t timestamp_usec);

/**
 * Traverses the list of transfers and removes those that were last updated more than timeout_usec microseconds ago.
 * This function must be invoked by the application periodically, about once a second.
 * Also refer to the constant CANARD_RECOMMENDED_STALE_TRANSFER_CLEANUP_INTERVAL_USEC.
 * 遍历传输列表，并删除上次在timeout_usec微秒前更新的传输。
 * 该功能必须由应用程序定期调用，大约每秒一次。
 * 另请参阅常量CANARD_RECOMMENDED_STALE_TRANSFER_CLEANUP_INTERVAL_USEC。
 */
void canardCleanupStaleTransfers(CanardInstance* ins,
                                 uint64_t current_time_usec);

/**
 * This function can be used to extract values from received UAVCAN transfers. It decodes a scalar value -
 * boolean, integer, character, or floating point - from the specified bit position in the RX transfer buffer.
 * Simple single-frame transfers can also be parsed manually.
 *
 * Returns the number of bits successfully decoded, which may be less than requested if operation ran out of
 * buffer boundaries, or negated error code, such as invalid argument.
 *
 * Caveat:  This function works correctly only on platforms that use two's complement signed integer representation.
 *          I am not aware of any modern microarchitecture that uses anything else than two's complement, so it should
 *          not affect portability in any way.
 *
 * The type of value pointed to by 'out_value' is defined as follows:
 *
 *  | bit_length | value_is_signed | out_value points to                      |
 *  |------------|-----------------|------------------------------------------|
 *  | 1          | false           | bool (may be incompatible with uint8_t!) |
 *  | 1          | true            | N/A                                      |
 *  | [2, 8]     | false           | uint8_t, or char                         |
 *  | [2, 8]     | true            | int8_t, or char                          |
 *  | [9, 16]    | false           | uint16_t                                 |
 *  | [9, 16]    | true            | int16_t                                  |
 *  | [17, 32]   | false           | uint32_t                                 |
 *  | [17, 32]   | true            | int32_t, or 32-bit float                 |
 *  | [33, 64]   | false           | uint64_t                                 |
 *  | [33, 64]   | true            | int64_t, or 64-bit float                 |
 * 
 * 此功能可用于从接收到的UAVCAN传输中提取值。它解码一个标量值--布尔值，整数，字符或浮点数-从RX传输缓冲区中的指定位位置开始。
 * 简单的单帧传输也可以手动解析。
 *
 * 返回成功解码的位数，如果操作用完，则可能少于请求的位数缓冲区边界或否定的错误代码，例如无效的参数。
 *
 * 警告：此功能仅在使用二进制补码有符号整数表示的平台上正常工作。
 * 我不知道任何现代微体系结构使用除二进制补码之外的任何东西，因此应该不会以任何方式影响可移植性。
 *
 *“ out_value”所指向的值的类型定义如下：
 * 
 */
int16_t canardDecodeScalar(const CanardRxTransfer* transfer,    ///< The RX transfer where the data will be copied from
                           uint32_t bit_offset,                 ///< Offset, in bits, from the beginning of the transfer
                           uint8_t bit_length,                  ///< Length of the value, in bits; see the table
                           bool value_is_signed,                ///< True if the value can be negative; see the table
                           void* out_value);                    ///< Pointer to the output storage; see the table

/**
 * This function can be used to encode values for later transmission in a UAVCAN transfer. It encodes a scalar value -
 * boolean, integer, character, or floating point - and puts it to the specified bit position in the specified
 * contiguous buffer.
 * Simple single-frame transfers can also be encoded manually.
 *
 * Caveat:  This function works correctly only on platforms that use two's complement signed integer representation.
 *          I am not aware of any modern microarchitecture that uses anything else than two's complement, so it should
 *          not affect portability in any way.
 *
 * The type of value pointed to by 'value' is defined as follows:
 *
 * 此功能可用于编码值，以便以后通过UAVCAN传输进行传输。 它编码一个标量值-布尔值，整数，字符或浮点数-并将其放在指定位置的指定位连续缓冲区。
 * 简单的单帧传输也可以手动编码。
 *
 * 警告：此功能仅在使用二进制补码有符号整数表示的平台上正常工作。
 * 我不知道任何现代微体系结构使用除二进制补码之外的任何东西，因此应该不会以任何方式影响可移植性。
 *
 * “value”所指向的值的类型定义如下：
 * 
 *  | bit_length | value points to                          |
 *  |------------|------------------------------------------|
 *  | 1          | bool (may be incompatible with uint8_t!) |
 *  | [2, 8]     | uint8_t, int8_t, or char                 |
 *  | [9, 16]    | uint16_t, int16_t                        |
 *  | [17, 32]   | uint32_t, int32_t, or 32-bit float       |
 *  | [33, 64]   | uint64_t, int64_t, or 64-bit float       |
 */
void canardEncodeScalar(void* destination,      ///< Destination buffer where the result will be stored
                        uint32_t bit_offset,    ///< Offset, in bits, from the beginning of the destination buffer
                        uint8_t bit_length,     ///< Length of the value, in bits; see the table
                        const void* value);     ///< Pointer to the value; see the table

/**
 * This function can be invoked by the application to release pool blocks that are used
 * to store the payload of the transfer.
 *
 * If the application needs to send new transfers from the transfer reception callback, this function should be
 * invoked right before calling canardBroadcast() or canardRequestOrRespond(). Not releasing the buffers before
 * transmission may cause higher peak usage of the memory pool.
 *
 * If the application didn't call this function before returning from the callback, the library will do that,
 * so it is guaranteed that the memory will not leak.
 * 该功能可以由应用程序调用以释放使用的池块存储传输的有效负载。
 *
 * 如果应用程序需要从转移接收回调发送新转移，则此功能应为：在调用canardBroadcast（）或canardRequestOrRespond（）之前被调用。之前不释放缓冲区传输可能会导致更高的内存池峰值使用率。
 * 如果应用程序未从回调返回之前调用此函数，则库将执行此操作，因此可以确保内存不会泄漏。
 */
void canardReleaseRxTransferPayload(CanardInstance* ins,
                                    CanardRxTransfer* transfer);

/**
 * Returns a copy of the pool allocator usage statistics.
 * Refer to the type CanardPoolAllocatorStatistics.
 * Use this function to determine worst case memory needs of your application.
 * 返回池分配器使用情况统计信息的副本。请参阅类型CanardPoolAllocatorStatistics。
 * 使用此功能可以确定应用程序的最坏情况的内存需求。
 */
CanardPoolAllocatorStatistics canardGetPoolAllocatorStatistics(CanardInstance* ins);

/**
 * Float16 marshaling helpers.
 * These functions convert between the native float and 16-bit float.
 * It is assumed that the native float is IEEE 754 single precision float, otherwise results will be unpredictable.
 * Vast majority of modern computers and microcontrollers use IEEE 754, so this limitation should not affect
 * portability.
 * Float16编组助手。
 * 这些函数在本机浮点数和16位浮点数之间转换。假定本机浮点数是IEEE 754单精度浮点数，否则结果将不可预测。
 * 绝大多数现代计算机和微控制器都使用IEEE 754，因此此限制不应影响可移植性。
 */
uint16_t canardConvertNativeFloatToFloat16(float value);
float canardConvertFloat16ToNativeFloat(uint16_t value);

/// Abort the build if the current platform is not supported.
/// 如果不支持当前平台，则中止构建。
CANARD_STATIC_ASSERT(((uint32_t)CANARD_MULTIFRAME_RX_PAYLOAD_HEAD_SIZE) < 32,
                     "Platforms where sizeof(void*) > 4 are not supported. "
                     "On AMD64 use 32-bit mode (e.g. GCC flag -m32).");

#ifdef __cplusplus
}
#endif
#endif
