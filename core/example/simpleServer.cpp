#include <iostream>
#include "TCPEpollServer.h"
#include "IQuestProcessor.h"
#include "TaskThreadPool.h"
#include "msec.h"
#include "Setting.h"

using namespace std;
using namespace fpnn;


class QuestProcessor: public IQuestProcessor
{

	QuestProcessorClassPrivateFields(QuestProcessor)

public:

	//-- oneway
	FPAnswerPtr demoOneway(const FPReaderPtr args, const FPQuestPtr quest, const ConnectionInfo& ci)
	{
		cout<<"One way method demo is called. socket: "<<ci.socket<<", address: "<<ci.ip<<":"<<ci.port<<endl;
		cout<<"quest data: "<<(quest->json())<<endl;
		
		//do something
		
		return NULL;
	}
	//-- towway
	FPAnswerPtr demoTowway(const FPReaderPtr args, const FPQuestPtr quest, const ConnectionInfo& ci)
	{
		cout<<"Tow way method demo is called. socket: "<<ci.socket<<", address: "<<ci.ip<<":"<<ci.port<<endl;
		cout<<"quest data: "<<(quest->json())<<endl;
        cout<<"get=>quest:"<<args->get("quest", string(""))<<endl;
        cout<<"get=>int:"<<args->get("int", (int)0)<<endl;
        cout<<"get=>double:"<<args->get("double", (double)0)<<endl;
        cout<<"get=>boolean:"<<args->get("boolean", (bool)0)<<endl;
        tuple<string, int> tup;
        tup = args->get("ARRAY", tup);
        cout<<"get=>array_tuple:"<<std::get<0>(tup)<<"  "<<std::get<1>(tup)<<endl;
		
		OBJECT obj = args->getObject("MAP");
		FPReader fpr(obj);
		cout<<"map1=>"<<fpr.get("map1",string(""))<<endl;
		cout<<"map2=>"<<fpr.get("map2",(bool)false)<<endl;
		cout<<"map3=>"<<fpr.get("map3",(int)0)<<endl;
		cout<<"map4=>"<<fpr.get("map4",(double)0.0)<<endl;
		cout<<"map5=>"<<fpr.get("map5",string(""))<<endl;
		try{
			cout<<"WANT:"<<fpr.want("map4", (double)0.0)<<endl;
			cout<<"WANT:"<<fpr.want("map4", string(""))<<endl;
		}
		catch(...){
			cout<<"EXECPTION: double value, but want string value"<<endl;
		}
		

		//Do some thing
		//

		//return
		FPAWriter aw(6, quest);
		aw.param("answer", "one");
		aw.param("int", 2);
		aw.param("double", 3.3);
		aw.param("boolean", true);
		aw.paramArray("ARRAY",2);
		aw.param(make_tuple ("tuple1", 3.1, 14, false));
		aw.param(make_tuple ("tuple2", 5.7, 9, true));
		aw.paramMap("MAP",5);
		aw.param("m1","first_map");
		aw.param("m2",true);
		aw.param("m3",5);
		aw.param("m4",5.7);
		aw.param("m5","中文2");	 
		return aw.take();
	}

	QuestProcessor()
	{
		registerMethod("one way demo", &QuestProcessor::demoOneway);
		registerMethod("two way demo", &QuestProcessor::demoTowway);

	}

	~QuestProcessor(){
	}

	QuestProcessorClassBasicPublicFuncs
};

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout<<"Usage: "<<argv[0]<<" config"<<endl;
		return 0;
	}
	if(!Setting::load(argv[1])){
		cout<<"Config file error:"<< argv[1]<<endl;
		return 1;
	}

	ServerPtr server = TCPEpollServer::create();
	server->setQuestProcessor(std::make_shared<QuestProcessor>());
	server->startup();
	server->run();

	return 0;
}

