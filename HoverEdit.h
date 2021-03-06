//////////////////////////////////////////////////
//类名：CHoverEdit
//功能：具有热点跟踪的平面编辑框显示类
//修改：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2001.12.1
//////////////////////////////////////////////////
#if !defined(AFX_HOVEREDIT_H__372AC76D_6B84_435C_8300_9519EB021C8C__INCLUDED_)
#define AFX_HOVEREDIT_H__372AC76D_6B84_435C_8300_9519EB021C8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<class BaseClass>
class CTrackControl : public BaseClass
{
// Construction
public:
	CTrackControl()
	{
		m_bTracking=m_bHover=FALSE;
	}
	virtual ~CTrackControl() {}
	BOOLEAN IsHover()
	{
		return m_bHover;
	}
	
// Implementation
public:
	virtual void OnHoverEnter()=0;
	virtual void OnHoverLeave()=0;
	
	// Generated message map functions
protected:
	virtual LRESULT WindowProc(UINT nMessage, WPARAM wParam, LPARAM lParam)
	{
		LRESULT nResult=BaseClass::WindowProc(nMessage,wParam,lParam);
		switch(nMessage)
		{
		case WM_MOUSEMOVE:
			{
				if (!m_bTracking)
				{
					TRACKMOUSEEVENT Tme;
					Tme.cbSize = sizeof(Tme);
					Tme.hwndTrack = GetSafeHwnd();
					Tme.dwFlags = TME_LEAVE|TME_HOVER;
					Tme.dwHoverTime = 1;
					if (_TrackMouseEvent(&Tme))
						m_bTracking=TRUE;
				}
				break;				
			}
		case WM_MOUSEHOVER:
			m_bHover=TRUE;
			OnHoverEnter();
			break;
		case WM_MOUSELEAVE:
			m_bTracking=m_bHover=FALSE;
			OnHoverLeave();
			break;
		}
		return nResult;
	}

private:
	BOOL m_bTracking;
	BOOL m_bHover;
};

/////////////////////////////////////////////////////////////////////////////
// CHoverEdit window

class CHoverEdit : public CTrackControl<CEdit>
{
// Construction
public:
	CHoverEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHoverEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void OnHoverLeave();
	virtual void OnHoverEnter();
	virtual ~CHoverEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CHoverEdit)
	afx_msg void OnNcPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	inline void Redraw();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOVEREDIT_H__372AC76D_6B84_435C_8300_9519EB021C8C__INCLUDED_)
