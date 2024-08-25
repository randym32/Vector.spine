#include <vector>
#include <cstdint>

#define Stream MockStream
#include "mockStream.h"
MockStream Serial;

#include "listener.cpp" // Include the file to test
#include <CppUnitTest.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(ListenerTests)
{
public:

    TEST_METHOD(TestProcessAck)
    {
        Ack ack;
        // Initialize ack if needed
        Assert::IsFalse(process(ack)); // Expecting false as no modification
    }

    TEST_METHOD(TestProcessDataCharacter)
    {
        DataCharacter dataChar;
        // Initialize dataChar.text with some values
        strcpy(dataChar.text, "Test");
        Assert::IsFalse(process(dataChar)); // Expecting false as no modification
    }

    TEST_METHOD(TestProcessB2HDataFrame)
    {
        B2HDataFrame frame;
        // Initialize frame if needed
        Assert::IsFalse(process(frame)); // Expecting false as no modification
    }

    TEST_METHOD(TestProcessBody2HeadAck)
    {
        MessageType msgType = MessageType::ack;
        Assert::IsFalse(processBody2Head(msgType)); // Expecting false as no modification
    }

    TEST_METHOD(TestProcessBody2HeadDataCharacter)
    {
        MessageType msgType = MessageType::dataCharacter;
        Assert::IsFalse(processBody2Head(msgType)); // Expecting false as no modification
    }

    TEST_METHOD(TestProcessBody2HeadDataFrame)
    {
        MessageType msgType = MessageType::dataFrame;
        Assert::IsFalse(processBody2Head(msgType)); // Expecting false as no modification
    }

};
