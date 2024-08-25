/* Vector’s body & head board communication protocol
   Copyright 2024 Randall Maas
*//**@file
    @brief Vector’s body & head board communication protocol.

    This header file defines the communication protocol between the head board
    and the body board.  Communication with the body-board, is structured as a
    request-response protocol and a streaming data update.  


    The messages from the head board to the body-board have the content:

    - Checking that the application firmware is running and its version
    - Boot-loader updates to the firmware: Entering the boot-loader, erasing
      flash, writing a new application, and verifying it
    - The 4 LED RGB states
    - Controls for the motors: direction and enable; direction and duty cycle;
      or a target position and speed.
    - Power control information: disable power to the system, turn off
      distance, cliff sensors, etc.

    In turn, the body board messages to the head-board can contain (depending
    on the type of packet):

    -	The touch sensor ADC value, and state of the backpack button
    -	The microphone samples for all 4 microphones.
    -	The battery voltage,
    -	The charging terminal voltage
    -	State of the charger – on docked, charging, battery critically low
    -	The temperature of the charger/battery
    -	The state of 4 motor encoders, possibly as encoder counters, possibly
        as IO state
    -	The time of flight readings, these are used to reconstruct histogram
        counts and SPAD reflectivity measures.
    -	The values from  each of the 4 cliff proximity sensors
    -	Which peripherals are enabled and disabled (powered down)
*/
#include <inttypes.h>
#include "pack.h"
class Stream;

/// The number of microphones.
#define MICROPHONE_COUNT (4)
/// The number of samples per frame for each microphone.
#define MICROPHONE_SAMPLES_PER_FRAME (80)


/** The Spine namespace encapsulates the definitions and structures used for 
    communication between the head board and the body board in Vector.

    Key features of the Spine namespace:

    - Defines message types for different communication scenarios (e.g., data
      frames, shutdown commands, firmware updates).
    - Provides structures for encapsulating data related to sensors, controls,
      and system states.
    - Functions to receive and send messages.
*/
namespace Spine {

enum {
    /// offset of the payload size
    payload_size_ofs = 6,
    /// offset of the payload
    payload_ofs = 8
};


/** The kinds of messages.
  
    The letters to describe the frame type are in the order sent (effectively
    the opposite of the 16-bit values)

    note the character codes are reversed due to little endian order
*/
enum class MessageType
{
    /// 'dc' [Data character? charger data?]  
    /// From head board, sends text back to the body board and out its back end;
    /// From body board, sends text to the head board from the chsarger pin.
    /// Note: this message is likely not supported in production application
    /// firmware (i.e. 1.6) but may be in the application software.
    dataCharacter = 0x6364,

    /// 'fd':Data frame.
    /// From head board, this has all the bits for the LEDs, motor drivers, power
    /// controls, etc.
    /// From body board, this has battery state – level, temperature, flags.
    /// The size of the message suggests that it holds 128 samples from one to three
    /// microphones (4 microphones × 2bytes/sample × 80 samples/microphone == 768 bytes)
    /// for the voice activity detection audio processing.
    dataFrame = 0x6466, 

    /// 'sd': Shutdown.
    /// From head board, disconnect the battery, to shutoff the system.
    shutdown = 0x6473, 

    /// 'uf': Update firmware frame.
    /// From head board, sends a 1024B as part of the DFU payload. The first 16b
    /// is the offset in the program memory to update; the next 16b are the
    /// number of 32-bit words in the payload to write.  (The packet is a fixed size,
    /// so may be padded out)
    updateFirmware = 0x6675, 

    /// 'dm': Go to DFU mode? Goto app mode? Change the mode: enter the boot-loader?
    /// start regular reports?
    mode = 0x6D64, 

    /// 'vr': Version.
    /// From head board, requests the application version.
    /// if there is an application, it responds with a 0x7276.
    /// If there isn’t application, the boot-loader responds with a 0x6B61 with
    /// a 0 payload (a NAK).
    /// From body board, the first 28 payload bytes are TBD.
    /// This is followed by a 16-byte version (often printable characters).
    /// The first 16 bytes of the DFU file are also the version.
    version = 0x7276, 

    /// 'ls': Lights.
    /// From head board, LED control.
    lights = 0x736C, 

    /// 'ts':Test
    /// From head board, Validate the flash, to check that the newly downloaded
    /// program and that it passed signature checks.  The boot-loader sends
    /// back a 0x6B61 to ACK to indicate that the firmware passed checks, or
    /// NACK that it does not.  If successful, the application is started.
    validate = 0x7374, 

