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
#include "stdafx.h"
#include "resource.h"
#include "Splash.h"

//////////////////// Defines ////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define MOUSE_MOVE 0xF012

BEGIN_MESSAGE_MAP(CSplashThread, CWinThread)
	//{{AFX_MSG_MAP(CSplashThread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CSplashThread, CWinThread)

CSplashThread::CSplashThread()
{
}

CSplashThread::~CSplashThread()
{
}

BOOL CSplashThread::InitInstance()
{
  //Attach this threads UI state to the main one, This will ensure that 
  //the activation state is managed consistenly across the two threads
  ASSERT(AfxGetApp());
  BOOL bSuccess = AttachThreadInput(m_nThreadID, AfxGetApp()->m_nThreadID, TRUE);
  if (!bSuccess)
    TRACE(_T("Failed in call to AttachThredInput, GetLastError:%d\n"), ::GetLastError());

  // 此处创建菲屏窗口
  BOOL bCreated = m_SplashScreen.Create();
	VERIFY(bCreated);

  m_pMainWnd = &m_SplashScreen;
	return bCreated;
}

void CSplashThread::HideSplash()
{
  //Wait until the splash screen has been created
  //before trying to close it
  while (!m_SplashScreen.GetSafeHwnd());

  m_SplashScreen.SetOKToClose();
  m_SplashScreen.SendMessage(WM_CLOSE);

}

void CSplashThread::SetBitmapToUse(const CString& sFilename)
{
  m_SplashScreen.SetBitmapToUse(sFilename);
}

void CSplashThread::SetBitmapToUse(UINT nResourceID)
{
  m_SplashScreen.SetBitmapToUse(nResourceID);
}

void CSplashThread::SetBitmapToUse(LPCTSTR pszResourceName)
{
  m_SplashScreen.SetBitmapToUse(pszResourceName);
}

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
void CSplashThread::SetShowMode(int nShowMode)   
{
	m_SplashScreen.SetShowMode(nShowMode);
}

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
  //{{AFX_MSG_MAP(CSplashWnd)
  ON_WM_CREATE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_CLOSE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ---------------------------------------------------------
// 名称: CSplashWnd()
// 功能：构造涵数初始化工作
// 参数: 无
// 返回: 无
// 编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
// 组织: 未来工作室(Future Studio)
// 日期: 2002.1.8
// ---------------------------------------------------------
CSplashWnd::CSplashWnd()
{
	m_bOKToClose = FALSE;
	m_nHeight = 0;
	m_nWidth = 0;

	// 默认菲屏显示效果为随机显示	 
	m_nShowMode = -1;       
}

// ---------------------------------------------------------
// 名称: ~CSplashWnd()
// 功能：退出时清扫工作
// 参数: 无
// 返回: 无
// 编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
// 组织: 未来工作室(Future Studio)
// 日期: 2002.1.8
// ---------------------------------------------------------
CSplashWnd::~CSplashWnd()
{
  if(NULL != m_bmpDst)
	  DeleteObject(m_bmpDst);
  if(NULL != m_bmpSrc)
	  DeleteObject(m_bmpSrc);
  // 关闭DrawDib库
  if (m_hDrawDib)
  {
	  DrawDibClose(m_hDrawDib);
	  m_hDrawDib = NULL;
  }

  //destroy our invisible owner when we're done
  if (m_wndOwner.m_hWnd != NULL)
    m_wndOwner.DestroyWindow();
}

