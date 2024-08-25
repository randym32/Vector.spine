/// Mock stream for testing 
class MockStream
{
public:
    MockStream() : readIndex(0) {}

    // Simulate writing to the stream
    void write(const uint8_t* data, size_t size)
    {
        buffer.insert(buffer.end(), data, data + size);
    }

    // Simulate reading from the stream
    int read()
    {
        if (readIndex < buffer.size())
        {
            return buffer[readIndex++];
        }
        return -1; // Indicate end of stream
    }

    // Simulate reading multiple bytes
    void readBytes(uint8_t* outBuffer, size_t size)
    {
        for (size_t i = 0; i < size && readIndex < buffer.size(); ++i)
        {
            outBuffer[i] = buffer[readIndex++];
        }
    }

    // Set the buffer for testing
    void setBuffer(const std::vector<uint8_t>& data)
    {
        buffer = data;
        readIndex = 0; // Reset read index
    }

    // Clear the buffer
    void clear()
    {
        buffer.clear();
        readIndex = 0;
    }

private:
    std::vector<uint8_t> buffer;
    size_t readIndex;
};
