//////////////////////////////////////////////////
//类名：CTransparentWnd
//功能：透明窗体实现
//作者：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2001.12.1
//////////////////////////////////////////////////
#include "stdafx.h"
#include "TransparentWnd.h"
#include "HelpTip.h"
#include "HelpTipDlg.h"
#include "DownloadAddress.h"
#include "About.h"
#include "Advertise.h"			// 广告特效窗体
#include "MirrorInWater.h"		// 水中倒影窗体

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 匿名名字空间，可存放全局成员(变量或涵数)
namespace
{
	// 自定义系统托盘消息函数
	const int WM_TRAYNOTIFY  = WM_USER + 100;  
	
	// 精灵运动方式(定时器ID)
	const int TIMER_SET			=	1;
	const int TIMER_WALK		=	2;
	const int TIMER_IDLE		=	3;
	const int TIMER_RUN			=   4;
	const int TIMER_PLAYSOUND	=	5;
	const int TEMER_DragDrop	=	6;
	
	// 精灵运动方式(毫秒数)
	const int g_SETSECOND	= 100;
	const int g_WALKSECOND	= 500;
	const int g_IDLESECOND	= 500;
	const int g_RUNSECOND	= 300;
	const int g_SOUNDSECOND	= 840;   // 播放声音0.84秒
	const int g_DRAGSECOND	= 60;
	
	// 精灵所在位图行号
	const int g_bmpRUN  = 0;
	const int g_bmpWALK = 1;
	const int g_bmpIDLE = 2;
	const int g_bmpFUNK = 3;
	
	// 每一帧大小60X60像素(宽、高，目前相等)
	const int g_nSIZE  = 60;
	
	// 设置开机程序自启动键值位置和其下新建子键值(可设为程序名)
	const TCHAR g_cszAutoRunKey[]= _T( "Software\\Microsoft\\windows\\CurrentVersion\\Run" );
	const TCHAR	g_cszWindowClass[] = _T("HelpTip");
}

//********************************************************************************
//* 名称:	CTransparentWnd()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	构造涵数，在此初始化变量
//********************************************************************************
CTransparentWnd::CTransparentWnd()
{

}

//********************************************************************************
//* 名称:	~CTransparentWnd()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	析构涵数，可在此进行清扫工作
//********************************************************************************
CTransparentWnd::~CTransparentWnd()
{
	// 关闭拖动窗体时，窗体实时移动系统设置
//	SystemParametersInfo(SPI_SETDRAGFULLWINDOWS,false,NULL,SPIF_SENDCHANGE);

	// 消毁广告窗体
	if (m_pADdlg)
	{
		delete m_pADdlg;
		m_pADdlg = NULL;
 	}

	// 消毁水中倒影窗体
	if(m_pMirrorInWaterDlg)
	{
		delete m_pMirrorInWaterDlg;
		m_pMirrorInWaterDlg	= NULL;
	}
}	

BEGIN_MESSAGE_MAP(CTransparentWnd, CWnd)
	//{{AFX_MSG_MAP(CTransparentWnd)
	ON_MESSAGE(WM_TRAYNOTIFY, OnTrayNotification)  // 自定义系统托盘消息涵数 
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDR_EXIT, OnExit)
	ON_COMMAND(IDR_FULLSCREEN_WALK, OnFullscreenWalk)
	ON_COMMAND(IDR_SHOWHIDE, OnShowhide)
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(IDR_DOWNLOAD, OnDownload)
	ON_COMMAND(IDR_ACTION_RUN, OnActionRun)
	ON_COMMAND(IDR_ACTION_WALK, OnActionWalk)
	ON_COMMAND(IDR_ACTION_IDLE, OnActionIdle)
	ON_WM_MEASUREITEM()
	ON_WM_MENUCHAR()
	ON_COMMAND(IDR_ABOUT, OnAbout)
	ON_WM_LBUTTONUP()
	ON_COMMAND(IDR_AUTORUN, OnAutorun)
	ON_COMMAND(IDR_EFFECT_WATER, OnEffectWater)
	ON_COMMAND(IDR_EFFECT_FIRE, OnEffectFire)
	ON_COMMAND(IDR_EFFECT_PLASMA, OnEffectPlasma)
	ON_COMMAND(IDR_EFFECT_FONT, OnEffectFont)
	ON_COMMAND(IDR_EFFECT_ELECTRICITY, OnEffectElectricity)
	ON_COMMAND(IDR_EFFECT_PARTICLE, OnEffectParticle)
	ON_COMMAND(IDR_EFFECT_SNOW, OnEffectSnow)
	ON_COMMAND(IDR_EFFECT_SCREENSAVER, OnEffectScreensaver)
	ON_COMMAND(IDR_EFFECT_LOGO, OnEffectLogo)
	ON_COMMAND(IDR_EFFECT_FIREWORKS, OnEffectFireworks)
	ON_COMMAND(ID_MIRROR_IN_WATER, OnMirrorInWater)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//********************************************************************************
