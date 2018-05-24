#pragma once

class Session : public std::enable_shared_from_this<Session> 
{
public:
	friend class SessionManager;
	
	Session(tcp::socket socket);
	~Session();
	
	void start();
public:
	void Init();
	
	void SetUser(std::shared_ptr<class User> user);

	//POST
	void PushSend(const PS& symbol, void* buffer_pointer, short buffersize);
	void PushSend(const PS & symbol, std::shared_ptr<flatbuffers::FlatBufferBuilder> fbb);
	std::weak_ptr<class User> GetUser() { return user; }

private:
	int key;
	//������ �ۼ����ϴ� �Լ�
	void do_read();
	void do_write();
	

	//Recv��� ó�� �� buffer��Ʈ�� �ϴ� �Լ�
	EResult RecvResult(std::size_t length);
	EResult SendResult(SendBuffer& send_buffer, std::size_t length);

	tcp::socket socket_;					//�ڽ��� ����
	asio::steady_timer timer_;				//Ÿ�̸�.
	asio::io_service::strand strand_;	//��Ʈ����

	std::weak_ptr<class User> user;
	
	RecvBuffer recv_buffer;
	unsigned short com_recvSize;

	std::queue<SendBuffer*> send_buffers;
	SendBuffer* current_send_buffer;
private:
	//State
	class SessionState* currentState;
	class InGameState* ingameState;

};