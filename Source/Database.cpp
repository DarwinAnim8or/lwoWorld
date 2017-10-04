#include "Database.h"
using namespace std;

sql::Driver * Database::driver;
sql::Connection * Database::con;

void Database::Connect(const string& host, const string& database, const string& username, const string& password) {
	driver = get_driver_instance();
	con = driver->connect("tcp://" + host, username, password);
	con->setSchema(database);
} //Connect

void Database::Destroy() {
	cout << "Destroying MySQL connection!" << endl;
	con->close();
	delete con;
} //Destroy

sql::Statement* Database::CreateStmt() {
	sql::Statement* toReturn = con->createStatement();
	return toReturn;
} //CreateStmt

sql::PreparedStatement* Database::CreatePreppedStmt(std::string query) {
	sql::PreparedStatement* toReturn = con->prepareStatement(query);
	return toReturn;
} //CreatePreppedStmt