//* 名称:	CreateTransparent()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			pTitle	:	创建的窗体标题
//*			rect	:	创建的窗体矩形大小
//*	返回:
//*			无
//* 功能：	建立透明窗体
//********************************************************************************
void CTransparentWnd::CreateTransparent(LPCTSTR pTitle, RECT &rect)
{
	CreateEx(	0,
		AfxRegisterWndClass(0,AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
						pTitle,
						WS_POPUP ,
						rect,
						NULL,
						NULL,
						NULL );

}

//********************************************************************************
//* 名称：	GetSourceHtml()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			theUrl	:	下载网页网址
//*	返回:
//*			TRUE	:	成功
//*			FALSE	:	失败
//* 功能：
//*			下载网页涵数
//********************************************************************************
BOOL CTransparentWnd::GetSourceHtml(CString theUrl) 
{
 CInternetSession session;
 CInternetFile* file = NULL;
 try
 {
    // 试着连接到指定URL
    file = (CInternetFile*) session.OpenURL(theUrl); 
 }
 catch (CInternetException* m_pException)
 {
	TRACE0(_T("到指定服务器的连接建立失败...\n"));	
    // 如果有错误的话，置文件为空
    file = NULL; 
    m_pException->Delete();
    return FALSE;
 }

 // 用dataStore来保存读取的网页文件
 CStdioFile dataStore;

 if (file)
 {
    CString  somecode;							// 也可采用LPTSTR类型，将不会删除文本中的\n回车符

	BOOL bIsOk = dataStore.Open(strPath+"\\Tip.txt",
		CFile::modeCreate 
		| CFile::modeWrite 
		| CFile::shareDenyWrite 
		| CFile::typeText);
	
	if (!bIsOk)
		return FALSE;
	
	// 读写网页文件，直到为空
	while (NULL != file->ReadString(somecode)) // 如果采用LPTSTR类型，读取最大个数nMax置0，使它遇空字符时结束
	{
		dataStore.WriteString(somecode);
		dataStore.WriteString("\n");		   // 如果somecode采用LPTSTR类型,可不用此句
	}
	
	file->Close();
	delete file;
 }
 else
 {
	TRACE0(_T("到指定服务器的连接建立失败...\n"));	
    return FALSE;
 }

 return TRUE;
}

//********************************************************************************
//* 名称：	SoundPlay()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			播放内镶WAV声音文件
//********************************************************************************
void CTransparentWnd::SoundPlay(void)
{
    // 先关闭原声音播放
	PlaySound("IDR_WAVE",AfxGetResourceHandle(),SND_RESOURCE|SND_PURGE|SND_NODEFAULT  ); 
    // 设置播放声音时间0.84秒
	SetTimer(TIMER_PLAYSOUND,g_SOUNDSECOND,NULL); 
	// 资源WAV文件的ID须加双引号，用下API函数播放
    PlaySound("IDR_WAVE",AfxGetResourceHandle(),SND_RESOURCE|SND_ASYNC|SND_NODEFAULT  ); 
}

//********************************************************************************
//* 名称：	SetupRegion()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			pDC	:	传入透明显示位图的dc指针
//*	返回:
//*			无
//* 功能：
//*			将窗体背景透明化
//********************************************************************************
void CTransparentWnd::SetupRegion(CDC *pDC)
{

	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));
	
	HRGN hRgn = NULL;

	CBitmap			&cBitmap=m_bmpDraw;
	CRect			cRect;

	GetWindowRect(&cRect);
	CPoint ptOrg=cRect.TopLeft();

	// 获取位图大小
	BITMAP bm;
	cBitmap.GetObject(sizeof(bm),&bm);
	CRect rcNewWnd=CRect(ptOrg,CSize(bm.bmWidth,bm.bmHeight));

		// 创建兼容DC，只便扫描它的元素
		HDC hMemDC = CreateCompatibleDC(NULL);
		if (hMemDC)
		{
			// 创建一32位位图，并放到兼容DC中 
			BITMAPINFOHEADER RGB32BITSBITMAPINFO = {	
					sizeof(BITMAPINFOHEADER),	// 信息头大小 
					bm.bmWidth,					// 位图宽度 
					bm.bmHeight,				// 位图高度
					1,							// 位面数(总设为1) 
					32,							// 像素位数 
					BI_RGB,						// 压缩标志 
					0,							// 图像大小 
					0,							// 水平像素数 
					0,							// 垂直像素数 
					0,							// 实际使用的颜色索引数 
					0							// 要求的颜色索引数
			};
			VOID * pbits32; 
			HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)&RGB32BITSBITMAPINFO, DIB_RGB_COLORS, &pbits32, NULL, 0);
			if (hbm32)
			{
				HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);

				// 创建DC，放位图到它里面
				HDC hDC = CreateCompatibleDC(hMemDC);
				if (hDC)
				{
					// 获取每行多少位
					BITMAP bm32;
					GetObject(hbm32, sizeof(bm32), &bm32);
					while (bm32.bmWidthBytes % 4)
						bm32.bmWidthBytes++;

					// 复制位图到兼容DC中
					HBITMAP holdBmp = (HBITMAP)SelectObject(hDC, (HBITMAP)cBitmap );
					BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDC, 0, 0, SRCCOPY);

					// 屏蔽色范围
					COLORREF cTolerance = 0x101010;
					COLORREF cTransparentColor=::GetPixel(hMemDC,0,0);

					// 为了更好的实现，将使用 ExtCreateRegion() 涵数来创建一区域，这个涵数要用来
					// RGNDATA 结构. 我们将用ALLOC_UNI来添加矩形框。
					#define ALLOC_UNIT	100
					DWORD maxRects = ALLOC_UNIT;
					HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
					RGNDATA *pData = (RGNDATA *)GlobalLock(hData);
					pData->rdh.dwSize = sizeof(RGNDATAHEADER);
					pData->rdh.iType = RDH_RECTANGLES;
					pData->rdh.nCount = pData->rdh.nRgnSize = 0;
					SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

					// 为了获得透明像素，要分别保留其要透明背景色的最高、最低值
					// (其中从COLORREF转换为RGB时，两端红、蓝色需互换，jingzhou xu)
					BYTE lr = GetBValue(cTransparentColor);
					BYTE lg = GetGValue(cTransparentColor);
					BYTE lb = GetRValue(cTransparentColor);
					BYTE hr = min(0xff, lr + GetBValue(cTolerance));
					BYTE hg = min(0xff, lg + GetGValue(cTolerance));
					BYTE hb = min(0xff, lb + GetRValue(cTolerance));

					// 从下往上扫描每一个位图(位图是竖直反方向的)
					BYTE *p32 = (BYTE *)bm32.bmBits + (bm32.bmHeight - 1) * bm32.bmWidthBytes;
					for (int y = 0; y < bm.bmHeight; y++)
					{
						// 从左往右扫描每一个像素
						for (int x = 0; x < bm.bmWidth; x++)
						{
							// 连续搜索非透明元素
							int x0 = x;
							LONG *p = (LONG *)p32 + x;
							while (x < bm.bmWidth)
							{
								BYTE b = GetRValue(*p);
								if (b >= lr && b <= hr)
								{
									b = GetGValue(*p);
									if (b >= lg && b <= hg)
									{
										b = GetBValue(*p);
										if (b >= lb && b <= hb)
											// 如果是透明元素的话
											break;
									}
								}
								p++;
								x++;
							}

							if (x > x0)
							{
								// 添加像素(x0, y) 到 (x, y+1) ，作为区域中的新矩形
								if (pData->rdh.nCount >= maxRects)
								{
									GlobalUnlock(hData);
									maxRects += ALLOC_UNIT;
									hData = GlobalReAlloc(hData, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), GMEM_MOVEABLE);
									pData = (RGNDATA *)GlobalLock(hData);
								}
								RECT *pr = (RECT *)&pData->Buffer;
								SetRect(&pr[pData->rdh.nCount], x0, y, x, y+1);
								if (x0 < pData->rdh.rcBound.left)
									pData->rdh.rcBound.left = x0;
								if (y < pData->rdh.rcBound.top)
									pData->rdh.rcBound.top = y;
								if (x > pData->rdh.rcBound.right)
									pData->rdh.rcBound.right = x;
								if (y+1 > pData->rdh.rcBound.bottom)
									pData->rdh.rcBound.bottom = y+1;
								pData->rdh.nCount++;

								// 在Windows98中, 如果矩形数太多的话(ie: > 4000)，ExtCreateRegion()涵数可能失败. 
								//　因此，我们必须通过多步来创建矩形。
								if (2000 == pData->rdh.nCount)
								{
									HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
									if (hRgn)
									{
										CombineRgn(hRgn, hRgn, h, RGN_OR);
										DeleteObject(h);
									}
									else
										hRgn = h;
									pData->rdh.nCount = 0;
									SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
								}
							}
						}

						// 到下一行(注意： 位图是竖直反方向的)
						p32 -= bm32.bmWidthBytes;
					}

					// 根据保留的矩形来创建或廷伸区域
					HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pData);
					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, h, RGN_OR);
						DeleteObject(h);
					}
					else
						hRgn = h;

					// 释放
					GlobalFree(hData);
					SelectObject(hDC, holdBmp);
					DeleteDC(hDC);
				}

				DeleteObject(SelectObject(hMemDC, holdBmp));
			}

			DeleteDC(hMemDC);
		}	

	SetWindowRgn(hRgn, TRUE);
	MoveWindow(rcNewWnd);
}

