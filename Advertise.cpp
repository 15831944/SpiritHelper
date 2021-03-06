//////////////////////////////////////////////////
//类名：CAdvertise
//功能：广告特效对话框实现
//作者：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2004.12.1
//////////////////////////////////////////////////
#include "stdafx.h"
#include "helptip.h"
#include "Advertise.h"
#include "math.h"					// 支持数学运算头文件

#include "WaterRoutine.h"			// 水纹特效头文件
#include "FireRoutine.h"			// 火焰特效头文件
#include "PlasmaRoutine.h"			// 熔浆特效头文件
#include "DIBSectionLite.h"			// DIB位图处理头文件
#include "ParticleRoutine.h"		// 爆炸特效头文件
#include "Animation.h"				// 屏保文字效果
#include "AsciiAnimation.h"			// ASCII动画

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 匿名名字空间，可存放全局成员(变量或涵数)
namespace
{
	const int TIMER_ADVERTISE	= 11;	// 定时器触发ID
	
	const int g_nBMPWIDTH		= 30;	// 爆炸单帧位图宽度
	const int g_nBMPHEIGHT		= 22;	// 爆炸单帧位图高度
	const int g_nBMPROCKHEIGHT	= 32;	// 相撞击石块单帧位图高度
	const int g_nBMPROCKWIDTH	= 32;	// 相撞击石块单帧位图宽度
	
	// 雪花效果结构
	const int NUM_SNOWS = 100;
	typedef struct tagSnow
	{
		int x;
		int y;
		int z;
	}SNOW;
}

// ---------------------------------------------------------
//	类名: CAdvertisImpl
//	功能: CAdvertis中镶套类，实现编译时间缩减的防火墙类
//	附注: 可以将CAdvertis类中所有私有成员(包括变量和涵数),
//		  全部放入此类中，虚涵数和保护成员不要放入。
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
class CAdvertise::CAdvertiseImpl
{
public:
	// 绘制全透明位图显示(默认透明显示色为黑),jingzhou xu
	void DrawTransparentBitmap(CDC* pDC,int x, int y, CBitmap *bmpTransparent, COLORREF clrTrans=RGB(0,0,0));
	// 将总位图bmpMap中相应单帧位图投映到位图bmpFrame中
	void Map2Frame(CDC *pDC,const CBitmap& bmpMap,CBitmap& bmpFrame,int nColFrame,int nRowFrame,int bmpWidth,int bmpHeight);
	// 获取缩放处理后的位图
	HBITMAP GetScaleBitmap(CDC* pDC, HBITMAP hBitmap, double nScale = 1.0);

	// 窗体背景上绘制字符串
	CRect DrawTitleInRect(CDC *pDC, CString szString, LPRECT lpRect, long lMode, long lHori, long lVert);
	void DrawBodyText(CDC* pDC);
	void SetFont(const CString& strName, int nSize, int nWeight);			

	// 绘制爆炸效果
	void DarwExplosion(CDC *pDC);

	// 绘制雪花效果
	void DrawSnow(CDC *pDC);

	// 绘制仿闪电效果
	void DrawElectricity(CDC *pDC, CRect rect);

	// 屏保文字效果
	void LayoutScreenSaver(CDC& dc);

	// 绘制动态徽标效果
	void DrawLogo(CDC *pDC);

	// 绘制烟花效果
	void DrawAsciiAnimation(CDC* pDC);

	// 获取指定范围内的随机数
	inline int GetRandomRange(int nMin,int nMax)
	{
		return (rand() % ((nMax+1)-nMin))+nMin;
	}

	// 获取系统区范围大小
	CRect GetTrayWndRect();				

	// 绘制路径特效字体(备用)
	void DrawPathText(CDC* pDC);
	// 绘制垂直文字(默认居中对齐方式)(备用)
	BOOL DrawVerticalText(CDC* pDC, const CRect& rectWindow, LPCSTR szcText, const DWORD dwStyle=SS_CENTER);
	// 获取指定窗体图像到位图bmpWindow中(备用)
	void GetWindowBitmap(CWnd* pWnd, CBitmap* bmpWindow);
	// 获取桌面图像到位图pBitmap中(备用)
	void GetDesktopImage( CBitmap* pBitmap, CSize* pSize = NULL);
	// 透明显示位图并加入旋转指定角度(备用)
	void BlitBitmap( CDC *pDC, CRect rect, UINT uResourceID, COLORREF crMask = RGB(0,0,0), double dDegreesRotation = 0);
	// 将指定位图中一种颜色转换为另一种新颜色(备用)
	HBITMAP ReplaceColor(HBITMAP hBmp,COLORREF cOldColor,COLORREF cNewColor,HDC hBmpDC = NULL);


	CDIBSectionLite		m_bmpRenderSource;		// 源参照图
	CDIBSectionLite		m_bmpRenderTarget;		// 目标效果图
										
	SNOW				m_StarArray[NUM_SNOWS];	// 雪花效果对象
	CWaterRoutine		m_Water;				// 水纹效果对象
	CFireRoutine		m_Fire;					// 火焰效果对象
	CPlasmaRoutine		m_Plasma;				// 熔浆效果对象
	CParticleRoutine	m_Particle;				// 爆炸效果对象
	CAnimationEngine	m_ae;					// 屏保效果对象
												// 烟花效果对象
	FIREWORKS			m_Green, m_Purple, m_Orange;
	CAsciiAnimation		m_AsciiAnimation;
		
	CRect				m_rcClient;				// 窗体客户区大小
	int					m_bmpWidth;				// 载入位图宽度
	int					m_bmpHeight;			// 载入位图高度

	int					m_nAnimation;			// 爆炸位图动画计数
	CBitmap				m_bmpExplosion;			// 爆炸单帧位图
	CBitmap				m_bmpExplosionMap;		// 爆炸总位图

	int					m_nRockAni;				// 撞击石块动画计数
	CBitmap				m_bmpRock;				// 相撞击的石块单帧位图
	CBitmap				m_bmpRockMap;			// 相撞击的石块总位图
	BOOL				m_bExpolsionNow;		// 绘制爆炸效果标志
	int					m_nRockPos;				// 相撞击石块移动位置

	BOOL				m_bRunAsciiAnimation;	// 特效标志(烟花效果)
	BOOL				m_bRunLogo;				// 特效标志(动态徽标)
	BOOL				m_bRunScreenSaver;		// 特效标志(屏保文字)
	BOOL				m_bRunSnow;				// 特效标志(雪花)
	BOOL				m_bRunParticle;			// 特效标志(爆炸、环形波、尾气)
	BOOL				m_bRunWater;			// 特效标志(水纹)
	BOOL				m_bRunFire;				// 特效标志(火焰)
	BOOL				m_bRunPlasma;			// 特效标志(熔浆)
	BOOL				m_bRunElectricity;		// 特效标志(闪电)
	BOOL				m_bRunFont;				// 是否显示背景文字

	CString				m_strText;				// 背景绘制字符串
	CFont				m_font;					// 背景字体
};

// ---------------------------------------------------------
//	名称: CAdvertise
//	功能: 构造涵数，初始化工作
//	参数: 无
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CAdvertise::CAdvertise(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvertise::IDD, pParent)
	, pImpl( new CAdvertiseImpl)				// 初始化编译器防火墙
{
	//{{AFX_DATA_INIT(CAdvertise)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------
//	名称: ~CAdvertise
//	功能: 退出时清除工作
//	参数: 无
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CAdvertise::~CAdvertise()
{
	// 由于pImpl为智能指针，不用在此自己清除
//	delete pImpl;
//	pImpl = NULL;
}

void CAdvertise::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvertise)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvertise, CDialog)
	//{{AFX_MSG_MAP(CAdvertise)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// 设置显示爆炸等特效
void CAdvertise::SetParticleRoutine(BOOL bShow)
{
	pImpl->m_bRunParticle = bShow;
}
// 获取爆炸特效标志
BOOL CAdvertise::GetParticleRoutine()	const
{
	return pImpl->m_bRunParticle;
}

// 设置显示水纹特效
void CAdvertise::SetWaterRoutine(BOOL bShow)
{
	pImpl->m_bRunWater = bShow;
}
// 获取水纹特效标志
BOOL CAdvertise::GetWaterRoutine()	const
{
	return pImpl->m_bRunWater;
}

// 设置显示火焰特效
void CAdvertise::SetFireRoutine(BOOL bShow)
{
	pImpl->m_bRunFire = bShow;
}
// 获取火焰特效标志
BOOL CAdvertise::GetFireRoutine()	const
{
	return pImpl->m_bRunFire;
}

// 设置显示熔浆特效
void CAdvertise::SetPlasmaRoutine(BOOL bShow)
{
	pImpl->m_bRunPlasma = bShow;
}
// 获取熔浆特效标志
BOOL CAdvertise::GetPlasmaRoutine()	const
{
	return pImpl->m_bRunPlasma;
}

// 设置显示屏保文字特效
void CAdvertise::SetShowScreenSaver(BOOL bShow)
{
	pImpl->m_bRunScreenSaver = bShow;
}
// 获取屏保文字特效标志
BOOL CAdvertise::GetShowScreenSaver()	const
{
	return pImpl->m_bRunScreenSaver;
}

// 设置雪花效果
void CAdvertise::SetShowSnow(BOOL bShow)
{
	pImpl->m_bRunSnow = bShow;
}
// 获取雪花特效标志
BOOL CAdvertise::GetShowSnow()	const
{
	return pImpl->m_bRunSnow;
}

// 设置仿闪电效果
void CAdvertise::SetShowElectricity(BOOL bShow)
{
	pImpl->m_bRunElectricity = bShow;
}
// 获取仿闪电特效标志
BOOL CAdvertise::GetShowElectricity()	const
{
	return pImpl->m_bRunElectricity;
}

// 是否显示背景文字
void CAdvertise::SetShowFont(BOOL bShow)
{
	pImpl->m_bRunFont = bShow;
}
// 获取显示背景文字特效标志
BOOL CAdvertise::GetShowFont()	const
{
	return pImpl->m_bRunFont;
}

