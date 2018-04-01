/*
 * DBStore.cpp
 *
 *  Created on: 2017年12月2日
 *      Author: xueda
 */

#include <glog/logging.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/metadata.h>
#include <cppconn/exception.h>
#include "DBStore.h"

namespace storage {

static int kConnectCount = 4;

DBStore::DBConf DBStore::dbconf_;
DBStore *DBStore::instance_ = nullptr;

DBStore::DBStore(const DBConf& dbconf) {
  dbconf_ = dbconf;
  try {
    for (int index = 0; index < kConnectCount; index++) {
      auto item = CreateConnection();
      CHECK_NOTNULL(item.get());
      connections_.push(item);
    }
  } catch (const std::exception& e) {
    LOG(FATAL)<< "Connect to database failed! Message : " << e.what();
  }
  LOG(INFO)<< "Connected to database : " << dbconf.host << ":" << dbconf.port
  << " " << dbconf.username << "/" << dbconf.password;
}

DBStore::~DBStore() {
  for (int index = 0; index < kConnectCount; index++) {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !connections_.empty();});
    auto connection = connections_.front();
    connections_.pop();
    connection->Close();
  }
  LOG(INFO)<< "Disconnect to database.";
}

std::shared_ptr<DBConnection> DBStore::CreateConnection() {
  sql::Connection *connection = nullptr;
  auto driver = sql::mysql::get_mysql_driver_instance();
  CHECK_NOTNULL(driver);
  std::string hostname = "tcp://" + dbconf_.host + ":"
      + std::to_string(dbconf_.port) + "/" + dbconf_.dbname;
  connection = driver->connect(hostname, dbconf_.username, dbconf_.password);
  CHECK_NOTNULL(connection);
  connection->setAutoCommit(false);
  auto item = std::make_shared<DBConnection>(connection);
  CHECK_NOTNULL(item.get());
  return item;
}

std::shared_ptr<DBConnection> DBStore::GetConnection() {
  std::shared_ptr<DBConnection> connection;
  do {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {return !connections_.empty();});
    connection = connections_.front();
    CHECK_NOTNULL(connection.get());
    connections_.pop();
    // If the connection is closed and reconnected fail.
  } while (connection->IsClosed() && !connection->Reconnect());
  return connection;
}

void DBStore::PutConnection(const std::shared_ptr<DBConnection>& connection) {
  connection->Rollback();
  std::lock_guard<std::mutex> lock(mutex_);
  connections_.push(connection);
  cond_var_.notify_one();
}

void DBStore::Init(const DBConf& dbconf) {
  if (instance_ == nullptr) {
    instance_ = new DBStore(dbconf);
    CHECK_NOTNULL(instance_);
  }
}

void DBStore::Destory() {
  CHECK_NOTNULL(instance_);
  delete instance_;
  instance_ = nullptr;
}

DBStore* DBStore::instance() {
  CHECK_NOTNULL(instance_);
  return instance_;
}

DBConnection::DBConnection(const sql::Connection* connection)
    : connection_(nullptr) {
  CHECK_NOTNULL(connection);
  connection_ = const_cast<sql::Connection*>(connection);
}

DBConnection::~DBConnection() {
  CHECK_NOTNULL(connection_);
  delete connection_;
  connection_ = nullptr;
}

std::shared_ptr<Statement> DBConnection::CreateStatement(
    const std::string& sql) {
  return std::shared_ptr<Statement>(new Statement(sql, connection_));
}

void DBConnection::Commit() {
  connection_->commit();
}

void DBConnection::Rollback() {
  connection_->rollback();
}

void DBConnection::Close() {
  connection_->close();
}

bool DBConnection::IsClosed() {
  return connection_->isClosed();
}

bool DBConnection::Reconnect() {
  return connection_->reconnect();
}

Statement::Statement(const std::string& sql, sql::Connection* conn)
    : stmt_(nullptr) {
  stmt_ = conn->prepareStatement(sql);
  CHECK_NOTNULL(stmt_);
}

Statement::~Statement() {
  delete stmt_;
  stmt_ = nullptr;
}

std::shared_ptr<ResultSet> Statement::ExecuteQuery() {
  ResultSet* resultset;
  try {
    resultset = new ResultSet(stmt_->executeQuery());
  } catch (const std::exception& e) {
    LOG(FATAL)<< e.what();
  }
  return std::shared_ptr<ResultSet>(resultset);
}

std::shared_ptr<ResultSet> Statement::ExecuteQuery(const std::string& sql) {
  ResultSet* resultset;
  try {
    resultset = new ResultSet(stmt_->executeQuery(sql));
  } catch (const std::exception& e) {
    LOG(FATAL)<< e.what();
  }
  return std::shared_ptr<ResultSet>(resultset);
}

void Statement::ExecuteUpdate() {
  try {
    stmt_->executeUpdate();
  } catch (const std::exception& e) {
    LOG(FATAL)<< e.what();
  }
}