/*
要与 

simpleServer.cpp

 中的服务器进行通信并验证其功能，可以编写一个客户端程序来发送请求并接收响应。假设服务器使用 TCP 协议，并且已经实现了 [`demoOneway`](command:_github.copilot.openSymbolFromReferences?%5B%22%22%2C%5B%7B%22uri%22%3A%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2Fworkspaces%2FMyDbproxy%2Ffpnn%2Fcore%2Fexample%2FsimpleServer.cpp%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%2C%22pos%22%3A%7B%22line%22%3A80%2C%22character%22%3A50%7D%7D%5D%2C%2251a93a69-4434-48d1-b3b7-11bba18f67d6%22%5D "Go to definition") 和 [`demoTowway`](command:_github.copilot.openSymbolFromReferences?%5B%22%22%2C%5B%7B%22uri%22%3A%7B%22scheme%22%3A%22file%22%2C%22authority%22%3A%22%22%2C%22path%22%3A%22%2Fworkspaces%2FMyDbproxy%2Ffpnn%2Fcore%2Fexample%2FsimpleServer.cpp%22%2C%22query%22%3A%22%22%2C%22fragment%22%3A%22%22%7D%2C%22pos%22%3A%7B%22line%22%3A81%2C%22character%22%3A50%7D%7D%5D%2C%2251a93a69-4434-48d1-b3b7-11bba18f67d6%22%5D "Go to definition") 方法。

以下是一个简单的客户端示例，展示了如何与服务器进行通信：

### 客户端程序：simpleClient.cpp

```cpp
#include <iostream>
#include <string>
#include <memory>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

class TCPClient {
public:
    TCPClient(const string& serverIP, int serverPort);
    ~TCPClient();
    bool connectToServer();
    bool sendRequest(const string& request);
    string receiveResponse();

private:
    string serverIP;
    int serverPort;
    int sockfd;
};

TCPClient::TCPClient(const string& serverIP, int serverPort)
    : serverIP(serverIP), serverPort(serverPort), sockfd(-1) {}

TCPClient::~TCPClient() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

bool TCPClient::connectToServer() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        return false;
    }

    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        return false;
    }

    return true;
}

bool TCPClient::sendRequest(const string& request) {
    if (send(sockfd, request.c_str(), request.size(), 0) < 0) {
        perror("send");
        return false;
    }
    return true;
}

string TCPClient::receiveResponse() {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived < 0) {
        perror("recv");
        return "";
    }
    return string(buffer);
}

int main() {
    string serverIP = "127.0.0.1"; // 服务器 IP 地址
    int serverPort = 12345; // 服务器端口号

    TCPClient client(serverIP, serverPort);
    if (!client.connectToServer()) {
        cerr << "Failed to connect to server" << endl;
        return 1;
    }

    // 发送请求并接收响应
    string request = "two way demo"; // 示例请求
    if (!client.sendRequest(request)) {
        cerr << "Failed to send request" << endl;
        return 1;
    }

    string response = client.receiveResponse();
    cout << "Received response: " << response << endl;

    return 0;
}
```

### 解释

1. **TCPClient 类**：
   - `TCPClient` 类封装了与服务器的连接、发送请求和接收响应的功能。
   - 构造函数初始化服务器 IP 地址和端口号。
   - `connectToServer` 方法用于连接到服务器。
   - `sendRequest` 方法用于发送请求到服务器。
   - `receiveResponse` 方法用于接收服务器的响应。

2. **main 函数**：
   - 创建 `TCPClient` 实例并连接到服务器。
   - 发送示例请求 `"two way demo"` 并接收响应。
   - 输出接收到的响应。

### 编译和运行

确保服务器正在运行，然后编译并运行客户端程序：

```sh
g++ -o simpleClient simpleClient.cpp
./simpleClient
```

通过这种方式，你可以与 

simpleServer.cpp

 中的服务器进行通信，并验证其功能。根据服务器的实现，调整请求的格式和内容，以匹配服务器的预期输入。
*/