BOOL CSplashWnd::LoadBitmap()
{     
  //Use LoadImage to get the image loaded into a DIBSection
  HBITMAP hBitmap;
  if (m_bUseFile)
    hBitmap = (HBITMAP) ::LoadImage(NULL, m_sFilename, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
  else
    hBitmap = (HBITMAP) ::LoadImage(AfxGetResourceHandle(), m_pszResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

  //Check that we could load it up  
  if (hBitmap == NULL)       
    return FALSE;

  //Get the width and height of the DIBSection
  BITMAP bm;
  GetObject(hBitmap, sizeof(BITMAP), &bm);
  m_nHeight = bm.bmHeight;
  m_nWidth = bm.bmWidth;

  //Covert from the SDK bitmap handle to the MFC equivalent
  m_Bitmap.Attach(hBitmap);

  return TRUE;   
}

void CSplashWnd::CreatePaletteFromBitmap()
{
  //Get the color depth of the DIBSection
  BITMAP bm;
  m_Bitmap.GetObject(sizeof(BITMAP), &bm);

  //If the DIBSection is 256 color or less, it has a color table
  if ((bm.bmBitsPixel * bm.bmPlanes) <= 8 )     
  {
    //Create a memory DC and select the DIBSection into it
    CDC memDC;
    memDC.CreateCompatibleDC(NULL);
    CBitmap* pOldBitmap = memDC.SelectObject(&m_Bitmap);

    //Get the DIBSection's color table
    RGBQUAD rgb[256];
    ::GetDIBColorTable(memDC.m_hDC, 0, 256, rgb);

    //Create a palette from the color table
    LPLOGPALETTE pLogPal = (LPLOGPALETTE) new BYTE[sizeof(LOGPALETTE) + (256*sizeof(PALETTEENTRY))];
    pLogPal->palVersion = 0x300;       
    pLogPal->palNumEntries = 256;

    for (WORD i=0; i<256; i++)       
    {
      pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
      pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
      pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
      pLogPal->palPalEntry[i].peFlags = 0;
    }
    VERIFY(m_Palette.CreatePalette(pLogPal));
    
    //Clean up
    delete pLogPal;
    memDC.SelectObject(pOldBitmap);

	//zwh Adding,2002.6.20 
	memDC.DeleteDC();
  }
  else  //It has no color table, so use a halftone palette     
  {
    CDC* pRefDC = GetDC();
    m_Palette.CreateHalftonePalette(pRefDC);
    ReleaseDC(pRefDC);     
  }     
}

// ---------------------------------------------------------
// 名称: Create()
// 功能：菲屏创建初始化工作
// 参数: 无
// 返回: TRUE -- 成功，　FALSE -- 失败
// 编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
// 组织: 未来工作室(Future Studio)
// 日期: 2002.1.8
// ---------------------------------------------------------
BOOL CSplashWnd::Create()
{                   
  //Load up the bitmap from file or from resource
  VERIFY(LoadBitmap());

  //Modify the owner window of the splash screen to be an invisible WS_POPUP 
  //window so that the splash screen does not appear in the task bar
  LPCTSTR pszWndClass = AfxRegisterWndClass(0);
  VERIFY(m_wndOwner.CreateEx(0, pszWndClass, _T(""), WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0));

  //Create this window
  pszWndClass = AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW));
  VERIFY(CreateEx(0, pszWndClass, _T(""), WS_POPUP | WS_VISIBLE, 0, 0, m_nWidth, m_nHeight, m_wndOwner.GetSafeHwnd(), NULL));

  //Create the palette, We need to do this after the window is created because
  //we may need to access the DC associated with it
  CreatePaletteFromBitmap();

  //realize the bitmap's palette into the DC
  OnQueryNewPalette();

// ----------------------------------------------------------------------------------
  // 下面部分初始化位图渐隐、渐显效果数据，jingzhou xu
  m_bmpSrc		= NULL;
  m_bmpDst		= NULL;

  // 打开DrawDib库
  m_hDrawDib = DrawDibOpen();

  HDC hMemDC	= CreateCompatibleDC(NULL);
  if(hMemDC)
  {
	  // 创建二个32位深度位图，源和目标
	  BITMAPINFOHEADER RGB32BITSBITMAPINFO =
	  {
			sizeof(BITMAPINFOHEADER),
			m_nWidth, m_nHeight,
			1, 32, BI_RGB,
			0,0,0,0,0
	  };

	  m_bmpSrc	= CreateDIBSection(	hMemDC,
									(BITMAPINFO*)&RGB32BITSBITMAPINFO,
									DIB_RGB_COLORS,(VOID**)&m_clrSrc,
									NULL,0);
	  m_bmpDst	= CreateDIBSection(	hMemDC,
									(BITMAPINFO*)&RGB32BITSBITMAPINFO,
									DIB_RGB_COLORS,(VOID**)&m_clrDst,
									NULL,0);
	  
	  if(m_bmpSrc && m_bmpDst)
	  {
			HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, m_bmpSrc);
			HDC		hDC		= CreateCompatibleDC(hMemDC);
			if(hDC)
			{
				// 将要显示的菲屏位图分别复制到创建的32位色源和目标位图中
				HBITMAP hOldBmp2 = (HBITMAP)SelectObject(hDC, (HBITMAP)m_Bitmap);
				BitBlt(hMemDC, 0, 0, m_nWidth, m_nHeight, hDC, 0, 0, SRCCOPY);

				SelectObject(hMemDC, m_bmpDst);
				BitBlt(hMemDC, 0, 0, m_nWidth, m_nHeight, hDC, 0, 0, SRCCOPY);

				// 恢复及清扫工作
				SelectObject(hDC, hOldBmp2);
				DeleteDC(hDC);
			}
			SelectObject(hMemDC, hOldBmp);
	  }
	  DeleteDC(hMemDC);
  }
