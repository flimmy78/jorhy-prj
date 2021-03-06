#pragma once

// active_pluginCtrl.h : Declaration of the Cactive_pluginCtrl ActiveX Control class.


// Cactive_pluginCtrl : See active_pluginCtrl.cpp for implementation.

class Cactive_pluginCtrl : public COleControl
{
	DECLARE_DYNCREATE(Cactive_pluginCtrl)

// Constructor
public:
	Cactive_pluginCtrl();

// Overrides
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

public:
	LRESULT OnCallBack(WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	~Cactive_pluginCtrl();

	DECLARE_OLECREATE_EX(Cactive_pluginCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(Cactive_pluginCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(Cactive_pluginCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(Cactive_pluginCtrl)		// Type name and misc status

// Message maps
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	DECLARE_DISPATCH_MAP()

// Event maps
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
		dispidResgisterFunction = 2L,
		dispidPlugin_Interface = 1L,
	};

protected:
	BSTR Plugin_Interface(LONG l, BSTR p);
	LONG ResgisterFunction(LPDISPATCH fun, LONG l);

private:
	void DefaultInvoke(UINT nType, int args[],UINT argCount);
	BSTR SetWorkModel(char *js_workmodel);
	BSTR Play(char * js_playInfo);
	BSTR ChangeLayout(char * js_layout);
	BSTR ChangePath(char * js_path);
	BSTR SetLayout();
	BSTR StopAllPlay();
	BSTR VodPlayJump(char * js_time);
	BSTR GetWndParm(int nType);
	BSTR SleepPlayer(bool bSleep);
	BSTR SetRetValue(char *psz_ret);

	static void OnEvent(void *pUser,UINT nType, int args[],UINT argCount);

private:
	VOID *m_pl_ctrl;
	CComDispatchDriver m_CallBkPtz;
	CComDispatchDriver m_CallBkState;
	CComDispatchDriver m_CallBkVod;
	long volatile m_bCbReturn;
	DWORD m_lastInvokeTime;	
};

