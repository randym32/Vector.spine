/* Helper to process messages between the body board and the head board.
   Copyright 2024 Randall Maas
*//**@file
    @brief Helper to process messages between the body board and the head board.

    Allows listening, and sending messages to the head board.
 */
#include <Arduino.h>
#include <esp32/rom/crc.h>
#include "spine.h"
// not sure if it should be crc32_be or crc32_le
#define crc32 crc32_le

using namespace Spine;


/** Process ack message from the body board to the head board
 
    1. process message fields
    2. update message fields, if needed
    3. return true if the message was modified (thus needs a new CRC), false if not.
*/
bool process(Ack& value)
{
    // return true if the message was modified (thus needs a new CRC), false if not.
    return false;
}


/** Process data character message from the body board to the head board
 
    1. process message fields
    2. update message fields, if needed
    3. return true if the message was modified (thus needs a new CRC), false if not.
*/
bool process(DataCharacter& value)
{
    // print the text
    for (int idx = 0; idx < 32 && value.text[idx] != 0; idx++)
    {
        Serial.write(value.text[idx]);
    }

    // return true if the message was modified (thus needs a new CRC), false if not.
    return false;
}


/** Process data frame message from the body board to the head board
    @param frame the data frame message
    @return true if the message was modified (thus needs a new CRC), false if not.
 
    1. process message fields
    2. update message fields, if needed
    3. return true if the message was modified (thus needs a new CRC), false if not.
*/
bool process(B2HDataFrame& frame)
{
    // return true if the message was modified (thus needs a new CRC), false if not.
    return false;
}

/** Process a received message.
    @param msg_type the type of the message
    @return true if the message was modified (thus needs a new CRC), false if not.

    This dispatch function is used to call the appropriate processing function
    for each message type. It uses a switch statement to determine the
    appropriate action for each message type.

    You can implement your own processing for each message type.
*/
bool processBody2Head(MessageType msg_type)
{
    // make decision on what the message type
    switch (msg_type)
    {
        default                         : break;
        case MessageType::ack           : return process(((Ack*)(B2H::recv_buffer+payload_ofs))[0]);
        case MessageType::dataCharacter : return process(((DataCharacter*)(B2H::recv_buffer+payload_ofs))[0]);
        case MessageType::dataFrame     : return process(((B2HDataFrame*)(B2H::recv_buffer+payload_ofs))[0]);
        case MessageType::bootFrame     : break;
        case MessageType::updateFirmware: break;
        case MessageType::version       : break;
        case MessageType::validate      : break;
    }

    // Not modified
    return false;
}


/** Rewrite a message from the body board and send it to the head board.
    @param in the stream to receive the message from
    @param out the stream to send the message to

 */
void ReceiveAndRewriteB2HMessage(Stream& in, Stream& out)
{
    // wait for a message
    size_t payload_size = 0;
    auto msg_type = B2H::ReceiveMessage(in, payload_size);

    // process the message
    processBody2Head(msg_type);

    // calculate new crc
    auto crc = crc32(~0U, B2H::recv_buffer+payload_ofs, payload_size);
    *(uint32_t*)(B2H::recv_buffer+payload_ofs+ payload_size+4) = crc;

    // send to head board
    out.write(B2H::recv_buffer, payload_size+payload_ofs+4);
}



