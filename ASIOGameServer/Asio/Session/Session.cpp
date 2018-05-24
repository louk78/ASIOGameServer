#include "../../Global.h"
#include "Session.h"
#include "SessionManager.h"
#include "../../User/User.h"
#include "SessionState/InGameState.h"


Session::Session(tcp::socket socket)
	: socket_(std::move(socket)),
	timer_(socket_.get_io_service()),
	strand_(socket_.get_io_service())
{
}

Session::~Session()
{
	
}


void Session::start()
{
	do_read();
}

void Session::Init()
{
	current_send_buffer = nullptr;
	auto pThis = shared_from_this();
	ingameState = new InGameState(pThis);
	currentState = ingameState;
	com_recvSize = 0;
}



void Session::do_read()
{
	auto self(shared_from_this());
	strand_.post([this, self]()
	{
		socket_.async_read_some(asio::buffer(recv_buffer.buffer+ com_recvSize, BUFSIZE)
			, [this, self](std::error_code ec, std::size_t length)
		{
			printf(" read ���� ũ�� : %zd\n" , length);
			//���� ���� ��û
			if (length == 0)
			{
				//printf("Close\n");
				auto sm = WorkerGruop::Get_IOGroup()->GetRoot()->GetComponent<SessionManager>();
				auto func = std::make_shared<Function<std::shared_ptr<SessionManager>>>(
					[this, self](std::shared_ptr<SessionManager> sm)
				{
					sm->RemoveSocket(key);
				}, move(sm));
				WorkerGruop::Get_IOGroup()->PostFuction(move(func));
				return;
			}

			auto retval = RecvResult(length);
			
			switch (retval)
			{
			case EResult::LOW:				
				printf("���� ����\n");
				break;
			case EResult::COMPLETE:
				printf("���������� ����\n");
				//recv_buffer.packet.symbol
				currentState->On_Read(recv_buffer.packet.symbol,&recv_buffer.packet.buffer, recv_buffer.packet.size);
				com_recvSize = 0;
				break;
			case EResult::OVER:
				printf("Over�ؼ� ����\n");
				while (com_recvSize>recv_buffer.packet.size)
				{
					currentState->On_Read(recv_buffer.packet.symbol,&recv_buffer.packet.buffer, recv_buffer.packet.size);
					memmove(recv_buffer.buffer, recv_buffer.buffer + recv_buffer.packet.size, com_recvSize - recv_buffer.packet.size);
					com_recvSize -= recv_buffer.packet.size;
				}				
				break;
			}
			do_read();
		});
	});
}

//
void Session::do_write()
{
	auto self(shared_from_this());

	strand_.post([this, self]()
	{

		if (current_send_buffer == nullptr)
		{
			if (send_buffers.empty() == true)
			{
				return;
			}

			else
			{
				current_send_buffer = send_buffers.front();
			}
		}

		socket_.async_write_some(asio::buffer(
			&current_send_buffer->buffer.start_pointer + current_send_buffer->com_sendSize,
			current_send_buffer->buffer.packet.size - current_send_buffer->com_sendSize)
		,[this, self](std::error_code ec, std::size_t length)
		{
			auto retval = SendResult(*current_send_buffer, length);
			switch (retval)
			{
			case EResult::LOW:
				do_write();
				break;
			case EResult::COMPLETE:
				send_buffers.pop();
				delete current_send_buffer;
				current_send_buffer = nullptr;
				do_write();
				break;
			}
		});
	});
}

EResult Session::RecvResult( std::size_t length)
{
	//���� ���� �����Ϳ� ������ �Ϸ�� �������� ũ�Ⱑ ���ٸ� low ����
	com_recvSize += length;
	if (com_recvSize < sizeof(unsigned short))
	{
		return EResult::LOW;
	}
	//�޾ƾ��� �����Ϳ� ���� �������� ���� ���� �� 
	if (recv_buffer.packet.size == com_recvSize)
	{
		return EResult::COMPLETE;
	}
	//�޾ƾ��� �������� ���� �� ���� ��
	else if (recv_buffer.packet.size > com_recvSize)
	{
		return EResult::LOW;
	}
	//��ġ�� �޾��� ��
	return EResult::OVER;
}

EResult Session::SendResult(SendBuffer& send_buffer,std::size_t length)
{
	send_buffer.com_sendSize += length;
	if (send_buffer.com_sendSize < sizeof(unsigned short))
	{
		return EResult::LOW;
	}
	//�޾ƾ��� �����Ϳ� ���� �������� ���� ���� �� 
	if (send_buffer.buffer.packet.size == send_buffer.com_sendSize)
	{
		return EResult::COMPLETE;
	}
	return EResult::LOW;
}

void Session::PushSend(const PS & symbol, void * buffer_pointer, short buffersize)
{
	auto self(shared_from_this());

	strand_.post([this, self, buffer_pointer, symbol, buffersize]
	{
		send_buffers.push(new SendBuffer(symbol, buffer_pointer, buffersize));
		do_write();
	});

}

void Session::PushSend(const PS & symbol, std::shared_ptr<flatbuffers::FlatBufferBuilder> fbb)
{
	auto self(shared_from_this());

	strand_.post([this, self, symbol,fbb]
	{
		send_buffers.push(new SendBuffer(symbol,move(fbb)));
		do_write();
	});
}

void Session::SetUser(std::shared_ptr<class User> _user)
{
	user = _user;
}