void Statement::ExecuteUpdate(const std::string& sql) {
  try {
    stmt_->executeUpdate(sql);
  } catch (const std::exception& e) {
    LOG(FATAL)<< e.what();
  }
}

void Statement::SetBoolean(unsigned int parameterIndex, bool value) {
  stmt_->setBoolean(parameterIndex, value);
}

void Statement::SetDouble(unsigned int parameterIndex, double value) {
  stmt_->setDouble(parameterIndex, value);
}

void Statement::SetInt(unsigned int parameterIndex, int32_t value) {
  stmt_->setInt(parameterIndex, value);
}

void Statement::SetUInt(unsigned int parameterIndex, uint32_t value) {
  stmt_->setUInt(parameterIndex, value);
}

void Statement::SetInt64(unsigned int parameterIndex, int64_t value) {
  stmt_->setInt64(parameterIndex, value);
}

void Statement::SetUInt64(unsigned int parameterIndex, uint64_t value) {
  stmt_->setUInt64(parameterIndex, value);
}

void Statement::SetString(unsigned int parameterIndex,
                          const sql::SQLString& value) {
  stmt_->setString(parameterIndex, value);
}

ResultSet::ResultSet(const sql::ResultSet* resultset)
    : resultset_(nullptr) {
  resultset_ = const_cast<sql::ResultSet*>(resultset);
  CHECK_NOTNULL(resultset_);
}

ResultSet::~ResultSet() {
  delete resultset_;
  resultset_ = nullptr;
}

bool ResultSet::Next() {
  return resultset_->next();
}

std::string ResultSet::GetString(unsigned int columnIndex) {
  return resultset_->getString(columnIndex);
}

std::string ResultSet::GetString(const std::string& columnLabel) {
  return resultset_->getString(columnLabel);
}

bool ResultSet::GetBoolean(unsigned int columnIndex) {
  return resultset_->getBoolean(columnIndex);
}

bool ResultSet::GetBoolean(const std::string& columnLabel) {
  return resultset_->getBoolean(columnLabel);
}

long double ResultSet::GetDouble(unsigned int columnIndex) {
  return resultset_->getDouble(columnIndex);
}

long double ResultSet::GetDouble(const std::string& columnLabel) {
  return resultset_->getDouble(columnLabel);
}

int ResultSet::GetInt(unsigned int columnIndex) {
  return resultset_->getInt(columnIndex);
}

int ResultSet::GetInt(const std::string& columnLabel) {
  return resultset_->getInt(columnLabel);
}

unsigned int ResultSet::GetUInt(unsigned int columnIndex) {
  return resultset_->getUInt(columnIndex);
}

unsigned int ResultSet::GetUInt(const std::string& columnLabel) {
  return resultset_->getUInt(columnLabel);
}

int64_t ResultSet::GetInt64(unsigned int columnIndex) {
  return resultset_->getInt64(columnIndex);
}

int64_t ResultSet::GetInt64(const std::string& columnLabel) {
  return resultset_->getInt64(columnLabel);
}

uint64_t ResultSet::GetUInt64(unsigned int columnIndex) {
  return resultset_->getUInt64(columnIndex);
}

uint64_t ResultSet::GetUInt64(const std::string& columnLabel) {
  return resultset_->getUInt64(columnLabel);
}

} /* namespace storage */

int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::SetLogDestination(google::GLOG_INFO, "/tmp/log_");
  google::SetStderrLogging(google::GLOG_INFO);
  FLAGS_colorlogtostderr = true;

  storage::DBStore::DBConf conf;
  conf.dbname = "test1";
  conf.host = "localhost";
  conf.username = "root";
  conf.password = "123456";
  conf.port = 3306;

  storage::DBStore::Init(conf);

//	{
//		auto conn = storage::DBStore::Instance()->GetConnection();
//		auto stmt = conn->CreateStatement(" INSERT INTO student (id, name)"
//				" values(?, ?) ");
//		stmt->SetString(1, "008");
//		stmt->SetString(2, "xueda8");
//		stmt->ExecuteUpdate();
//		stmt.reset();
//
//		conn->Rollback();
//		stmt = conn->CreateStatement(" INSERT INTO student (id, name)"
//				" values(?, ?) ");
//		stmt->SetString(1, "009");
//		stmt->SetString(2, "xueda9");
//		stmt->ExecuteUpdate();
//
//		conn->Commit();
//		storage::DBStore::Instance()->PutConnection(conn);
//	}

  {
    auto conn = storage::DBStore::instance()->GetConnection();
    auto stmt = conn->CreateStatement(" SELECT * FROM student ");
    auto resultset = stmt->ExecuteQuery();
    while (resultset->Next()) {
      DLOG(INFO)<< resultset->GetString(1);
      DLOG(INFO) << resultset->GetString(2);
    }
    storage::DBStore::instance()->PutConnection(conn);
  }

  storage::DBStore::Destory();

  return 0;
}

