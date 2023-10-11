#include "LoginImpl.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <openssl/sha.h>

LoginModuleImpl::LoginModuleImpl(const std::string& dbPath) : m_dbPath(dbPath) {
    // 在实际应用中，初始化用户凭据（用户名和哈希密码）的数据结构
    // 这里为示例，使用硬编码的用户名和密码
    m_Db = new SQLite::Database(m_dbPath);
    std::cout << "SQLite database file '" << m_Db->getFilename().c_str() << "' opened successfully\n";

    SQLite::Statement query(*m_Db, "select username from users where user_id = ?");
    std::cout << "SQLite statement '" << query.getQuery().c_str() << "' compiled (" << query.getColumnCount() << " columns in the result)\n";
    query.bind(1, 1);
    while (query.executeStep())
    {
        const std::string username  = query.getColumn(0); // = query.getColumn(1).getText();
        const int         bytes  = query.getColumn(0).size(); // .getColumn(1).getBytes();
        std::cout << "row (" << username.c_str() << "\"(" << bytes << "))\n";
    }
    query.reset();

}

LoginModuleImpl::~LoginModuleImpl() {}

bool LoginModuleImpl::AuthenticateUser(const std::string& username, const std::string& password) {
    // 从user_credentials_中获取存储的哈希密码
    std::string stored_password = user_credentials_[username];
    
    // 在实际应用中，你应该使用密码哈希函数（如SHA-256）对用户提供的密码进行哈希，然后与存储的哈希密码进行比较
    // 这里为示例，假设密码已经哈希
    return (password == stored_password);
}

std::string LoginModuleImpl::GenerateSessionId() {
    // 在实际应用中，你应该生成一个随机的、唯一的会话ID
    // 这里为示例，使用当前时间戳作为会话ID
    time_t current_time = time(nullptr);
    std::stringstream session_id;
    session_id << "session_" << current_time;
    return session_id.str();
}

