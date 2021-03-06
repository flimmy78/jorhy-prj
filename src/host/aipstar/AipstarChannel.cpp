#include "AipstarChannel.h"
#include "AipstarStream.h"

CAipstarChannel::CAipstarChannel(const j_char_t *pResid, J_Obj *pOwner, j_int32_t nChannel, j_int32_t nStream, j_int32_t nMode)
{
	m_bOpened = false;

	m_pAdapter = dynamic_cast<CAipstarAdapter *> (pOwner);
	m_nChannel = nChannel;
	m_nStreamType = nStream;
	m_nProtocol = nMode;

	m_resid = pResid;
	m_pStream = NULL;
	//TMCC_PtzOpen(m_pAdapter->GetClientHandle(), m_nChannel - 1);
	m_hStream = TMCC_Init(TMCC_INITTYPE_REALSTREAM);
}

CAipstarChannel::~CAipstarChannel()
{
	//TMCC_PtzClose(m_pAdapter->GetClientHandle());
	//assert(nRet == TMCC_ERR_SUCCESS);

	TMCC_Done(m_hStream);
}

j_result_t CAipstarChannel::OpenStream(J_Obj *&pObj, CRingBuffer *pRingBuffer)
{
	if (m_pAdapter->GetStatus() != jo_dev_ready)
	{
		m_pAdapter->Broken();
		m_pAdapter->Relogin();
		m_bOpened = false;
		return J_DEV_BROKEN;
	}
	
	//m_pStream = pObj;
	if (m_bOpened && pObj != NULL)
	{
	    TMCC_MakeKeyFrame(m_hStream);
		(dynamic_cast<CAipstarStream *> (pObj))->AddRingBuffer(pRingBuffer);
		return J_OK;
	}

	pObj = new CAipstarStream(m_resid, m_nChannel);
	TMCC_RegisterStreamCallBack(m_hStream, CAipstarStream::OnStreamCallBack, pObj);
	j_result_t nRet = StartView();
	if (nRet != J_OK)
	{
		m_pAdapter->Broken();
		//m_pAdapter->Relogin();
		delete pObj;
		J_OS::LOGINFO("CAipstarChannel::OpenStream StartView error, ret = %d", nRet);
		return J_STREAM_ERROR;
	}

	m_bOpened = true;
	m_pStream = pObj;
	(dynamic_cast<CAipstarStream *> (pObj))->AddRingBuffer(pRingBuffer);
	(dynamic_cast<CAipstarStream *> (pObj))->Startup();

	return J_OK;
}

j_result_t CAipstarChannel::CloseStream(J_Obj *pObj, CRingBuffer *pRingBuffer)
{
	if (!m_bOpened)
		return J_OK;

	CAipstarStream *pStream = dynamic_cast<CAipstarStream *>(pObj);
	if (pStream == NULL)
		return J_OK;

	if (pStream->RingBufferCount() == 1)
	{
		StopView();

		m_bOpened = false;
		(dynamic_cast<CAipstarStream *> (pObj))->Shutdown();
		pStream->DelRingBuffer(pRingBuffer);
		delete pObj;
		J_OS::LOGINFO("7");
		m_pStream = NULL; 

		return J_NO_REF;
	}

	if (pStream->RingBufferCount() > 0)
		pStream->DelRingBuffer(pRingBuffer);

	return J_OK;
}

