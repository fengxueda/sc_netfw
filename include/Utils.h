/*
 * Utils.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

/* 获取当前毫秒数 */
unsigned long GetCurrentTimestamp();

/* 日期转UNIX时间戳, 精确到毫秒 */
unsigned long Date2Timestamp(const std::string& date);

/* 获得格式化字符串
 * Tips : 参数传入是字符串时(如%s),要传入(char*),比如string.c_str().
 * */
std::string Format(const std::string &fmt, ...);

/* 分离字符串 */
void Split(const std::string& src, const std::string &delim,
    std::vector<std::string> *ret);

/* 去除string的空格 */
void SubSpace(std::string* src);

/* 获取字符串中的整数 */
int GetInteger(const std::string& src);

/* 获取当地日期, 格式 年月日 时分秒 */
std::string GetLocalDate();

class Restriction;
const std::string CreateSqlByFilter(const Restriction& restriction);

/* 与操作 */
template<typename T>
void VectorsAnd(const std::vector<T>* first, const std::vector<T>* second,
    std::vector<T>* dst);
/* 或操作 */
template<typename T>
void VectorsOr(const std::vector<T>* first, const std::vector<T>* second,
    std::vector<T>* dst);

/* MD5加密 */
std::string GetMd5Code(const std::string& source);

int GetErrorCodeBySocket(int socket_id);

const std::string GetIpAdressBySocket(int sockfd);

const std::string GetPortBySocket(int sockfd);

#endif /* INCLUDE_UTILS_H_ */
