#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
// 使用数据预准备
void test_fuc1(sqlite3 *db, int N)
{
	for (int i = 0; i < N; ++i)
	{
		std::stringstream ssm;
		ssm << "insert into testInfo values(" << i << "," << i * 2 << "," << i / 2 << "," << i * i << ")";
		sqlite3_exec(db, ssm.str().c_str(), 0, 0, 0);
	}
}

//3
float test_fuc3(sqlite3 *db, int N)
{
	clock_t t1 = clock();

	clock_t t2 = clock();
	float ret = (t2 - t1) / 1.;

	std::string sql = "insert into testInfo values";
	for (int i = 0; i < N; ++i)
	{
		sql += "," + "," + "," + ")"
	}

	return ret;
}

//4
float test_fuc4(sqlite3 *db, int N)
{
	clock_t t1 = clock();

	clock_t t2 = clock();

	float ret = (t2 - t1) / 1.;

	return ret;
}

// 使用数据预准备
void test_fuc5(sqlite3 *db, int N)
{
	sqlite3_stmt *stmt;
	const char *sql = "insert into testInfo values(?,?,?,?)";
	sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);

	for (int i = 0; i < N; ++i)
	{
		sqlite3_reset(stmt);
		sqlite3_bind_int(stmt, 1, i);
		sqlite3_bind_int(stmt, 2, i * 2);
		sqlite3_bind_int(stmt, 3, i / 2);
		sqlite3_bind_double(stmt, 4, i * i);
		sqlite3_step(stmt);
	}
	sqlite3_finalize(stmt);
}

/** 
      * 计算两个时间的间隔，得到时间差 
      * @param struct timeval* resule 返回计算出来的时间 
      * @param struct timeval* x 需要计算的前一个时间 
      * @param struct timeval* y 需要计算的后一个时间 
      * return -1 failure ,0 success 
  **/
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
	int nMsec = 0;

	if (x->tv_sec > y->tv_sec)
		return -1;

	if ((x->tv_sec == y->tv_sec) && (x->tv_usec > y->tv_usec))
		return -1;

	result->tv_sec = (y->tv_sec - x->tv_sec);
	result->tv_usec = (y->tv_usec - x->tv_usec);

	if (result->tv_usec < 0)
	{
		result->tv_sec--;
		result->tv_usec += 1000000;
	}

	nMsec = result->tv_sec * 1000000 + result->tv_usec;

	return nMsec;
}

int main(int argc, char **argv)
{
	sqlite3 *db;
	int N = 0;
	int fuc_num = 0;
	float cost_time = 0;
	if (remove("testdb.db") == 0)
		printf("Removed %s.", "testdb.db");
	else
		perror("remove");

	sqlite3_open("testdb.db", &db);
	sqlite3_exec(db, "drop table if exists testInfo", 0, 0, 0);
	sqlite3_exec(db, "create table testInfo(id integer,x integer,y integer ,weight real)", 0, 0, 0);
	sqlite3_exec(db, "PRAGMA synchronous = OFF; ", 0, 0, 0);

	std::cout << "input number of date:";
	std::cin >> N;
	std::cout << "func number:";
	std::cin >> fuc_num;

	switch (fuc_num)
	{
	case 1:
	{
		//sqlite3_exec(db, "PRAGMA synchronous = FULL; ", 0, 0, 0);
		struct timeval start, stop, diff;
		gettimeofday(&start, 0);
		//
		test_fuc1(db, N);
		//
		gettimeofday(&stop, 0);
		float cost_time = timeval_subtract(&diff, &start, &stop);
		printf("1:总计用时:%f 毫秒\n", cost_time);

		break;
	}
	case 2:
	{
		struct timeval start, stop, diff;
		gettimeofday(&start, 0);
		//
		sqlite3_exec(db, "begin;", 0, 0, 0);
		test_fuc1(db, N);
		sqlite3_exec(db, "commit;", 0, 0, 0);

		//
		gettimeofday(&stop, 0);
		float cost_time = timeval_subtract(&diff, &start, &stop);
		printf("2:总计用时:%f 微秒\n", cost_time);

		break;
	}
	case 3:
	{
		cost_time = test_fuc3(db, N);
		break;
	}
	case 4:
	{
		cost_time = test_fuc4(db, N);
		break;
	}
	case 5:
	{
		struct timeval start, stop, diff;
		gettimeofday(&start, 0);
		//
		sqlite3_exec(db, "begin;", 0, 0, 0);
		test_fuc5(db, N);
		sqlite3_exec(db, "commit;", 0, 0, 0);
		//
		gettimeofday(&stop, 0);

		float cost_time = timeval_subtract(&diff, &start, &stop);
		printf("5:总计用时:%f 微秒\n", cost_time);
		break;
	}
	default:
	{
		struct timeval start, stop, diff;
		gettimeofday(&start, 0);
		sleep(10);
		gettimeofday(&stop, 0);

		float cost_time = timeval_subtract(&diff, &start, &stop);
		printf("总计用时:%f 微秒\n", cost_time);
		break;
	}
	}

	sqlite3_close(db);
	std::cout << "total:" << N << "cost time: " << cost_time << "s" << std::endl;

	return 1;
}
