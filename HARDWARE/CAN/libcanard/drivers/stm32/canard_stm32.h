/*
 * Copyright (c) 2017 UAVCAN Team
 *
 * Distributed under the MIT License, available in the file LICENSE.
 *
 * Author: Pavel Kirienko <pavel.kirienko@zubax.com>
 */

#ifndef CANARD_STM32_H
#define CANARD_STM32_H

#include <canard.h>
#include <string.h>     // NOLINT


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Set this build config macro to 1 to use CAN2 instead of CAN1, if available.
 * Setting this parameter when CAN2 is not available may not be detected at compile time!
 * 将此构建配置宏设置为1以使用CAN2代替CAN1（如果有）。编译时可能无法检测到当CAN2不可用时设置此参数！
 */
#if !defined(CANARD_STM32_USE_CAN2)
# define CANARD_STM32_USE_CAN2                                  1
#endif

/**
 * Trigger an assertion failure if inner priority inversion is detected at run time.
 * This setting has no effect in release builds, where NDEBUG is defined.
 * 如果在运行时检测到内部优先级反转，则触发断言失败。此设置在定义NDEBUG的发行版本中无效。
 */
#if !defined(CANARD_STM32_DEBUG_INNER_PRIORITY_INVERSION)
# define CANARD_STM32_DEBUG_INNER_PRIORITY_INVERSION            1
#endif

/**
 * Driver error codes.
 * These values are returned negated from API functions that return int.
 * 驱动程序错误代码。这些值与返回int的API函数取反。
 */
#define CANARD_STM32_ERROR_UNSUPPORTED_BIT_RATE                         1000
#define CANARD_STM32_ERROR_MSR_INAK_NOT_SET                             1001
#define CANARD_STM32_ERROR_MSR_INAK_NOT_CLEARED                         1002
#define CANARD_STM32_ERROR_UNSUPPORTED_FRAME_FORMAT                     1003

/**
 * This is defined by the bxCAN hardware.
 * Devices with only one CAN interface have 14 filters (e.g. F103).
 * Devices with two CAN interfaces have 28 filters, which are shared between two interfaces (e.g. F105, F446).
 * The filters are distributed between CAN1 and CAN2 by means of the CAN2 start filter bank selection,
 * which is a number from 1 to 27 inclusive. Seeing as the start bank cannot be set to 0, CAN2 has one filter less
 * to use.
 * 这是由bxCAN硬件定义的。仅具有一个CAN接口的设备具有14个过滤器（例如F103）。
 * 具有两个CAN接口的设备具有28个过滤器，这两个接口在两个接口之间共享（例如F105，F446）。
 * 通过选择CAN2启动滤波器组，在CAN1和CAN2之间分配滤波器。是1到27之间的一个数字。 鉴于起始存储区不能设置为0，CAN2少了一个滤波器使用。
 * 
 */
#define CANARD_STM32_NUM_ACCEPTANCE_FILTERS                            14U

/**
 * The interface can be initialized in either of these modes.
 *
 * The Silent mode is useful for automatic CAN bit rate detection, where the interface is initialized at an
 * arbitrarily guessed CAN bit rate (typically either 1 Mbps, 500 Kbps, 250 Kbps, or 125 Kbps, these are the
 * standard values defined by the UAVCAN specification), and the bus is then listened for 1 second in order to
 * determine whether the bit rate was guessed correctly. It is paramount to use the silent mode in this case so
 * as to not interfere with ongoing communications on the bus if the guess was incorrect.
 *
 * The automatic TX abort on error mode should be used during dynamic node ID allocation. The reason for that
 * is well explained in the UAVCAN specification, please read it.
 *
 * The normal mode should be used for all other use cases, particularly for the normal operation of the node,
 * hence the name.
 * 
 * 可以在这两种模式下初始化接口。
 * 静默模式对于自动CAN比特率检测非常有用，该接口在任意猜测的CAN比特率（通常为1 Mbps，500 Kbps，250 Kbps或125 Kbps，这些是由UAVCAN规范定义的标准值），
 * 然后监听总线1秒钟，以便确定是否正确猜测了比特率。 在这种情况下，使用静音模式至关重要。
 * 如果猜测不正确，则不会干扰总线上正在进行的通信。
 *
 * 在动态节点ID分配期间，应使用错误模式下的自动TX中止。 的原因在UAVCAN规范中有很好的解释，请仔细阅读。
 * 普通模式应用于所有其他用例，尤其是节点的正常操作，由此得名。
 * 
 */
