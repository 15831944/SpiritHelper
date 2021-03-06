/////////////////////////////////////////////////////////////////////////
// 类名：CSplashWnd
// 功能：动态菲屏显示效果，具有下面属性:
//		(-1	-- 随机显示,		0	-- 水平交错效果,		1	-- 垂直交错效果,
//		 2	-- 水平百叶窗效果,	3	-- 垂直百叶窗效果,		4	-- 随机积木效果,
//		 5	-- 滤镜效果,		6	-- 垂直平分扫描,		7	-- 水平平分扫描,
//		 8	-- 水平交叉移动,	9	-- 垂直交叉移动,		10	-- 垂直栅条效果,
//		 11	-- 水平栅条效果,	12	-- 环形扫锚效果,		13	-- 缩放效果,
//		 14	-- 矩形扩张效果,	15	-- 椭圆扩张效果,		16	-- 圆角矩形扩张,
//		 17	-- 渐显效果,		18	-- 渐隐效果,			19	-- 正常显示)	
// 修改：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
// 组织：未来工作室(Future Studio)
// 日期：2002.1.8
////////////////////////////////////////////////////////////////////////
#ifndef __SPLASHER_H__
#define __SPLASHER_H__

#include <vfw.h>				// 导入DrawDib库文件,jingzhou xu
#pragma comment(lib, "vfw32")
///////////////// Classes //////////////////////////
class CSplashWnd : public CWnd
{
public:
  CSplashWnd();
  ~CSplashWnd();

// Operations
  void SetBitmapToUse(const CString& sFilename);
  void SetBitmapToUse(UINT nResourceID);
  void SetBitmapToUse(LPCTSTR pszResourceName); 
  BOOL Create();
  void SetOKToClose() { m_bOKToClose = TRUE; };  
  
  // ---------------------------------------------------------
  // 名称: SetShowMode()
  // 功能：设置动态菲屏显示效果，具有下面属性:
  //		(-1	-- 随机显示,		0	-- 水平交错效果,		1	-- 垂直交错效果,
  //		 2	-- 水平百叶窗效果,	3	-- 垂直百叶窗效果,		4	-- 随机积木效果,
  //		 5	-- 滤镜效果,		6	-- 垂直平分扫描,		7	-- 水平平分扫描,
  //		 8	-- 水平交叉移动,	9	-- 垂直交叉移动,		10	-- 垂直栅条效果,
  //		 11	-- 水平栅条效果,	12	-- 环形扫锚效果,		13	-- 缩放效果,
  //		 14	-- 矩形扩张效果,	15	-- 椭圆扩张效果,		16	-- 圆角矩形扩张,
  //		 17	-- 渐显效果,		18	-- 渐隐效果,			19	-- 正常显示)	
  // 参数: nShowMode -- 显示效果，参见功能部分
  // 返回: 无
  // 编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
  // 组织: 未来工作室(Future Studio)
  // 日期: 2002.1.8
  // ---------------------------------------------------------
  inline void SetShowMode(int nShowMode) 
  { 
	  m_nShowMode = nShowMode;
  };

protected:
  //{{AFX_VIRTUAL(CSplashWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

  //{{AFX_MSG(CSplashWnd)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
  afx_msg BOOL OnQueryNewPalette();
  afx_msg void OnClose();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
  
  BOOL SelRelPal(BOOL bForceBkgnd);
  BOOL LoadBitmap();
  void CreatePaletteFromBitmap();

// -------------- 位图渐隐、渐显效果数据 -------------- //

  HBITMAP	m_bmpSrc;				// 源位图句柄
  HBITMAP	m_bmpDst;				// 目标位图句柄
  COLORREF*	m_clrSrc;				// 源位图数据地址
  COLORREF*	m_clrDst;				// 目标位图数据地址
  HDRAWDIB	m_hDrawDib;				// DrawDib设备句柄

// -------------- 位图渐隐、渐显效果数据结束 -------------- //

  int       m_nShowMode;            // 菲屏显示效果
  BOOL      m_bOKToClose;
  CBitmap   m_Bitmap;
  CPalette  m_Palette;
  int       m_nHeight;
  int       m_nWidth;
  CWnd      m_wndOwner;                   
  BOOL      m_bUseFile;
  LPCTSTR   m_pszResourceName;
  CString   m_sFilename;
};


class CSplashThread : public CWinThread
{
public:
  void HideSplash();
  void SetBitmapToUse(const CString& sFilename);
  void SetBitmapToUse(UINT nResourceID);
  void SetBitmapToUse(LPCTSTR pszResourceName); 
  void SetShowMode(int nShowMode);	// 设置动态菲屏显示效果

protected:
	CSplashThread();
  virtual ~CSplashThread();

	DECLARE_DYNCREATE(CSplashThread)

	//{{AFX_VIRTUAL(CSplashThread)
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CSplashThread)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  CSplashWnd m_SplashScreen;
};


#endif //__SPLASHER_H__


