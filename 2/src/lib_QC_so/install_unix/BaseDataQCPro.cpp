//BaseDataQCPro.cpp
/*
 质量控制函数
*/
#ifdef _WINDOWS
#include "stdafx.h"
#endif
////#include <iostream>
////#include <windows.h>
#include "CommVar.h"
#include "BaseDataQCPro.h"
#include "ThreadsParam.h"
#include "ProcessIO.h"
#include "MsgMemManager.h"
#include "WriteLogFile.h"
//#include "DisplayData.h"
#include "CommFunctions.h"
#include "SaveLog.h"
#include "GDef.h"//added by chenxiang for 跨平台统一定义

////as by chenxiang
//void QC(RADARSITEINFO &SiteInfo , 
//	   LPVOID STRCT_REF, LPVOID STRCT_VEL, 
//	   LPVOID STRCT_SPW, int *nCuts, 
//	   char *PathName,	unsigned int strLen, 
//	   char *BackPathName, unsigned int strLen1)
//{
//	P01_MAIN_PROCESS_QC_(SiteInfo, STRCT_REF, STRCT_VEL, STRCT_SPW, nCuts, PathName, strLen, BackPathName, strLen1);////ms by chenxiang for linux
//}
//void NO_QC(RADARSITEINFO &SiteInfo , 
//	  LPVOID STRCT_REF, LPVOID STRCT_VEL,
//	  LPVOID STRCT_SPW, int *nCuts, 
//	  char *PathName, unsigned int strLen)
//{
//	P01_MAIN_PROCESS_NO_QC_(SiteInfo, STRCT_REF, STRCT_VEL, STRCT_SPW, nCuts, PathName, strLen);////ms by chenxiang for linux
//}
////ae by chenxiang