typedef enum
{
    CanardSTM32IfaceModeNormal,                         //!< Normal mode
    CanardSTM32IfaceModeSilent,                         //!< Do not affect the bus, only listen不影响总线，只听
    CanardSTM32IfaceModeAutomaticTxAbortOnError         //!< Abort pending TX if a bus error has occurred，如果发生总线错误，则中止挂起的TX
} CanardSTM32IfaceMode;

/**
 * Interface statistics; these values can be queried using a dedicated API call.
 * 接口统计； 可以使用专用的API调用查询这些值。
 */
typedef struct
{
    uint64_t rx_overflow_count;
    uint64_t error_count;
} CanardSTM32Stats;

/**
 * ID and Mask of a hardware acceptance filter.硬件验收过滤器的ID和掩码。
 * The ID and Mask fields support flags @ref CANARD_CAN_FRAME_EFF and @ref CANARD_CAN_FRAME_RTR.
 * ID和掩码字段支持标志
 */
typedef struct
{
    uint32_t id;
    uint32_t mask;
} CanardSTM32AcceptanceFilterConfiguration;

/**
 * These parameters define the timings of the CAN controller.
 * Please refer to the documentation of the bxCAN macrocell for explanation.
 * These values can be computed by the developed beforehand if ROM size is of a concern,
 * or they can be computed at run time using the function defined below.
 * 这些参数定义了CAN控制器的时序。
 * 请参考bxCAN宏单元的说明文件。
 * 如果需要考虑ROM的大小，可以通过预先开发的方法来计算这些值，或可以使用下面定义的函数在运行时计算它们。
 */
typedef struct
{
    uint16_t bit_rate_prescaler;                        /// [1, 1024]
    uint8_t bit_segment_1;                              /// [1, 16]
    uint8_t bit_segment_2;                              /// [1, 8]
    uint8_t max_resynchronization_jump_width;           /// [1, 4] (recommended value is 1)
} CanardSTM32CANTimings;

/**
 * Initializes the CAN controller at the specified bit rate.
 * The mode can be either normal, silent, or auto-abort on error;
 * in silent mode the controller will be only listening, not affecting the state of the bus;
 * in the auto abort mode the controller will cancel the pending transmissions if a bus error is encountered.
 * The auto abort mode is needed for dynamic node ID allocation procedure; please refer to the UAVCAN specification
 * for more information about this topic.
 *
 * This function can be invoked any number of times; every invocation re-initializes everything from scratch.
 *
 * WARNING: The clock of the CAN module must be enabled before this function is invoked!
 *          If CAN2 is used, CAN1 must be also enabled!
 *
 * WARNING: The driver is not thread-safe!
 *          It does not use IRQ or critical sections though, so it is safe to invoke its API functions from the
 *          IRQ context from the application.
 *
 * @retval      0               Success
 * @retval      negative        Error
 * 以指定的比特率初始化CAN控制器。该模式可以是正常，静默或发生错误时自动中止；
 * 在静音模式下，控制器将仅在侦听，不会影响总线状态；
 * 在自动中止模式下，如果遇到总线错误，控制器将取消挂起的传输。
 * 动态节点ID分配过程需要自动中止模式；请参考UAVCAN规范有关此主题的更多信息。
 *
 * 该函数可以被调用多次；每次调用都会从头开始重新初始化所有内容。
 *
 * 警告：调用该功能之前，必须先启用CAN模块的时钟！
 * 如果使用CAN2，则必须同时启用CAN1！
 *
 * 警告：驱动程序不是线程安全的！
 * 尽管它不使用IRQ或关键部分，所以可以安全地从应用程序的IRQ上下文。
 * 
 */
int16_t canardSTM32Init(const CanardSTM32CANTimings* const timings,
                        const CanardSTM32IfaceMode iface_mode);

/**
 * Pushes one frame into the TX buffer, if there is space.
 * Note that proper care is taken to ensure that no inner priority inversion is taking place.
 * This function does never block.
 * 如果有空间，则将一帧推入TX缓冲区。
 * 请注意，要确保没有发生内部优先级倒置。
 * 此功能永不阻塞。
 *
 * @retval      1               Transmitted successfully
 * @retval      0               No space in the buffer
 * @retval      negative        Error
 */
