/*
 * DBStore.h
 *
 *  Created on: 2017年12月2日
 *      Author: xueda
 */

#ifndef SRC_DBSTORE_H_
#define SRC_DBSTORE_H_

#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace sql {
namespace mysql {
class MySQL_Driver;
class Statement;
}
}

namespace storage {

class ResultSet {
public:
	ResultSet(const sql::ResultSet* resultset);
	virtual ~ResultSet();

	bool Next();
	virtual std::string GetString(unsigned int columnIndex);
	virtual std::string GetString(const std::string& columnLabel);

	virtual bool GetBoolean(unsigned int columnIndex);
	virtual bool GetBoolean(const std::string& columnLabel);

	virtual long double GetDouble(unsigned int columnIndex);
	virtual long double GetDouble(const std::string& columnLabel);

	virtual int GetInt(unsigned int columnIndex);
	virtual int GetInt(const std::string& columnLabel);

	virtual unsigned int GetUInt(unsigned int columnIndex);
	virtual unsigned int GetUInt(const std::string& columnLabel);

	virtual int64_t GetInt64(unsigned int columnIndex);
	virtual int64_t GetInt64(const std::string& columnLabel);

	virtual uint64_t GetUInt64(unsigned int columnIndex);
	virtual uint64_t GetUInt64(const std::string& columnLabel);

private:
	sql::ResultSet* resultset_;
};

class Statement {
public:
	Statement(const std::string& sql, sql::Connection* conn);
	virtual ~Statement();

	virtual std::shared_ptr<ResultSet> ExecuteQuery();
	virtual std::shared_ptr<ResultSet> ExecuteQuery(const std::string& sql);
	virtual void ExecuteUpdate();
	virtual void ExecuteUpdate(const std::string& sql);

	virtual void SetBoolean(unsigned int parameterIndex, bool value);
	virtual void SetDouble(unsigned int parameterIndex, double value);
	virtual void SetInt(unsigned int parameterIndex, int32_t value);
	virtual void SetUInt(unsigned int parameterIndex, uint32_t value);
	virtual void SetInt64(unsigned int parameterIndex, int64_t value);
	virtual void SetUInt64(unsigned int parameterIndex, uint64_t value);
	virtual void SetString(unsigned int parameterIndex,
			const sql::SQLString& value);

private:
	sql::PreparedStatement* stmt_;
};

class DBConnection {
public:
	DBConnection(const sql::Connection* connection);
	virtual ~DBConnection();

	virtual std::shared_ptr<Statement> CreateStatement(const std::string& sql = "");
	virtual void Commit();
	virtual void Rollback();
	virtual void Close();
	virtual bool IsClosed();
	virtual bool Reconnect();

	sql::Connection* connection_;
};

class DBStore {
public:
	struct DBConf {
		std::string host;
		std::string dbname;
		std::string username;
		std::string password;
		unsigned int port;
	};
	typedef struct DBConf DBConf;

	DBConf& operator =(const DBConf& dbconf) {
		dbconf_.host = dbconf.host;
		dbconf_.dbname = dbconf.dbname;
		dbconf_.username = dbconf.username;
		dbconf_.password = dbconf.password;
		dbconf_.port = dbconf.port;
		return dbconf_;
	}

	virtual std::shared_ptr<DBConnection> CreateConnection();
	virtual std::shared_ptr<DBConnection> GetConnection();
	virtual void PutConnection(const std::shared_ptr<DBConnection>& connection);

	static void Init(const DBConf& dbconf);
	static void Destory();
	static DBStore* Instance();

	static DBConf dbconf_;
	static DBStore *instance_;

private:
	DBStore(const DBConf& dbconf);
	virtual ~DBStore();

	std::mutex mutex_;
	std::condition_variable cond_var_;
	std::queue<std::shared_ptr<DBConnection>> connections_;
};

} /* namespace storage */

#endif /* SRC_DBSTORE_H_ */