// 设置显示动态徽标特效
void CAdvertise::SetShowLogo(BOOL bShow)
{
	pImpl->m_bRunLogo = bShow;
}
// 获取动态徽标特效标志
BOOL CAdvertise::GetShowLogo()	const
{
	return pImpl->m_bRunLogo;
}

// 设置显示烟花效果
void CAdvertise::SetShowAsciiAnimation(BOOL bShow)
{
	pImpl->m_bRunAsciiAnimation = bShow;
}
// 获取烟花效果特效标志
BOOL CAdvertise::GetShowAsciiAnimation()	const
{
	return pImpl->m_bRunAsciiAnimation;
}

/////////////////////////////////////////////////////////////////////////////
// CAdvertise message handlers
// ---------------------------------------------------------
//	名称: OnInitDialog
//	功能: 初始化广告对话框
//	参数: 无
//	返回: TRUE --成功，FALSE -- 失败
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
BOOL CAdvertise::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// 获取载入位图的宽度、高度
	BITMAP bm;
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_ADVERTISE);
	bitmap.GetObject(sizeof(BITMAP), &bm);
	pImpl->m_bmpWidth  = bm.bmWidth;
	pImpl->m_bmpHeight = bm.bmHeight;
	bitmap.DeleteObject();

	// 获取窗体客户区大小
	GetClientRect(&pImpl->m_rcClient);

	// 获取边框及标题栏大小
//	int nBorderHeight  = GetSystemMetrics(SM_CYBORDER);
//	int nBorderWidth   = GetSystemMetrics(SM_CXBORDER);
	int nBorderHeight  = GetSystemMetrics(SM_CYDLGFRAME);
	int nBorderWidth   = GetSystemMetrics(SM_CXDLGFRAME);
	int nCaptionHeight = GetSystemMetrics(SM_CYCAPTION);
	
	// 初始置窗体大小为背景位图大小
	CRect rcWindowRect,rcRect,rcDeskTopWnd;
	rcWindowRect.SetRect(0,
		0,
		pImpl->m_bmpWidth + 2*nBorderWidth,
		pImpl->m_bmpHeight + nCaptionHeight + 2*nBorderHeight);

	// 计算广告窗体到系统区位置(注：暂只限任务条在屏幕下方时)
	rcRect=pImpl->GetTrayWndRect();				
	
	::GetWindowRect(::GetDesktopWindow(),&rcDeskTopWnd);

	rcWindowRect.top	= rcDeskTopWnd.bottom -rcRect.Height()-rcWindowRect.Height();
	rcWindowRect.bottom = rcDeskTopWnd.bottom-rcRect.Height();
	rcWindowRect.right	= rcRect.right;
	rcWindowRect.left	= rcWindowRect.right - (pImpl->m_bmpWidth+2*nBorderWidth);

	// 置窗体为显示及其位置
	MoveWindow(rcWindowRect,FALSE);

	// 初始化特效位图
	CPictureHolder TmpPicture;
	TmpPicture.CreateFromBitmap(IDB_ADVERTISE);
	pImpl->m_bmpRenderSource.Create32BitFromPicture(&TmpPicture,pImpl->m_bmpWidth,pImpl->m_bmpHeight);
	pImpl->m_bmpRenderTarget.Create32BitFromPicture(&TmpPicture,pImpl->m_bmpWidth,pImpl->m_bmpHeight);
	
	// 创建水纹对象
	pImpl->m_Water.Create(pImpl->m_bmpWidth,pImpl->m_bmpHeight);
	// 创建火焰对象
	pImpl->m_Fire.m_iAlpha		= 80;		// 80% alpha
	pImpl->m_Fire.m_iHeight		= pImpl->m_bmpHeight;
	pImpl->m_Fire.m_iWidth		= pImpl->m_bmpWidth;
	pImpl->m_Fire.InitFire();
	
	// 创建溶浆对象
	pImpl->m_Plasma.Create(pImpl->m_bmpWidth,pImpl->m_bmpHeight);
	pImpl->m_Plasma.m_iAlpha = 90;					// 90% alpha

	// 创建爆炸对象
	pImpl->m_Particle.Reset_Particles(pImpl->m_bmpWidth,pImpl->m_bmpHeight);
	
	// 初始化屏保效果文字
	pImpl->m_ae.SetScreen(CRect(0,0,pImpl->m_bmpWidth,pImpl->m_bmpHeight));

	// 初始化烟花效果(三种颜色)
	pImpl->m_AsciiAnimation.Create(pImpl->m_bmpWidth,pImpl->m_bmpHeight);
	pImpl->m_Green.Init((pImpl->m_bmpWidth/4), pImpl->m_bmpHeight, '.', pImpl->m_bmpHeight*2/3, 20);
	pImpl->m_Orange.Init(pImpl->m_bmpWidth/2, pImpl->m_bmpHeight, '.', pImpl->m_bmpHeight/2, 25);
	pImpl->m_Purple.Init((pImpl->m_bmpWidth/4)*3, pImpl->m_bmpHeight, '.', pImpl->m_bmpHeight*2/3, 15);

	// 初始化将显示的特效标志
	pImpl->m_bRunWater			= TRUE;
	pImpl->m_bRunFire			= FALSE;
	pImpl->m_bRunPlasma			= FALSE;
	pImpl->m_bRunParticle		= FALSE;
	pImpl->m_bRunSnow			= FALSE;
	pImpl->m_bRunElectricity	= FALSE;
	pImpl->m_bRunLogo			= FALSE;
	pImpl->m_bRunScreenSaver	= FALSE;
	pImpl->m_bRunFont			= FALSE;
	pImpl->m_bRunAsciiAnimation	= FALSE;

	// 初始化背景显示文字及字体
	pImpl->m_strText =  "祝大家身体健康，多多发财!";
	if (NULL == pImpl->m_font.GetSafeHandle())
      pImpl->SetFont("楷体_GB2312", 10, FW_SEMIBOLD);

	// 初始化雪花效果数据
	for (int i = 0; i < NUM_SNOWS; i++)
	{
		pImpl->m_StarArray[i].x  = pImpl->GetRandomRange(0, 1024);
		pImpl->m_StarArray[i].x -= 512;
		pImpl->m_StarArray[i].y  = pImpl->GetRandomRange(0, 1024);
		pImpl->m_StarArray[i].y -= 512;
		pImpl->m_StarArray[i].z  = pImpl->GetRandomRange(0, 512);
		pImpl->m_StarArray[i].z -= 256;
	}

	// 爆炸效果位图动画数据初始化
	pImpl->m_nAnimation		= 0;
	pImpl->m_bmpExplosionMap.LoadBitmap(IDB_EXPLOSION);

	// 相撞击石头数据初始化
	pImpl->m_nRockAni			= 0;
	pImpl->m_bExpolsionNow		= FALSE;
	pImpl->m_nRockPos			= 0;
	pImpl->m_bmpRockMap.LoadBitmap(IDB_ROCK);

	// 启动定时器，显示默认特效
	SetTimer(TIMER_ADVERTISE,100,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ---------------------------------------------------------
//	名称: OnLButtonDown
//	功能: 鼠标单击时水纹特效
//	参数: nFlags	--	按钮标志，point	--	当前坐标像素
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcPicture;

	rcPicture.left		= 15;
	rcPicture.top		= 20;
	rcPicture.right		= rcPicture.left + pImpl->m_bmpRenderSource.GetWidth();
	rcPicture.bottom	= rcPicture.top + pImpl->m_bmpRenderSource.GetHeight();

	if(TRUE == rcPicture.PtInRect(point))
	{
		point.y -= 15;					// 屏幕上偏移(竖直方向相反)
		point.y = pImpl->m_bmpHeight - point.y;

		pImpl->m_Water.HeightBlob(point.x-15,point.y,30,300,pImpl->m_Water.m_iHpage);
	}

	CDialog::OnLButtonDown(nFlags, point);
}

// ---------------------------------------------------------
//	名称: OnMouseMove
//	功能: 鼠标移动时水纹特效
//	参数: nFlags	--	按钮标志，point	--	当前坐标像素
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rcPicture;

	rcPicture.left = 15;
	rcPicture.top = 20;
	rcPicture.right = rcPicture.left + pImpl->m_bmpRenderSource.GetWidth();
	rcPicture.bottom = rcPicture.top + pImpl->m_bmpRenderSource.GetHeight();

	if(TRUE == rcPicture.PtInRect(point))
	{
		point.y -= 5;					// 屏幕上偏移(竖直方向相反)
		point.y = pImpl->m_bmpHeight - point.y;

		pImpl->m_Water.HeightBlob(point.x -5,point.y,5,50,pImpl->m_Water.m_iHpage);
	}

	CDialog::OnMouseMove(nFlags, point);
}

