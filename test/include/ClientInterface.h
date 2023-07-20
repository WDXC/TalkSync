#ifndef _CLIENTINTERFACE_H_
#define _CLIENTINTERFACE_H_

#include <string>
class ClientInterface {
public:
    virtual ~ClientInterface() {}
    virtual bool Connect(const std::string& serverAddress, int port) = 0;
    virtual void Disconnect() = 0;
    virtual bool SendData(const std::string& data) = 0;
    virtual std::string GetMsg() = 0;
};

#endif
