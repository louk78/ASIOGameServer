#pragma once

union RecvBuffer
{
	uint8 buffer[BUFSIZE];
	struct RecvPacket
	{
		unsigned short size;
		PS symbol;
		uint8 buffer;
	} packet;
};

struct SendBuffer : public std::enable_shared_from_this<SendBuffer>
{
	SendBuffer(const PS& symbol, std::shared_ptr<flatbuffers::FlatBufferBuilder> fbb)
	{
		buffer.packet.size = fbb->GetSize() + sizeof(unsigned short) + sizeof(PS);

		buffer.packet.symbol = symbol;
		memcpy(buffer.packet.data_buffer, fbb->GetBufferPointer(), fbb->GetSize());
	}

	SendBuffer(const PS& symbol, void* buffer_pointer, short buffersize)
	{		
		buffer.packet.size = buffersize + sizeof(unsigned short) + sizeof(PS);

		buffer.packet.symbol = symbol;
		memcpy(buffer.packet.data_buffer, buffer_pointer, buffersize);
	}

	union SendPacket
	{
		uint8 start_pointer;
		struct Packet
		{
			unsigned short  size;
			PS symbol;
			uint8 data_buffer[1024];
		} packet;
	} buffer;

	//unsigned short com_sendSize;
};

class Func
{
public:
	virtual void func() = 0;
};

template<typename T>
class Function :public Func, public std::enable_shared_from_this<Function<T>>
{
public:
	Function(std::function<void(T)> fun, T param) :
		data(param), fun(fun)
	{}
	void func()
	{
		fun(data);
	}
private:
	std::function<void(T)> fun;
	T data;	
};


template<>
class Function<void>
{
public:
	Function(std::function<void()> fun)
		:fun(fun)
	{}
private:
	std::function<void()> fun;
	void func()
	{
		fun();
	}
};
