/* Vector’s body & head board communication protocol
   Copyright 2024 Randall Maas
*//**@file
    @brief Vector’s body & head board communication protocol.

    This file contains the implementation details for communication between the
    body board and the head board in the Vector system.  It provides functions
    for receiving and sending messages.

    The H2B namespace encapsulates the definitions and structures used for
    communication from the head board to the body board, while the B2H
    namespace handles communication in the opposite direction. 
*/
#include <algorithm>
#include <Arduino.h>
#include <esp32/rom/crc.h>
#include "spine.h"
// not sure if it should be crc32_be or crc32_le
#define crc32 crc32_le

namespace Spine {


// some helpful constants.. 
enum
{
    /// sync byte
    sync=0xAA,

    /// offset of the message type
    message_type_ofs = 4,
};


/// wait for a byte
/// @param c the byte to wait for
#define waitFor(c) do{if ((recv_buffer[offset] = in.read()) != (c)) return (MessageType)-1;offset++;}while(0)


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
uint8_t recv_buffer[1028+payload_ofs+4];

/** The sizes of the messages when sent from the head board to the body board.
    @param command the command to get the size of
    @return the size of the message
*/
int size(MessageType command)
{
    // lookup the size of the message
    switch (command)
    {
        // message type size
        default: return -1;
        case MessageType::dataCharacter : return 32;
        case MessageType::dataFrame     : return 64;
        case MessageType::shutdown      : return 0;  
        case MessageType::updateFirmware: return 1028;
        case MessageType::mode          : return 0;
        case MessageType::version       : return 0;
        case MessageType::lights        : return 16;
        case MessageType::validate      : return 0;
        case MessageType::erase         : return 0;
    }
}



/** Populate the header of a message
    @param buffer the buffer to populate
    @param message_type the type of the message
    @return the size of the message payload
*/
size_t populateHeader(uint8_t* buffer, MessageType message_type)
{
    buffer[0] = 0xAA;
    buffer[1] = 'H';
    buffer[2] = '2';
    buffer[3] = 'B';

    // Add the message type and size
    // assumes alignment, little endian host
    *(uint16_t*)(buffer+message_type_ofs) = (uint16_t) message_type;
    size_t payload_size = size(message_type);
    *(uint16_t*)(buffer+payload_size_ofs) = (uint16_t) payload_size;

    // return the size of the message payload
    return payload_size;
}


/** Send a data character message to the head board.
    @param text the text to send
    @param numBytes the number of bytes to send (max 31)
    @return the size of the message
 */
size_t DataCharacterMsg(const char* text, int numBytes)
{
    // Create a DataCharacter message
    populateHeader(recv_buffer, MessageType::dataCharacter);
    auto ptr= (DataCharacter*)(recv_buffer+payload_ofs);

    // Limit the number of bytes to 31
    numBytes = std::min(numBytes, 31);

    // Populate the text field
    memcpy(ptr->text, text, numBytes);
    // Null terminate the string
    ptr->text[numBytes] = 0;

    // Add the CRC
    auto payload_size = size(MessageType::dataCharacter);
    auto crc = crc32(~0UL, recv_buffer+payload_ofs, payload_size);

    // put the value into the buffer
    // assumes alignment, little endian host
    *(uint32_t*)(recv_buffer+payload_ofs+ payload_size+4) = crc;

    return payload_size;
}


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
MessageType ReceiveMessage(Stream& in, size_t& payload_size)
{
    // reset the offset into the buffer
    int offset = 0;

    // wait for message start: 0xAA 'H' '2' 'B'
    waitFor(sync);
    waitFor('H');
    waitFor('2');
    waitFor('B');

    // receive the payload type and size
    in.readBytes(recv_buffer+message_type_ofs, 4);

    // Check the payload type and size
    // The message type is 16 bits. The message type implies both the size of the
    // payload, and the contents.  If the message type is not recognized, or the
    // implied size does not match the passed payload size, the packet is
    // considered in error.
    auto message_type = (MessageType) *(uint16_t*)(recv_buffer+message_type_ofs);
    // The payload size is a 16 bit number.  The maximum payload size is 1280 bytes. 
    payload_size = *(uint16_t*)(recv_buffer+payload_size_ofs);

    // lookup the expected size of the message
    auto expected_size = size(message_type);
    // and check if the passed size is correct for the message type
    if (expected_size < 0 || expected_size != payload_size)
    {
        // the message is bad: didnt pass type and size checks
        // go back to the start to look for a new message
        payload_size = 0;
        return (MessageType)-1;
    }

    // read those bytes, including the crc
    in.readBytes(recv_buffer+payload_ofs, payload_size+4);

    // check crc of buffer
    auto crc = crc32(~0UL, recv_buffer+payload_ofs, payload_size);
    // assumes alignment, little endian host
    auto crc_in_buffer = *(uint32_t*)(recv_buffer+payload_ofs+ payload_size+4);

    // if crc is bad, go back to the start
    if (crc != crc_in_buffer)
    {
        // the message is bad: didnt pass type and size checks
        // go back to the start to look for a new message
        payload_size = 0;
        return (MessageType)-1;
    }

    // return the message type
    return message_type;
}



/** Send a message to the head board.
    @param out the stream to send the message to
    @param payload_size the size of the payload
*/
void SendMessage(Stream& out, size_t payload_size)
{
    // send the message
    out.write(recv_buffer, payload_size+payload_ofs+4);
}
}



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
uint8_t recv_buffer[1028+payload_ofs+4];