// ---------------------------------------------------------
//	名称: OnTimer
//	功能: 定时更新显示
//	参数: nIDEvent	--	定时器ID
//	返回: 无
//	附注: 其中水纹、火焰、溶浆和爆炸后发散粒子效果直接修改背
//		  景位图中数据来实现，其它则直接在上面处理后的背景位
//		  图上进行绘制得到
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::OnTimer(UINT nIDEvent) 
{
	if(TIMER_ADVERTISE == nIDEvent)
	{	
		// 水纹效果处理
		if(!pImpl->m_bRunWater)
			pImpl->m_Water.FlattenWater();			// 清空数据
		else
		{	
			int nTemp	= pImpl->GetRandomRange(0,100);
			int nX		= pImpl->GetRandomRange(5,pImpl->m_bmpWidth-5);
			int nY		= pImpl->GetRandomRange(5,pImpl->m_bmpHeight-5);
			if(0 == nTemp)					// 仅随机显示各种水纹效果
				pImpl->m_Water.WarpBlob(nX,nY,20,300,pImpl->m_Water.m_iHpage);
			else if(2 == nTemp)
				pImpl->m_Water.HeightBox(nX,nY,10,100,pImpl->m_Water.m_iHpage);
			else if(4 == nTemp)
				pImpl->m_Water.SineBlob(nX,nY,10,100,pImpl->m_Water.m_iHpage);
			else if(8 == nTemp)
				pImpl->m_Water.HeightBlob(nX,nY,10,100,pImpl->m_Water.m_iHpage);
		}
		// 当m_bRunWater为假时(清空数据后)，此语句仅复制正常背景图m_bmpRenderSource到效果图m_bmpRenderTarget中，既恢复目标
		// 图为源图。否则当m_bRunWater为真时，进行水纹效果处理。 要想在火焰和溶浆中背景位图正常显示，必须先执行这条语句
		pImpl->m_Water.Render((DWORD*)pImpl->m_bmpRenderSource.GetDIBits(),(DWORD*)pImpl->m_bmpRenderTarget.GetDIBits());

		// 火焰效果处理(注：经测试，火焰效果最浪费CPU资源，请慎用，其它效果还可以),jingzhou xu
		if(pImpl->m_bRunFire)
			pImpl->m_Fire.Render((DWORD*)pImpl->m_bmpRenderTarget.GetDIBits(),pImpl->m_bmpWidth,pImpl->m_bmpHeight);

		// 熔浆效果处理
		if(pImpl->m_bRunPlasma)
			pImpl->m_Plasma.Render((DWORD*)pImpl->m_bmpRenderTarget.GetDIBits(),pImpl->m_bmpWidth,pImpl->m_bmpHeight,pImpl->m_bmpWidth);

		// 爆炸后发散粒子效果处理(以背景位图中心点为中心来处理),石头相撞后真正产生
		if(pImpl->m_bRunParticle)
		{
			// ------------ 下面为模拟爆炸后发散的粒子现象 ------------ //
			if(pImpl->m_bExpolsionNow)
			{
				int nTemp = pImpl->GetRandomRange(0,2);
				if(0 == nTemp)					// 显示环形波效果
					pImpl->m_Particle.Set_Particle_Ring(1,1,30,pImpl->m_bmpWidth/2,pImpl->m_bmpHeight/2,0,0);
				else if(1 == nTemp)				// 显示尾汽效果
					pImpl->m_Particle.Set_Particle_Gas(0,6,pImpl->m_bmpWidth/2,pImpl->m_bmpHeight/2,1,-4);
				else							// 显示爆炸效果
					pImpl->m_Particle.Set_Particle_Explosion(1,3,30,pImpl->m_bmpWidth/2,pImpl->m_bmpHeight/2,1,5);
			}

			pImpl->m_Particle.Process_Particles((DWORD*)pImpl->m_bmpRenderTarget.GetDIBits());
			// -------------------------------------------------------- //
		}

		// 绘制背景位图内部数据处理后的效果
		CClientDC dc(this);
		CPoint ptOrigin(0,0);
		pImpl->m_bmpRenderTarget.Stretch(&dc,ptOrigin,CSize(pImpl->m_bmpWidth,pImpl->m_bmpHeight));

		// 爆炸时动画位图效果(在背景位图绘制后进行)
		if(pImpl->m_bRunParticle)
			pImpl->DarwExplosion(&dc);

		// 绘制烟花效果
		if(pImpl->m_bRunAsciiAnimation)
			pImpl->DrawAsciiAnimation(&dc);

		// 绘制动态徽标效果
		if(pImpl->m_bRunLogo)
			pImpl->DrawLogo(&dc);

		// 绘制屏保效果文字
		if(pImpl->m_bRunScreenSaver)
			pImpl->LayoutScreenSaver(dc);

		// 绘制雪花效果
		if(pImpl->m_bRunSnow)
			pImpl->DrawSnow(&dc);

		// 绘制仿闪电效果
		if(pImpl->m_bRunElectricity)
			pImpl->DrawElectricity(&dc,CRect(0,0,pImpl->m_bmpWidth,pImpl->m_bmpHeight));

		// 绘制背景文本
		if(pImpl->m_bRunFont)
			pImpl->DrawBodyText(&dc);

	}

	CDialog::OnTimer(nIDEvent);
}

// ---------------------------------------------------------
//	名称: DestroyWindow
//	功能: 退出前清除工作
//	参数: 无
//	返回: TRUE--成功，FALSE--失败
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
BOOL CAdvertise::DestroyWindow() 
{
	// 清除屏保效果
	pImpl->m_ae.DestroyGDIs();

	// 清除定时器
	KillTimer(TIMER_ADVERTISE);

	return CDialog::DestroyWindow();
}

// ---------------------------------------------------------
//	名称: OnPaint
//	功能: 刷新窗体背景位图
//	参数: 无
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CPoint ptOrigin(0,0);
	pImpl->m_bmpRenderTarget.Stretch(&dc,ptOrigin,CSize(pImpl->m_bmpWidth,pImpl->m_bmpHeight));
}

// ---------------------------------------------------------
//	名称: PreTranslateMessage
//	功能: 截获ESC和回车键，避免按下此键时关闭对话框
//	参数: pMsg -- 消息
//	返回: 成功 -- TRUE，失败 -- FALSE
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
BOOL CAdvertise::PreTranslateMessage(MSG* pMsg) 
{
	//截获ESC和回车键，避免按下此键时关闭对话框
	if (pMsg->message == WM_KEYDOWN)
    { 
      if(pMsg->wParam==VK_ESCAPE)
		  return true; 
	  if(pMsg->wParam==VK_RETURN) 
	  {
		  return true; 
	  }
    } 
	
	return CDialog::PreTranslateMessage(pMsg);
}

// ---------------------------------------------------------
//	名称: GetTrayWndRect
//	功能: 获取系统区范围大小
//	参数: 无
//	返回: CRect -- 系统区大小
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
CRect CAdvertise::CAdvertiseImpl::GetTrayWndRect()             
{
	// 查找并返回任务栏窗口大小 
	CRect rect(0,0,0,0);
    CWnd* pWnd = FindWindow("Shell_TrayWnd", NULL);
    if (pWnd)
    {
        pWnd->GetWindowRect(&rect);
	}
	return rect;
}

// ---------------------------------------------------------
//	名称: DarwExplosion
//	功能: 绘制移动中石块相撞效果，并模仿相撞后爆炸动画位图，
//		  以及爆炸中会产生爆炸粒子、冲击形粒子效果等
//  附注: 左侧大石块撞击右侧小石块后，在撞击处爆炸的同时，
//		  左侧大石块会分裂产生反方向运动的二个小石块
//	参数: pDC -- 相关DC
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DarwExplosion(CDC *pDC)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	if(m_bExpolsionNow)			// 显示爆炸动画
	{
		// 映射总位图m_bmpExplosionMap(共二行)中相应帧位图到m_bmpExplosion中
		Map2Frame(pDC, m_bmpExplosionMap, m_bmpExplosion, m_nAnimation%4, m_nAnimation>3 ? 1 : 0, g_nBMPWIDTH, g_nBMPHEIGHT);
		
		// 将映射后的相应位图背景透明化后绘制(在背景位图中心点绘制)
		DrawTransparentBitmap(pDC, m_bmpWidth/2 - g_nBMPWIDTH/2, m_bmpHeight/2 - g_nBMPHEIGHT/2, &m_bmpExplosion);

		// --------------- 绘制左边大石块撞击后，分裂为二个反方向的小石块动画 -----------------
		
		// 映射总位图m_bmpRockMap(一行8个)中相应帧位图到m_bmpRock中
		Map2Frame(pDC, m_bmpRockMap, m_bmpRock, m_nAnimation%8, 0, g_nBMPROCKWIDTH, g_nBMPROCKHEIGHT);

		// 位图缩小一倍
		CBitmap bmpTemp;
		bmpTemp.Attach(GetScaleBitmap(pDC, m_bmpRock, 0.5));
		// 撞击后从中心点开始，右向左移动中二个小石块(缩小一倍)
		DrawTransparentBitmap(pDC, m_bmpWidth/2 - g_nBMPROCKWIDTH/4 - m_nRockPos, m_bmpHeight/2 - g_nBMPROCKHEIGHT/2 - m_nRockPos/2, &bmpTemp);
		DrawTransparentBitmap(pDC, m_bmpWidth/2 - g_nBMPROCKWIDTH/4 - m_nRockPos, m_bmpHeight/2 + m_nRockPos/2, &bmpTemp);

		// 石块移动速度
		m_nRockPos			+= 8;

		// --------------------------------------------------------------------------------------

		// 爆炸位图动画计数(当帧数大于8时，重新从0开始)
		++m_nAnimation;
		if(m_nAnimation >= 8)
			m_nAnimation = 0;

		// 当爆炸动画显示一轮后停止，重新从移动撞击石块开始
		if(0 == m_nAnimation)
		{
			m_bExpolsionNow	= FALSE;
			// 在重新开始撞击石块动画前，石块置初始位置为0
			if(0 != m_nRockPos)
				m_nRockPos	= 0;
		}
	}
	else						// 显示相撞石块移动动画
	{
		// 映射总位图m_bmpRockMap(一行)中相应帧位图到m_bmpRock中
		Map2Frame(pDC, m_bmpRockMap, m_bmpRock, m_nRockAni%8, 0, g_nBMPROCKWIDTH, g_nBMPROCKHEIGHT);

		// 从左向右移动中石块
		DrawTransparentBitmap(pDC, m_nRockPos, m_bmpHeight/2 - g_nBMPROCKHEIGHT/2, &m_bmpRock);
		
		// 位图缩小1/5
		CBitmap bmpTemp;
		bmpTemp.Attach(GetScaleBitmap(pDC, m_bmpRock, 0.8));
		// 从右向左移动中石块(缩小1/5)
		DrawTransparentBitmap(pDC, m_bmpWidth-g_nBMPROCKWIDTH-m_nRockPos, static_cast<int>(m_bmpHeight/2 - g_nBMPROCKHEIGHT*0.4), &bmpTemp);

		// 石块移动速度
		m_nRockPos			+= 10;

		// 撞击石块动画计数(当帧数大于8时，重新从0开始)
		++m_nRockAni;
		if(m_nRockAni >= 8)
			m_nRockAni	= 0;

		// 相撞处理
		if(m_nRockPos > m_bmpWidth/2 - g_nBMPROCKWIDTH/2)
		{
			m_nRockPos		= 0;
			m_bExpolsionNow	= TRUE;
		}
	}
}

// ---------------------------------------------------------
//	名称: GetScaleBitmap
//	功能: 获取缩放比例处理后的位图
//	参数: pDC -- 相关DC，hBitmap -- 进行缩放处理的位图
//		  nScale :
//					= 0,1,-1, 不进行缩放处理，原倍数输出(默认设置1.0)
//					= 当nScale>1时，放大处理; 当0<nScale<1时，缩小处理
//					= 当nScale<-1时,缩小处理; 当-1<nScale<0时，放大处理
//	返回: 缩放处理后的位图
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
HBITMAP CAdvertise::CAdvertiseImpl::GetScaleBitmap(CDC* pDC, HBITMAP hBitmap, double nScale /* = 1.0 */)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	// 比例因子为0,1,-1时，不进行缩放处理，原倍数返回
	if(1.0 == nScale || -1.0 == nScale || 0.0 == nScale)
		return hBitmap;

	// 创建兼容DC
	CDC sourceDC, destDC;
	sourceDC.CreateCompatibleDC(pDC);
	destDC.CreateCompatibleDC(pDC);

	// 获取位图宽、高
	BITMAP bm;
	::GetObject(hBitmap, sizeof( bm ), &bm);

	// 缩放比例处理
	int nWidth	= bm.bmWidth;
	int nHeight	= bm.bmHeight;
	if(nScale > 0.0)				// 当nScale>1时，放大处理; 当0<nScale<1时，缩小处理
	{
		nWidth	= static_cast<int>(nWidth * nScale);
		nHeight	= static_cast<int>(nHeight * nScale);
	}
	else if(nScale < 0.0)			// 当nScale<-1时,缩小处理; 当-1<nScale<0时，放大处理
	{
		nWidth	= static_cast<int>(nWidth / fabs(nScale));
		nHeight	= static_cast<int>(nHeight / fabs(nScale));
	}

	// 创建缩放处理结果位图
	HBITMAP hbmResult = ::CreateCompatibleBitmap(pDC->m_hDC, 
						nWidth, nHeight);

	// 选入相应位图到DC中
	HBITMAP hbmOldSource = (HBITMAP)::SelectObject(sourceDC.m_hDC, hBitmap);
	HBITMAP hbmOldDest = (HBITMAP)::SelectObject(destDC.m_hDC, hbmResult);
	
	// 位图缩放处理
	destDC.StretchBlt(0, 0, nWidth, nHeight, &sourceDC, 
				0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

	// 恢复及清扫工作
	::SelectObject(sourceDC.m_hDC, hbmOldSource);
	::SelectObject(destDC.m_hDC, hbmOldDest);

	// 返回缩放处理后的hbmResult，不能清除
	return hbmResult;
}

// ---------------------------------------------------------
//	名称: DrawTransparentBitmap
//	功能: 绘制背景全透明位图
//	参数: pDC -- 相关DC，x,y -- 绘制左、上角坐标
//		  bmpTransparent -- 透明显示位图
//		  crTrans -- 要透明显示的颜色(默认为黑色)
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawTransparentBitmap(CDC* pDC, int x, int y, CBitmap *bmpTransparent, COLORREF clrTrans)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));
	ASSERT(FALSE == IsBadReadPtr(bmpTransparent,sizeof(CBitmap*)));

	COLORREF crOldBack = pDC->SetBkColor(RGB(255,255,255));
	COLORREF crOldText = pDC->SetTextColor(RGB(0,0,0));
	CDC dcImage,dcTrans,dcBack;
	
	// 创建透明显示位图dcImage，转换二色位图dcTrans以及背景位图dcBack的兼容DC
	dcImage.CreateCompatibleDC(pDC);
	dcTrans.CreateCompatibleDC(pDC);
	dcBack.CreateCompatibleDC(pDC);

	// 获取要透明显示位图宽、高
	BITMAP bm;
	bmpTransparent->GetObject(sizeof(BITMAP), &bm);
	int nWidth	=  bm.bmWidth;
	int nHeight = bm.bmHeight;

	// 选入要透明显示的位图
	CBitmap* pOldBitmapImage; 
	pOldBitmapImage = dcImage.SelectObject(bmpTransparent);

	// 创建当前背景兼容位图，并选入dcBack
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC,nWidth,nHeight);
	CBitmap* pOldBitmapBack = dcBack.SelectObject(&bmp);

	// 创建二色位图，并选入dcTrans
	CBitmap bitmapTrans;
	bitmapTrans.CreateBitmap(nWidth,nHeight,1,1,NULL);	
	CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

	// 设置屏蔽色(默认为黑色)
	dcImage.SetBkColor(clrTrans);
	// 创建用于透明变换的二色位图(透明部分用1表示，不透明部分用0表示)
	dcTrans.BitBlt(0,0,nWidth,nHeight,&dcImage,0,0,SRCCOPY);
	// 将当前屏幕内容复制到dcBack中
	dcBack.BitBlt(0,0,nWidth,nHeight,pDC,x,y,SRCCOPY);
	// 将dcBack中内容与要透明显示目标位图进行异或操作
	dcBack.BitBlt(0,0,nWidth,nHeight,&dcImage,0,0,SRCINVERT);
	// 将二色位图与dcBack中内容进行与操作
	dcBack.BitBlt(0,0,nWidth,nHeight,&dcTrans,0,0,SRCAND);
	// 再将dcBack中内容与要透明显示目标位图进行异或操作
	dcBack.BitBlt(0,0,nWidth,nHeight,&dcImage,0,0,SRCINVERT);

	// 将经过透明变换后的位图输出
	pDC->BitBlt(x,y,nWidth,nHeight,&dcBack,0,0,SRCCOPY);
	
	// 还原设置
	dcImage.SelectObject(pOldBitmapImage);
	dcTrans.SelectObject(pOldBitmapTrans);
	dcBack.SelectObject(pOldBitmapBack);
	pDC->SetBkColor(crOldBack);
	pDC->SetTextColor(crOldText);
	// 清扫工作
	bmp.DeleteObject();
	bitmapTrans.DeleteObject();
}

// ---------------------------------------------------------
//	名称: Map2Frame
//	功能: 将总位图bmpMap中相应单帧位图映射到bmpFrame中
//	参数: 
//			pDC			:	传入显示位图的dc指针
//			bmpMap		:	传入显示总位图引用	
//			bmpFrame	:	传入映射后的单帧位图引用
//			nColFrame	:	映射总位图中的那一列号
//			nRowFrame	:	映射总位图中的那一行号
//			bmpWidth	:	单帧位图的宽度
//			bmpHeight	:	单帧位图的高度
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::Map2Frame(CDC *pDC,const CBitmap& bmpMap,CBitmap& bmpFrame,int nColFrame,int nRowFrame,int bmpWidth,int bmpHeight)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	// 先清空原单帧位图
	bmpFrame.DeleteObject();

	// 选入总位图到MapDC中
	CDC MapDC;
	MapDC.CreateCompatibleDC(pDC);
	MapDC.SelectObject(bmpMap);

	// 选入单帧位图到FrameDC中
	bmpFrame.CreateCompatibleBitmap(pDC, bmpWidth, bmpHeight);
	CDC FrameDC;
	FrameDC.CreateCompatibleDC(pDC);
	CBitmap* pOB = FrameDC.SelectObject(&bmpFrame);

	// 将总位图bmpMap中相应行、列单帧位图投映到bmpFrame中
	FrameDC.StretchBlt(0, 0, bmpWidth, bmpHeight, &MapDC, nColFrame*bmpWidth,nRowFrame*bmpHeight,bmpWidth,bmpHeight,SRCCOPY);

	// 恢复源设置
	FrameDC.SelectObject(pOB);
}

// ---------------------------------------------------------
//	名称: SetFont
//	功能: 设置显示m_font字体
//	参数: strName -- 字体名，nSize -- 字体高度，nWeight -- 字体宽度
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::SetFont(const CString& strName, int nSize, int nWeight)
{
   if ( NULL != m_font.GetSafeHandle() )
      m_font.DeleteObject();

   LOGFONT logFont;
   memset(&logFont,0,sizeof(logFont));
   strncpy(logFont.lfFaceName,strName,LF_FACESIZE);
   logFont.lfPitchAndFamily = FF_SWISS;
   logFont.lfQuality = ANTIALIASED_QUALITY;
   logFont.lfWeight = nWeight;
   logFont.lfHeight = nSize*10;

   if ( !m_font.CreatePointFontIndirect(&logFont) )
      m_font.CreateStockObject(SYSTEM_FONT);
}

// ---------------------------------------------------------
//	名称: DrawPathText
//	功能: 绘制用位图填充的路径特效文字
//	参数: pDC -- 相关DC 
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawPathText(CDC* pDC)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	CFont fnt;
	// 创建字体宽度50
	fnt.CreateFont(50,0,0,0,FW_BLACK,FALSE,FALSE,FALSE,GB2312_CHARSET,
					OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,
					FIXED_PITCH|FF_MODERN,"楷体_GB2312");

	CRect rect(m_rcClient);

	CFont* pOldFont = (CFont*)pDC->SelectObject(&fnt);
	pDC->SetBkMode(TRANSPARENT);
	
	int nPos = rect.Width()/4;			// 绘制四个字x位置
	pDC->BeginPath();
	pDC->TextOut(0, 10, "特", 2);
	pDC->TextOut(nPos, 10, "效", 2);
	pDC->TextOut(2*nPos, 10, "窗", 2);
	pDC->TextOut(3*nPos, 10, "体", 2);
	pDC->EndPath();

	// 恢复源字体
	pDC->SelectObject(pOldFont);
	// 清扫工作
	fnt.DeleteObject();

	// 获取路径数据
	int nCount		= pDC->GetPath(NULL,NULL,0);
	CPoint* points	= new CPoint[nCount];
	BYTE* bytes		= new BYTE[nCount];
	pDC->GetPath(points,bytes,nCount);

	// 对路径点进行角度变换
	int i;
	for(i=0; i < nCount; ++i)
		points[i].y = points[i].y + static_cast<int>(80*sin(points[i].x/300.*3.1415926) + 100);

	// 构造新路径
	CPoint ptStart;
	pDC->BeginPath();
	CPen pen(PS_SOLID,1,RGB(0,0,0));
	CPen *pOldPen = pDC->SelectObject(&pen);
	for(i=0; i < nCount; ++i)
	{
		switch(bytes[i])
		{
		case PT_MOVETO:
			pDC->MoveTo(points[i]);
			ptStart	= points[i];
			
			break;
		case PT_LINETO:
			pDC->LineTo(points[i]);

			break;
		case PT_BEZIERTO:
			pDC->PolyBezierTo(points+i, 3);
			i +=2;

			break;
		case PT_BEZIERTO|PT_CLOSEFIGURE:
			points[i+2]	= ptStart;
			pDC->PolyBezierTo(points+i, 3);
			i +=2;

			break;
		case PT_LINETO|PT_CLOSEFIGURE:
			pDC->LineTo(ptStart);

			break;
		}
	}
	pDC->SelectObject(pOldPen);
	pDC->CloseFigure();
	pDC->EndPath();

	// 清扫工作
	delete [] points;
	delete [] bytes;

	// 设置裁剪路径
	pDC->SetPolyFillMode(WINDING);
	pDC->SelectClipPath(RGN_COPY);

	// 用位图填充剪裁区域
	CBitmap		bmp;
	CBitmap*	pBmpOld;
	bmp.LoadBitmap(IDB_SPLASH);

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	pBmpOld	= dcMem.SelectObject(&bmp);

	// 其中232,327为位图宽、高
	pDC->StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, 232, 327, SRCCOPY);

	// 恢复及清扫工作
	dcMem.SelectObject(pBmpOld);
	dcMem.DeleteDC();
	bmp.DeleteObject();
}

// 匿名名字空间，可存放全局成员(变量或涵数)
namespace
{
	int nTop = 0;				// 背景文字顶部显示位置初始化
}
// ---------------------------------------------------------
//	名称: DrawBodyText
//	功能: 窗体中绘制动态文字m_strText
//	参数: pDC -- 相关DC 
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawBodyText(CDC* pDC)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	CRect rect(m_rcClient);

	CFont* pOldFont = pDC->SelectObject(&m_font);
	pDC->SetBkMode(TRANSPARENT);
	
	// 偏移字体大小1/10
	LOGFONT logFont;
	m_font.GetLogFont(&logFont);
	int nShadowOffset = MulDiv(-logFont.lfHeight, 72, pDC->GetDeviceCaps(LOGPIXELSY)*10);

	// 计算动态移动文字矩形位置
	nTop -= 2;
	int nTempHeight = nTop + rect.Height();
	CRect rcTemp = CRect(rect.left,nTop,rect.left+rect.Width(),nTempHeight);
	if(0 > nTempHeight)
		nTop = rect.top + rect.Height();

	// 绘制3D文字
	pDC->SetTextColor(RGB(255,255,255));
	rcTemp.OffsetRect(nShadowOffset,nShadowOffset);
	DrawTitleInRect(pDC,m_strText,rcTemp,0,1,0);

	rcTemp.OffsetRect(-nShadowOffset,-nShadowOffset);
	pDC->SetTextColor(RGB(0,0,0));
	DrawTitleInRect(pDC,m_strText,rcTemp,0,1,0);

	rcTemp.OffsetRect(-nShadowOffset,-nShadowOffset);
	pDC->SetTextColor(RGB(192,192,192));
	DrawTitleInRect(pDC,m_strText,rcTemp,0,1,0);

	pDC->SelectObject(pOldFont);

}

// ---------------------------------------------------------
//	名称: DrawTitleInRect
//	功能: 在矩形框中水平或垂直显示多行文字
//	参数: pDC		-- 相关DC
//		  szString	-- 绘制的字符串
//		  lpRect	-- 绘制的矩形范围
//		  lMode		-- 排列方式，0:水平方式; 1:垂直方式    
//		  lHori		-- 水平对齐方式, 0:左对齐; 1:居中; 2:右对齐; 3：自定义
//		  lVert		-- 垂直对齐方式, 0:顶对齐; 1:居中; 2:底对齐; 3：自定义
//	返回: 真正绘制时时采用的矩形
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
CRect CAdvertise::CAdvertiseImpl::DrawTitleInRect(CDC *pDC, CString szString, LPRECT lpRect, long lMode, long lHori, long lVert)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

    CRect rcInner(lpRect);

    if(0 == rcInner.Width())
        return rcInner;

    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    int tmpWidth=tm.tmAveCharWidth;		// tmpHeight=tm.tmHeight;

    //---------------------------------------------------------------------------------------------
    //功能：根据新、老矩形，重新计算行数，使文字多行显示,jingzhou xu
    //---------------------------------------------------------------------------------------------
    //一行中最大字符数
    int nMaxLineChar = abs(lpRect->right - lpRect->left) / tmpWidth;    
    if(nMaxLineChar < 2)               //应该至少能显示一个汉字
        return rcInner;

    //记录当前行的宽度
    short theLineLength=0; 
    //记录当前行中汉字字节数，以防止将一半汉字分为两行
    unsigned short halfChinese=0;

    for(int i=0; i<=szString.GetLength()-1; i++)
    {
        if((0x0d == (unsigned char)szString.GetAt(i)) && (0x0a == (unsigned char)szString.GetAt(i+1)))
            theLineLength=0;

        // 在此加入"||"字符为换行标志字符，输入时可根据此字符串来自动换行
        if(('|' == (unsigned char)szString.GetAt(i)) && ('|' == (unsigned char)szString.GetAt(i+1)))
        {
            szString.SetAt(i,(unsigned char)0x0d);
            szString.SetAt(i+1,(unsigned char)0x0a);
			theLineLength=0;
        }

        //大于0xa1的字节为汉字字节
        if((unsigned char)szString.GetAt(i) >= 0xA1)
            halfChinese++;
        theLineLength++;

        //如果行宽大于每行最大宽度，进行特殊处理
        if(theLineLength > nMaxLineChar)
        {
            //防止将一个汉字分为两行，回溯
            if(!(halfChinese%2) && (unsigned char)szString.GetAt(i) >= 0xA1)
            {
                szString.Insert(i-1,(unsigned char)0x0a);
                szString.Insert(i-1,(unsigned char)0x0d);
                //注：此处不加一跳过，是由于它是在i-1处添加，只需跳到<i+1>处，故只需在循环处加一次既可。
            }
            else
            {
                szString.Insert(i,(unsigned char)0x0a);
                szString.Insert(i,(unsigned char)0x0d);

                i++;                      //跳过新增的换行符，应跳到<i+2>处(循环中加一次，故这里只加一次)
            }
            
            theLineLength = 0;
            halfChinese=0;
        }
    }

    if(0 == lMode)                          //水平排列 
    {
        rcInner.left+=tmpWidth;
        rcInner.right-=tmpWidth;
        rcInner.top+=tmpWidth;
        rcInner.bottom-=tmpWidth;
    }
    if(1 == lMode)                          //垂直排列
    {
        rcInner.left+=tmpWidth;
        rcInner.right=rcInner.left+tmpWidth;
        rcInner.top+=tmpWidth;
        rcInner.bottom-=tmpWidth;
    }

    //重新计算矩形边界范围
    pDC->DrawText(szString, rcInner,DT_WORDBREAK|DT_LEFT|DT_CALCRECT);

    switch(lHori)
    {
    case 0:
        break;
    case 1:
        {
            long xOutCent=(lpRect->right+lpRect->left)/2;
            long xInnCent=(rcInner.right+rcInner.left)/2;
            rcInner.left+=(xOutCent-xInnCent);
            rcInner.right+=(xOutCent-xInnCent);
        }
        break;
    case 2:
        {
            long lInWidth=rcInner.right-rcInner.left;
            rcInner.right=lpRect->right-tmpWidth;
            rcInner.left=rcInner.right-lInWidth;
        }
        break;
    default:
        break;
    }
    
    switch(lVert)
    {
    case 0:
        break;
    case 1:
        {
            long yOutCent=(lpRect->bottom+lpRect->top)/2;
            long yInnCent=(rcInner.bottom+rcInner.top)/2;
            rcInner.top-=(yInnCent-yOutCent);
            rcInner.bottom-=(yInnCent-yOutCent);
        }
        break;
    case 2:
        {
            long lInHeigh=rcInner.top-rcInner.bottom;
            rcInner.bottom=lpRect->bottom+tmpWidth;
            rcInner.top=rcInner.bottom+lInHeigh;
        }
        break;
    default:
        break;
    }

    if(rcInner.bottom < lpRect->bottom)
        rcInner.bottom = lpRect->bottom;
    if(rcInner.top > lpRect->top)
        rcInner.top = lpRect->top;

    //---------------------------------------------------------------------------------------------

    if(0 == lHori)
        pDC->DrawText(szString, rcInner, DT_WORDBREAK|DT_LEFT);
    else if(1 == lHori)
        pDC->DrawText(szString, rcInner, DT_WORDBREAK|DT_CENTER);
    else if(2 == lHori)
        pDC->DrawText(szString, rcInner, DT_WORDBREAK|DT_RIGHT);

    return rcInner;
}

