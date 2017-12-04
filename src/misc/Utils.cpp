/*
 * Utils.cpp
 *
 *  Created on: 2017年8月14日
 *      Author: xueda
 */

#include <set>
#include <sys/time.h>
#include <stdarg.h>
#include <assert.h>
#include <openssl/md5.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

unsigned long GetCurrentTimestamp() {
  struct timeval tv;
  unsigned long current;
  gettimeofday(&tv, NULL);
  current = tv.tv_sec * 1000 + tv.tv_usec / 1000;

  return current;
}

/* 日期转UNIX时间戳, 精确到毫秒 */
unsigned long Date2Timestamp(const std::string& date) {
  struct tm time;
  time_t seconds;
  if (nullptr == strptime(date.c_str(), "%Y-%m-%d %H:%M:%S", &time)) {
    LOG(ERROR)<< "Date format error";
    return 0;
  }
  seconds = mktime(&time);
  return seconds * 1000;
}

void Split(const std::string& src, const std::string &delim,
           std::vector<std::string> *ret) {
  size_t last = 0;
  size_t index = src.find_first_of(delim, last);
  while (index != std::string::npos) {
    ret->push_back(src.substr(last, index - last));
    last = index + 1;
    index = src.find_first_of(delim, last);
  }
  if (index - last > 0) {
    ret->push_back(src.substr(last, index - last));
  }
}

std::string Format(const std::string &fmt, ...) {
  assert(fmt.size() > 0);
  std::string Result = "";
  char strbuf[1024] = { 0 };

  va_list args_list;

  try {
    va_start(args_list, fmt);
    vsnprintf(strbuf, sizeof(strbuf), fmt.c_str(), args_list);
    va_end(args_list);
  } catch (const std::exception& e) {
    DLOG(WARNING)<< e.what() << " : " << "Format error.";
  }
  Result.append(strbuf);

  return Result;
}

void SubSpace(std::string* src) {
  int index = 0;
  if (!src->empty()) {
    while ((index = src->find(' ', index)) != (int) std::string::npos) {
      src->erase(index, 1);
    }
  }
}

int GetInteger(const std::string& src) {
  int retcode = 0;
  std::string dst;
  for (unsigned int index = 0; index < src.size(); index++) {
    if (src[index] >= '0' && src[index] <= '9') {
      dst.push_back(src[index]);
    }
  }
  try {
    retcode = std::stoi(dst);
  } catch (const std::exception& e) {
    DLOG(WARNING)<< e.what() << " : " << "Can't convert string to integer.";
  }

  return retcode;
}

std::string GetLocalDate() {
  time_t t = time(0);
  const int BUFLEN = 20;
  char tmpbuf[BUFLEN] = { 0 };
  strftime(tmpbuf, BUFLEN, "%Y-%m-%d %H:%M:%S", localtime(&t));
  std::string str = tmpbuf;
  return str;
}

/* 与操作 */
template<typename T>
void VectorsAnd(const std::vector<T>* first, const std::vector<T>* second,
                std::vector<T>* dst) {
  std::set<T> items;
  for (unsigned int i = 0; i < first->size(); i++) {
    for (unsigned int j = 0; j < second->size(); j++) {
      if ((*first)[i] != (*second)[j]) {
        continue;
      }
      dst->push_back((*first)[i]);
      items.insert((*first)[i]);
    }
  }
  for (auto iter = items.begin(); iter != items.end(); iter++) {
    dst->push_back(*iter);
  }
}

/* 或操作 */
template<typename T>
void VectorsOr(const std::vector<T>* first, const std::vector<T>* second,
               std::vector<T>* dst) {
  std::set<T> items;
  for (unsigned int index = 0; index < first->size(); index++) {
    items.insert((*first)[index]);
  }
  for (unsigned int index = 0; index < second->size(); index++) {
    items.insert((*second)[index]);
  }
  for (auto iter = items.begin(); iter != items.end(); iter++) {
    dst->push_back(*iter);
  }
}

std::string GetMd5Code(const std::string& source) {
  MD5_CTX ctx;
  std::string md5_code;
  char buf[3];
  unsigned char md[16] = { 0 };
  MD5_Init(&ctx);
  MD5_Update(&ctx, source.c_str(), strlen(source.c_str()));
  MD5_Final(md, &ctx);

  for (int i = 0; i < 16; i++) {
    sprintf(buf, "%02x", md[i]);
    md5_code.append(buf);
  }
  return md5_code;
}