int16_t canardSTM32Transmit(const CanardCANFrame* const frame);

/**
 * Reads one frame from the hardware RX FIFO, unless all FIFO are empty.
 * This function does never block.
 * 除非所有FIFO为空，否则从硬件RX FIFO读取一帧。此功能永不阻塞。
 *
 * @retval      1               Read successfully
 * @retval      0               The buffer is empty
 * @retval      negative        Error
 */
int16_t canardSTM32Receive(CanardCANFrame* const out_frame);

/**
 * Sets up acceptance filters according to the provided list of ID and masks.
 * Note that when the interface is reinitialized, hardware acceptance filters are reset.
 * Also note that during filter reconfiguration, some RX frames may be lost.
 *
 * Setting zero filters will result in rejection of all frames.
 * In order to accept all frames, set one filter with ID = Mask = 0, which is also the default configuration.
 * 
 * 根据提供的ID和掩码列表设置接受过滤器。
 * 请注意，在重新初始化接口时，将重置硬件接受过滤器。另请注意，在重新配置滤波器时，某些RX帧可能会丢失。
 * 设置零滤镜将导致拒绝所有帧。为了接受所有帧，将一个过滤器设置为ID = Mask = 0，这也是默认配置。
 *
 * @retval      0               Success
 * @retval      negative        Error
 */
int16_t canardSTM32ConfigureAcceptanceFilters(const CanardSTM32AcceptanceFilterConfiguration* const filter_configs,
                                              const uint8_t num_filter_configs);

/**
 * Returns the running interface statistics.
 */
CanardSTM32Stats canardSTM32GetStats(void);

/**
 * Given the rate of the clock supplied to the bxCAN macrocell (typically PCLK1) and the desired bit rate,
 * this function iteratively solves for the best possible timing settings. The CAN bus timing parameters,
 * such as the sample point location, the number of time quantas per bit, etc., are optimized according to the
 * recommendations provided in the specifications of UAVCAN, DeviceNet, and CANOpen.
 *
 * Unless noted otherwise, all units are SI units; particularly, frequency is specified in hertz.
 *
 * The implementation is adapted from libuavcan.
 *
 * This function is defined in the header in order to encourage the linker to discard it if it is not used.
 * 
 * 假定提供给bxCAN宏单元的时钟速率（通常为PCLK1）和所需的比特率，此功能迭代解决最佳时序设置。 CAN总线时序参数
 * 如采样点位置，每位时间量化数量等，根据UAVCAN，DeviceNet和CANOpen规范中提供的建议。
 *
 * 除非另有说明，否则所有单位均为国际单位； 特别是频率以赫兹为单位。
 * 该实现改编自libuavcan。
 * 此函数在标头中定义，以鼓励链接器在不使用它时丢弃它。
 *
 * @retval 0            Success
 * @retval negative     Solution could not be found for the provided inputs.找不到提供的输入的解决方案。
 */