// ---------------------------------------------------------
//	名称: DrawVerticalText
//	功能: 绘制垂直方向文字
//	参数: pDC -- 相关DC, rectWindow -- 矩形大小, szcText -- 文字串,
//		  dwStyle -- 垂直显示风格(默认为居中对齐SS_CENTER)，包括：
//		　SS_CENTER(居中对齐)，SS_RIGHT(右对齐)和SS_LEFT(左对齐)				
//	返回: 成功 -- TRUE, 失败 -- FALSE
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
BOOL CAdvertise::CAdvertiseImpl::DrawVerticalText(CDC* pDC, const CRect& rectWindow, LPCSTR szcText, const DWORD dwStyle)
{
	int nSavedDC = pDC->SaveDC();

	// 获取当前DC字体
	LOGFONT lf;
	CFont *pTmpFont = pDC->GetCurrentFont();

	if( NULL == pTmpFont )
	{
		ASSERT( pTmpFont );		// 当前DC中没有选取字体
		return FALSE;
	}

	pTmpFont->GetObject( sizeof(LOGFONT), &lf );
	
    // 将文字方向顺时针旋转90度
	lf.lfOrientation = lf.lfEscapement = 900;

    lstrcpy( lf.lfFaceName, _T("宋体") );

	// 创建新的处理后的字体
    CFont font;
    if( FALSE == font.CreateFontIndirect( &lf ) )
	{
		TRACE2("Error creating font! Line: %d, File: %s\n", __LINE__, __FILE__ );
		return FALSE;
	}

	CFont *pfontOld = pDC->SelectObject( &font );

	CRect rectText( rectWindow );

	// 计算绘制文字原水平方向的矩形大小
	pDC->DrawText ( szcText, 
		            rectText,
                    DT_LEFT 
				   |DT_TOP 
				   |DT_CALCRECT 
				   |DT_SINGLELINE );

	// 根据属性标志重新计算垂直文字后的矩形大小
	CSize szOffset(0,0);
    if ( dwStyle & SS_CENTER )
	{
	    szOffset = CSize( (rectWindow.Width()-rectText.Height())/2, 
			              -(rectWindow.Height()-rectText.Width())/2+rectText.Height() );
	}
    else if ( dwStyle & SS_RIGHT )
	{
	    szOffset = CSize( (rectWindow.Width()-rectText.Height())/2, 
			              -rectWindow.Height()+rectText.Width()+rectText.Height() );
	}
    else
	{
	    szOffset = CSize( (rectWindow.Width()-rectText.Height())/2, 
			              rectText.Height() );
	}
	
	// rectText.SetRect( rectWindow.left, 0, rectWindow.left + rectText.Height(), rectWindow.Height());
	// rectText.SetRect( rectWindow.left, rectWindow.top, rectText.right, rectWindow.Height());
	rectText.top    = rectWindow.top;
	rectText.bottom = rectWindow.bottom;
	
	rectText.OffsetRect(szOffset);
	rectText.top = 0;

	if( rectWindow.Height() < rectText.Width() )
		rectText.right -= ((rectText.Width() - rectWindow.Height())/2);

	pDC->SetBkMode( TRANSPARENT );
	pDC->DrawText( szcText, 
		           rectText,
                   DT_LEFT 
				  |DT_BOTTOM 
				  |DT_SINGLELINE 
				  |DT_END_ELLIPSIS );

    pDC->SelectObject ( pfontOld );
	pDC->RestoreDC( nSavedDC );
    
	return TRUE;
}

// ---------------------------------------------------------
//	名称: GetWindowBitmap
//	功能: 获取指定窗体图像到位图bmpWindow中
//	参数: pWnd -- 指定窗体, bmpWindow -- 要放入窗体图像的返回位图指针
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::GetWindowBitmap(CWnd* pWnd, CBitmap* bmpWindow)
{

	DWORD dwWndStyle = pWnd->GetStyle();
	// 取消滚动条显示
	if(dwWndStyle & WS_HSCROLL)
		pWnd->ShowScrollBar(SB_HORZ, FALSE);

	if(dwWndStyle & WS_VSCROLL)
		pWnd->ShowScrollBar(SB_VERT, FALSE);


	// 获取窗体大小
	CRect rectWindow;
	pWnd->GetClientRect(rectWindow);

	// 创建兼容位图
	CClientDC dc(pWnd);
	VERIFY(bmpWindow->CreateCompatibleBitmap(&dc, rectWindow.Width(), rectWindow.Height()));

	// 将创建好的目标位图选入兼容DC中
	CDC tmpdc;
	tmpdc.CreateCompatibleDC( &dc );

//	int nSavedDC = tmpdc.SaveDC();
	CBitmap* pOldBmp = tmpdc.SelectObject( bmpWindow );

	pWnd->Print(&tmpdc, PRF_CLIENT|PRF_NONCLIENT|PRF_ERASEBKGND|PRF_CHILDREN);

	tmpdc.SelectObject(pOldBmp);

	// 重新恢复滚动条显示
	if(dwWndStyle & WS_HSCROLL)
		pWnd->ShowScrollBar(SB_HORZ, TRUE);

	if(dwWndStyle & WS_VSCROLL)
		pWnd->ShowScrollBar(SB_VERT, TRUE);

	// 恢复及清扫工作
	tmpdc.SelectObject(pOldBmp);

	return;
}

// ---------------------------------------------------------
//	名称: GetDesktopImage
//	功能: 获取桌面图像到位图pBitmap中
//	参数: pBitmap -- 要放入桌面图像的返回参数
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::GetDesktopImage(CBitmap* pBitmap, CSize* pSize)
{

    CDC dcDesktop;

    CSize DesktopSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

	// 获取桌面DC
    dcDesktop.m_hDC = ::GetDC(NULL);

	// 将参数位图指针放入桌面兼容DC中
	CRect Client;	
	dcDesktop.GetClipBox(&Client);

	int nWidth, nHeight;
	if (NULL ==  pSize)
	{
		nWidth	= Client.Width();
		nHeight	= Client.Height();
	}
	else
	{
		nWidth	= pSize->cx;
		nHeight	= pSize->cy;
	}
		
	CDC MemDC;
	MemDC.CreateCompatibleDC(&dcDesktop);

	pBitmap->CreateCompatibleBitmap(&dcDesktop, nWidth, nHeight);

	MemDC.SelectObject(pBitmap);

    MemDC.BitBlt(0, 0, DesktopSize.cx, DesktopSize.cy, &dcDesktop, 0, 0, SRCCOPY);


    dcDesktop.DeleteDC();
	MemDC.DeleteDC();
}

// ---------------------------------------------------------
//	名称: BlitBitmap
//	功能: 透明显示位图并加入旋转指定角度
//	参数: pDC -- 相关dc指针，rect -- 显示范围，uResourceID -- 透明旋转位图资源ID
//		  crMask -- 背景透明色(默认黑色)，dDegreesRotation -- 旋转角度(默认0度)
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::BlitBitmap( CDC *pDC, CRect rect, UINT uResourceID, COLORREF crMask, double dDegreesRotation)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	const double PI = (22.00 / 7.00);

	CSize	ImageSize;
	COLORREF crPixel;

	CBitmap bmpImage;

	HANDLE hBitmap = ::LoadImage(	AfxGetInstanceHandle(),
									MAKEINTRESOURCE (uResourceID), 
									IMAGE_BITMAP, 
									0, 
									0, 
									LR_CREATEDIBSECTION);

	ASSERT(hBitmap); 

	bmpImage.Attach(hBitmap);
	
	BITMAP bmpInfo;
	bmpImage.GetBitmap(&bmpInfo);
	ImageSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);

	CDC memDC;
    memDC.CreateCompatibleDC(pDC);
    memDC.SelectObject(&bmpImage);

	const int nWidth		= ImageSize.cx;
	const int nHeight		= ImageSize.cy;
    CPoint ptAxle			= CPoint(nWidth / 2, nHeight / 2);
    CPoint ptOffset			= CPoint((rect.Width() - nWidth) / 2, (rect.Height() - nHeight) / 2);


	if ( dDegreesRotation > 360.0 )
		dDegreesRotation = (int)dDegreesRotation % 360;

	double dRadians = ( (PI/180) * dDegreesRotation );

    for (int x1 = 0; x1 < nWidth; x1++) 
	{
        for (int y1 = 0; y1 < nHeight; y1++) 
		{
			crPixel = memDC.GetPixel(x1, y1);

            int x2 = rect.left + ptOffset.x + (int)(ptAxle.x + ((x1 - ptAxle.x) * sin(dRadians)) + ((y1 - ptAxle.y) * cos(dRadians)));
            int y2 = rect.top + ptOffset.y + (int)(ptAxle.y + ((y1 - ptAxle.y) * sin(dRadians)) - ((x1 - ptAxle.x) * cos(dRadians)));
            
			if (crPixel != crMask) 
			{
                pDC->SetPixel( x2,		y2, crPixel );
                pDC->SetPixel( x2 + 1,	y2, crPixel );
            }
        }
    }

}

// ---------------------------------------------------------
//	名称: ReplaceColor
//	功能: 将指定位图hBmp中的颜色cOldColor转换为新的颜色cNewColor
//	参数: hBmp -- 要转变颜色的位图，cOldColor -- 旧色，及要转变的颜色
//		  cNewColor -- 转变后的颜色, hBmpDC -- hBmp的DC(默认为空)
//	返回: 成功 -- 转变后的位图，失败 -- 空位图
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
#define COLORREF2RGB(Color) (Color & 0xff00) | ((Color >> 16) & 0xff) | ((Color << 16) & 0xff0000)
HBITMAP CAdvertise::CAdvertiseImpl::ReplaceColor(HBITMAP hBmp,COLORREF cOldColor,COLORREF cNewColor,HDC hBmpDC)
{
	HBITMAP RetBmp=NULL;
	if (hBmp)
	{	
		HDC BufferDC=CreateCompatibleDC(NULL);		// 源位图DC
		if (BufferDC)
		{
			HBITMAP hTmpBitmap = (HBITMAP) NULL;
			if (hBmpDC)
				if (hBmp == (HBITMAP)GetCurrentObject(hBmpDC, OBJ_BITMAP))
				{
					hTmpBitmap = CreateBitmap(1, 1, 1, 1, NULL);
					SelectObject(hBmpDC, hTmpBitmap);
				}
			HGDIOBJ PreviousBufferObject=SelectObject(BufferDC,hBmp);
		
			HDC DirectDC=CreateCompatibleDC(NULL);	// 进行转换的DC		
			if (DirectDC)
			{
				// 获取位图长、宽
				BITMAP bm;
				GetObject(hBmp, sizeof(bm), &bm);
				
				// 创建位图信息，以求最小量初始化涵数CreateDIBSection()所需参数
				BITMAPINFO RGB32BitsBITMAPINFO; 
				ZeroMemory(&RGB32BitsBITMAPINFO,sizeof(BITMAPINFO));
				RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
				RGB32BitsBITMAPINFO.bmiHeader.biWidth=bm.bmWidth;
				RGB32BitsBITMAPINFO.bmiHeader.biHeight=bm.bmHeight;
				RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
				RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;
				UINT * ptPixels;					// 指向位图像素值的指针

				HBITMAP DirectBitmap= CreateDIBSection(DirectDC, (BITMAPINFO *)&RGB32BitsBITMAPINFO, DIB_RGB_COLORS,(void **)&ptPixels, NULL, 0);
				if (DirectBitmap)
				{
					HGDIOBJ PreviousObject=SelectObject(DirectDC, DirectBitmap);
					BitBlt(DirectDC,0,0,bm.bmWidth,bm.bmHeight,BufferDC,0,0,SRCCOPY);					

					// 转换COLORREF为RGB值(及翻转红、蓝两色值)
					cOldColor=COLORREF2RGB(cOldColor);
					cNewColor=COLORREF2RGB(cNewColor);

					// 处理旧、新颜色转换
					for (int i=((bm.bmWidth*bm.bmHeight)-1);i>=0;i--)
					{
						if (ptPixels[i]==cOldColor) ptPixels[i]=cNewColor;
					}
					// 恢复源设置
					SelectObject(DirectDC,PreviousObject);
					
					// 转换后的位图
					RetBmp=DirectBitmap;
				}
				// 清扫工作
				DeleteDC(DirectDC);
			}			
			if (hTmpBitmap)
			{
				SelectObject(hBmpDC, hBmp);
				DeleteObject(hTmpBitmap);
			}
			SelectObject(BufferDC,PreviousBufferObject);
		
			DeleteDC(BufferDC);
		}
	}
	return RetBmp;
}

