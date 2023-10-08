#ifndef LOGIN_MODULE_IMPL_H
#define LOGIN_MODULE_IMPL_H

#include "LoginInterface.h"
#include <string>
#include <unordered_map>

class LoginModuleImpl : public LoginModule {
public:
    LoginModuleImpl();
    virtual ~LoginModuleImpl();
    virtual bool AuthenticateUser(const std::string& username, const std::string& password) override;
    virtual std::string GenerateSessionId() override;

private:
    std::unordered_map<std::string, std::string> user_credentials_;
};

#endif // LOGIN_MODULE_IMPL_H