static inline
int16_t canardSTM32ComputeCANTimings(const uint32_t peripheral_clock_rate,
                                     const uint32_t target_bitrate,
                                     CanardSTM32CANTimings* const out_timings)
{
    if (target_bitrate < 1000)
    {
        return -CANARD_STM32_ERROR_UNSUPPORTED_BIT_RATE;
    }

    CANARD_ASSERT(out_timings != NULL);  // NOLINT
    memset(out_timings, 0, sizeof(*out_timings));

    /*
     * Hardware configuration
     */
    static const uint8_t MaxBS1 = 16;
    static const uint8_t MaxBS2 = 8;

    /*
     * Ref. "Automatic Baudrate Detection in CANopen Networks", U. Koppe, MicroControl GmbH & Co. KG
     *      CAN in Automation, 2003
     *
     * According to the source, optimal quanta per bit are:
     *   Bitrate        Optimal Maximum
     *   1000 kbps      8       10
     *   500  kbps      16      17
     *   250  kbps      16      17
     *   125  kbps      16      17
     */
    const uint8_t max_quanta_per_bit = (uint8_t)((target_bitrate >= 1000000) ? 10 : 17);    // NOLINT
    CANARD_ASSERT(max_quanta_per_bit <= (MaxBS1 + MaxBS2));

    static const uint16_t MaxSamplePointLocationPermill = 900;

    /*
     * Computing (prescaler * BS):
     *   BITRATE = 1 / (PRESCALER * (1 / PCLK) * (1 + BS1 + BS2))       -- See the Reference Manual
     *   BITRATE = PCLK / (PRESCALER * (1 + BS1 + BS2))                 -- Simplified
     * let:
     *   BS = 1 + BS1 + BS2                                             -- Number of time quanta per bit
     *   PRESCALER_BS = PRESCALER * BS
     * ==>
     *   PRESCALER_BS = PCLK / BITRATE
     */
    const uint32_t prescaler_bs = peripheral_clock_rate / target_bitrate;

    /*
     * Searching for such prescaler value so that the number of quanta per bit is highest.
     */
    uint8_t bs1_bs2_sum = (uint8_t)(max_quanta_per_bit - 1);    // NOLINT

    while ((prescaler_bs % (1U + bs1_bs2_sum)) != 0)
    {
        if (bs1_bs2_sum <= 2)
        {
            return -CANARD_STM32_ERROR_UNSUPPORTED_BIT_RATE;          // No solution
        }
        bs1_bs2_sum--;
    }

    const uint32_t prescaler = prescaler_bs / (1U + bs1_bs2_sum);
    if ((prescaler < 1U) || (prescaler > 1024U))
    {
        return -CANARD_STM32_ERROR_UNSUPPORTED_BIT_RATE;              // No solution
    }

    /*
     * Now we have a constraint: (BS1 + BS2) == bs1_bs2_sum.
     * We need to find such values so that the sample point is as close as possible to the optimal value,
     * which is 87.5%, which is 7/8.
     *
     *   Solve[(1 + bs1)/(1 + bs1 + bs2) == 7/8, bs2]  (* Where 7/8 is 0.875, the recommended sample point location *)
     *   {{bs2 -> (1 + bs1)/7}}
     *
     * Hence:
     *   bs2 = (1 + bs1) / 7
     *   bs1 = (7 * bs1_bs2_sum - 1) / 8
     *
     * Sample point location can be computed as follows:
     *   Sample point location = (1 + bs1) / (1 + bs1 + bs2)
     *
     * Since the optimal solution is so close to the maximum, we prepare two solutions, and then pick the best one:
     *   - With rounding to nearest
     *   - With rounding to zero
     */
    uint8_t bs1 = (uint8_t)(((7 * bs1_bs2_sum - 1) + 4) / 8);       // Trying rounding to nearest first  // NOLINT
    uint8_t bs2 = (uint8_t)(bs1_bs2_sum - bs1);  // NOLINT
    CANARD_ASSERT(bs1_bs2_sum > bs1);

    {
        const uint16_t sample_point_permill = (uint16_t)(1000U * (1U + bs1) / (1U + bs1 + bs2));  // NOLINT

        if (sample_point_permill > MaxSamplePointLocationPermill)   // Strictly more!
        {
            bs1 = (uint8_t)((7 * bs1_bs2_sum - 1) / 8);             // Nope, too far; now rounding to zero
            bs2 = (uint8_t)(bs1_bs2_sum - bs1);
        }
    }

    const bool valid = (bs1 >= 1) && (bs1 <= MaxBS1) && (bs2 >= 1) && (bs2 <= MaxBS2);

    /*
     * Final validation
     * Helpful Python:
     * def sample_point_from_btr(x):
     *     assert 0b0011110010000000111111000000000 & x == 0
     *     ts2,ts1,brp = (x>>20)&7, (x>>16)&15, x&511
     *     return (1+ts1+1)/(1+ts1+1+ts2+1)
     */
    if ((target_bitrate != (peripheral_clock_rate / (prescaler * (1U + bs1 + bs2)))) ||
        !valid)
    {
        // This actually means that the algorithm has a logic error, hence assert(0).
        CANARD_ASSERT(0);  // NOLINT
        return -CANARD_STM32_ERROR_UNSUPPORTED_BIT_RATE;
    }

    out_timings->bit_rate_prescaler = (uint16_t) prescaler;
    out_timings->max_resynchronization_jump_width = 1;      // One is recommended by UAVCAN, CANOpen, and DeviceNet
    out_timings->bit_segment_1 = bs1;
    out_timings->bit_segment_2 = bs2;

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif
