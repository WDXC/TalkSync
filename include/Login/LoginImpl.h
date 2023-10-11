#ifndef LOGIN_MODULE_IMPL_H
#define LOGIN_MODULE_IMPL_H

#include "LoginInterface.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <string>
#include <unordered_map>

class LoginModuleImpl : public LoginModule {
public:
    LoginModuleImpl(const std::string& dbPath);
    virtual ~LoginModuleImpl();
    virtual bool AuthenticateUser(const std::string& username, const std::string& password) override;
    virtual std::string GenerateSessionId() override;

private:
    SQLite::Database* m_Db;
    std::string m_dbPath;
    std::unordered_map<std::string, std::string> user_credentials_;
};

#endif // LOGIN_MODULE_IMPL_H