/** The sizes of the messages when sent from the body board to the head board.
    @param command the command to get the size of
    @return the size of the message
*/
int size(MessageType command)
{
    // lookup the size of the message
    switch (command)
    {
        // message type size
        default: return -1;
        case MessageType::dataCharacter : return 32;
        case MessageType::updateFirmware: return 32;
        case MessageType::dataFrame     : return 768;
        case MessageType::bootFrame     : return 0;
        case MessageType::ack           : return 4;
        case MessageType::version       : return 40;
        case MessageType::validate      : return 0;
    }
}


/** Populate the header of a message
    @param buffer the buffer to populate
    @param message_type the type of the message
    @return the size of the message payload
*/
size_t populateHeader(uint8_t* buffer, MessageType message_type)
{
    buffer[0] = 0xAA;
    buffer[1] = 'B';
    buffer[2] = '2';
    buffer[3] = 'H';

    // Add the message type and size
    // assumes alignment, little endian host
    *(uint16_t*)(buffer+message_type_ofs) = (uint16_t) message_type;
    size_t payload_size = size(message_type);
    *(uint16_t*)(buffer+payload_size_ofs) = (uint16_t) payload_size;
    return payload_size;
}


/** Send a data character message to the head board.
    @param text the text to send
    @param numBytes the number of bytes to send (max 31)
    @return the size of the message payload
 */
size_t DataCharacterMsg(const char* text, int numBytes)
{
    // Create a DataCharacter message
    populateHeader(recv_buffer, MessageType::dataCharacter);
    auto ptr= (DataCharacter*)(recv_buffer+payload_ofs);

    // Limit the number of bytes to 31
    numBytes = std::min(numBytes, 31);

    // Populate the text field
    memcpy(ptr->text, text, numBytes);
    // Null terminate the string
    ptr->text[numBytes] = 0;

    // Add the CRC
    auto payload_size = size(MessageType::dataCharacter);
    auto crc = crc32(~0UL, recv_buffer+payload_ofs, payload_size);
    // assumes alignment, little endian host
    *(uint32_t*)(recv_buffer+payload_ofs+ payload_size+4) = crc;

    return payload_size;
}


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
MessageType ReceiveMessage(Stream& in, size_t& payload_size)
{
    // reset the offset into the buffer
    int offset = 0;

    // wait for message start: 0xAA 'B' '2' 'H'
    waitFor(sync);
    waitFor('B');
    waitFor('2');
    waitFor('H');

    // receive the payload type and size
    in.readBytes(recv_buffer+message_type_ofs, 4);

    // Check the payload type and size
    // The message type is 16 bits. The message type implies both the size of the
    // payload, and the contents.  If the message type is not recognized, or the
    // implied size does not match the passed payload size, the packet is
    // considered in error.
    auto message_type = (MessageType) *(uint16_t*)(recv_buffer+message_type_ofs);
    // The payload size is a 16 bit number.  The maximum payload size is 1280 bytes. 
    payload_size = *(uint16_t*)(recv_buffer+payload_size_ofs);

    // lookup the expected size of the message
    auto expected_size = size(message_type);
    // and check if the passed size is correct for the message type
    if (expected_size < 0 || expected_size != payload_size)
    {
        // the message is bad: didnt pass type and size checks
        // go back to the start to look for a new message
        payload_size = 0;
        return (MessageType)-1;
    }

    // read those bytes, including the crc
    in.readBytes(recv_buffer+payload_ofs, payload_size+4);

    // check crc of buffer
    auto crc = crc32(~0UL, recv_buffer+payload_ofs, payload_size);
    auto crc_in_buffer = *(uint32_t*)(recv_buffer+payload_ofs+ payload_size+4);

    // if crc is bad, go back to the start
    if (crc != crc_in_buffer)
    {
        // the message is bad: didnt pass type and size checks
        // go back to the start to look for a new message
        payload_size = 0;
        return (MessageType)-1;
    }

    // return the message type
    return message_type;
}


/** Send a message to the head board.
    @param out the stream to send the message to
    @param payload_size the size of the payload
*/
void SendMessage(Stream& out, size_t payload_size)
{
    // send the message
    out.write(recv_buffer, payload_size+payload_ofs+4);
}

}

}