//********************************************************************************
//* 名称：	DoSet()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：
//*			判断该进行何种运行方式
//********************************************************************************
void CTransparentWnd::DoSet(void)
{
	int iSW=GetSystemMetrics(SM_CXFULLSCREEN);
	int	iSH=GetSystemMetrics(SM_CYFULLSCREEN);

	KillTimer(TIMER_SET);
	switch(rand()%5)
	{
		case(0):  // 散步
		case(1):
		m_ptDest.x=rand()%(iSW - 10);
		m_ptDest.y=rand()%(iSH - 10);
		SetTimer(TIMER_WALK,g_WALKSECOND,NULL);
		break;

		case(2):  // 奔跑
		case(3):  
		m_ptDest.x=rand()%(iSW - 10);
		m_ptDest.y=rand()%(iSH - 10);
		SetTimer(TIMER_RUN,g_RUNSECOND,NULL);
		break;

	    case(4):  // 休息
		SetTimer(TIMER_IDLE,g_IDLESECOND,NULL);
		break;
	}
}

//********************************************************************************
//* 名称：	DoWalk()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			散步处理
//********************************************************************************
void CTransparentWnd::DoWalk(void)
{
	CRect rcW;
	GetWindowRect(rcW);

	// 决定全屏或局部运动的范围
	if(m_bFullScreenWalk)
	{
		if((m_ptDest.x> rcW.left && m_ptDest.x < rcW.right)||(m_ptDest.y> rcW.top && m_ptDest.y < rcW.bottom ))
		{
			KillTimer(TIMER_WALK);		
			SetTimer(TIMER_SET,g_SETSECOND,NULL);
			return;
		}
		
		CPoint ptOffset((m_ptDest.x > rcW.left)?5:-5,(m_ptDest.y > rcW.top)?5:-5);

		rcW+=ptOffset;
		MoveWindow(rcW);
		
		if(m_ptDest.x<rcW.left)		// 向左运动
		{
			m_iLastDirection=1;
		}
		else						// 向右运动
		{
			m_iLastDirection=2;
		}
	
	}
	else
	{
		int xcounter=10,ycounter=10;

		m_Rect=GetTopWndTitleRect();
		if(rcW.left < m_Rect.left +30 || 2 == m_iLastDirection) 
		{
			xcounter=10;
			m_iLastDirection=2;
		}
		if(rcW.left > m_Rect.right -30 || 1 == m_iLastDirection)
		{
			xcounter=-10;
			m_iLastDirection=1;
		}
		ycounter=0;                       // 上、下位置不变

		rcW.top =m_Rect.top-rcW.Height();
		rcW.bottom =m_Rect.top;

		CPoint ptOffset(xcounter,ycounter);
 		rcW+=ptOffset;

		MoveWindow(rcW);

		if(2 == m_iLastDirection)           // 局部运动时，往右为奔跑
		{
//			KillTimer(GetCurAction());
//			SetTimer(TIMER_RUN,g_RUNSECOND,NULL);	
		}
		else if(0 == (m_iAniSeq%30))
		{
			// 休息后会自动选择新的运动方式
			KillTimer(GetCurAction());
			SetTimer(TIMER_IDLE,g_IDLESECOND,NULL);	
		}

	}

	CWindowDC dc(this);
	// 映射相应帧位图到m_bmpDraw中(向左或向右)
	(2 == m_iLastDirection) ? Map2Frame(&dc,m_bmpMirrorMap,4-(m_iAniSeq%5), g_bmpWALK) : Map2Frame(&dc,m_bmpMap,m_iAniSeq%5, g_bmpWALK);

	SetupRegion(&dc);
	Invalidate();
}

//********************************************************************************
//* 名称：	DoRun()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			奔跑处理
//********************************************************************************
void CTransparentWnd::DoRun(void)
{
	CRect rcW;
	GetWindowRect(rcW);

	// 决定全屏或局部运动的范围
	if(m_bFullScreenWalk)
	{
		if((m_ptDest.x> rcW.left && m_ptDest.x < rcW.right)||(m_ptDest.y> rcW.top && m_ptDest.y < rcW.bottom ))
		{
			KillTimer(TIMER_RUN);
			SetTimer(TIMER_SET,g_SETSECOND,NULL);
			return;
		}
		CPoint ptOffset((m_ptDest.x > rcW.left)?10:-10,(m_ptDest.y > rcW.top)?5:-5);
		rcW+=ptOffset;
		MoveWindow(rcW);
		
		if(m_ptDest.x<rcW.left)			// 向左运动
		{
			m_iLastDirection=1;
		}
		else							// 向右运动
		{
			m_iLastDirection=2;
		}
	}
	else
	{
		int xcounter=10,ycounter=10;

		m_Rect=GetTopWndTitleRect();
		if(rcW.left < m_Rect.left +30 || 2 == m_iLastDirection) 
		{
			xcounter=10;
			m_iLastDirection=2;
		}
		if(rcW.left > m_Rect.right -30 || 1 == m_iLastDirection)
		{
			xcounter=-10;
			m_iLastDirection=1;
		}
		ycounter=0;                        // 上、下位置不变
		
		rcW.top =m_Rect.top-rcW.Height();
		rcW.bottom =m_Rect.top;

		CPoint ptOffset(xcounter,ycounter);
 		rcW+=ptOffset;

		MoveWindow(rcW);

		if(1 == m_iLastDirection)			// 局部运动时，往左为散步
		{
//			KillTimer(GetCurAction());
//			SetTimer(TIMER_WALK,g_WALKSECOND,NULL);	
		}
		else if(0 == (m_iAniSeq%30))
		{
			// 休息后会自动选择新的运动方式
			KillTimer(GetCurAction());
			SetTimer(TIMER_IDLE,g_IDLESECOND,NULL);	
		}

	}

	CWindowDC dc(this);
	// 映射相应帧位图到m_bmpDraw中(向左或向右)
	(2 == m_iLastDirection) ? Map2Frame(&dc,m_bmpMirrorMap,4-(m_iAniSeq%5), g_bmpRUN) : Map2Frame(&dc,m_bmpMap,m_iAniSeq%5, g_bmpRUN);
	
	SetupRegion(&dc);
	Invalidate();
}

//********************************************************************************
//* 名称：	DoIdle()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			休息处理
//********************************************************************************
void CTransparentWnd::DoIdle(void)
{
	if(0 == m_iAniSeq%12)
	{
		if(IsWindowVisible())
			SoundPlay();				// 当窗体显示时，播放声音    

		// 显示提示窗信息
	    CRect rc;
		GetWindowRect(&rc);
		CPoint pt;
		pt.x=rc.left +rc.Width ()/2;
		pt.y=rc.top;
		if(m_bDownloadFailed)
		{
			CString str;
			if(NULL == m_sdFile.ReadString(str))
			{
				m_sdFile.SeekToBegin();
				m_sdFile.ReadString(str);
			}
			if(IsWindowVisible())
				m_Tip.Show(str,&pt);	// 注：同一行中显示的字数不能过大
		}
		else
			if(IsWindowVisible())
				m_Tip.Show(_T("欢迎您，来到未来工作室！"),&pt);
		
		if(IsWindowVisible())
			m_Tip.Hide();

		KillTimer(TIMER_IDLE);
		SetTimer(TIMER_SET,g_SETSECOND,NULL);
		return;
	}
	
	CWindowDC dc(this);
	// 映射相应帧位图到m_bmpDraw中(注：4-(m_iAniSeq%4)中第一个4本来应该是(4-1)，但因为总位图中IDLE这一行最后少一帧，水平映射后就应该加1才对)
	(2 == m_iLastDirection) ? Map2Frame(&dc,m_bmpMirrorMap,4-(m_iAniSeq%4), g_bmpIDLE) : Map2Frame(&dc,m_bmpMap,m_iAniSeq%4, g_bmpIDLE);

	SetupRegion(&dc);
	Invalidate();
}

