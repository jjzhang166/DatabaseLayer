/*! 
	\file		DatabaseLayer.h
*	\brief		数据库适配组件的基类
	\remmarks	只定义了基本的函数，具体请查看各个子类
*/

#pragma once

#ifndef _CppDBLayer_H_
#define _CppDBLayer_H_

#define POCO_VERSION 0x01010001

#ifdef _SQLITE3_DB
class	CppSQLite3Exception;
class	CppSQLite3DB;
class	CppSQLite3Query;
class	CppSQLite3ResultSet;
class	CppSQLite3Statement;

typedef  CppSQLite3Exception	CppDBException;
typedef  CppSQLite3DB			CppDB; 
typedef  CppSQLite3Query		CppDBQuery; 
typedef  CppSQLite3ResultSet	CppDBResultSet; 
typedef  CppSQLite3Statement	CppDBStatement; 
#endif

#ifdef _MySQL_DB
class	CppMySQLException;
class	CppMySQLDB;
class	CppMySQLQuery;
class	CppMySQLResultSet;
class	CppMySQLStatement;

typedef  CppMySQLException	CppDBException;
typedef  CppMySQLDB			CppDB; 
typedef  CppMySQLQuery		CppDBQuery; 
typedef  CppMySQLResultSet	CppDBResultSet; 
typedef  CppMySQLStatement	CppDBStatement;
#endif

/*!
	\class		DatabaseException 
	\brief		数据库异常处理基类
*/
class DatabaseException
{
public:
	virtual const int		errorCode()		= 0;
	virtual const char*		errorMessage()	= 0;
};

/*!
	\class		DatabaseLayer 
	\brief		数据库基本操作基类
*/
class DatabaseLayer	
{
public:
	virtual void			open(const char* szFile)	= 0;
	virtual void			close()						= 0;

	virtual bool			tableExists(const char* szTable) = 0;
	virtual int				execDML(const char* szSQL)	= 0;
	virtual CppDBQuery		execQuery(const char* szSQL) = 0;
	virtual int				execScalar(const char* szSQL, int nNullValue=0) = 0;

	virtual void			startTransaction()			= 0;
	virtual void			commitTransaction()			= 0;
	virtual void			rollback()					= 0;
	virtual bool			isTransaction()				= 0;
};

/*!
	\class		DatabaseQuery 
	\brief		数据库查询基类，不包含所有的数据
*/
class DatabaseQuery
{
public:
    virtual int				numFields()						= 0;
    virtual int				fieldIndex(const char* szField) = 0;
    virtual const char*		fieldName(int nCol)				= 0;
    virtual int				fieldDataType(int nCol)			= 0;

	virtual const char*		fieldValue(int nField)			= 0;
	virtual const char*		fieldValue(const char* szField) = 0;
	virtual bool			fieldDataIsNull(int nField)		= 0;

    virtual bool			eof()					= 0;
    virtual void			nextRow()				= 0;
	virtual	void			clear()					= 0;

	virtual	int				getIntField(int nField, int nNullValue = 0)							= 0;
	virtual	int				getIntField(const char* szField, int nNullValue = 0)				= 0;
	virtual	double			getDoubleField(int nField, double fNullValue = 0.0)					= 0;
	virtual	double			getDoubleField(const char* szField, double fNullValue = 0.0)		= 0;	
	virtual	const char*		getStringField(int nField, const char* szNullValue = "")			= 0;
	virtual	const char*		getStringField(const char* szField, const char* szNullValue = "")	= 0;
};

/*!
	\class		DatabaseResultSet 
	\brief		数据库查询结果集基类，包含所有的数据
*/
class DatabaseResultSet
{
public:
    virtual int				numFields() = 0;
    virtual unsigned long	numRows()	= 0;

	virtual int				FieldColIndex(const char* szField)	= 0;
    virtual const char*		fieldName(int nCol)					= 0;
    virtual const char*		fieldValue(int nField)				= 0;
    virtual const char*		fieldValue(const char* szField)		= 0;
	virtual bool			fieldDataIsNull(int nField)			= 0;

	virtual bool			eof()		= 0;
	virtual void			nextRow()	= 0;
    virtual int				seekRow(unsigned long nRow) = 0;
	virtual	void			clear()		= 0;
};

/*!
	\class		DatabaseStatement 
	\brief		Statement基类
				
*/
class DatabaseStatement
{
public:
	virtual int				execDML()		= 0;

	virtual void			bind(int nParam, const char* szValue)	= 0;
	virtual void			bind(int nParam, const int nValue)		= 0;
	virtual void			bind(int nParam, const double dwValue)	= 0;

	virtual void			reset() = 0;
	virtual void			clear() = 0;
};

#if defined _SQLITE3_DB && !defined _CppSQLite3_H_
#include "CppSQLite3.h"
#endif

#if defined _MySQL_DB && !defined _CPPMYSQL_H_
#include "CppMySQL.h"
#endif

#ifdef _WIN32
#include "Platform_WIN32.h"
#endif

#endif
