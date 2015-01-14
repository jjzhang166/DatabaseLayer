#DatabaseLayer

DatabaseLayer是一个数据库组件，参考了CppSQLite和POCO中的数据库实现。目前支持Sqlite3和MySQL，支持跨平台。

特点：

1、提供统一的Sqlite3和MySQL的操作接口

2、只对Sqlite3和MySQL的API进行简单封装，代码简单易懂

3、CppSqlite3部分没有用到stl，支持嵌入式平台使用；CppMySQL部分用到少量STL，支持跨平台。

4、不支持Unicode







