#pragma once

#include <string>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/sqlstring.h>

class MySqlException : public std::exception {
public:
	MySqlException() : std::exception("MySQL error!") {}
	MySqlException(const std::string& msg) : std::exception(msg.c_str()) {}
};

class Database {
private:
	static sql::Driver *driver;
	static sql::Connection *con;
public:
	static void Connect(const std::string& host, const std::string& database, const std::string& username, const std::string& password);
	static void Destroy();
	static sql::Statement* CreateStmt();
	static sql::PreparedStatement* CreatePreppedStmt(std::string query);
};