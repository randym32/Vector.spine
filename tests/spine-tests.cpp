#include <vector>
#include <cstdint>

#define Stream MockStream
#include "mockStream.h"

#include "../src/spine.cpp"

#include <CppUnitTest.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Spine;
namespace LE {
/** Decodes a 16-bit unsigned number from the buffer using LSB order.
    @param buffer  Pointer to where the decoded value shall be fetched from.
    @return The decoded value.
*/
uint16_t inline uint16(const uint8_t* buffer)
{
    uint16_t Ret = buffer[0];
    Ret |= ((uint16_t)buffer[1]) << 8;
    return Ret;
}
/** Decodes a 32-bit unsigned number from the buffer using little-endian order.
    @param buffer  Pointer to where the decoded value shall be fetched from.
    @return The decoded value.
*/
uint32_t uint32(const uint8_t* buffer)
{
    uint32_t Ret = uint16(buffer);
    Ret |= uint16(buffer+2) << 16;
    return Ret;
}
}
TEST_CLASS(SpineTests)
{
public:


// Test for each MessageType
    TEST_METHOD(TestPopulateHeader_H2B_DataCharacter)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::dataCharacter;

        size_t size = H2B::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 32, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::dataCharacter, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(32), LE::uint16(buffer + payload_size_ofs));
    }

    TEST_METHOD(TestPopulateHeader_H2B_DataFrame)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::dataFrame;

        size_t size = H2B::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 64, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::dataFrame, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(64), LE::uint16(buffer + payload_size_ofs));
    }

    TEST_METHOD(TestPopulateHeader_H2B_shutdownMessage)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::shutdown;

        size_t size = H2B::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 0, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::shutdown, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual((uint16_t)0, LE::uint16(buffer + payload_size_ofs));
    }