//********************************************************************************
//* 名称：	DoDrag()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：
//*			拖动窗体时行为表现
//********************************************************************************
void CTransparentWnd::DoDrag(void)
{

	CRect rcW;
	POINT point;
	GetWindowRect(rcW);

	// 实现拖动时窗体跟着移动
    ::GetCursorPos(&point);			
	MoveWindow(point.x-m_ptCurPoint.x ,point.y-m_ptCurPoint.y ,rcW.Width(),rcW.Height() ,true); 

	CWindowDC dc(this);
	// 映射相应帧位图到m_bmpDraw中
	Map2Frame(&dc,m_bmpMap,m_iAniSeq%4, g_bmpFUNK);

	SetupRegion(&dc);
	Invalidate();

}

//********************************************************************************
//* 名称：	OnTimer()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			nIDEvent	:	定时器ID值
//*	返回:
//*			无
//* 功能：	
//*			触发定时器
//********************************************************************************
void CTransparentWnd::OnTimer(UINT nIDEvent) 
{

	switch(nIDEvent)
	{
	case(TIMER_SET):				// 判定何种运行方式
		DoSet();
		break;

	case(TIMER_WALK):				// 散步
		DoWalk();
		break;

	case(TIMER_IDLE):				// 休息
		DoIdle();
		break;

	case(TIMER_RUN):				// 奔跑
		DoRun();
		break;

	case(TIMER_PLAYSOUND):			// 播放声音
		KillTimer(TIMER_PLAYSOUND);
		break;

	case(TEMER_DragDrop):
		DoDrag();					// 实时拖动
		break;

	default:
		break;
	}
	
	m_iAction= nIDEvent;			// 当前行为表现方式标志

	m_iAniSeq++;					// 下一幅图像       
	if(m_iAniSeq>80) m_iAniSeq=0;

	CWnd::OnTimer(nIDEvent);
}

//********************************************************************************
//* 名称:	OnLButtonDown()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	实现窗体无标题拖动
//********************************************************************************
void CTransparentWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnLButtonDown(nFlags, point);

	KillTimer(GetCurAction());						// 关闭当前行为表现

	SetTimer(TEMER_DragDrop,g_DRAGSECOND,NULL);		// 打开拖动时计数器

	// 鼠标移动到窗体时改变鼠标指针
	::SetCursor(AfxGetApp()->LoadCursor(IDC_DROP));

	m_bDragDrop=true;								// 精灵助手拖动标志为真
	SetCapture();									// 设置鼠标全屏捕捉
    m_ptCurPoint=point;								// 记下按下时的鼠标坐标

//	PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x,point.y));

}

//********************************************************************************
//* 名称:	OnLButtonUp()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	实现窗体实时拖动效果
//********************************************************************************
void CTransparentWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	
	m_bDragDrop=false;							// 精灵助手拖动标志为假
	ReleaseCapture();
	KillTimer(TEMER_DragDrop);					// 关闭拖动时记数器 
	SetTimer(TIMER_SET,g_SETSECOND,NULL);
	
	CWnd::OnLButtonUp(nFlags, point);
}

//********************************************************************************
//* 名称:	OnCreate()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 参数:
//*			lpCreateStruct	:	窗体结构风格
//* 返回:	无
//* 功能:	创建窗体,可在此进行初始化工作
//********************************************************************************
int CTransparentWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (-1 == CWnd::OnCreate(lpCreateStruct))
		return -1;
	
	// 系统区显示的图标文件
	m_hIcon[0]=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
	// 创建系统区图标
	if (!m_TrayIcon.Create(this, 
				IDR_POPUP, 
				_T("精灵助手 V1.1"), 
				m_hIcon, 
				1,								// 一幅图像
				1000,							// 延时1秒变化
				WM_TRAYNOTIFY))
	{
		AfxMessageBox(_T("错误：创建系统图标失败！"), MB_OK | MB_ICONSTOP);
		return -1;
	}

	// 获取主程序所在路径,存在全局变量strPath中
	GetModuleFileName(NULL,strPath.GetBufferSetLength (MAX_PATH+1),MAX_PATH);
	strPath.ReleaseBuffer ();
    int nPos;
	nPos=strPath.ReverseFind ('\\');
	strPath=strPath.Left (nPos);

	CFileFind  fFind;
	BOOL bFailed;
	bFailed=fFind.FindFile(strPath+"\\Tip.txt",0);
    // 是否已下载提示文件，否则下载它并打开文件
	if(!bFailed)
		m_bDownloadFailed=GetSourceHtml("http://www.microsoft.com/");	// 下载提示文件的默认网址
	else
		m_bDownloadFailed=TRUE;
    fFind.Close ();
	if(m_bDownloadFailed)
		m_bSuccess=m_bDownloadFailed=m_sdFile.Open(strPath+"\\Tip.txt",
		CFile::modeRead | CFile::typeText);
	
	// 初始化动画计数等
	m_iAniSeq	= 0;
	strPath		= "";	
	m_bSuccess	= FALSE;
	m_pADdlg	= NULL;

	// 建立信息提示窗
	m_Tip.Create(GetDesktopWindow());

	// 全屏运动标志初始置真
	m_bFullScreenWalk = TRUE;										

	// 打开拖动窗体时，窗体实时移动系统设置
//	SystemParametersInfo(SPI_SETDRAGFULLWINDOWS,true,NULL,SPIF_SENDCHANGE);

	// 窗体实时拖动时变量初始化
	m_bDragDrop=false;  
	m_ptCurPoint=CPoint(0,0);

	// 判断开机自启动标志是否已注册
	m_bAutorun = StartupAutorun(TRUE);

	// 置窗体初始显示位置
	CRect rcW;
	GetWindowRect(&rcW);
	m_Rect		= GetTopWndTitleRect();	
	rcW.top		= m_Rect.top-rcW.Height();
	rcW.bottom	= m_Rect.top;
	rcW.left	= 10;
	// 置窗体为最顶层显示及其位置
	SetWindowPos(&wndTopMost,rcW.left,rcW.top,rcW.Width(),rcW.Height(),SWP_NOSIZE|SWP_NOMOVE);

	// 同时在系统区上面显示广告窗体，只能是无模式对话框
    m_pADdlg = new CAdvertise(this);
    m_pADdlg->Create(IDD_ADVERTISE,this);
	m_pADdlg->ShowWindow(SW_SHOW);

	// 水中倒影无模式对话框，初始为空
	m_pMirrorInWaterDlg	= NULL;

	// 载入精灵总位图
	m_bmpMap.LoadBitmap("SPRITEMAP");

	// 将同一个总位图水平镜像后放到m_bmpMirrorMap中
	m_bmpMirrorMap.LoadBitmap("SPRITEMAP");
	MirrorLeftAndRightHalves(&m_bmpMirrorMap);
