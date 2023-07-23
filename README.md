# 提高Sqlite数据库插入的几种思路
![](https://www.runoob.com/wp-content/uploads/2014/01/sqlite-vector-logo-small.png)
## 前言
&nbsp;&nbsp;&nbsp;&nbsp;SQlite是遵守ACID的关系数据库管理系统，它包含在一个相对小的C程序库中。与许多其它数据库管理系统不同，SQLite不是一个客户端/服务器结构的数据库引擎，而是被集成在用户程序中。--引自[维基百科](https://zh.wikipedia.org/wiki/SQLite)
&nbsp;&nbsp;&nbsp;&nbsp;由于其轻量且易集成的特性，在嵌入式产品中常被作为首选数据库解决方案。
&nbsp;&nbsp;&nbsp;&nbsp;在具体应用过程中不同的使用方式将直接影响到数据库的执行效率，因此本文从数据插入为切入点，对这些操作的性能进行分析。为后续项目中数据优化提供思路。

## 一、逐条插入
sqlite提供了统一的数据库操作接口，我们唯一需要做的就是自己组装sql语句传入其api中，即可进行数据操作
其接口如下：
 ```c
  int sqlite3_exec(sqlite3*, const char *sql, int (*callback)(void*,int,char**,char**), void *, char **errmsg) 
```
直接使用INSERT语句的字符串进行插入，程序代码如下：
```c
for(int i=0;i<nCount;++i)  
{  
    std::stringstream ssm;  
    ssm<<"insert into t1 values("<<i<<","<<i*2<<","<<i/2<<","<<i*i<<")";  
    sqlite3_exec(db,ssm.str().c_str(),0,0,0);  
} 
```
**耗时分析**：
执行数据1000条，耗时199毫秒

## 二、批量插入（拼接sql）
既然逐条插入的效率很慢，那么最为直观的做法就是sql语句拼接然后一次性插入。具体做法直接看代码：
```c
for(int i=0;i<nCount;++i)  
{  
    std::stringstream ssm;  
    ssm<<"insert into t1 values("<<i<<","<<i*2<<","<<i/2<<","<<i*i<<")";  
    sqlite3_exec(db,ssm.str().c_str(),0,0,0);  
} 
```

## 三、批量插入（事务）
所谓”事务“就是指一组SQL命令，这些命令要么一起执行，要么都不被执行。在SQLite中，每调用一次sqlite3_exec()函数，就会隐式地开启了一个事务，如果插入一条数据，就调用该函数一次，事务就会被反复地开启、关闭，会增大IO量。如果在插入数据前显式开启事务，插入后再一起提交，则会大大提高IO效率，进而加数据快插入速度。 
开启事务只需在上述代码的前后各加一句开启与提交事务的命令即可：

```C
sqlite3_exec(db,"begin;",0,0,0);  
for(int i=0;i<nCount;++i)  
{  
    std::stringstream ssm;  
    ssm<<"insert into t1 values("<<i<<","<<i*2<<","<<i/2<<","<<i*i<<")";  
    sqlite3_exec(db,ssm.str().c_str(),0,0,0);  
}  
sqlite3_exec(db,"commit;",0,0,0);  
```


## 四、写同步
在SQLite中，数据库配置的参数都由编译指示（pragma）来实现的，而其中synchronous选项有三种可选状态，分别是full、normal、off。
简要说来，full写入速度最慢，但保证数据是安全的，不受断电、系统崩溃等影响，而off可以加速数据库的一些操作，但如果系统崩溃或断电，则数据库可能会损毁。

SQLite3中，该选项的默认值就是full，如果我们再插入数据前将其改为off，则会提高效率。如果仅仅将SQLite当做一种临时数据库的话，完全没必要设置为full。在代码中，设置方法就是在打开数据库之后，直接插入以下语句：

```c
sqlite3_exec(db,"PRAGMA synchronous = OFF; ",0,0,0);
```

##  五、执行准备 
&nbsp;&nbsp;&nbsp;&nbsp;SQLite执行SQL语句的时候，有两种方式：
* 一、直接使用函数sqlite3_exec()，该函数直接调用包含SQL语句的字符串；
* 二、另一种方法就是“执行准备”操作，即先将SQL语句编译好，然后再一行一行地执行。
前者会对每一条sql进行读取、校验等操作，显然会比较耗时。
具体处理：
 ```c
sqlite3_exec(db,"begin;",0,0,0);
	sqlite3_stmt *stmt;
	const char* sql = "insert into t1 values(?,?,?,?)";
	sqlite3_prepare_v2(db,sql,strlen(sql),&stmt,0);
	
	for(int i=0;i<nCount;++i)
	{		
		sqlite3_reset(stmt);
		sqlite3_bind_int(stmt,1,i);
		sqlite3_bind_int(stmt,1,i*2);
		sqlite3_bind_int(stmt,1,i/2);
		sqlite3_bind_double(stmt,1,i*i);
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(db,"commit;",0,0,0);
```

## 6、效率对比


## 后记
通过对比上述测试结果可以看出，不同的处理方式对数据插入的效率有非常大的影响，但是在实际使用中需要考虑以下因素：
* 1、设备内存资源是否充足；
* 2、是否存在大批量数据同时处理；
* 3、数据是否需要保证其安全性；