// ---------------------------------------------------------
//	名称: DrawElectricity
//	功能: 绘制闪电效果
//	参数: pDC -- 相关DC，rect -- 绘制范围
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawElectricity(CDC *pDC, CRect rect)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	static int on1,on2,oon1,oon2;

	CRgn ClipRegion;

	ClipRegion.CreateRectRgnIndirect(&rect);

	pDC->SelectClipRgn(&ClipRegion);
	
	int nPivot = rect.Height();

	int n1,n2;
	n1 = GetRandomRange(1,nPivot/2);
	n2 = GetRandomRange(1,nPivot/2);
//	n1 = rand()*nPivot/RAND_MAX-nPivot/2;
//	n2 = rand()*nPivot/RAND_MAX-nPivot/2;

//	int nRandElement = rand()*10/RAND_MAX;

//	if(nRandElement < 2)
//		n1 = rand()*(nPivot*2)/RAND_MAX-nPivot;
//	else if(rand() < RAND_MAX/10)
//		n2 = rand()*(nPivot*2)/RAND_MAX-nPivot;

	CPoint TopRight		( rect.right,	rect.top	);
	CPoint TopLeft		( rect.left,	rect.top	);
	CPoint BottomLeft	( rect.left,	rect.bottom );
	CPoint BottomRight	( rect.right,	rect.bottom );
	
	int	nMidHorizontal	= rect.left + ((rect.right-rect.left)/2);
	int	nMidVertical	= rect.top + ((rect.bottom-rect.top)/2);

	// 右上到左下
	CPoint points1[4] = {
		TopRight,
		CPoint( nMidHorizontal+n1,	nMidVertical	),
		CPoint( nMidHorizontal,		nMidVertical-n1	),
		BottomLeft
	};

	//	左上到右下
	CPoint points2[4] = {
		TopLeft,
		CPoint( nMidHorizontal-n2,nMidVertical ),
		CPoint( nMidHorizontal,nMidVertical-n2 ),
		BottomRight
	};

	//	右上到左下
	CPoint opoints1[4] = {
		TopRight,
		CPoint( nMidHorizontal+on1,nMidVertical ),
		CPoint( nMidHorizontal,nMidVertical-on1 ),
		BottomLeft
	};

	//	左上到右下
	CPoint opoints2[4] = {
		TopLeft,
		CPoint( nMidHorizontal-on2,nMidVertical ),
		CPoint( nMidHorizontal,nMidVertical-on2 ),
		BottomRight
	};

	//	右上到左下
	CPoint oopoints1[4] = {
		TopRight,
		CPoint( nMidHorizontal+oon1,nMidVertical ),
		CPoint( nMidHorizontal,nMidVertical-oon1 ),
		BottomLeft
	};

	//	左上到右下
	CPoint oopoints2[4] = {
		TopLeft,
		CPoint( nMidHorizontal-oon2,nMidVertical ),
		CPoint( nMidHorizontal,nMidVertical-oon2 ),
		BottomRight
	};

	CPen wpen(	PS_SOLID,1,RGB(150,220,255));
	CPen pen(	PS_SOLID,2,RGB(50,100,255));
	CPen open(	PS_SOLID,1,RGB(50,100,255));
	CPen oopen(	PS_SOLID,1,RGB(0,30,150));
	
	CPen *pOldPen = pDC->SelectObject(&oopen);
	pDC->PolyBezier(oopoints1,4);
	pDC->PolyBezier(oopoints2,4);

	pDC->SelectObject(&open);
	pDC->PolyBezier(opoints1,4);
	pDC->PolyBezier(opoints2,4);
	
	pDC->SelectObject(&pen);
	pDC->PolyBezier(points1,4);
	pDC->PolyBezier(points2,4);
	
	pDC->SelectObject(&wpen);
	pDC->PolyBezier(points1,4);
	pDC->PolyBezier(points2,4);
	
	pDC->SelectObject(pOldPen);

	oon1 = on1;
	oon2 = on2;
	on1 = n1;
	on2 = n2;

	pDC->SelectClipRgn( NULL );
	ClipRegion.DeleteObject();
}

// ---------------------------------------------------------
//	名称: DrawSnow
//	功能: 绘制雪花效果
//	参数: pDC -- 相关DC
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawSnow(CDC *pDC)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	int nFunFactor = 100;
	int x, y, z;
	for(int i = 0; i < NUM_SNOWS; ++i)
	{
        m_StarArray[i].z = m_StarArray[i].z - 30;		// 这里30为雪花移动速度
        if (m_StarArray[i].z > 255)
		{
			m_StarArray[i].z = -255;
		}
        if (m_StarArray[i].z < -255)
		{
			m_StarArray[i].z = 255;
		}
        
		z = m_StarArray[i].z + 256;
        x = (m_StarArray[i].x * nFunFactor / z) + (m_bmpWidth / 2);
        y = (m_StarArray[i].y * nFunFactor / z) + (m_bmpHeight / 2);
		
		CPen Pen;

		// 创建笔的颜色以z为参照
		int nColor = 255 - m_StarArray[i].z;
		Pen.CreatePen(PS_COSMETIC, 1, RGB(nColor,nColor,nColor));

		CPen *pOldPen = (CPen *)pDC->SelectObject(&Pen);
		// 绘制雪花
		pDC->Ellipse(CRect(x, y, x+3, y+3));
		pDC->SelectObject(pOldPen);
    }
}

// ---------------------------------------------------------
//	名称: LayoutScreenSaver
//	功能: 绘制屏保文字效果
//	参数: dc -- 相关DC
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::LayoutScreenSaver(CDC& dc)
{
	int nItems = m_ae.RunFrame(dc);

	if ( nItems < 3 )
	{
		struct DisconnectItem { LPCTSTR pszTitle; LPCTSTR pszComment; LPCTSTR pszNote; }
		aDI[] =
		{
				"未来工作室(Future Studio)",
				"徐景周...", "xujingzhou2016@gmail.com",
				"特效窗体",
				"涟漪、火焰、溶浆、爆炸...", "等等",
				"动态菲屏",
				"具有多种图形动画...", "易用",
				"精灵修订版",
				"内部作了一些优化...", "程序结构大体没变",
				NULL, NULL, NULL
		};
		
		// 居中显示屏保主标题
		CPoint ptIntro = m_ae.RandPt( 100 );
		POSITION pos = m_ae.CreateItem( "特效窗体显示效果" )
			.SetStart(ptIntro.x, ptIntro.y - 70)
			.SetTextColor(RGB(m_ae.Rand(256),m_ae.Rand(256),m_ae.Rand(256)))
			.SetFont("12,宋体,B")
			.SetPoint(m_ae.HCenter(), m_ae.VCenter())
			.AlignCenter()
			.FadeOut()
			.SetFrames(40)
			.GetPos();
		
		// 随机显示笑脸位图
		if ( m_ae.Rand(20) < 20 )						// 一半显示的机会
		{
			int nEmots = m_ae.Rand(m_ae.Height()/10) + 3;
			for ( int nEmot=0; nEmot<nEmots; ++nEmot )
			{
				POSITION posEmot = m_ae.CreateItem( "" ).GoAfter( pos )
					.SetBitmap( IDB_SMILEFACE,-1 )       // 只有一个(不用分割)，故为-1
					.SetStart( m_ae.RandPt() )
					.SetFinish( m_ae.RandPt() )
					.SetFrames( 100 )
					.GetPos();
				m_ae.CreateItemAfter( posEmot ).SetFrames( 15 ).SetFinish( m_ae.RandPt() );
			}
		}

		// 随机显示屏保文字
		for ( int nDI=0; aDI[nDI].pszTitle; ++nDI )
		{
			int nY = m_ae.Rand( max( 0, m_ae.Height() - 40 ) );
			int nX = m_ae.Rand( m_ae.Width() / 2 );
			POSITION posSubj = m_ae.CreateItem( aDI[nDI].pszTitle )
				.GoAfter( pos )
				.SetTextColor(RGB(m_ae.Rand(256),0,0))
				.SetFont("12,宋体,B")
				.SetStart( m_ae.RandPt() )
				.SetFinish( nX, nY )
				.SetFrames( 40 )
				.GetPos();
			POSITION posSubj2 =	m_ae.CreateItemAfter( posSubj ).SetFrames( 60 ).GetPos();
			m_ae.CreateItemAfter( posSubj2 ).SetFrames( 30 ).SetFinish( nX, 0 ).FadeOut();
			nY += 20;
			
			POSITION posName = m_ae.CreateItem( aDI[nDI].pszComment )
				.GoAfter( pos )
				.SetTextColor(RGB(0,m_ae.Rand(256),0))
				.SetStart( m_ae.RandPt() )
				.SetFinish( nX, nY )
				.SetFrames( 40 )
				.SetFont("11,楷体_GB2312")
				.SetTextColor( RGB(255,153,0) )
				.GetPos();
			POSITION posName2 =	m_ae.CreateItemAfter( posName ).SetFrames( 60 ).GetPos();
			m_ae.CreateItemAfter( posName2 ).SetFrames( 30 ).FadeOut();
			nY += 20;
			
			POSITION posAuth = m_ae.CreateItem( aDI[nDI].pszNote )
				.GoAfter( pos )
				.SetTextColor(RGB(0,0,m_ae.Rand(256)))
				.SetStart( m_ae.RandPt() )
				.SetFinish( nX, nY )
				.SetFrames( 40 )
				.SetFont("11,楷体_GB2312")
				.GetPos();
			POSITION posAuth2 =	m_ae.CreateItemAfter( posAuth ).SetFrames( 60 ).GetPos();
			m_ae.CreateItemAfter( posAuth2 ).SetFrames( 30 ).FadeOut();
			
			pos = posAuth2;
		}
	}
}

