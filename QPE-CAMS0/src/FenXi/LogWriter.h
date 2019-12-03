#pragma once

/*
* File Name:	LogWriter.h
* File ID:		01
* Abstract:		Write Log Record to "./Log/int(Date).log" (LogWriter.dll/ LogWriter.so)
		(Competely  compatible to Linux !)

* Version:		1.2	
* Author:		LeiNi
* Date:		2014-12-17 18:56:13
* Update	abstract:	 LogWriter can write record without timestamp!		
*
* Previous Version:	1.1	
* Author:		LeiNi
* Date:		2014-12-6 11:08:30
* Update	abstract:	 It can be used in Linux Environment so long as it is recompiled in Linux as a .so file. 				
*/

#ifdef _WIN32
	#ifdef _EXPORTING
   		#define DECLSPEC    __declspec(dllexport)
	#else
   		#define DECLSPEC    __declspec(dllimport)
	#endif
#else
	#define DECLSPEC  
#endif


#include <string>
using  std::string;

#define LOGDIR "LOG"
#define WITOUT_TIMESTAMP false

class DECLSPEC CLogWriter
{
public:
	CLogWriter(void);
	~CLogWriter(void);

	//void LoadMessage(const char msg[]);
	void WriteMsg(const string msg, int iIndentLevel=0,bool isCarryTimeStamp=true);	
    //函数功能：write message to "int(Date).log"
    //参数说明：	msg,			message
    //			iIndentLevel, 	0	2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //							1		2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //							2			2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //							3				2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //
				
	void WriteStatusMsg(const string msg,bool isCarryTimeStamp=true );
    //函数功能：write message to "int(Date)_Status.log"
    //参数说明：	msg,			message
    //
	void WriteErrorMsg(const string msg,bool isCarryTimeStamp=true );
    //函数功能：write message to "int(Date)_Error.log"
    //参数说明：	msg,			message
    //
	void DeleteExpiredLogFile(const int iReserveDay=7);
    //函数功能：Delete all  "./Log/*.log"  that is expired.
    //参数说明：	iReserveDay,	Number of Reserve Days of Log files
    //

private:
	int m_iReserveDay;			//	Reserve Days

	char  m_strLogType[10];		//	LogFileType(determin log file name type)

	string m_strMessage;		//	Message to be written into the log file

	void AddOneRecord(int iIndentLevel=0, bool isCarryTimeStamp=true);
    //函数功能：Write m_strMessage as an unique row into file "YYYYMMdd.log"
    //参数说明：iIndentLevel, 	0	2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //							1		2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //							2			2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //							3				2014xxxx>> strrrrrrrrrrrrrrrrrrring
    //			isCarryTimeStamp,	weather to carry Time Stamp when Writting log record.


	bool CheckExpire(const string & fileName, const long & lDate);
    //函数功能：Check out if "./Log/*.log"  is expired
    //参数说明：	fileName,	Name of  the file to be checked
    //			lDate,		Current Date in Long type
    //返回值：	true,		is expired
    //			false,		 not expired
    //
};

