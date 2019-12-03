 //BaseDataQCPro.h

#ifndef __BASEDATAQC_H
#define __BASEDATAQC_H

#include "GDef.h"//added by chenxiang for 跨平台统一定义

DWORD BaseDataQC( LPVOID lpParam  );
BOOL WriteData(char* fName, LPVOID lpData, DWORD dataLen);
//Extern function
#if defined (WIN32)|| defined(WIN64)
#	define P01_MAIN_PROCESS_QC_		P01_MAIN_PROCESS_QC
#	define P01_MAIN_PROCESS_NO_QC_	P01_MAIN_PROCESS_NO_QC
#elif defined (UNIX)
#	define P01_MAIN_PROCESS_QC_		p01_main_process_qc__
#	define P01_MAIN_PROCESS_NO_QC_	p01_main_process_no_qc__
#else
#	define P01_MAIN_PROCESS_QC_		p01_main_process_qc_
#	define P01_MAIN_PROCESS_NO_QC_	p01_main_process_no_qc_
#endif

extern "C" {void STDCALL_GSYS P01_MAIN_PROCESS_QC_(RADARSITEINFO &SiteInfo , 
												  LPVOID STRCT_REF, LPVOID STRCT_VEL, 
												  LPVOID STRCT_SPW, int *nCuts, 
												  char *PathName,	unsigned int strLen, 
												  char *BackPathName, unsigned int strLen1);}
extern "C" {void STDCALL_GSYS P01_MAIN_PROCESS_NO_QC_(RADARSITEINFO &SiteInfo , 
													 LPVOID STRCT_REF, LPVOID STRCT_VEL,
													 LPVOID STRCT_SPW, int *nCuts, 
													 char *PathName, unsigned int strLen);}

//////extern "C" {void P01_MAIN_PROCESS_QC( RADARSITEINFO SiteInfo ,void *lpRef,void *lpVel,void *lpSw,
//////									 int *nCuts, char *pfileName, unsigned int strLen, char *QcFileName, unsigned int pathLen );}
//////extern "C" {void P01_MAIN_PROCESS_NO_QC( RADARSITEINFO SiteInfo ,void  *lpRef,void *lpVel,void *lpSw,
//////										int *nCuts, char *pfileName, unsigned int strLen);}

#endif
