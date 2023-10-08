#ifndef LOGIN_MODULE_H
#define LOGIN_MODULE_H

#include <string>

class LoginModule {
public:
    virtual ~LoginModule() {}
    virtual bool AuthenticateUser(const std::string& username, const std::string& password) = 0;
    virtual std::string GenerateSessionId() = 0;
};

#endif // LOGIN_MODULE_H