// Test for each MessageType in B2H
    TEST_METHOD(TestPopulateHeader_B2H_UpdateFirmware)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::updateFirmware;

        size_t size = B2H::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 32, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::updateFirmware, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(32), LE::uint16(buffer + payload_size_ofs));
    }

    TEST_METHOD(TestPopulateHeader_B2H_DataFrame)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::dataFrame;

        size_t size = B2H::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 768, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::dataFrame, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(768), LE::uint16(buffer + payload_size_ofs));
    }

    TEST_METHOD(TestPopulateHeader_B2H_BootFrame)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::bootFrame;

        size_t size = B2H::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 0, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::bootFrame, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(0), LE::uint16(buffer + payload_size_ofs));
    }

    TEST_METHOD(TestPopulateHeader_Ack)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::ack;

        size_t size = B2H::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 4, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::ack, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(4), LE::uint16(buffer + payload_size_ofs)); 
    }

    TEST_METHOD(TestPopulateHeader_Version)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::version;

        size_t size = B2H::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 40, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::version, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(40), LE::uint16(buffer + payload_size_ofs)); 
    }

    TEST_METHOD(TestPopulateHeader_Validate)
    {
        uint8_t buffer[12];
        MessageType messageType = MessageType::validate;

        size_t size = B2H::populateHeader(buffer, messageType);

        Assert::AreEqual((size_t) 0, size);
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);
        Assert::AreEqual((uint16_t)MessageType::validate, LE::uint16(buffer + message_type_ofs));
        Assert::AreEqual(static_cast<uint16_t>(0), LE::uint16(buffer + payload_size_ofs));
    }


    TEST_METHOD(TestB2H_DataCharacterMsg)
    {
        MockStream mockStream;
        const char* testText = "Hello, World!";
        size_t numBytes = strlen(testText);

        // Call the function to create a DataCharacter message
        size_t messageSize = B2H::DataCharacterMsg(testText, (int) numBytes);

        // Check the size of the message
        Assert::AreEqual((size_t) 32, messageSize); // Adjust based on expected size

        // Check the contents of the buffer
        uint8_t* buffer = B2H::recv_buffer; // Access the buffer directly for testing
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[3]);

        // Check the text in the payload
        DataCharacter* dataChar = reinterpret_cast<DataCharacter*>(buffer + payload_ofs);
        Assert::AreEqual(testText, dataChar->text); // Check if the text matches
        Assert::AreEqual('\0', dataChar->text[numBytes]); // Check null termination

        // Check the CRC
        auto expectedCrc = crc32(~0UL, buffer + payload_ofs, (int) messageSize);
        auto actualCrc = LE::uint32(buffer + payload_ofs + messageSize + 4);
        Assert::AreEqual(expectedCrc, actualCrc);
    }

    TEST_METHOD(TestH2B_DataCharacterMsg)
    {
        MockStream mockStream;
        const char* testText = "Hello, H2B!";
        size_t numBytes = strlen(testText);

        // Call the function to create a DataCharacter message
        size_t messageSize = H2B::DataCharacterMsg(testText, (int) numBytes);

        // Check the size of the message
        Assert::AreEqual(static_cast<size_t>(32), messageSize); // Adjust based on expected size

        // Check the contents of the buffer
        uint8_t* buffer = H2B::recv_buffer; // Access the buffer directly for testing
        Assert::AreEqual(static_cast<uint8_t>(0xAA), buffer[0]);
        Assert::AreEqual(static_cast<uint8_t>('H'), buffer[1]);
        Assert::AreEqual(static_cast<uint8_t>('2'), buffer[2]);
        Assert::AreEqual(static_cast<uint8_t>('B'), buffer[3]);

        // Check the text in the payload
        DataCharacter* dataChar = reinterpret_cast<DataCharacter*>(buffer + payload_ofs);
        Assert::AreEqual(testText, dataChar->text); // Check if the text matches
        Assert::AreEqual('\0', dataChar->text[numBytes]); // Check null termination

        // Check the CRC
        auto expectedCrc = crc32(~0UL, buffer + payload_ofs, (int) messageSize);
        auto actualCrc = LE::uint32(buffer + payload_ofs + messageSize + 4);
        Assert::AreEqual(expectedCrc, actualCrc);
    }

    // Test receiving a valid message from the head board to the body board

    /// Test Method for Valid Message:
    /// This test simulates receiving a valid message with the correct sync bytes, message type, payload size, and CRC.
    /// It checks that the received message type matches the expected value.
    TEST_METHOD(TestH2B_ReceiveMessage_ValidMessage)
    {
        MockStream mockStream;
        uint8_t validMessage[] = {
            0xAA, 'H', '2', 'B', // Sync bytes
            0x64, 0x63, // Message type dataCharacter
            32, 0, // Payload size (32)
            // Payload (example data)
            'H', 'e', 'l', 'l', 'o', ' ', 'H', '2', 'B', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // CRC (example, should be calculated based on the payload)
            0xDE, 0xAD, 0xBE, 0xEF // Placeholder CRC
        };

        // Set the buffer for the mock stream
        mockStream.setBuffer(std::vector<uint8_t>(validMessage, validMessage + sizeof(validMessage)));

        // Call the function to receive a message
        size_t payload_size = 0;
        MessageType result = H2B::ReceiveMessage(mockStream, payload_size);

        // Check that the received message type is as expected
        Assert::AreEqual((int)MessageType::dataCharacter, (int)result); 
        Assert::AreEqual((size_t)32, payload_size);
    }

    /// Test Method for Invalid Message Type:
    /// This test simulates receiving a message with an invalid message type.
    /// It checks that the function returns an invalid message type.
    TEST_METHOD(TestH2B_ReceiveMessage_InvalidMessageType)
    {
        MockStream mockStream;
        uint8_t invalidMessage[] = {
            0xAA, 'H', '2', 'B', // Sync bytes
            0x00, 0xFF, // Invalid message type (255)
            0x00, 0x20, // Payload size (32)
            // Payload (example data)
            'H', 'e', 'l', 'l', 'o', ' ', 'H', '2', 'B', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // CRC (example, should be calculated based on the payload)
            0xDE, 0xAD, 0xBE, 0xEF // Placeholder CRC
        };

        // Set the buffer for the mock stream
        mockStream.setBuffer(std::vector<uint8_t>(invalidMessage, invalidMessage + sizeof(invalidMessage)));

        // Call the function to receive a message
        size_t payload_size = 0;
        MessageType result = H2B::ReceiveMessage(mockStream, payload_size);

        // Check that the received message type is invalid
        Assert::AreEqual(-1, (int) result);
        Assert::AreEqual((size_t)0, payload_size);
    }

    /// Test Method for CRC Error:
    /// This test simulates receiving a message with a correct structure but an incorrect CRC.
    /// It checks that the function returns an invalid message type due to the CRC error.
    TEST_METHOD(TestH2B_ReceiveMessage_CRCError)
    {
        MockStream mockStream;
        uint8_t messageWithErrorCRC[] = {
            0xAA, 'H', '2', 'B', // Sync bytes
            0x00, 0x01, // Message type (1)
            0x00, 0x20, // Payload size (32)
            // Payload (example data)
            'H', 'e', 'l', 'l', 'o', ' ', 'H', '2', 'B', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // Incorrect CRC
            1, 2, 3, 4 // Incorrect CRC
        };

        // Set the buffer for the mock stream
        mockStream.setBuffer(std::vector<uint8_t>(messageWithErrorCRC, messageWithErrorCRC + sizeof(messageWithErrorCRC)));

        // Call the function to receive a message
        size_t payload_size = 0;
        MessageType result = H2B::ReceiveMessage(mockStream, payload_size);


        // Check that the received message type is invalid due to CRC error
        Assert::AreEqual(-1, (int) result);
        Assert::AreEqual((size_t)0, payload_size);
    }

    // Test receiving a valid message from the body board to the head board
    /// @brief Test Method for Valid Message:
    /// This test simulates receiving a valid message with the correct sync bytes, message type, payload size, and CRC.
    /// It checks that the received message type matches the expected value.
    TEST_METHOD(TestB2H_ReceiveMessage_ValidMessage)
    {
        MockStream mockStream;
        uint8_t validMessage[] = {
            0xAA, 'B', '2', 'H', // Sync bytes
            0x64, 0x63, // Message type dataCharacter
            32, 0x00, // Payload size (32)
            // Payload (example data)
            'H', 'e', 'l', 'l', 'o', ' ', 'B', '2', 'H', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // CRC (example, should be calculated based on the payload)
            0, 0, 0, 0 // Placeholder CRC
        };

        // Set the buffer for the mock stream
        mockStream.setBuffer(std::vector<uint8_t>(validMessage, validMessage + sizeof(validMessage)));

        // Call the function to receive a message
        size_t payload_size = 0;
        MessageType result = B2H::ReceiveMessage(mockStream, payload_size);

        // Check that the received message type is as expected
        Assert::AreEqual((int)MessageType::dataCharacter, (int)result);
        Assert::AreEqual((size_t)32, payload_size);
    }

    /// @brief Test Method for Invalid Message Type:
    /// This test simulates receiving a message with an invalid message type.
    /// It checks that the function returns an invalid message type.
    TEST_METHOD(TestB2H_ReceiveMessage_InvalidMessageType)
    {
        MockStream mockStream;
        uint8_t invalidMessage[] = {
            0xAA, 'B', '2', 'H', // Sync bytes
            0x00, 0xFF, // Invalid message type (255)
            0x00, 0x20, // Payload size (32)
            // Payload (example data)
            'H', 'e', 'l', 'l', 'o', ' ', 'B', '2', 'H', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // CRC (example, should be calculated based on the payload)
            0, 0, 0, 0 // Placeholder CRC
        };

        // Set the buffer for the mock stream
        mockStream.setBuffer(std::vector<uint8_t>(invalidMessage, invalidMessage + sizeof(invalidMessage)));

        // Call the function to receive a message
        size_t payload_size = 0;
        MessageType result = B2H::ReceiveMessage(mockStream, payload_size);

        // Check that the received message type is invalid
        Assert::AreEqual(-1, (int) result);
        Assert::AreEqual((size_t)0, payload_size);
    }

    /// @brief Test Method for CRC Error:
    /// This test simulates receiving a message with a correct structure but an incorrect CRC.
    /// It checks that the function returns an invalid message type due to the CRC error.
    TEST_METHOD(TestB2H_ReceiveMessage_CRCError)
    {
        MockStream mockStream;
        uint8_t messageWithErrorCRC[] = {
            0xAA, 'B', '2', 'H', // Sync bytes
            0x00, 0x01, // Message type (1)
            0x00, 0x20, // Payload size (32)
            // Payload (example data)
            'H', 'e', 'l', 'l', 'o', ' ', 'B', '2', 'H', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // Incorrect CRC
            1, 2, 3, 4 // Incorrect CRC
        };

        // Set the buffer for the mock stream
        mockStream.setBuffer(std::vector<uint8_t>(messageWithErrorCRC, messageWithErrorCRC + sizeof(messageWithErrorCRC)));

        // Call the function to receive a message
        size_t payload_size = 0;
        MessageType result = B2H::ReceiveMessage(mockStream, payload_size);

        // Check that the received message type is invalid due to CRC error
        Assert::AreEqual(-1, (int) result);
        Assert::AreEqual((size_t)0, payload_size);
    }


    /// @brief Test Method for Sending a Message:
    /// This test simulates sending a message from the body board to the head board.
    /// It checks that the message is sent correctly.
    TEST_METHOD(TestB2H_SendMessage)
    {
        MockStream mockStream;
        uint8_t testMessage[32]; // Example message buffer
        // Populate the test message with some data
        testMessage[0] = 0xAA; // Sync byte
        testMessage[1] = 'B';
        testMessage[2] = '2';
        testMessage[3] = 'H';
        // Fill the rest of the message as needed...

        // Set the buffer for the B2H recv_buffer
        memcpy(B2H::recv_buffer, testMessage, sizeof(testMessage));

        // Call the function to send the message
        B2H::SendMessage(mockStream, sizeof(testMessage));

        // Check that the message was sent correctly
        uint8_t sentBuffer[32];
        mockStream.readBytes(sentBuffer, sizeof(sentBuffer));

        // Verify the sent message matches the original test message
        for (size_t i = 0; i < sizeof(testMessage); ++i)
        {
            Assert::AreEqual(testMessage[i], sentBuffer[i]);
        }
    }

    /// @brief Test Method for Sending a Message:
    /// This test simulates sending a message from the head board to the body board.
    /// It checks that the message is sent correctly.
    TEST_METHOD(TestH2B_SendMessage)
    {
        MockStream mockStream;
        uint8_t testMessage[32]; // Example message buffer
        // Populate the test message with some data
        testMessage[0] = 0xAA; // Sync byte
        testMessage[1] = 'H';
        testMessage[2] = '2';
        testMessage[3] = 'B';
        // Fill the rest of the message as needed...

        // Set the buffer for the H2B recv_buffer
        memcpy(H2B::recv_buffer, testMessage, sizeof(testMessage));

        // Call the function to send the message
        H2B::SendMessage(mockStream, sizeof(testMessage));

        // Check that the message was sent correctly
        uint8_t sentBuffer[32];
        mockStream.readBytes(sentBuffer, sizeof(sentBuffer));

        // Verify the sent message matches the original test message
        for (size_t i = 0; i < sizeof(testMessage); ++i)
        {
            Assert::AreEqual(testMessage[i], sentBuffer[i]);
        }
    }
};
