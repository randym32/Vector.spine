/* Helper to process messages between the body board and the head board.
   Copyright 2024 Randall Maas
*//**@file
    @brief Helper to process messages between the body board and the head board.

    Allows listening, and sending messages to the head board.
 */
#pragma once

/** Process ack message from the body board to the head board
 
    1. process message fields
    2. update message fields, if needed
    3. return true if the message was modified (thus needs a new CRC), false if not.
*/
bool process(Ack& value);


/** Process data character message from the body board to the head board
 
    1. process message fields
    2. update message fields, if needed
    3. return true if the message was modified (thus needs a new CRC), false if not.
*/
bool process(DataCharacter& value);


/** Process data frame message from the body board to the head board
    @param frame the data frame message
    @return true if the message was modified (thus needs a new CRC), false if not.
 
    1. process message fields
    2. update message fields, if needed
    3. return true if the message was modified (thus needs a new CRC), false if not.
*/
bool process(B2HDataFrame& frame);


/** Rewrite a message from the body board and send it to the head board.
    @param in the stream to receive the message from
    @param out the stream to send the message to

 */
void ReceiveAndRewriteB2HMessage(Stream& in, Stream& out);