/********************************************************************************
*                                                                               *
*  Function:  Do quality controlling of base data, and send base processed data *
*             to the next task to do further work.                              *
*                                                                               *
*  Parameter: pointer to the pameter passed to thie task(File name)             *
*                                                                               *
*  Output:    DWORD                                                             *
*                                                                               *
********************************************************************************/
DWORD BaseDataQC( LPVOID lpParam  )
{	
	SHOW_FUNCTION_NAME();//added by chenxiang for 显示日志 20090508

	LPBYTE lpInIndex=NULL;
	DWORD dwTmpMsgLen = 0;
	LPVOID lpTmpMsg=NULL;

	int  nCuts = 12;		//Number of elevation cuts
	int  nData = 3;			//Tyoe of data : REF, VEL, SPW

	char QcFileName[PATH_LEN];
	char pfileName[PATH_LEN*2]={0};////ms by chenxiang for linux 下崩溃问题
	////char *pfileName=NULL;	
	////char strYear[5]		="    ";
	////char strMonth[4]	="   ";
	////char strDay[4]		="   ";
	////char strHour[4]		="   ";
	////char strMinute[4]	="   ";
	////char strYear[8]		="    ";//ms by chenxiang for 溢出 del by chenxiang for无用代码
	////char strMonth[8]	="   ";
	////char strDay[8]		="   ";
	////char strHour[8]		="   ";
	////char strMinute[8]	="   ";

	struct PPISTRUCT *lpRef=NULL;
	struct PPISTRUCT *lpVel=NULL;
	struct PPISTRUCT *lpSw=NULL;

	struct RADARSITEINFO SiteInfo;
	int		iSiteID=0;
	int    iMosaicParamIndex=-1;
	int     k=0, i, siteIdx;

	int ThisTaskID=-1, DestTaskID=-1;
	unsigned long ulLen;
	MYMESSAGEPARAM *pOutMsg=NULL;
	LPBYTE pByte=NULL;
	CMsgMemManager MsgMemMan;
	////bool bChB=false,bChJ=false;////del by chenxiang for无用代码

	CProcessIO ProcessIO;
	CWriteLogFile WriteLogFile;
	char strMsg[80];
	//////////////////////////////////////////////////////////////////

	//获取指向发送给本模块的消息的指针
	lpInIndex = (LPBYTE)lpParam;
	//获取本时次，数据文件的序号在(从0开始)
	CopyMemory(&iMosaicParamIndex, lpInIndex, sizeof(int));	
	//获取雷达站在软件中的编号
	lpInIndex+=sizeof(int);
	CopyMemory(&iSiteID, lpInIndex, sizeof(int));
	//获取数据文件完整名称
	lpInIndex=lpInIndex+sizeof(int);
	////pfileName = (char*)lpInIndex;
	strncpy(pfileName,(char*)lpInIndex,PATH_LEN);////ms by chenxiang for linux 下崩溃问题

	//Allocate memory for all data
	dwTmpMsgLen = sizeof(PPISTRUCT)*nCuts*nData;
	lpTmpMsg = (LPVOID)malloc(dwTmpMsgLen);
	if(lpTmpMsg==NULL)
	{//Report error
		sprintf(strMsg,"Procedure: %s: Failed to allocate memory","BaseDataQCPro");
		WriteLogFile.LoadMessage(strMsg);
		WriteLogFile.WriteErrorMsg();
		return (0);
	}

	//初始化数据空间
	ZeroMemory(lpTmpMsg, dwTmpMsgLen);

	//Set pointer to ref., vel., and sw..
	lpRef = (struct PPISTRUCT *)lpTmpMsg;
	lpVel = lpRef + nCuts;
	lpSw  = lpVel + nCuts;

	//Initialize 
	for(k=0; k<nCuts; k++)
	{
		memset(lpRef+k,0x0, 70);
		memset(lpVel+k,0x0, 70);
		memset(lpSw+k,0x0, 70);

		////memset(lpRef[k].field, VALUE_INVALID,nazim*nazim);
		////memset(lpVel[k].field, VALUE_INVALID,nazim*nazim);
		////memset(lpSw[k].field, VALUE_INVALID,nazim*nazim);
		////ms by chenxiang for 原来的代码有误
		memset(lpRef[k].field, VALUE_INVALID,sizeof(short int)*nazim*nazim);
		memset(lpVel[k].field, VALUE_INVALID,sizeof(short int)*nazim*nazim);
		memset(lpSw[k].field,  VALUE_INVALID,sizeof(short int)*nazim*nazim);
	}

	//确定雷达站信息
	for(k=0; k<g_iSitesInZone; k++)
	{//Find the correct site inforamtion from the site list
		if(iSiteID==g_cRadarSiteInfo[k].SiteID)
		{
			SiteInfo = g_cRadarSiteInfo[k];
			siteIdx = k;			
			//int len = strlen(g_cRadarSiteInfo[k].SiteName);
			//for(int mm=len; mm<SITE_NAME_LEN; mm++)
			//	g_cRadarSiteInfo[k].SiteName[mm]=' ';
			break;
		}
	}

	//进行原始体扫数据读取和质量控制选择
	nCuts = 0;

	//是否保存质量控制基数据
	for(i=strlen(pfileName); i>0; i--)
	{
		if(pfileName[i] == CHASPLSH)break;
	}
	char fileName[256]={0};
	strcpy(fileName,&pfileName[i+1]);
	strtok(fileName,".");//基数据文件名
	//结果文件名
	if(g_iSaveQcData==1)
	{
		if (strlen(g_strQcDataPath[siteIdx])<2)//如果未设置则为默认路径
		{
			char qcFolder[256];
			strcpy(qcFolder, g_DataDir.strProdDataDir);//	/
			//strcat(strThisFolder, STRSPLSH);

			strcat(qcFolder,STATION_FOLDER);// /STATION
			strcat(qcFolder, STRSPLSH);// /STATION/
			char strSiteID[10];
			strcpy(strSiteID,"Z");//Z
			char ch[8];
			itoa(iSiteID,ch,10);
			strcat(strSiteID,ch);//Z9010

			strcat(qcFolder,strSiteID);// /STATION/Z9010
			CreateDir(qcFolder,true,R_OK|W_OK,00755);//

			strcat(qcFolder, STRSPLSH);// /STATION/Z9010
			strcat(qcFolder, QC_FOLDER);// /STATION/Z9010/QC
			CreateDir(qcFolder/*,true,R_OK|W_OK,00755*/);//

			sprintf(QcFileName, "%s%s%s_QC.bin", qcFolder, STRSPLSH, fileName);
		}
		else
		{
			CreateDir( g_strQcDataPath[siteIdx]);
			////sprintf(QcFileName, "%s%s", g_strQcDataPath[siteIdx], &pfileName[i+1]);
			////sprintf(QcFileName, "%s%c%s", g_strQcDataPath[siteIdx], CHASPLSH, &pfileName[i+1]);////ms by chenxiang for 添加'/'
			//sprintf(QcFileName, "%s%c%s_QC", qcFolder, '/', &pfileName[i+1]);
			sprintf(QcFileName, "%s%c%s_QC.bin", g_strQcDataPath[siteIdx], CHASPLSH, fileName);
		}
	}
	else
	{
		////sprintf(QcFileName, "");
		QcFileName[0]=0;//ms by chenxiang
	}

	strncpy(pfileName+PATH_LEN,QcFileName,PATH_LEN);////added by chenxiang for linux 下崩溃问题

	CWriteLogFile logfile;
	//cx：这里需要fortran编译
	if(g_iQualityControl==1) //采用有质量控制的模块
	{
		SHOW_LOG("#Fortran#P01_MAIN_PROCESS_QC");//added by chenxiang for 显示日志 20090508
		char strMsg[512];
		sprintf(strMsg, "QC being filename=%s",pfileName);
		logfile.LoadMessage(strMsg);
		logfile.WriteStatusMsg();
		////P01_MAIN_PROCESS_QC(SiteInfo, lpRef, lpVel, lpSw, &nCuts, pfileName, PATH_LEN, QcFileName, PATH_LEN);
		P01_MAIN_PROCESS_QC_(SiteInfo, lpRef, lpVel, lpSw, &nCuts, pfileName, PATH_LEN*2, QcFileName, PATH_LEN);////ms by chenxiang for linux
		//QC(SiteInfo, lpRef, lpVel, lpSw, &nCuts, pfileName, PATH_LEN*2, QcFileName, PATH_LEN);////ms by chenxiang for linux
		sprintf(strMsg, "QC end filename=%s",pfileName);
		logfile.LoadMessage(strMsg);
		logfile.WriteStatusMsg();

		if (1 == g_iSaveQcData)
		{
			gChmod(QcFileName);//更改为最高权限
			CSaveLog log;
			char LogfileName[256]; 	
			sprintf(LogfileName, "_STATION_QC");
			log.SendLogMsg(g_DataDir.strProdDataDir, LogfileName, QcFileName);	
		}
	}
	else//采用没有质量控制的模块
	{
		SHOW_LOG("#Fortran#P01_MAIN_PROCESS_NO_QC");//added by chenxiang for 显示日志 20090508
		char strMsg[512];
		sprintf(strMsg, "NoQC being filename=%s",pfileName);
		logfile.LoadMessage(strMsg);
		logfile.WriteStatusMsg();
		////P01_MAIN_PROCESS_NO_QC(SiteInfo, lpRef, lpVel, lpSw, &nCuts, pfileName, PATH_LEN);
		P01_MAIN_PROCESS_NO_QC_(SiteInfo, lpRef, lpVel, lpSw, &nCuts, pfileName, PATH_LEN);////ms by chenxiang for linux
		//NO_QC(SiteInfo, lpRef, lpVel, lpSw, &nCuts, pfileName, PATH_LEN);////ms by chenxiang for linux
		sprintf(strMsg, "NoQC end filename=%s",pfileName);
		logfile.LoadMessage(strMsg);
		logfile.WriteStatusMsg();
	}

	if(nCuts==0)
	{
		WriteLogFile.LoadMessage("Error in QC function, number of el. cuts is 0");
		WriteLogFile.WriteErrorMsg();
	}
	
	//向画图的任务发送PPI数据，绘制PPI图
/*	if(g_iSaveQcData==1 && nCuts>=2)
	{
		int m;
		for(m=0; m<20; m++)
		{
			if(SiteInfo.SiteName[m]=='B' ||SiteInfo.SiteName[m]=='b') 
			{
				bChB = true;
				break;
			}
		}
		for(; m<20; m++)
		{
			if(SiteInfo.SiteName[m]=='J'|| SiteInfo.SiteName[m]=='j') 
				
			{
				bChJ = true;
				break;
			}
		}
	
		if(bChB && bChJ) //表明是北京数据
		{
			//图像保存的路径
			wsprintf(QcFileName,"%s%s",g_strQcDataPath[siteIdx],"TempPPI/");
			CreateDir(QcFileName);
			wsprintf(QcFileName,"%s%s",g_strQcDataPath[siteIdx],"TempPPI/tmpRef.ppi");

			FILE *fp=fopen(QcFileName, "wb");
			if(fp==NULL)
			{
				WriteLogFile.LoadMessage("Error in open file for ref PPI!");
				WriteLogFile.WriteErrorMsg();
			}
			else
			{
				//调用绘图函数	
				if(fwrite(lpRef+1, sizeof(PPISTRUCT), 1, fp)<1)
				{
					WriteLogFile.LoadMessage("Faile to save temp. ref PPI!");
					WriteLogFile.WriteErrorMsg();
				}
				fclose(fp);
			}
		}
	}//end
*/	

	//本任务ID
	ThisTaskID = GetTaskID("TaskBaseDataQCPro");
	//目标任务ID
	DestTaskID = GetTaskID("TaskVSToCartesain");

	//发送消息的内容
	ulLen = 3*sizeof(int)+sizeof(PPISTRUCT)*nCuts;
	//Get message memory
	pOutMsg = (MYMESSAGEPARAM*)MsgMemMan.AllocateMsgOut(ThisTaskID, ulLen);
	if(pOutMsg==NULL)
	{
		goto ERRCODE1;
	}

	pByte = (LPBYTE)pOutMsg->lpParams;
	//Message includes: Total number of files, file index, nCuts, Ref PPIS
	//(1): Index of this file at this time
	CopyMemory(pByte, &iMosaicParamIndex, sizeof(int));
	CopyMemory(pByte+sizeof(int), &iSiteID, sizeof(iSiteID));
	//(2):
	CopyMemory(pByte+sizeof(int)+sizeof(iSiteID), &nCuts, sizeof(nCuts));
	//(3):
	if(nCuts!=0) //转数据正确, 同时发送体扫数据
		CopyMemory(pByte+sizeof(int)+sizeof(iSiteID)+sizeof(int), lpRef, sizeof(PPISTRUCT)*nCuts);

	//Send message to the next task
	MsgMemMan.SendMsgOut(ThisTaskID, 1, &DestTaskID, pOutMsg);

	/////////////////////////////////////////////////////////////////////

/*	//向画图的任务发送PPI数据，绘制PPI图
	if(g_iSaveQcData==1 && nCuts>=2)
	{
		int m;
		for(m=0; m<20; m++)
		{
			if(SiteInfo.SiteName[m]=='B' ||SiteInfo.SiteName[m]=='b') 
			{
				bChB = true;
				break;
			}
		}
		for(; m<20; m++)
		{
			if(SiteInfo.SiteName[m]=='J'|| SiteInfo.SiteName[m]=='j') 
				
			{
				bChJ = true;
				break;
			}
		}
	
		if(bChB && bChJ) //表明是北京数据
		{
			//图像保存的路径
			itoa(lpRef->year, strYear, 10);
			itoa(100+lpRef->month, strMonth, 10);
			itoa(100+lpRef->day, strDay, 10);
			itoa(100+lpRef->hour, strHour, 10);
			itoa(100+lpRef->minute, strMinute, 10);

			wsprintf(QcFileName,"%s%s",g_strQcDataPath[siteIdx],"image/");
			CreateDir(QcFileName);

			wsprintf(QcFileName,"%s%s%4s%2s%2s%2s%2s.%s",g_strQcDataPath[siteIdx],"image/",
				strYear, strMonth+1, strDay+1, strHour+1, strMinute+1, "gif");	//V file name
			
			//调用绘图函数		
			//dis_ppi.ReadData((LPBYTE)(lpRef+1), QcFileName);
		}
		//目标任务ID
		DestTaskID = GetTaskID("DrawPpiImageForBJ");
		
		//发送消息的内容
		ulLen = PATH_LEN+sizeof(PPISTRUCT)*1;
		//Get message memory
		pOutMsg1 = (MYMESSAGEPARAM*)MsgMemMan.AllocateMsgOut(ThisTaskID, ulLen);
		if(pOutMsg1!=NULL)
		{
			pByte = (LPBYTE)pOutMsg1->lpParams;
			//Message includes: Total number of files, file index, nCuts, Ref PPIS
			//(1) Path length
			CopyMemory(pByte, QcFileName, sizeof(char)*PATH_LEN);
			pByte+=sizeof(char)*PATH_LEN;
			//(2): PPI data
			CopyMemory(pByte, lpRef+1, sizeof(PPISTRUCT));
			
			//Send message to the next task
			MsgMemMan.SendMsgOut(ThisTaskID, 1, &DestTaskID, pOutMsg1);
		}

	}
*/
	//写数据到文件中
/*	itoa(lpRef->year, strYear, 10);
	itoa(100+lpRef->month, strMonth, 10);
	itoa(100+lpRef->day, strDay, 10);
	itoa(100+lpRef->hour, strHour, 10);
	itoa(100+lpRef->minute, strMinute, 10);

	char fileName[PATH_LEN];
	char folder[PATH_LEN];
	wsprintf(folder,"%s%s", g_DataDir.strProdDataDir, QC_FOLDER);
	//Create folder for 3D radar data 
	CreateDirectory(folder,NULL);
    wsprintf(fileName,"%s/%s_%4s%2s%2s%2s%2s.%s",folder, SiteInfo.SiteName, 
			     strYear, strMonth+1, strDay+1, strHour+1, strMinute+1, "REF");	//V file name
	if(!ProcessIO.WriteData(fileName, lpRef, sizeof(PPISTRUCT)*nCuts, TRUE))	//Save V data
	{
		goto ERRCODE1;
	}
*/
  /*  wsprintf(fileName,"%s/%s_%4s%2s%2s%2s%2s%s",folder, SiteInfo.SiteName, 
			     strYear, strMonth+1, strDay+1, strHour+1, strMinute+1, QC_V_FILE);	//V file name
	if(!ProcessIO.WriteData(fileName, lpVel, sizeof(PPISTRUCT)*nCuts, TRUE))	//Save V data
	{
		goto ERRCODE1;
	}
	
	wsprintf(fileName,"%s/%s_%4s%2s%2s%2s%2s%s",folder, s_siteInfo.SiteName, 
			     strYear, strMonth+1, strDay+1, strHour+1, strMinute+1, QC_W_FILE);	//W file name
	if(!ProcessIO.WriteData(fileName, lpSw, sizeof(PPISTRUCT)*nCuts, TRUE))		//Save W Data
	{
		goto ERRCODE1;
	}

*/


	goto ERRCODE1;

ERRCODE1:
	if(lpTmpMsg!=NULL) 
	{
		free(lpTmpMsg);
		lpTmpMsg = NULL;
	}

	return (1);
}