// 匿名名字空间，可存放全局成员(变量或涵数)
namespace
{
	int	m_xPos   = 0;			// 初始x位置
	int m_yPos   = 0;			// 初始y位置
	int	m_xAngle = 90;			// 初始徽标x方向旋转角度
	int	m_yAngle = 60;			// 初始徽标y方向旋转角度
	int	m_nSpeed = 10;			// 初始速度
}
// ---------------------------------------------------------
//	名称: DrawLogo
//	功能: 绘制动态徽标效果
//	参数: pDC -- 相关DC
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawLogo(CDC *pDC)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

// ------------ 以下部分为不透明显示带角度徽标位图 ------------ //
	// 选取徽标位图到LogoDC中
	CBitmap bmpImage;
	bmpImage.LoadBitmap(IDB_LOGO);
	BITMAP bm;
	bmpImage.GetObject(sizeof(BITMAP), &bm);
	int m_cxLogo  = bm.bmWidth;
	int m_cyLogo  = bm.bmHeight;
	CDC LogoDC,MemDC;
    LogoDC.CreateCompatibleDC(pDC);
    LogoDC.SelectObject(&bmpImage);

	// 移动位置越界处理
	m_xPos = m_xPos + 2;
	if(m_xPos > m_bmpWidth)
	{	
		m_xPos = -m_cxLogo / 2;
	}

	m_yPos = m_yPos + 1;
	if(m_yPos > m_bmpHeight)
	{
		m_yPos = -m_cyLogo;
	}
	
	for(int i = 1; i < m_cxLogo; ++i)
	{
		// 绘制具有角度效果的徽标(注：其中0.0174= 3.14159/180为平均弧度值)
		pDC->BitBlt((int)(cos((m_xAngle + i)* 0.0174) * (m_cxLogo / 4.25) + m_xPos), 
								 (int)(sin((m_yAngle + i)* 0.0174) * 10 + 2.5 + m_yPos), 
								 1, m_cyLogo, &LogoDC, i, 0, SRCAND);
	}

	m_xAngle = m_xAngle + (int)(m_nSpeed * 0.5);	// 徽标x方向旋转角度
	m_yAngle = m_yAngle + m_nSpeed ;				// 徽标y方向旋转角度
       
	// 旋转角度越界处理
	if(m_xAngle >= 360)
		m_xAngle = 0;	
	else if(m_xAngle <= -180) 
		m_nSpeed = m_nSpeed * -1;
	else if(m_yAngle >= 360)
		m_yAngle = 0;

	// 清除工作
	if (LogoDC != NULL)
		LogoDC.DeleteDC();
	bmpImage.DeleteObject();
// ------------ 不透明显示带角度徽标位图部分结束 ------------ //


/*
// ------------ 以下部分为透明显示带角度徽标位图 ------------ //
	// 选取徽标位图到LogoDC中
	CBitmap bmpImage;
	bmpImage.LoadBitmap(IDB_LOGO1);
	BITMAP bm;
	bmpImage.GetObject(sizeof(BITMAP), &bm);
	int m_cxLogo  = bm.bmWidth;
	int m_cyLogo  = bm.bmHeight;

	// 移动位置越界处理
	m_xPos = m_xPos + 2;
	if(m_xPos > m_bmpWidth)
	{	
		m_xPos = -m_cxLogo / 2;
	}

	m_yPos = m_yPos + 1;
	if(m_yPos > m_bmpHeight)
	{
		m_yPos = -m_cyLogo;
	}

	// 创建透明位图(利用三张内存位图实现:背景图、透明显示图和二色图)
	COLORREF crOldBack = pDC->SetBkColor(RGB(255,255,255));
	COLORREF crOldText = pDC->SetTextColor(RGB(0,0,0));
	CDC dcImage,dcTrans,dcBack;
	
	// 创建透明显示位图dcImage，转换二色位图dcTrans以及背景位图dcBack的兼容DC
	dcImage.CreateCompatibleDC(pDC);
	dcTrans.CreateCompatibleDC(pDC);
	dcBack.CreateCompatibleDC(pDC);

	// 选入要透明显示的位图
	CBitmap* pOldBitmapImage = dcImage.SelectObject(&bmpImage);

	CBitmap bitmapTrans;
	int nWidth	= bm.bmWidth;
	int nHeight = bm.bmHeight;

	// 创建当前背景兼容位图，并选入dcBack
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC,nWidth,nHeight);
	CBitmap* pOldBitmapBack = dcBack.SelectObject(&bmp);
	// 创建二色位图，并选入dcTrans
	bitmapTrans.CreateBitmap(nWidth,nHeight,1,1,NULL);	
	CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);
	// 屏蔽色为黑色
	dcImage.SetBkColor(RGB(0,0,0));

	// 循环每一列逐次透明处理并带角度效果绘制
	for(int i = 0; i < m_cxLogo; ++i)
	{
		int left = (int)(cos((m_xAngle + i)* 0.0174) * (m_cxLogo / 4.25) + m_xPos);
		int top	 = (int)(sin((m_yAngle + i)* 0.0174) * 10 + 2.5 + m_yPos);
		// 创建用于透明变换的二色位图(透明部分用1表示，不透明部分用0表示)
		dcTrans.BitBlt(i,0,1,m_cyLogo,&dcImage,i,0,SRCCOPY);
		// 将当前屏幕内容复制到dcBack中
		dcBack.BitBlt(i,0,1,m_cyLogo,pDC,left,top,SRCCOPY);
		// 将dcBack中内容与要透明显示目标位图进行异或操作
		dcBack.BitBlt(i,0,1,m_cyLogo,&dcImage,i,0,SRCINVERT);
		// 将二色位图与dcBack中内容进行与操作
		dcBack.BitBlt(i,0,1,m_cyLogo,&dcTrans,i,0,SRCAND);
		// 再将dcBack中内容与要透明显示目标位图进行异或操作
		dcBack.BitBlt(i,0,1,m_cyLogo,&dcImage,i,0,SRCINVERT);
		
		// 绘制透明具有角度效果的徽标(注：其中0.0174= 3.14159/180为平均弧度值)
		pDC->BitBlt(left, 
					top, 
					1, m_cyLogo, &dcBack, i, 0, SRCAND);
	}	

	m_xAngle = m_xAngle + (int)(m_nSpeed * 0.5);	// 徽标x方向旋转角度
	m_yAngle = m_yAngle + m_nSpeed ;				// 徽标y方向旋转角度
       
	// 旋转角度越界处理
	if(m_xAngle >= 360)
		m_xAngle = 0;	
	else if(m_xAngle <= -180) 
		m_nSpeed = m_nSpeed * -1;
	else if(m_yAngle >= 360)
		m_yAngle = 0;

	// 还原设置
	dcImage.SelectObject(pOldBitmapImage);
	dcTrans.SelectObject(pOldBitmapTrans);
	dcBack.SelectObject(pOldBitmapBack);
	pDC->SetBkColor(crOldBack);
	pDC->SetTextColor(crOldText);

	// 清除工作
	bmpImage.DeleteObject();
// ------------ 透明显示带角度徽标位图部分结束 ------------ //
*/
}

// ---------------------------------------------------------
//	名称: DrawAsciiAnimation
//	功能: 绘制动态烟花效果
//	参数: pDC -- 相关DC
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
void CAdvertise::CAdvertiseImpl::DrawAsciiAnimation(CDC* pDC)
{
	ASSERT(FALSE == IsBadReadPtr(pDC,sizeof(CDC*)));

	// 绿色烟花
	m_AsciiAnimation.SetForeColor(0,0,0);
	m_AsciiAnimation.Circle(pDC,m_Green.x, m_Green.y, m_Green.radius, m_Green.radius*5, m_Green.nAsciiCode, TRUE);
	
	m_AsciiAnimation.SetForeColor(0,255,0);
	m_Green.Calculate();
	m_AsciiAnimation.Circle(pDC,m_Green.x, m_Green.y, m_Green.radius, m_Green.radius*5, m_Green.nAsciiCode, FALSE);
	
	// 粉色烟花
	m_AsciiAnimation.SetForeColor(0,0,0);
	m_AsciiAnimation.Circle(pDC,m_Purple.x, m_Purple.y, m_Purple.radius, m_Purple.radius*5, m_Purple.nAsciiCode, TRUE);
	
	m_AsciiAnimation.SetForeColor(255,0,255);
	m_Purple.Calculate();
	m_AsciiAnimation.Circle(pDC,m_Purple.x, m_Purple.y, m_Purple.radius, m_Purple.radius*5, m_Purple.nAsciiCode, FALSE);
	
	// 橙色烟花
	m_AsciiAnimation.SetForeColor(0,0,0);
	m_AsciiAnimation.Circle(pDC,m_Orange.x, m_Orange.y, m_Orange.radius, m_Orange.radius*5, m_Orange.nAsciiCode, TRUE);
	
	m_AsciiAnimation.SetForeColor(255,200,0);
	m_Orange.Calculate();
	m_AsciiAnimation.Circle(pDC,m_Orange.x, m_Orange.y, m_Orange.radius, m_Orange.radius*5, m_Orange.nAsciiCode, FALSE);
}