j_result_t CAipstarChannel::PtzControl(j_int32_t nCmd, j_int32_t nParam)
{
	j_int32_t ptzCmd = 0;
	int nRet = TMCC_ERR_SUCCESS;
	nRet = TMCC_PtzOpen(m_pAdapter->GetClientHandle(), m_nChannel - 1);
	if (TMCC_ERR_SUCCESS != nRet)
		J_OS::LOGINFO("TMCC_PtzOpen ret = %X", nRet);
	if (nCmd == jo_ptz_pre_set || nCmd == jo_ptz_pre_clr || nCmd == jo_ptz_goto_pre)
	{
		switch (nCmd)
		{
		case jo_ptz_pre_set:
			ptzCmd = PTZ_SET_PRESET;
			break;
		case jo_ptz_pre_clr:
			ptzCmd = PTZ_CLE_PRESET;
			break;
		case jo_ptz_goto_pre:
			ptzCmd = PTZ_GOTO_PRESET;
			break;
		}
		nRet = TMCC_PtzPreset(m_pAdapter->GetClientHandle(), ptzCmd, nParam, 5);
	}
	else
	{
		switch (nCmd)
		{
		case jo_ptz_up:
            ptzCmd = PTZ_UP;
			break;
		case jo_ptz_down:
            ptzCmd = PTZ_DOWN;
			break;
		case jo_ptz_left:
            ptzCmd = PTZ_LEFT;
			break;
		case jo_ptz_right:
            ptzCmd = PTZ_RIGHT;
			break;
		case jo_ptz_up_left:
            ptzCmd = PTZ_LEFTUP;
			break;
		case jo_ptz_up_right:
            ptzCmd = PTZ_RIGHTUP;
			break;
		case jo_ptz_down_left:
            ptzCmd = PTZ_LEFTDOWN;
			break;
		case jo_ptz_down_right:
            ptzCmd = PTZ_RIGHTDOWN;
			break;
		case jo_ptz_zoom_in:
            ptzCmd = PTZ_ZOOM_IN;
			break;
		case jo_ptz_room_out:
            ptzCmd = PTZ_ZOOM_OUT;
			break;
		case jo_ptz_focus_near:
            ptzCmd = PTZ_FOCUS_NEAR;
			break;
		case jo_ptz_focus_far:
            ptzCmd = PTZ_FOCUS_FAR;
			break;
		case jo_ptz_iris_open:
            ptzCmd = PTZ_IRIS_ENLARGE;
			break;
		case jo_ptz_iris_close:
            ptzCmd = PTZ_IRIS_SHRINK;
			break;
		}
		if (nParam == 0)
		{
			nRet = TMCC_PtzControl(m_pAdapter->GetClientHandle(), ptzCmd, 0, (nParam / 4 + 1));
		}
		else
		{
			nRet = TMCC_PtzControl(m_pAdapter->GetClientHandle(), ptzCmd, 1, (nParam / 4 + 1));
		}
	}

	if (nRet != TMCC_ERR_SUCCESS)
	{
	    J_OS::LOGINFO("CAipstarChannel::PtzControl Error %X", nRet);
	    //m_pAdapter->Relogin();
	}
	TMCC_PtzClose(m_pAdapter->GetClientHandle());
	if (TMCC_ERR_SUCCESS != nRet)
		J_OS::LOGINFO("TMCC_PtzClose ret = %X", nRet);

	return (nRet == TMCC_ERR_SUCCESS ? J_OK : J_UNKNOW);
}

j_result_t CAipstarChannel::StartView()
{
	//m_pAdapter->Logout();
    //m_pAdapter->Login();
	
	tmPlayRealStreamCfg_t realInfo = {0};
	realInfo.dwSize = sizeof(tmRealStreamInfo_t);
	strcpy(realInfo.szAddress, m_pAdapter->GetRemoteIp());
	realInfo.iPort = m_pAdapter->GetRemotePort();
	strcpy(realInfo.szUser, m_pAdapter->GetRemoteUser());
	strcpy(realInfo.szPass, m_pAdapter->GetRemotePw());
	realInfo.byChannel = m_nChannel - 1;
	realInfo.byStream = m_nStreamType;
	realInfo.byReConnectNum = 1;
	realInfo.iReConnectTime = 10;

	int nRet = TMCC_ConnectStream(m_hStream, &realInfo, NULL);
    if (nRet != TMCC_ERR_SUCCESS)
	{
	    J_OS::LOGINFO("CAipstarChannel::StartView Error %d", nRet);
	    return J_DEV_BROKEN;
	}
	TMCC_MakeKeyFrame(m_hStream);

	return J_OK;
}

j_result_t CAipstarChannel::StopView()
{
	int nRet = TMCC_CloseStream(m_hStream);
    if (nRet != TMCC_ERR_SUCCESS)
	{
	    J_OS::LOGINFO("CAipstarChannel::StopView Error %d", nRet);
	}

	return J_OK;
}

int CAipstarChannel::Broken()
{
	if (m_pStream != NULL)
	{
		(dynamic_cast<CAipstarStream *> (m_pStream))->Broken();
		return J_OK;
	}
		
	return J_UNKNOW;
}