// ----------------------------------------------------------------------------------

  return TRUE;
}

int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  //Center the splash window on the screen
  CenterWindow();

  return 0;
}

// ---------------------------------------------------------
// 名称: OnPaint()
// 功能：绘制动态菲屏显示效果，具有下面属性:
//		(-1	-- 随机显示,		0	-- 水平交错效果,		1	-- 垂直交错效果,
//		 2	-- 水平百叶窗效果,	3	-- 垂直百叶窗效果,		4	-- 随机积木效果,
//		 5	-- 滤镜效果,		6	-- 垂直平分扫描,		7	-- 水平平分扫描,
//		 8	-- 水平交叉移动,	9	-- 垂直交叉移动,		10	-- 垂直栅条效果,
//		 11	-- 水平栅条效果,	12	-- 环形扫锚效果,		13	-- 缩放效果,
//		 14	-- 矩形扩张效果,	15	-- 椭圆扩张效果,		16	-- 圆角矩形扩张,
//		 17	-- 渐显效果,		18	-- 渐隐效果,			19	-- 正常显示)		 		 
// 参数: 无
// 返回: 无
// 编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
// 组织: 未来工作室(Future Studio)
// 日期: 2002.1.8
// ---------------------------------------------------------
void CSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	// 选择调色板及位图到DC中
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap* pOldBitmap = memDC.SelectObject(&m_Bitmap);
	CPalette* pOldPalette = dc.SelectPalette(&m_Palette, FALSE);
	dc.RealizePalette();

	//---------------------------------------------------------
	// 以下部分为动态显示菲屏效果代码，jingzhou xu 
	int i,j,step,stepx,stepy,dispnum,x,y; 
	BOOL bDone, bQuit;

	if(m_nShowMode == -1)						// 随机菲屏效果显示
	{
		srand((unsigned)time(NULL)); 
		m_nShowMode = rand()%20;				// 随机选择已有20种中的一种效果
	}

	switch(m_nShowMode)
	{
	case 0:
		// 一、水平交错效果算法
		for( i=0; i<=m_nHeight; i+=2 ) 
		{
			j = i; 
			while(j>0) 
			{
				dc.StretchBlt(					// 奇数，由上至下 
					0,j-1,						// 目标设备逻辑横、纵坐标 
					m_nWidth,1,					// 显示位图的像素宽、高度 
					&memDC,						// 源位图设备对象 
					0,m_nHeight-(i-j-1),		// 源位图的起始横、纵坐标 
					m_nWidth,1,					// 源位图的像素宽、高度 
					SRCCOPY); 
				
				dc.StretchBlt(					// 偶数，由下至上 
					0,m_nHeight-j,				// 目标设备逻辑横、纵坐标 
					m_nWidth,1, 				// 显示位图的像素宽、高度 
					&memDC, 					// 源位图设备对象 
					0,i-j, 						// 源位图的起始横、纵坐标 
					m_nWidth,1, 				// 源位图的像素宽、高度 
					SRCCOPY); 
				j-=2; 
			} 
			Sleep(20);
		} 
		dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);
		Sleep(500);

		break;
	case 1:
		// 二、垂直交错效果算法
		for( i=0; i<=m_nWidth; i+=2 ) 
		{
			j = i; 
			while(j>0) 
			{
				dc.StretchBlt(					// 奇数，由左至右 
					j-1,0,						// 目标设备逻辑横、纵坐标 
					1,m_nHeight,				// 显示位图的像素宽、高度 
					&memDC,						// 源位图设备对象 		
					m_nWidth-(i-j-1),0,			// 源位图的起始横、纵坐标 
					1,m_nHeight,				// 源位图的像素宽、高度 
					SRCCOPY); 
				
				dc.StretchBlt(					// 偶数，由右至左 
					m_nWidth-j, 0,				// 目标设备逻辑横、纵坐标 
					1, m_nHeight,				// 显示位图的像素宽、高度 
					&memDC,						// 源位图设备对象 
					i-j, 0,						// 源位图的起始横、纵坐标 
					1, m_nHeight,				// 源位图的像素宽、高度 
					SRCCOPY); 
				j-=2; 
			} 
			Sleep(20);
		} 
		dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);
		Sleep(500);

		break;
	case 2:
		// 三、水平百叶窗效果算法
		step=m_nHeight/15; 
		for ( i=0; i<=step; ++i ) 
		{
			for ( j=0; j<15; ++j ) 
				dc.StretchBlt( 
				0,j*step+i,						// 目标设备逻辑横、纵坐标 
				m_nWidth,1,						// 显示位图的像素宽、高度 
				&memDC, 						// 源位图设备对象 
				0,j*step+i, 					// 源位图的起始横、纵坐标 
				m_nWidth,1, 					// 源位图的像素宽、高度 
				SRCCOPY); 
			
			Sleep(30); 
		}

		break;
	case 3:
		// 四、垂直百叶窗效果算法
		step=m_nWidth/15; 
		for ( i=0; i<=step; ++i ) 
		{
			for ( j=0; j<15; ++j ) 
				dc.StretchBlt( 
				j*step+i,0, 					// 目标设备逻辑横、纵坐标 
				1,m_nHeight,					// 显示位图的像素宽、高度 
				&memDC, 						// 源位图设备对象 
				j*step+i,0, 					// 源位图的起始横、纵坐标 
				1, m_nHeight,					// 源位图的像素宽、高度 
				SRCCOPY); 
			
			Sleep(30); 
		}

		break;
	case 4:
		// 五、随机积木效果算法
		int pxy[10][10];						// 碎片数
		for ( i=0; i<10; ++i ) 
			for ( j=0; j<10; ++j ) 
				pxy[i][j]=0; 
		stepx=m_nWidth/10; 
		stepy=m_nHeight/10; 
		srand( (unsigned)time( NULL ) ); 
		dispnum=0; 
		while(1) 
		{ 
			x=rand() % 10; 
			y=rand() % 10; 
			if ( pxy[x][y] )					// 已经显示过，跳过
				continue; 
			pxy[x][y]=1;						// 当前显示的置1
			dc.StretchBlt( 
				x*stepx, y*stepy,				// 目标设备逻辑横、纵坐标 
				stepx,stepy, 					// 显示位图的像素宽、高度 
				&memDC, 						// 源位图设备对象 
				x*stepx, y*stepy, 				// 源位图的起始横、纵坐标 
				stepx,stepy, 					// 源位图的像素宽、高度 
				SRCCOPY); 
			dispnum++; 

			if ( dispnum >=100 )				// 是否显示完
				break; 

			Sleep(10); 
		} 
		Sleep(300);

		break;
	case 5:
		// 六、滤镜效果算法
		int i,j; 
		for (i=0; i<=m_nHeight; ++i) 
		{
			for (j=0; j<=m_nHeight-i; ++j) 
				dc.StretchBlt(0, j, m_nWidth, 1, &memDC, 0, m_nHeight-i, m_nWidth,1, SRCCOPY);
			
			Sleep(10); 
		} 
		Sleep(300);

		break;
	case 6:
		// 七、垂直平分扫描算法
		bDone	= FALSE;
		i		= 0;
		while(!bDone)
		{
			// 大于高度一半时，置结束标志为真
			if(i > (m_nHeight+1)/2)
			{
				i		= (m_nHeight+1)/2;
				bDone	= TRUE;
			}
			
			// 上半部分从上到下，一次5个像素高度
			dc.StretchBlt(0, i, m_nWidth, 5, &memDC, 0, i, m_nWidth, 5, SRCCOPY);
			// 下半部分从下到上，一次5个像素高度
			dc.StretchBlt(0, m_nHeight-i, m_nWidth, 5, &memDC, 0, m_nHeight-i, m_nWidth, 5, SRCCOPY);
			
			i += 5;

			Sleep(30); 
		}

		break;
	case 7:
		// 八、水平平分扫描算法
		bDone	= FALSE;
		i		= 0;
		while(!bDone)
		{
			// 大于宽度一半时，置结束标志为真
			if(i > (m_nWidth+1)/2)
			{
				i		= (m_nWidth+1)/2;
				bDone	= TRUE;
			}
			
			// 左半部分从左到右，一次5个像素宽度
			dc.StretchBlt(i, 0, 5, m_nHeight, &memDC, i, 0, 5, m_nHeight, SRCCOPY);
			// 右半部分从右到左，一次5个像素宽度
			dc.StretchBlt(m_nWidth-i, 0, 5, m_nHeight, &memDC, m_nWidth-i, 0, 5, m_nHeight, SRCCOPY);
			
			i += 5;

			Sleep(30); 
		}

		break;
	case 8:
		// 九、水平交叉移动算法
		bDone	= FALSE;
		i		= 0;
		while(!bDone)
		{
			// 大于宽度时，置结束标志为真
			if(i > m_nWidth)
			{
				i		= m_nWidth;
				bDone	= TRUE;
			}
			
			// 上半部分从右到左，一次移动10个像素宽度
			dc.StretchBlt(0, 0, i, (m_nHeight+1)/2, &memDC, m_nWidth-i, 0, i, (m_nHeight+1)/2, SRCCOPY);
			// 下半部分从左到右，一次移动10个像素宽度
			dc.StretchBlt(m_nWidth-i, (m_nHeight+1)/2, i, (m_nHeight+1)/2, &memDC, 0, (m_nHeight+1)/2, i, (m_nHeight+1)/2, SRCCOPY);
			
			i += 10;

			Sleep(30); 
		}

		break;
	case 9:
		// 十、垂直交叉移动算法
		bDone	= FALSE;
		i		= 0;
		while(!bDone)
		{
			// 大于高度时，置结束标志为真
			if(i > m_nHeight)
			{
				i		= m_nHeight;
				bDone	= TRUE;
			}
			
			// 左半部分从底到顶，一次移动10个像素宽度
			dc.StretchBlt(0, 0, (m_nWidth+1)/2, i, &memDC, 0, m_nHeight-i, (m_nWidth+1)/2, i, SRCCOPY);
			// 右半部分从顶到底，一次移动10个像素宽度
			dc.StretchBlt((m_nWidth+1)/2, m_nHeight-i, m_nWidth, i, &memDC, (m_nWidth+1)/2, 0, m_nWidth, i, SRCCOPY);
			
			i += 10;

			Sleep(30); 
		}

		break;
	case 10:
		// 十一、垂直栅条算法
		bDone	= FALSE;
		i		= 0;
		
		while(!bDone)
		{
			// 大于高度时，置结束标志为真
			if(i > m_nHeight)
			{
				i		= m_nHeight;
				bDone	= TRUE;
			}
			
			bQuit		= FALSE;
			j			= 0;

			while(!bQuit)
			{
				// 大于宽度，置退出内循环标志为真
				if(j > m_nWidth)
				{
					j		= m_nWidth;
					bQuit	= TRUE;
				}

				// 分为多个纵向垂直小条，从左向右，奇数向下移动，偶数向上移动
				dc.StretchBlt(j, 0, 10, i, &memDC, j, m_nHeight-i, 10, i, SRCCOPY);
				j += 10;

				// 大于宽度，置退出内循环标志为真
				if(j > m_nWidth)
				{
					j		= m_nWidth;
					bQuit	= TRUE;
				}

				// 分为多个纵向垂直小条，从左向右，奇数向下移动，偶数向上移动
				dc.StretchBlt(j, m_nHeight-i, 10, i, &memDC, j, 0, 10, i, SRCCOPY);
				j += 10;
			}

			++i;

			Sleep(20); 
		}
		Sleep(500);

		break;
	case 11:
		// 十二、水平栅条算法
		bDone	= FALSE;
		i		= 0;
		
		while(!bDone)
		{
			// 大于宽度时，置结束标志为真
			if(i > m_nWidth)
			{
				i		= m_nWidth;
				bDone	= TRUE;
			}
			
			bQuit		= FALSE;
			j			= 0;

			while(!bQuit)
			{
				// 大于高度，置退出内循环标志为真
				if(j > m_nHeight)
				{
					j		= m_nHeight;
					bQuit	= TRUE;
				}

				// 分为多个横向水平小条，从上向下，奇数向左移动，偶数向右移动
				dc.StretchBlt(0, j, i, 10, &memDC, m_nWidth-i, j, i, 10, SRCCOPY);
				j += 10;

				// 大于高度，置退出内循环标志为真
				if(j > m_nHeight)
				{
					j		= m_nHeight;
					bQuit	= TRUE;
				}

				// 分为多个横向水平小条，从上向下，奇数向左移动，偶数向右移动
				dc.StretchBlt(m_nWidth-i, j, i, 10, &memDC, 0, j, i, 10, SRCCOPY);
				j += 10;
			}

			++i;

			Sleep(20); 
		}
		Sleep(500);

		break;
	case 12:
		{
			// 十三、环形扫锚效果算法
			int nCenterX,nCenterY;

			// 设置从中心点开始扫锚
			nCenterY = (m_nHeight+1)/2;
			nCenterX = (m_nWidth+1)/2;

			// 右上部分逐列向右扫锚
			for(i = 0; i < nCenterX; )
			{
				i += 10;
				if(i > nCenterX)
					i	= nCenterX;
				
				dc.BitBlt(nCenterX, 0, i, nCenterY, &memDC, nCenterX, 0, SRCCOPY);
				
				Sleep(30);
			}

			// 右下部分逐行向下扫锚
			for(i = 0; i < nCenterY; )
			{
				i += 10;
				if(i > nCenterY)
					i	= nCenterY;
				
				dc.BitBlt(nCenterX, nCenterY, nCenterX, i, &memDC, nCenterX, nCenterY, SRCCOPY);
				
				Sleep(30);
			}

			// 左下部分逐列向左扫锚
			for(i = nCenterX; i > 0; )
			{
				i -= 10;
				if(i < 0)
					i	= 0;
				
				dc.BitBlt(i, nCenterY, nCenterX-i, nCenterY, &memDC, i, nCenterY, SRCCOPY);
				
				Sleep(30);
			}

			// 左上部分逐行向上扫锚
			for(i = nCenterY; i > 0; )
			{
				i -= 10;
				if(i < 0)
					i	= 0;
				
				dc.BitBlt(0, i, nCenterX, nCenterY-i, &memDC, 0, i, SRCCOPY);
				
				Sleep(30);
			}
		}

		break;
	case 13:
		{
			// 十四、缩放效果算法
			bDone			= FALSE;

			int nCenterX,nCenterY;
			// 设置中心点
			nCenterY		= (m_nHeight+1)/2;
			nCenterX		= (m_nWidth+1)/2;

			double dbScale	= 0.0;
			while(!bDone)
			{
				dbScale		+= 0.1; 
				// 越界处理
				if(dbScale >= 1.0)		
				{
					dbScale	= 1.0;
					bDone	= TRUE;
				}
				
				// 由内向外，逐渐放大
				dc.StretchBlt(nCenterX-static_cast<int>((m_nWidth*dbScale)/2), 
					nCenterY-static_cast<int>((m_nHeight*dbScale)/2), 
					static_cast<int>(m_nWidth*dbScale), static_cast<int>(m_nHeight*dbScale),
					&memDC, 0, 0, m_nWidth, m_nHeight, SRCCOPY);
				
				Sleep(50); 
			}
		}

		break;
	case 14:
		{
			// 十五、矩形扩张效果算法
			bDone	= FALSE;
			int left,right,top,bottom;

			// 设置从中心点开始扩张
			top = bottom = (m_nHeight+1)/2;
			left = right = (m_nWidth+1)/2;
			while(!bDone)
			{
				// 越界处理
				if(m_nHeight > m_nWidth)		// 高大于宽时
				{
					if(top < 0 && bottom > m_nHeight)
					{
						top		= 0;
						bottom	= m_nHeight;
						bDone	= TRUE;
					}
					else if(top < 0)
						top		= 0;
					else if(bottom > m_nHeight)
						bottom	= m_nHeight;

					if(left < 0)
						left	= 0;
					if(right > m_nWidth)
						right	= m_nWidth;
				}
				else							// 宽大于等于高时
				{
					if(left < 0 && right > m_nWidth)
					{
						left	= 0;
						right	= m_nWidth;
						bDone	= TRUE;
					}
					else if(left < 0)
						left	= 0;
					else if(right > m_nWidth)
						right	= m_nWidth;

					if(top < 0)
						top		= 0;
					if(bottom > m_nHeight)
						bottom	= m_nHeight;
				}
				
				// 由内向外，逐渐扩张
				dc.StretchBlt(left, top, right - left, bottom - top, &memDC, left, top, right - left, bottom - top, SRCCOPY);
				
				left	-= 10;
				right	+= 10;
				top		-= 10;
				bottom	+= 10;
				
				Sleep(50); 
			}
		}

		break;
	case 15:
		{
			// 十六、椭圆扩张效果算法
			bDone	= FALSE;
			int nCenterX,nCenterY;

			// 设置从中心点开始扩张
			nCenterY = (m_nHeight+1)/2;
			nCenterX = (m_nWidth+1)/2;

			CRgn  WinRgn;
			while(!bDone)
			{
				// 越界处理
				if(m_nHeight > m_nWidth)		// 高大于宽时
				{
					if(nCenterY < 0)
					{
						nCenterY	= 0;
						bDone		= TRUE;
					}

					if(nCenterX < 0)
						nCenterX	= 0;
				}
				else							// 宽大于等于高时
				{
					if(nCenterX < 0)
					{
						nCenterX	= 0;
						bDone		= TRUE;
					}

					if(nCenterY < 0)
						nCenterY	= 0;
				}
				
				// 扩张速度
				nCenterX	-= 5;
				nCenterY	-= 5;

				// 由内向外，逐渐扩张
				VERIFY(WinRgn.CreateEllipticRgn(nCenterX, nCenterY,
										m_nWidth - nCenterX, m_nHeight - nCenterY));
				VERIFY(SetWindowRgn(WinRgn , FALSE));

				dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);

				// 清扫工作
				WinRgn.DeleteObject();

				Sleep(50); 
			}

			// 显示整幅图
			SetWindowRgn(NULL , TRUE);
			dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);
		}

		break;
	case 16:
		{
			// 十七、圆角矩形扩张效果算法
			bDone	= FALSE;
			int nCenterX,nCenterY;

			// 设置从中心点开始扩张
			nCenterY = (m_nHeight+1)/2;
			nCenterX = (m_nWidth+1)/2;

			// 圆角宽、高弧度
			int nWidthRound, nHeightRound;
			nWidthRound = nHeightRound = 100;

			CRgn  WinRgn;
			while(!bDone)
			{
				// 越界处理
				if(m_nHeight > m_nWidth)		// 高大于宽时
				{
					if(nCenterY < 0)
					{
						nCenterY	= 0;
						bDone		= TRUE;
					}

					if(nCenterX < 0)
						nCenterX	= 0;
				}
				else							// 宽大于等于高时
				{
					if(nCenterX < 0)
					{
						nCenterX	= 0;
						bDone		= TRUE;
					}

					if(nCenterY < 0)
						nCenterY	= 0;
				}
				
				// 扩张速度
				nCenterX	-= 5;
				nCenterY	-= 5;

				// 由内向外，逐渐扩张
				VERIFY(WinRgn.CreateRoundRectRgn(nCenterX, nCenterY,
										m_nWidth - nCenterX, m_nHeight - nCenterY, 
										nWidthRound, nHeightRound));
				VERIFY(SetWindowRgn(WinRgn , FALSE));

				dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);

				// 清扫工作
				WinRgn.DeleteObject();

				Sleep(50); 
			}

			// 显示整幅图
			SetWindowRgn(NULL , TRUE);
			dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);
		}

		break;
	case 17:
		// 十八、渐显效果算法
		{
			if(NULL == m_bmpSrc || NULL == m_hDrawDib)
				return;
			
			// 清空目标32位位图数据
			memset((void*)m_clrDst, 0, m_nWidth*m_nHeight*sizeof(COLORREF));

			BOOL bDone = FALSE;
			int i,j;
			BYTE r,g,b,r0,g0,b0;

			BITMAPINFOHEADER RGB32BITSBITMAPINFO =
			{
				sizeof(BITMAPINFOHEADER),
					m_nWidth, m_nHeight,
					1, 32, BI_RGB,
					0,0,0,0,0
			};

			// 初始化DrawDib库
			DrawDibRealize(m_hDrawDib, dc.GetSafeHdc(), TRUE);

			while(!bDone)
			{
				bDone = TRUE;
				// 位图中每个像素颜色从黑色逐渐递增到原色为止
				for(i = 0; i < m_nWidth; ++i)
				{
					for(j = 0; j < m_nHeight; ++j)
					{
						r	= GetRValue(m_clrDst[i+j*m_nWidth]);
						g	= GetGValue(m_clrDst[i+j*m_nWidth]);
						b	= GetBValue(m_clrDst[i+j*m_nWidth]);
						r0	= GetRValue(m_clrSrc[i+j*m_nWidth]);
						g0	= GetGValue(m_clrSrc[i+j*m_nWidth]);
						b0	= GetBValue(m_clrSrc[i+j*m_nWidth]);

						r = r+10<r0 ? r+10 : r0;
						g = g+10<g0 ? g+10 : g0;
						b = b+10<b0 ? b+10 : b0;
						
						m_clrDst[i+j*m_nWidth] = RGB(r,g,b);

						if((r<r0) || (g<g0) ||(b<b0))
							bDone = FALSE;
					}
				}

				// 绘制颜色值处理后的位图(从黑到原色来实现渐显效果)
				DrawDibDraw(m_hDrawDib, dc.GetSafeHdc(), 0, 0, m_nWidth, m_nHeight, 
							&RGB32BITSBITMAPINFO,(LPVOID)m_clrDst, 0, 0, m_nWidth, 
							m_nHeight, DDF_BACKGROUNDPAL);

				Sleep(50); 
			}

			Sleep(300);
		}

		break;
	case 18:
		// 十九、渐隐效果算法
		{
			if(NULL == m_bmpSrc || NULL == m_hDrawDib)
				return;
			
			// 复制源位图数据到目标位图中
			memcpy((void*)m_clrDst, (void*)m_clrSrc, m_nWidth*m_nHeight*sizeof(COLORREF));

			BOOL bDone = FALSE;
			int i,j;
			BYTE r,g,b;

			BITMAPINFOHEADER RGB32BITSBITMAPINFO =
			{
				sizeof(BITMAPINFOHEADER),
					m_nWidth, m_nHeight,
					1, 32, BI_RGB,
					0,0,0,0,0
			};

			// 初始化DrawDib库
			DrawDibRealize(m_hDrawDib, dc.GetSafeHdc(), TRUE);

			// 先绘制源图，然后再逐渐将它转换为黑色来实现渐隐效果
			DrawDibDraw(m_hDrawDib, dc.GetSafeHdc(), 0, 0, m_nWidth, m_nHeight, 
							&RGB32BITSBITMAPINFO,(LPVOID)m_clrDst, 0, 0, m_nWidth, 
							m_nHeight, DDF_BACKGROUNDPAL);

			while(!bDone)
			{
				bDone = TRUE;
				// 位图中每个像素颜色从原色逐渐递减到黑色为止
				for(i = 0; i < m_nWidth; ++i)
				{
					for(j = 0; j < m_nHeight; ++j)
					{
						r	= GetRValue(m_clrDst[i+j*m_nWidth]);
						g	= GetGValue(m_clrDst[i+j*m_nWidth]);
						b	= GetBValue(m_clrDst[i+j*m_nWidth]);

						r = r-10>0 ? r-10 : 0;
						g = g-10>0 ? g-10 : 0;
						b = b-10>0 ? b-10 : 0;
						
						m_clrDst[i+j*m_nWidth] = RGB(r,g,b);

						// 当颜色接近于纯黑色时退出
						if((r>10) || (g>10) ||(b>10))
							bDone = FALSE;
					}
				}

				// 绘制颜色值处理后的位图(从黑到原色来实现渐显效果)
				DrawDibDraw(m_hDrawDib, dc.GetSafeHdc(), 0, 0, m_nWidth, m_nHeight, 
							&RGB32BITSBITMAPINFO,(LPVOID)m_clrDst, 0, 0, m_nWidth, 
							m_nHeight, DDF_BACKGROUNDPAL);

				Sleep(50); 
			}
		}

		break;
	default:
		// 其它、正常显示
		dc.BitBlt(0, 0, m_nWidth, m_nHeight, &memDC, 0, 0, SRCCOPY);

		break;
	}
	//---------------------------------------------------------

	// 恢复及清扫工作
	memDC.SelectObject(pOldBitmap);         
	dc.SelectPalette(pOldPalette, FALSE);

	memDC.DeleteDC();
}