/*  // 或用下面方法都可以
// -------  将同一个总位图水平镜像后放到m_bmpMirrorMap中 ------------------
	CBitmap bmpTemp;
	bmpTemp.LoadBitmap("SPRITEMAP");
	m_bmpMirrorMap.Attach(GetInvertedBitmap(bmpTemp));
// ----------------------------------------------------
*/
	// 精灵初始运动方式
	m_iAction = 1;
	SetTimer(TIMER_SET,g_SETSECOND,NULL);

	return 0;
}

//********************************************************************************
//* 名称:	OnTrayNotification()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	系统区创建的图标映射涵数
//********************************************************************************
LRESULT CTransparentWnd::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
  
  return m_TrayIcon.OnTrayNotification(wParam, lParam);
}

//********************************************************************************
//* 名称:	OnInitMenuPopup()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	加此涵数，使动态新增菜单条同步以XP风格显示
//********************************************************************************
void CTransparentWnd::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	if(!bSysMenu){
		if(BCMenu::IsMenu(pPopupMenu)) BCMenu::UpdateMenu(pPopupMenu);
	}
}

//********************************************************************************
//* 名称:	OnMeasureItem()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	重新测量菜单条
//********************************************************************************
void CTransparentWnd::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	BOOL setflag=FALSE;
	if(ODT_MENU == lpMeasureItemStruct->CtlType){
		if(IsMenu((HMENU)lpMeasureItemStruct->itemID)&&BCMenu::IsMenu((HMENU)lpMeasureItemStruct->itemID)){
			m_popmenu.MeasureItem(lpMeasureItemStruct);
			setflag=TRUE;
		}
	}
	if(!setflag)CWnd::OnMeasureItem(nIDCtl, lpMeasureItemStruct);	
}

//********************************************************************************
//* 名称:	OnMenuChar()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	重新绘制菜单条
//********************************************************************************
LRESULT CTransparentWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	LRESULT lresult;
	if(BCMenu::IsMenu(pMenu))
		lresult=BCMenu::FindKeyboardShortcut(nChar, nFlags, pMenu);
	else
		lresult=CWnd::OnMenuChar(nChar, nFlags, pMenu);

	return(lresult);
	
}

//********************************************************************************
//* 名称:	OnEraseBkgnd()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 功能:	擦除背景
//* 附注:	要想正常显示，必须保留该涵数
//********************************************************************************
BOOL CTransparentWnd::OnEraseBkgnd(CDC* pDC) 
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	CRect	rect;
	GetWindowRect(&rect);

	CDC memDC;
	CBitmap			&cBitmap=m_bmpDraw;;
	CBitmap*		pOldMemBmp = NULL;

	memDC.CreateCompatibleDC(pDC);
	pOldMemBmp = memDC.SelectObject(&cBitmap);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	if (pOldMemBmp) memDC.SelectObject( pOldMemBmp );

	return TRUE;
}

//********************************************************************************
//* 名称:	OnPaint()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 参数:
//*			无
//* 返回:	无
//* 功能:	重绘
//* 附注:	要想正常显示，必须保留该涵数
//********************************************************************************
void CTransparentWnd::OnPaint()
{
	CPaintDC dc(this);
	
}

//********************************************************************************
//* 名称:	OnRButtonDown()
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 参数:
//*			nFlags	:	右键按下标志位
//*			point	:	右键按下屏幕坐标
//* 返回:	无
//* 功能:	鼠标右键按下
//********************************************************************************
void CTransparentWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnRButtonDown(nFlags, point);

	// 屏幕上弹出右键菜单
	ClientToScreen(&point);
	RButtonMenu(point);
}

//********************************************************************************
//* 名称:	RButtonMenu
//* 作者:	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 参数:
//*			point:		右键菜单显示时屏幕坐标
//* 返回:	无
//* 功能:
//*			公共鼠标右键弹出菜单
//********************************************************************************
void CTransparentWnd::RButtonMenu(CPoint& point) 
{
	// 重新载入前删除
	if(m_popmenu)
		m_popmenu.DestroyMenu();

	m_popmenu.LoadMenu(IDR_POPUP);
	BCMenu::SetMenuDrawMode(BCMENU_DRAWMODE_XP);

	// 设置选中时的复选框效果
	if(m_bFullScreenWalk) 
		m_popmenu.ModifyODMenu(NULL, IDR_FULLSCREEN_WALK,IDB_CHECK);
	// 设置开机自启动复选框效果
	if(m_bAutorun)
		m_popmenu.ModifyODMenu(NULL, IDR_AUTORUN,IDB_CHECK);

	
	// 设置特效窗体溶浆显示效果
	if(m_pADdlg->GetPlasmaRoutine())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_PLASMA,IDB_CHECK);
	// 设置特效窗体火焰显示效果
	if(m_pADdlg->GetFireRoutine())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_FIRE,IDB_CHECK);
	// 设置特效窗体水纹显示效果
	if(m_pADdlg->GetWaterRoutine())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_WATER,IDB_CHECK);
	// 设置特效窗体仿爆炸等显示效果
	if(m_pADdlg->GetParticleRoutine())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_PARTICLE,IDB_CHECK);

	// 设置特效窗体仿闪电显示效果
	if(m_pADdlg->GetShowElectricity())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_ELECTRICITY,IDB_CHECK);
	// 设置特效窗体背景文本显示效果
	if(m_pADdlg->GetShowFont())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_FONT,IDB_CHECK);
	// 设置特效窗体雪花显示效果
	if(m_pADdlg->GetShowSnow())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_SNOW,IDB_CHECK);
	// 设置特效窗体屏保文字显示效果
	if(m_pADdlg->GetShowScreenSaver())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_SCREENSAVER,IDB_CHECK);
	// 设置特效窗体动态徽标显示效果
	if(m_pADdlg->GetShowLogo())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_LOGO,IDB_CHECK);
	// 设置特效窗体烟花显示效果
	if(m_pADdlg->GetShowAsciiAnimation())
		m_popmenu.ModifyODMenu(NULL, IDR_EFFECT_FIREWORKS,IDB_CHECK);
	
	BCMenu *psub = (BCMenu *)m_popmenu.GetSubMenu(0); 
	psub->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);
	m_popmenu.DestroyMenu();
}

//********************************************************************************
//* 名称：	PreTranslateMessage()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			pMsg	:	消息指针
//*	返回:
//*			TRUE	:	成功
//*			FALSE	:	失败
//* 功能：
//*			传递提示窗消息
//********************************************************************************
BOOL CTransparentWnd::PreTranslateMessage(MSG* pMsg) 
{
	m_Tip.RelayEvent(pMsg);
	
	return CWnd::PreTranslateMessage(pMsg);
}

