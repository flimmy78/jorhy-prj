#include "x_vod_manager.h"
#include "x_config.h"
#include "x_time.h"

JO_IMPLEMENT_SINGLETON(XVodManager)

CXVodManager::CXVodManager()
{

}

CXVodManager::~CXVodManager()
{

}

j_result_t CXVodManager::GetRecordInfo(const j_char_t *pResid, j_time_t &begin_time, j_time_t &end_time, j_int64_t &nSize)
{
	j_vec_file_info_t vecFileInfo;
	const char *pDir = CXConfig::GetVodPath();
	j_char_t file_name[256] = {0};

	begin_time = 0xFFFFFFFFFFFFFFFF;
	end_time = 0;
	nSize = 0;

#ifdef WIN32
	WIN32_FIND_DATA FindFileData;  
	HANDLE hFind;  

	sprintf(file_name, "%s/%s/*.*", pDir, pResid);
	hFind = FindFirstFile(file_name, &FindFileData);  
	if (hFind == INVALID_HANDLE_VALUE)   
	{  
		J_OS::LOGERROR("CXFile::GetRecordInfo FindFirstFile failed\n");  
		return J_UNKNOW;  
	}  
	
	do  
	{   
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			vecFileInfo.clear();
			SearchOneDayFiles(pResid, FindFileData.cFileName, 0, 0xFFFFFFFFFFFFFFFF, vecFileInfo);
			j_vec_file_info_t::iterator it = vecFileInfo.begin();
			for (; it!=vecFileInfo.end(); ++it)
			{
				if (it->tStartTime < begin_time)
					begin_time = it->tStartTime;
				if (it->tStoptime > end_time);
					end_time = it->tStoptime;
				nSize += it->nFileSize;
			}
		}
	} while (FindNextFile(hFind, &FindFileData));  
	FindClose(hFind);
#else
	DIR *dirPointer = NULL;
	struct dirent *entry;
	sprintf(file_name, "%s/%s", pDir, pResid);
	if ((dirPointer = opendir(file_name)) == NULL)
	{
		J_OS::LOGERROR("CXFile::ListFiles opendir error");
		return J_UNKNOW;
	}

	while ((entry = readdir(dirPointer)) != NULL)
	{
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			vecFileInfo.clear();
			SearchOneDayFiles(pResid, FindFileData.cFileName, 0, 0xFFFFFFFFFFFFFFFF, vecFileInfo);
			j_vec_file_info_t::iterator it = vecFileInfo.begin();
			for (; it!=vecFileInfo.end(); ++it)
			{
				if (it->tStartTime < begin_time)
					begin_time = it->tStartTime;
				if (it->tStoptime > end_time);
				end_time = it->tStoptime;
				nSize += it->nFileSize;
			}
		}
	}
	closedir(dirPointer);
#endif

	return J_OK;
}

j_result_t CXVodManager::SearchVodFiles(const j_char_t *pResid, j_time_t begin_time, j_time_t end_time, j_vec_file_info_t &vecFileInfo)
{
	j_string_t strBeginDate = JoTime->GetDate(begin_time);
	j_string_t strEndDate = JoTime->GetDate(end_time);
	SearchOneDayFiles(pResid, strBeginDate.c_str(), begin_time, end_time, vecFileInfo);
	if (strBeginDate != strEndDate)
		SearchOneDayFiles(pResid, strEndDate.c_str(), begin_time, end_time, vecFileInfo);

	return J_OK;
}

j_result_t CXVodManager::DelFiles(J_DelRecordCtrl &delRecordCtrl)
{
	if (delRecordCtrl.end_time == 0)
		delRecordCtrl.end_time = 0xFFFFFFFFFFFFFFFF;

	if (j_string_t(delRecordCtrl.resid) == "")//ɾ��ʱ���������¼��
	{
		j_vec_resid_t vecResid;
		GetRecordResid(vecResid);
		j_vec_resid_t::iterator it = vecResid.begin();
		for (; it!=vecResid.end(); ++it)
		{
			DeleteFilesByResid(it->c_str(), delRecordCtrl.begin_time, delRecordCtrl.end_time);
		}
	}
	else
	{
		if (delRecordCtrl.nType == 0)//��ʱ��ɾ��
		{
			DeleteFilesByResid(delRecordCtrl.resid, delRecordCtrl.begin_time, delRecordCtrl.end_time);
		}
		else//ɾ������
		{
			char file_name[256] = {0};
			sprintf(file_name, "%s/%s", CXConfig::GetVodPath(), delRecordCtrl.resid);
			DeleteDirectory(file_name);
		}
	}
	return J_OK;
}