//This message is an optional extra, If you do not want the splash screen
//to be not be dragable then remove this function and its message map entry
void CSplashWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{

}

void CSplashWnd::OnClose() 
{
  if (m_bOKToClose) 
    CWnd::OnClose();

//   AfxEndThread(0);
}

BOOL CSplashWnd::SelRelPal(BOOL bForceBkgnd)
{
  // We are going active, so realize our palette.
  CDC* pDC = GetDC();

  CPalette* pOldPal = pDC->SelectPalette(&m_Palette, bForceBkgnd);
  UINT u = pDC->RealizePalette();
  pDC->SelectPalette(pOldPal, TRUE);
  pDC->RealizePalette();

  ReleaseDC(pDC);

  // If any colors have changed or we are in the
  // background, repaint the lot.
  if (u || bForceBkgnd) 
    InvalidateRect(NULL, TRUE); // Repaint.
  
  return (BOOL) u; // TRUE if some colors changed.
}

void CSplashWnd::OnPaletteChanged(CWnd* pFocusWnd)
{
  // See if the change was caused by us and ignore it if not.
  if (pFocusWnd != this) 
    SelRelPal(TRUE); // Realize in the background. 
}

BOOL CSplashWnd::OnQueryNewPalette()
{
  return SelRelPal(FALSE); // Realize in the foreground.
}

void CSplashWnd::SetBitmapToUse(const CString& sFilename)
{
  m_bUseFile = TRUE;
  m_sFilename = sFilename;
}

void CSplashWnd::SetBitmapToUse(UINT nResourceID)
{
  m_bUseFile = FALSE;
  m_pszResourceName = MAKEINTRESOURCE(nResourceID);
}

void CSplashWnd::SetBitmapToUse(LPCTSTR pszResourceName)
{
  m_bUseFile = FALSE;
  m_pszResourceName = pszResourceName;
}


BOOL CSplashWnd::PreTranslateMessage(MSG* pMsg) 
{
	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
//		SendMessage(WM_CLOSE);
//		return TRUE;	// message handled here
	}	

	return CWnd::PreTranslateMessage(pMsg);
}