//********************************************************************************
//* 名称：	FindTrayWnd()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			hwnd	:	寻找的窗体句柄
//*			lParam	:	矩形大小值
//*	返回:
//*			TRUE	:	成功
//*			FALSE	:	失败
//* 功能：
//*			在显示窗体动画效果前，先寻找系统区位置
//********************************************************************************
BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam)
{
    TCHAR szClassName[256];
    GetClassName(hwnd, szClassName, 255);

    // 比较窗口类名
    if (0 == _tcscmp(szClassName, _T("TrayNotifyWnd")))
    {
        CRect *pRect = (CRect*) lParam;
        ::GetWindowRect(hwnd, pRect);
        return TRUE;
    }

    // 当找到时钟窗口时表示可以结束了
    if (0 == _tcscmp(szClassName, _T("TrayClockWClass")))
    {
        CRect *pRect = (CRect*) lParam;
        CRect rectClock;
        ::GetWindowRect(hwnd, rectClock);
        pRect->right = rectClock.left;
        return FALSE;
    }
 
    return TRUE;
}

//********************************************************************************
//* 名称：	WinAnimation()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			bShowFlag	
//*					= TRUE	:	窗体滑向系统区动画效果
//*					= FALSE	:	窗体从系统区滑出动画效果
//*	返回:
//*			无
//* 功能：
//*			显示窗口动画效果的涵数
//********************************************************************************
void CTransparentWnd::WinAnimation(bool bShowFlag) 
{
    CRect rect(0,0,0,0);

    // 查找托盘窗口 
    CWnd* pWnd = FindWindow("Shell_TrayWnd", NULL);
    if (pWnd)
    {
        pWnd->GetWindowRect(rect);
		EnumChildWindows(pWnd->m_hWnd, FindTrayWnd, (LPARAM)&rect);
		// rect 为托盘区矩形
		CRect rcWnd;
		GetWindowRect(rcWnd);
		if(bShowFlag)					// 窗体滑向系统区
		  DrawAnimatedRects(IDANI_CAPTION, rcWnd, rect);
		else							// 窗体从系统区滑出
		  DrawAnimatedRects(IDANI_CAPTION, rect, rcWnd);
    }
}

//********************************************************************************
//* 名称：	OnExit()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：
//*			退出程序
//********************************************************************************
void CTransparentWnd::OnExit()  
{
	CHelpTipDlg dlg;

	// 保证此对话框一次只弹出一个
	if( !FindWindow(NULL,"精灵助手 -- 未来工作室(Future Studio)"))
	{
		if(IDCANCEL == dlg.DoModal())
		{	
			if(m_bAutorun)
			{
				// 注册设置为开机自动运行　
				StartupAutorun(FALSE,TRUE);
			}
			else
			{
				// 注册取消开机自动运行　
				StartupAutorun(FALSE,FALSE);
			}

//			VERIFY(KillTimer(GetCurAction()));
			SendMessage(WM_CLOSE,0,0);
		}
	}
}

//********************************************************************************
//* 名称：	OnFullscreenWalk()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			全屏运动还是局部运动
//********************************************************************************
void CTransparentWnd::OnFullscreenWalk() 
{
	m_bFullScreenWalk = !m_bFullScreenWalk;	

}

//********************************************************************************
//* 名称：	OnShowhide()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			显示或隐藏精灵助手
//********************************************************************************
void CTransparentWnd::OnShowhide() 
{
	CWnd * pFrame;					 // 得到当前窗体指针

	pFrame=AfxGetApp()->GetMainWnd();
	if(!pFrame->IsWindowVisible())	 // 是否进行窗体动画,窗体已显示则不进行
	{
		WinAnimation(false);		 // 窗体动画从系统区滑出 
		pFrame->ShowWindow(SW_SHOW);
	}
    else							 // 是否进行窗体动画,窗体已隐藏则不进行
	{
        pFrame->ShowWindow(SW_HIDE);
	    WinAnimation(true);			 // 窗体动画滑入到系统区中 
	}	
}

//********************************************************************************
//* 名称：	OnDownload()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：
//*			设置下载网页地址
//********************************************************************************
void CTransparentWnd::OnDownload() 
{
	CDownloadAddress dlg;  
	CString str;

	// 保证此对话框一次只弹出一个
	if(!FindWindow(NULL,"设置下载网址"))
	{
		if(IDOK == dlg.DoModal())
		{
			if("" != (str=dlg.GetDownloadAddress()))
			{
				CWnd * pFrame;					        

				pFrame=AfxGetApp()->GetMainWnd();
				pFrame->ShowWindow(SW_HIDE);            // 下载前先隐藏精灵助手
				if(m_bSuccess)
				{
					m_sdFile.Close();                   // 先关闭原打开文件，以便新建文件
					m_bSuccess=FALSE;
				}
				m_bDownloadFailed= FALSE;               // 选置假，避免显示提示窗信息
				m_bDownloadFailed=GetSourceHtml(str);	// 下载新设置的网页

				if(m_bDownloadFailed)
					m_bSuccess=m_bDownloadFailed=m_sdFile.Open(strPath+"\\Tip.txt",
					CFile::modeRead | CFile::typeText);
			}
		}
	}
}

//********************************************************************************
//* 名称：	OnActionRun()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：
//*			奔跑方式
//********************************************************************************
void CTransparentWnd::OnActionRun() 
{
	KillTimer(GetCurAction());
	SetTimer(TIMER_RUN,g_RUNSECOND,NULL);	
}

//********************************************************************************
//* 名称：	OnActionWalk()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			散步方式
//********************************************************************************
void CTransparentWnd::OnActionWalk() 
{
	KillTimer(GetCurAction());
	SetTimer(TIMER_WALK,g_WALKSECOND,NULL);
}

//********************************************************************************
//* 名称：	OnActionIdle()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			休息方式
//********************************************************************************
void CTransparentWnd::OnActionIdle() 
{		
	KillTimer(GetCurAction());
	SetTimer(TIMER_IDLE,g_IDLESECOND,NULL);	
}

//********************************************************************************
//* 名称：	GetCurAction()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			获取当前行为表现方式
//********************************************************************************
int  CTransparentWnd::GetCurAction()
{
	return m_iAction;
}

//********************************************************************************
//* 名称：	GetTopWndTitleRect()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			CRect	:	Shell_TrayWnd(开始菜单条)矩形大小
//* 功能：	
//*			获取当前桌面最顶层窗口标题区大小
//********************************************************************************
CRect CTransparentWnd::GetTopWndTitleRect()             
{
/*
	HWND TopHwnd;
	CHelpTipDlg ExitDlg;
	CDownloadAddress AddressDlg; 
	CAbout aboutDlg;

	TopHwnd=::GetForegroundWindow();

	// 不能是本程序所具有的窗体，否则返回初窗口标题区大小
	if(TopHwnd==this->m_hWnd||TopHwnd==ExitDlg.m_hWnd||TopHwnd==AddressDlg.m_hWnd||TopHwnd==aboutDlg.m_hWnd)
		return m_Rect;

	CRect rtWnd;
	::GetWindowRect(TopHwnd,&rtWnd);

	return rtWnd;
*/

	// 查找并返回任务栏窗口大小 
	CRect rect(0,0,0,0);
    CWnd* pWnd = FindWindow("Shell_TrayWnd", NULL);
    if (pWnd)
    {
        pWnd->GetWindowRect(&rect);
	}
	return rect;
}

//********************************************************************************
//* 名称：	OnAbout()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			关于对话框
//********************************************************************************
void CTransparentWnd::OnAbout() 
{
	CAbout aboutDlg;

	// 保证此对话框一次只弹出一个
	if(!FindWindow(NULL,"关于 精灵助手"))
		aboutDlg.DoModal(); 
	
}