    /// 'xx': Erase
    /// From head board, Erases the current program memory.
    /// (the currently installed image).   The boot-loader sends back a 0x6B61
    /// to acknowledge that the erase when it has completed.
    erase = 0x7878,

    /// Boot-loader frames.
    /// From body board.
    bootFrame = 0x6662,

    /// 'ak': Acknowledge.
    /// From body board, the value is non-zero if an acknowledge.
    ack = 0x6B61,

    /// 'vs':
    /// From body board,
    /// note: this message is not supported in production application firmware.
    VS = 0x7376
};

/// Ack message from the body board to the head board.
PACK(struct Ack
{
    /// The ack value
    /// Positive: success
    /// Negative: failure code
    int32_t value;
});

static_assert(sizeof(Ack) == 4, "The size of the Ack struct is expected to be 4 bytes");



/** The data character message from the body board to the head board.

    The body-board has a bidirectional serial interface for test purposes.
    This is located on the charger positive pad.  The single connection is
    half-duplex -- it is used to both send and receive.  The data rate is
    115.3 Kbits/sec.

    Note: this communication is only implemented in DVT firmware; it is not
    implemented in production firmware.  It is not known how to put the DVT
    firmware into this mode.  The linux application may still support this
    interface, but it is not known how to use it..

    When the body board powers on it sends a few header bytes and a string:

    @code
    0xFF 0x16 0x92 0x16 0x1F 0x16 0xCF 0x16 0xFF 0x16 0xFF 0x16 0xFF 0x16 0xFF 0x16 “\nbooted\n”
    @endcode

    Thereafter body-board can receive characters from this interface and
    forward them with the 636416 message to the head-board for processing by
    vic-robot.  

    1. `vic-robot` receives these characters, and buffers them..  When it sees
       a new line or carriage return, it examines the line.  If the line starts
       with a `>` and is followed by a valid 3-letter command, it will carry out
       the command.  This may include reporting sensed values, writing the
       factor calibration values or EMR.

    2.	If `vic-robot` wishes to send text via the body-boards outgoing serial
        port, it uses the 0x6364 command to send the text characters to the
        body-board, which then sends them out the charger port.

    The text commands from this port are that vic-robot recognizes are:

    - `esn` gets the electronic serial number
    - `bsv`
    - `mot` controls the motors
    - `get`
    - `fcc`
    - `rlg` read log
    - `eng`
    - `smr` writes the EMR
    - `gmr` gets the EMR
    - `pwr` allows shutdown, and reboot
    - `led`
*/
PACK(struct DataCharacter
{
    /// The text from the manufacturing test station.
    /// Not sure if this is a fixed length (such as single character), or if it
    /// is a null terminated string.
    char text[32];
});

/// The indices of the motors.
enum class Motor
{
    /// The left wheel motor.
    frontLeft = 0,

    /// The right wheel motor
    frontRight = 1,

    /// The lift motor.
    backLeft = 2,

    /// The head motor.
    backRight = 3
};


/// The indices of the cliff sensors.
enum class CliffSensor
{
    /// The front-left cliff sensor.
    frontLeft = 0,

    /// The front-right cliff sensor.
    frontRight = 1,

    /// The back-left cliff sensor.
    backLeft = 2,

    /// The back-right cliff sensor.
    backRight = 3
};



/// The I2C addresses of the sensor that failed (ie could not be communicated
/// with) during a power on self test.
enum class I2CAddress
{
    /// No fault
    none = 0,

    ///  The time of flight distance sensor.
    timeOfFlight = 0x52,

