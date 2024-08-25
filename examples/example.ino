/**@file
    @brief The body board to head board communication.

    Allows listening, and sending messages to the head board..
 */
#include <Arduino.h>
#include "spine.h"
#include "listener.h"

using namespace Spine;


// populate these with the correct pins
// receive from the body board
#define RXD1 16
// send to the body board
#define TXD1 17
// receive from the head board
#define RXD2 18
// send to the head board
#define TXD2 19

/** setup the serial port for the body board
 
    Note:

    - Serial1 is the serial port used to receive messages from the body board.
    - Serial2 is the serial port used to send messages to the body board.

    Define the serial pins RXD1, TXD1, RXD2, TXD2.
 */
void setup()
{
    // start serial
    // From the body board, we have two serial ports:
    Serial1.begin(3000000, SERIAL_8N1, RXD1, TXD1);
    Serial1.setTimeout(100);
    Serial1.setRxBufferSize(2048);
    Serial1.setTxBufferSize(2048);

    // To the head board is 3000000 baud, 8N1, RXD, TXD
    Serial2.begin(3000000, SERIAL_8N1, RXD2, TXD2);
    Serial2.setTimeout(100);
    Serial2.setRxBufferSize(2048);
    Serial2.setTxBufferSize(2048);
}



/** The main task for the body board.

    This task will keep receiving and processing messages from the head board.

    What this isnt good at is sending messages to the head board on its own.

    I made a bridge between the USB serial and the DataCharacter message --
    the commands might do something interesting.
 */
void loop()
{
    // keep receiving and processing messages
    // receive and process message
    ReceiveAndRewriteB2HMessage(Serial1, Serial2);

    // See if there is USB serial data to forward to the head board
    auto numBytes = std::min(Serial.available(), 31);
    if (numBytes > 0)
    {
        // send to head board
        // Populate the text field
        char text[32];
        Serial.readBytes(text, numBytes);
        // Create a DataCharacter message
        auto payload_size = B2H::DataCharacterMsg(text, numBytes);

        // Send the message to the head board
        B2H::SendMessage(Serial2, payload_size);
    }
}