//********************************************************************************
//* 名称：	GetWidthAndHeight
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			pBitmap		:	位图指针
//*			pw,ph		:	返回位图的宽度、高度像素值			
//*	返回:
//*			无
//* 功能：	
//*			获取位图宽和高
//********************************************************************************
void CTransparentWnd::GetWidthAndHeight(CBitmap* pBitmap, int* pw, int* ph) const 
{
	ASSERT(FALSE == IsBadReadPtr(pBitmap,sizeof(CBitmap*)));

     if (! pBitmap && ! pBitmap->GetSafeHandle()) {     // 没有位图信息
          if (pw) *pw = 0;
          if (ph) *ph = 0;
     } else {
          BITMAP bm;
          pBitmap->GetObject(sizeof(bm), &bm);
          if (pw) *pw = bm.bmWidth;
          if (ph) *ph = bm.bmHeight;
     }
}

//*******************************************************************************
//* 名称：	SwapYInvertBlt
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			pDC1	:	目标dc指针
//*			x1,y1	:	目标dc左上角坐标
//*			nWidth	:	目标dc宽度
//*			nHeight	:	目标dc高度
//*			pDC2	:	源dc指针
//*			x2,y2	:	源dc左上角坐标
//*	返回:
//*			无
//* 功能：	
//*			参照Y轴方向水平翻转内存位图
//* 附注：	
//*			被MirrorLeftAndRightHalves()涵数使用
//*******************************************************************************
void CTransparentWnd::SwapYInvertBlt (CDC* pDC1, int x1, int y1,
                          int nWidth, int nHeight,
                          CDC* pDC2, int x2, int y2
                          )
{
	 ASSERT(FALSE == IsBadWritePtr(pDC1,sizeof(CDC*)));
	 ASSERT(FALSE == IsBadWritePtr(pDC2,sizeof(CDC*)));

     if (! nWidth || ! nHeight) return;
     pDC1->StretchBlt (
          x1,y1, nWidth,nHeight,
          pDC2,
          x2+nWidth-1,y2, -nWidth, nHeight,
          SRCINVERT
          );
     pDC2->StretchBlt (
          x2,y2, nWidth,nHeight,
          pDC1,
          x1+nWidth-1,y1, -nWidth, nHeight,
          SRCINVERT
          );
     pDC1->StretchBlt (
          x1,y1, nWidth,nHeight,
          pDC2,
          x2+nWidth-1,y2, -nWidth, nHeight,
          SRCINVERT
          );
}

//*******************************************************************************
//* 名称：	MirrorLeftAndRightHalves
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			pBitmap		:	传入要水平翻转方向的位图指针
//*	返回：
//*			无
//* 功能：	
//*			参照Y轴方向水平镜像
//*******************************************************************************
void CTransparentWnd::MirrorLeftAndRightHalves (CBitmap* pBitmap) 
{
	 ASSERT(FALSE == IsBadWritePtr(pBitmap,sizeof(CBitmap*)));
     if (!pBitmap || ! pBitmap->GetSafeHandle()) return;

     // 创建DC并选中位图
     CDC dc;
     CDC* pDC = &dc;
     pDC->CreateCompatibleDC( NULL );
     CBitmap* pBmpOldImage = pDC->SelectObject(pBitmap);
     // 获取位图大小
     int nWidth,nHeight;
     GetWidthAndHeight(pBitmap,&nWidth,&nHeight);
     // 进行转换(以位图宽度一半处为中心)
     int nHalfWidth = nWidth/2;
     if (nHalfWidth < 1) return;
     SwapYInvertBlt(pDC,0,0,nHalfWidth,nHeight,pDC,nWidth-nHalfWidth,0);
  
     pDC->SelectObject(pBmpOldImage);
}

//*******************************************************************************
//* 名称：	GetInvertedBitmap
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			hBitmap			:	传入要镜像的位图句柄
//*			bLateral	
//*					 = TRUE	:	水平方向镜像(默认值)
//*                  = FALSE:	垂直方向镜像
//*	返回：
//*			传回镜像处理后的HBITMAP
//* 功能：	
//*			进行水平或垂直方向镜像处理
//*******************************************************************************
HBITMAP CTransparentWnd::GetInvertedBitmap(HBITMAP hBitmap, BOOL bLateral)
{
	// 创建兼容DC
	CDC sourceDC, destDC;
	sourceDC.CreateCompatibleDC(NULL);
	destDC.CreateCompatibleDC(NULL);

	// 获取位图宽、高
	BITMAP bm;
	::GetObject(hBitmap, sizeof( bm ), &bm);

	// 创建返回结果位图
	HBITMAP hbmResult = ::CreateCompatibleBitmap(CClientDC(NULL), 
						bm.bmWidth, bm.bmHeight);

	// 选入相应位图到DC中
	HBITMAP hbmOldSource = (HBITMAP)::SelectObject(sourceDC.m_hDC, hBitmap);
	HBITMAP hbmOldDest = (HBITMAP)::SelectObject(destDC.m_hDC, hbmResult);
	
	if( bLateral )				// 水平镜像
		destDC.StretchBlt(0, 0, bm.bmWidth, bm.bmHeight, &sourceDC, 
				bm.bmWidth-1, 0, -bm.bmWidth, bm.bmHeight, SRCCOPY);
	else						// 垂直镜像
		destDC.StretchBlt(0, 0, bm.bmWidth, bm.bmHeight, &sourceDC, 
				0, bm.bmHeight-1, bm.bmWidth, -bm.bmHeight, SRCCOPY);

	// 恢复源设置
	::SelectObject(sourceDC.m_hDC, hbmOldSource);
	::SelectObject(destDC.m_hDC, hbmOldDest);

	return hbmResult;
}

//********************************************************************************
//* 名称：	Map2Frame()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 参数：
//*			dc			:	传入显示位图的dc指针
//*			bmpMap		:	传入显示总位图引用	
//*			nColFrame	:	映射总位图中的那一列号
//*			nRowFrame	:	映射总位图中的那一行号
//*	返回：
//*			无
//* 功能：	
//*			将总位图中相应单帧映射到m_bmpDraw中
//********************************************************************************
void CTransparentWnd::Map2Frame(CDC *dc,const CBitmap& bmpMap,int nColFrame,int nRowFrame)
{
	ASSERT(FALSE == IsBadReadPtr(dc,sizeof(CDC*)));

	// 先清空原位图
	m_bmpDraw.DeleteObject();

	CDC MapDC;
	MapDC.CreateCompatibleDC(dc);
	MapDC.SelectObject(bmpMap);

	m_bmpDraw.CreateCompatibleBitmap(dc, g_nSIZE, g_nSIZE);
	CDC FrameDC;
	FrameDC.CreateCompatibleDC(dc);
	CBitmap* pOB = FrameDC.SelectObject(&m_bmpDraw);
	// 将总位图bmpMap中相应行、列单帧投映到m_bmpDraw中
	FrameDC.StretchBlt(0, 0, g_nSIZE, g_nSIZE, &MapDC, nColFrame*g_nSIZE,nRowFrame*g_nSIZE,g_nSIZE,g_nSIZE,SRCCOPY);
	// 恢复源设置
	FrameDC.SelectObject(pOB);
}