j_result_t CXVodManager::GetRecordResid(j_vec_resid_t &vecResid)
{
	const char *pDir = CXConfig::GetVodPath();
	j_char_t file_name[256] = {0};
#ifdef WIN32
	WIN32_FIND_DATA FindFileData;  
	HANDLE hFind;  

	sprintf(file_name, "%s/*.*", pDir);
	hFind = FindFirstFile(file_name, &FindFileData);  
	if (hFind == INVALID_HANDLE_VALUE)   
	{  
		J_OS::LOGERROR("CXFile::GetRecordInfo FindFirstFile failed\n");  
		return J_UNKNOW;  
	}  

	do  
	{   
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			vecResid.push_back(FindFileData.cFileName);
		}
	} while (FindNextFile(hFind, &FindFileData));  
	FindClose(hFind);
#else
	DIR *dirPointer = NULL;
	struct dirent *entry;
	sprintf(file_name, "%s", pDir);
	if ((dirPointer = opendir(file_name)) == NULL)
	{
		J_OS::LOGERROR("CXFile::ListFiles opendir error");
		return J_UNKNOW;
	}

	while ((entry = readdir(dirPointer)) != NULL)
	{
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			vecResid.push_back(FindFileData.cFileName);
		}
	}
	closedir(dirPointer);
#endif
	return J_OK;
}

void CXVodManager::FillFileInfo(const char *pFileName, J_FileInfo &fileInfo)
{
	fileInfo.fileName = pFileName;
	const char *p = pFileName;
	const char *p2 = strstr(p, "_");
	if (p2 != NULL)
	{
		fileInfo.tStartTime = atoi(p2 + 1);
		
		p = p2 + 1;
		p2 = strstr(p, "_");
		if (p2 != NULL)
			fileInfo.tStoptime = atoi(p2 + 1);
	}
}

int CXVodManager::SearchOneDayFiles(const j_char_t *pResid, const char *pDate, j_time_t begin_time, j_time_t end_time, j_vec_file_info_t &vecFileInfo)
{
	const char *pDir = CXConfig::GetVodPath();
	j_char_t file_name[256] = {0};
#ifdef WIN32
	WIN32_FIND_DATA FindFileData;  
	HANDLE hFind;  

	sprintf(file_name, "%s/%s/%s/*.*", pDir, pResid, pDate);
	hFind = FindFirstFile(file_name, &FindFileData);  
	if (hFind == INVALID_HANDLE_VALUE)   
	{  
		J_OS::LOGERROR("CXFile::ListFiles FindFirstFile failed\n");  
		return J_UNKNOW;  
	}  

	J_FileInfo fileInfo;
	do  
	{   
		if (memcmp(FindFileData.cFileName, pResid, strlen(pResid)) == 0)
		{
			fileInfo.nFileSize = (FindFileData.nFileSizeHigh << 32) + FindFileData.nFileSizeLow;
			FillFileInfo(FindFileData.cFileName, fileInfo);
			if ((fileInfo.tStartTime >= begin_time && fileInfo.tStartTime <= end_time)
				|| (fileInfo.tStoptime >= begin_time && fileInfo.tStoptime <= end_time))
			{
				vecFileInfo.push_back(fileInfo);
			}
		}
	} while (FindNextFile(hFind, &FindFileData));  
	FindClose(hFind);
#else
	DIR *dirPointer = NULL;
	struct dirent *entry;
	sprintf(file_name, "%s/%s/%s", pDir, pResid, pDate);
	if ((dirPointer = opendir(file_name)) == NULL)
	{
		J_OS::LOGERROR("CXFile::ListFiles opendir error");
		return J_UNKNOW;
	}

	while ((entry = readdir(dirPointer)) != NULL)
	{
		if (memcmp(entry->d_name, pResid, strlen(pResid)) == 0)
		{
			fileInfo.nFileSize = (FindFileData.nFileSizeHigh << 32) + FindFileData.nFileSizeLow;
			FillFileInfo(entry->d_name, fileInfo);
			if ((fileInfo.tStartTime >= begin_time && fileInfo.tStartTime <= end_time)
				|| (fileInfo.tStoptime >= begin_time && fileInfo.tStoptime <= end_time))
			{
				vecFileInfo.push_back(fileInfo);
			}
		}
	}
	closedir(dirPointer);
#endif
	vecFileInfo.sort();

	return J_OK;
}