    /// A cliff sensor failed.  See the minor code for which sensor.
    cliff = 0xA6,
};


/** The motor status structure holds information about the motor's state.
 */
PACK(struct MotorState
{
    /// The encoder count, represents the current position of the motor.
    int32_t position;

    /// The change in encoder count from the previous position,
    /// indicating how much the motor has moved.
    int32_t delta;

    /// The number of ticks since the last change, providing a measure of time
    /// since the last update.
    uint32_t time;
});

/// Check the size of the MotorState struct
static_assert(sizeof(MotorState) == 12, "The size of the MotorState struct is expected to be 12 bytes");


/** The data frame from the body board to the head board.

    This structure represents the data frame that is sent from the body board
    to the head board. It contains various fields that provide information about
    the state of the system, including sensor statuses, motor states, and
    voltage readings. The data frame is designed to be packed to ensure that
    it is transmitted efficiently over the communication interface.

    The messages are sent fast enough to support microphone sample rate of
    15625 samples/second for each of the 4 microphones.

    Note: sasting to a struct can be risky --unaligned access of struct fields
    can cause faults on many targets.  For this reason, this struct 's fields
    are aligned properly.
*/
PACK(struct B2HDataFrame
{
    /// The sequence number of the message.  Likely a counter to detect lost messages.
    uint32_t sequenceNumber;

    /// This bit is set if the cliff sensor and time of flight sensor are on;
    /// it is clear if they are off.
    uint8_t sensorsOn:1,

    /// This bit is set if the motor encoders have been turned off.  This is
    /// done to save power when the motors are idle.
    /// If the bit is not set, the encoders are enabled.
            encodersOff:1,

    /// The head encoder has changed value (the head moved).
            headEncoderChanged:1,

    /// The lift encoder has changed value (the lift moved)
            liftEncoderChanged:1;


    /// Seems to be 0, no problem; otherwise, over temperature..?
    uint8_t temperstureStatus;

    /// I2C fault.
    /// 0 if no fault, otherwise the I2C address of the sensor that can’t communicate:
    uint8_t i2cFault;

    /// I2C fault item.
    /// If the fault is 'cliff', this is the index of the first cliff sensor
    /// that was detected to have failed.  See the enumeration above {TBD}
    uint8_t i2cFault_index;

    /// The motor status for each of the motors.
    MotorState motor[4];

    /// Sensor readings for each of the cliff sensors.
    uint16_t cliffSense[4];

    /// The battery voltage, scale by 0.00136719 to get volts
    int16_t battery_volt;

    /// The charger voltage, scale by 0.00136719 to get volts
    int16_t charger_volt;

    /// The body-board MCU temperature (proxy for the battery temperature)
    int16_t temperature;

    // Battery condition
    /// The charger is connected to a power source -- that is, the charger IC
    /// has detected a voltage supplied to the charging pins.
    uint16_t onCharger:1,

    /// The battery is charging.
            charging :1,

    /// The battery is disconnected.
            disconnect:1,

    /// The battery is overheated.
            overheated:1,

    /// unknown/reserved
            reserved1 :1,

    /// The battery voltage is low, below a critical threshold (probably as
    /// defined by the charger).
            voltageLow :1,

    /// Emergency shutdown imminent.
            shutdown :1,
            reserved:9;

    uint32_t unknown;

    // Time of flight sensor status
    /// The low 4 bits are some sort of state
    uint8_t prox_status;

    /// The time of flight sensor’s reported sigma
    uint8_t prox_sigma_mm;

    /// The time of flight sensor’s reported range
    uint16_t prox_range_mm;

    /// The time of flight sensor’s reported signal strength
    uint16_t prox_signalRate_mcps;

    /// The time of flight sensor’s reported ambient noise
    uint16_t prox_ambient;

    /// The time of flight sensor’s reported SPAD count
    uint16_t prox_SPADCount;

    /// The time of flight sensor’s reported sample count
    uint16_t prox_sampleCount;

    /// The time of flight sensor’s reported calibration result
    uint32_t prox_calibrationResult;

    /// Touch sensor readings.
    ///  Index: 0 is the touch sense ADC?
    ///  1 is the button,
    uint16_t touchLevel[2];

    /// Mic error.
    uint16_t micError[2]; // Raw bits from a segment of mic data (stuck bit detect)

    /// Something related to the touch sensing / button inputs
    uint16_t touchLevel2[2];

    /// Unknown/unused?
    uint8_t reserved_[24];

    /// The microphone samples for the voice activity detection audio processing.
    /// 80 samples from each of the 4 microphones.
    int16_t mic_samples[MICROPHONE_SAMPLES_PER_FRAME*MICROPHONE_COUNT];
});

// check the size of the struct
static_assert(sizeof(B2HDataFrame) == 768, "The size of the B2HDataFrame struct is expected to be 768 bytes");

/** The H2B namespace encapsulates the definitions and structures used for 
    communication from the head board to the body board in Vector.

    Key features of the H2B namespace:

    - Defines the buffer for receiving messages from the head board.
    - Provides a function to determine the size of messages based on their type.
    - Facilitates the population of message headers for communication.
*/
namespace H2B {

/** The buffer to receive messages into
    @note the buffer is 1028 bytes + 4 bytes for the payload size + 4 bytes for the crc

    The header is:
    @code
    0xAA ‘H’ ‘2’ ‘B’
    @endcode

    The rest of the frame:

    - The message type is 16 bits. It is both how to interpret the payload, and
      a cross-check on the size of the payload.  If the message type is not
      recognized, or the implied size does not match the passed payload size,
      the message is considered in error.
    - The payload size is a 16 bit number.  The maximum payload size is 1280 bytes. 
    - The CRC is 32 bits.  It is computed on the payload only.
*/
extern uint8_t recv_buffer[1028+payload_ofs+4];


/** Receive a message frame from the head board
    @param in the stream to receive the message from
    @param payload_size the size of the payload
    @return the message type

    This function receives a message from the head board over a serial
    connection.  It implements a framing layer that ensures the integrity
    of the received messages.   If the message doesn’t pass CRC checks, or
    the command is not recognize, the frame is skipped.
 
    The function begins by waiting for a specific sync byte sequence that
    indicates the start of a message:
    0xAA followed by the characters 'H', '2', and 'B'. If this sequence is
    not detected, the function will loop back to the start and continue
    waiting for the correct sequence.
  
    Once the sync sequence is detected, the function reads the message type and
    size from the incoming message.  It then verifies the message type against
    a predefined set of message types and checks if the size of the payload
    matches the expected size for that message type. If there is a mismatch,
    the function will again loop back to the start to wait for a new message.
  
    After validating the message type and size, the function reads the payload
    data and computes a CRC (Cyclic Redundancy Check) to ensure data integrity. 
    The computed CRC is compared against the CRC included in the received
    message. If the CRC check fails, the function will also loop back to the
    start.

    If all checks pass, the function successfully receives a valid message and
    returns the corresponding MessageType indicating the type of message
    received.
 */
MessageType ReceiveMessage(Stream& in, size_t& payload_size);

/** Send a message to the head board.
    @param out the stream to send the message to
    @param payload_size the size of the payload
*/
void SendMessage(Stream& out, size_t payload_size);
}


/** The B2H namespace encapsulates the definitions and structures used for 
    communication from the body board to the head board in Vector.

    Key features of the B2H namespace:

    - Defines the buffer for receiving messages from the body board.
    - Provides a function to determine the size of messages based on their type.
    - Facilitates the population of message headers for communication.
    - Provides functions to send messages to the head board.
*/
namespace B2H {

/** The buffer to receive messages into
    @note the buffer is 1028 bytes + 4 bytes for the payload size + 4 bytes for the crc

    The header is:
    @code
    0xAA ‘B’ ‘2’ ‘H’
    @endcode

    The rest of the frame:

    - The message type is 16 bits. It is both how to interpret the payload, and
      a cross-check on the size of the payload.  If the message type is not
      recognized, or the implied size does not match the passed payload size,
      the message is considered in error.
    - The payload size is a 16 bit number.  The maximum payload size is 1280 bytes. 
    - The CRC is 32 bits.  It is computed on the payload only.
*/
extern uint8_t recv_buffer[1028+payload_ofs+4];


/** Send a data character message to the head board.
    @param text the text to send
    @param numBytes the number of bytes to send (max 31)
    @return the size of the message
 */
size_t DataCharacterMsg(const char* text, int numBytes);


/** Receive a message frame from the body board
    @param in the stream to receive the message from
    @param payload_size the size of the payload
    @return the message type

    This function receives a message from the body board over a serial
    connection.  It implements a framing layer that ensures the integrity
    of the received messages.   If the message doesn’t pass CRC checks, or
    the command is not recognize, the frame is skipped.
 
    The function begins by waiting for a specific sync byte sequence that
    indicates the start of a message:
    0xAA followed by the characters 'B', '2', and 'H'. If this sequence is
    not detected, the function will loop back to the start and continue
    waiting for the correct sequence.
  
    Once the sync sequence is detected, the function reads the message type and
    size from the incoming message.  It then verifies the message type against
    a predefined set of message types and checks if the size of the payload
    matches the expected size for that message type. If there is a mismatch,
    the function will again loop back to the start to wait for a new message.
  
    After validating the message type and size, the function reads the payload
    data and computes a CRC (Cyclic Redundancy Check) to ensure data integrity. 
    The computed CRC is compared against the CRC included in the received
    message. If the CRC check fails, the function will also loop back to the
    start.

    If all checks pass, the function successfully receives a valid message and
    returns the corresponding MessageType indicating the type of message
    received.
 */
MessageType ReceiveMessage(Stream& in, size_t& payload_size);

/** Send a message to the head board.
    @param out the stream to send the message to
    @param payload_size the size of the payload
*/
void SendMessage(Stream& out, size_t payload_size);

}


}