//********************************************************************************
//* 名称：	StartupAutorun()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//* 参数：
//*			bExist 
//*					 = TRUE:  仅判断该注册键是否存在
//*                  = FALSE: 增加或删除注册键
//*
//*			bAddDel     (仅在bExist为FALSE时有效)
//*					 = TRUE:  加入注册键(默认值)
//*					 = FALSE: 删除注册键
//* 返回：
//*         TRUE	:  成功
//*			FASLE	:  失败
//* 功能：  
//*			开机时程序自动运行
//********************************************************************************
BOOL CTransparentWnd::StartupAutorun(BOOL bExist, BOOL bAddDel)
{
	HKEY hKey;
	LONG lRet, lRet2;
	DWORD dwLength, dwDataType;
	TCHAR szItemValue[MAX_PATH], szPrevValue[MAX_PATH];
	TCHAR szBuffer[MAX_PATH];

	// 得到程序全路径名
	GetModuleFileName( NULL, szItemValue, MAX_PATH );


	// 打开注册表键
	lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, g_cszAutoRunKey,
			0, KEY_READ | KEY_WRITE, &hKey );
	if( lRet != ERROR_SUCCESS )
			return FALSE;

	// 查询自动运行项目是否存在
	dwLength = sizeof( szBuffer );
	lRet = RegQueryValueEx( hKey, g_cszWindowClass, 
			NULL, &dwDataType, (LPBYTE)szBuffer, &dwLength );

	if( TRUE == bExist)			// 仅读取判断注册键值是否存在时
	{
		if( lRet != ERROR_SUCCESS )
		{
			// 关闭注册表键
			RegCloseKey( hKey );
			return FALSE;       // 不存在
		}
		else
		{
			// 关闭注册表键
			RegCloseKey( hKey );
			return TRUE;		// 存在
		}
	}
	else						// 写入或删除注册键时
	{
		// 添加
		if( TRUE == bAddDel ) 
		{
			// 自动运行项目不存在
			if( lRet != ERROR_SUCCESS )
				lRet2 = RegSetValueEx( hKey, g_cszWindowClass,
					0, REG_SZ, (LPBYTE)szItemValue, strlen( szItemValue ) );
			else 
			{
				// 存在, 比较二者是否相同
				dwLength = sizeof( szPrevValue );
				lRet2 = RegQueryValueEx( hKey, g_cszWindowClass, 
						0, &dwDataType,(LPBYTE)szPrevValue, &dwLength );
				// 不相同则替换
				if( lstrcmp( szItemValue, szPrevValue ) ) 
				{
					lRet2 = RegDeleteValue( hKey, g_cszWindowClass );
					lRet2 = RegSetValueEx( hKey, g_cszWindowClass, 
							0, REG_SZ,(LPBYTE)szItemValue, strlen( szItemValue ) );
				}
			}
		}
		// 删除
		else 
			// 自动运行项目已存在则删除
			if( lRet == ERROR_SUCCESS )
				lRet2 = RegDeleteValue( hKey, g_cszWindowClass );
	}

	// 关闭注册表键
	RegCloseKey( hKey );

	if( lRet2 != ERROR_SUCCESS )
		return FALSE;

	return TRUE;
}

//********************************************************************************
//* 名称：	OnAutorun()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			激活开机时程序自动运行
//********************************************************************************
void CTransparentWnd::OnAutorun() 
{
	m_bAutorun = !m_bAutorun;	
}

//********************************************************************************
//* 名称：	OnEffectWater()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中进行水纹效果显示
//********************************************************************************
void CTransparentWnd::OnEffectWater() 
{
	m_pADdlg->SetWaterRoutine(!m_pADdlg->GetWaterRoutine());
}

//********************************************************************************
//* 名称：	OnEffectFire()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中进行火焰效果显示
//********************************************************************************
void CTransparentWnd::OnEffectFire() 
{
	m_pADdlg->SetFireRoutine(!m_pADdlg->GetFireRoutine());
}

//********************************************************************************
//* 名称：	OnEffectPlasma()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中进行溶浆效果显示
//********************************************************************************
void CTransparentWnd::OnEffectPlasma() 
{
	m_pADdlg->SetPlasmaRoutine(!m_pADdlg->GetPlasmaRoutine());
}

//********************************************************************************
//* 名称：	OnEffectParticle()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示仿爆炸等效果
//********************************************************************************
void CTransparentWnd::OnEffectParticle() 
{
	m_pADdlg->SetParticleRoutine(!m_pADdlg->GetParticleRoutine());
}

//********************************************************************************
//* 名称：	OnEffectFont()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示文字
//********************************************************************************
void CTransparentWnd::OnEffectFont() 
{
	m_pADdlg->SetShowFont(!m_pADdlg->GetShowFont());	
}

//********************************************************************************
//* 名称：	OnEffectElectricity()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示仿闪电效果
//********************************************************************************
void CTransparentWnd::OnEffectElectricity() 
{
	m_pADdlg->SetShowElectricity(!m_pADdlg->GetShowElectricity());	
}

//********************************************************************************
//* 名称：	OnEffectSnow()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示雪花效果
//********************************************************************************
void CTransparentWnd::OnEffectSnow() 
{
	m_pADdlg->SetShowSnow(!m_pADdlg->GetShowSnow());
}

//********************************************************************************
//* 名称：	OnEffectScreensaver()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示屏保文字效果
//********************************************************************************
void CTransparentWnd::OnEffectScreensaver() 
{
	m_pADdlg->SetShowScreenSaver(!m_pADdlg->GetShowScreenSaver());
}

//********************************************************************************
//* 名称：	OnEffectLogo()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示动态徽标效果
//********************************************************************************
void CTransparentWnd::OnEffectLogo() 
{
	m_pADdlg->SetShowLogo(!m_pADdlg->GetShowLogo());
	
}

//********************************************************************************
//* 名称：	OnEffectFireworks()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			特效窗体中显示烟花效果
//********************************************************************************
void CTransparentWnd::OnEffectFireworks() 
{
	m_pADdlg->SetShowAsciiAnimation(!m_pADdlg->GetShowAsciiAnimation());
}

//********************************************************************************
//* 名称：	OnMirrorInWater()
//* 作者：	徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//*	参数：
//*			无
//*	返回:
//*			无
//* 功能：	
//*			弹出水中倒影效果无模式对话框
//********************************************************************************
void CTransparentWnd::OnMirrorInWater() 
{
	// 保证此对话框只弹出一个
	if(!FindWindow(NULL,"水中倒影 -- 未来工作室"))
	{
		if(m_pMirrorInWaterDlg)
		{
			delete m_pMirrorInWaterDlg;
			m_pMirrorInWaterDlg	= NULL;
		}
		
		m_pMirrorInWaterDlg = new CMirrorInWater(this);
		m_pMirrorInWaterDlg->Create(IDD_MIRROR_IN_WATER,this);
		m_pMirrorInWaterDlg->ShowWindow(SW_SHOW);
	}
	else if(!m_pMirrorInWaterDlg->IsWindowVisible())
		m_pMirrorInWaterDlg->ShowWindow(SW_SHOW);
}