int CXVodManager::DeleteFilesByResid(const j_char_t *pResid, j_time_t begin_time, j_time_t end_time)
{
	j_vec_file_info_t vecFileInfo;
	const char *pDir = CXConfig::GetVodPath();
	j_char_t file_name[256] = {0};

#ifdef WIN32
	WIN32_FIND_DATA FindFileData;  
	HANDLE hFind;  

	sprintf(file_name, "%s/%s/*.*", pDir, pResid);
	hFind = FindFirstFile(file_name, &FindFileData);  
	if (hFind == INVALID_HANDLE_VALUE)   
	{  
		J_OS::LOGERROR("CXFile::GetRecordInfo FindFirstFile failed\n");  
		return J_UNKNOW;  
	}  

	do  
	{   
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			j_time_t date_time = atoi(FindFileData.cFileName);
			if (begin_time < date_time  && end_time > date_time + 86400)
			{
				memset(file_name, 0, sizeof(file_name));
				sprintf(file_name, "%s/%s/%s", pDir, pResid, FindFileData.cFileName);
				DeleteDirectory(file_name);
			}
			else if ((begin_time >= date_time && begin_time <= date_time + 86400)
				|| (end_time >= date_time && end_time <= date_time + 86400))
			{
				SearchOneDayFiles(pResid, FindFileData.cFileName, begin_time, end_time, vecFileInfo);
				j_vec_file_info_t::iterator it = vecFileInfo.begin();
				for (; it!=vecFileInfo.end(); ++it)
				{
					if ((it->tStartTime >= begin_time) && it->tStoptime <= end_time)
					{
						memset(file_name, 0, sizeof(file_name));
						sprintf(file_name, "%s/%s/%s/%s", pDir, pResid, FindFileData.cFileName, it->fileName.c_str());
						DeleteFile(file_name);
					}
				}
			}
		}
	} while (FindNextFile(hFind, &FindFileData));  
	FindClose(hFind);
#else
	DIR *dirPointer = NULL;
	struct dirent *entry;
	sprintf(file_name, "%s/%s", pDir, pResid);
	if ((dirPointer = opendir(file_name)) == NULL)
	{
		J_OS::LOGERROR("CXFile::ListFiles opendir error");
		return J_UNKNOW;
	}

	while ((entry = readdir(dirPointer)) != NULL)
	{
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			vecFileInfo.clear();
			SearchOneDayFiles(pResid, FindFileData.cFileName, 0, 0xFFFFFFFFFFFFFFFF, vecFileInfo);
			j_vec_file_info_t::iterator it = vecFileInfo.begin();
			for (; it!=vecFileInfo.end(); ++it)
			{
				if (it->tStartTime < begin_time)
					begin_time = it->tStartTime;
				if (it->tStoptime > end_time);
				end_time = it->tStoptime;
				nSize += it->nFileSize;
			}
		}
	}
	closedir(dirPointer);
#endif
	return J_OK;
}

int CXVodManager::DeleteDirectory(char *DirName)
{
#ifdef WIN32
	WIN32_FIND_DATA FindFileData;  
	HANDLE hFind;  
	char tempFileFind[200];
	sprintf(tempFileFind,"%s/*.*",DirName);
	hFind = FindFirstFile(tempFileFind, &FindFileData);  
	if (hFind == INVALID_HANDLE_VALUE)   
	{  
		J_OS::LOGERROR("CXFile::GetRecordInfo FindFirstFile failed\n");  
		return J_UNKNOW;  
	}  

	do
	{
		if (memcmp(FindFileData.cFileName, ".", 1) != 0 
			&& memcmp(FindFileData.cFileName, "..", 2) != 0)
		{
			char foundFileName[200];
			strcpy(foundFileName,FindFileData.cFileName);
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char tempDir[200];
				sprintf(tempDir,"%s/%s",DirName,foundFileName);
				DeleteDirectory(tempDir);
			}
			else
			{
				char tempFileName[200];
				sprintf(tempFileName,"%s/%s",DirName,foundFileName);
				DeleteFile(tempFileName);
			}
		}
	} while (FindNextFile(hFind, &FindFileData));  
	FindClose(hFind);
	if(!RemoveDirectory(DirName))
	{
		J_OS::LOGINFO("delete current directory failed,please try again");
		return J_UNKNOW;
	}
#endif
	return J_OK;
}