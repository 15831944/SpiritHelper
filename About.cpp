//////////////////////////////////////////////////
//类名：CAbout
//功能：关于对话框实现
//作者：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2001.12.1
//////////////////////////////////////////////////
#include "stdafx.h"
#include "helptip.h"
#include "About.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAbout dialog


CAbout::CAbout(CWnd* pParent /*=NULL*/)
	: CDialog(CAbout::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAbout)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAbout)
	DDX_Control(pDX, IDC_DYCREDITS, m_DyCredits);
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, ID_MORE, m_More);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAbout, CDialog)
	//{{AFX_MSG_MAP(CAbout)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_MORE, OnMore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------
//	名称: OnInitDialog
//	功能: 初始化关于对话框
//	变量: 无
//	返回: 成功返回TRUE，否则返回FALSE
//	编写: 徐景周(Johnny Xu)，2002.4.8
// ---------------------------------------------------------
BOOL CAbout::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//"关于"对话框动态显示效果设置
	CRect dlgRect; 
	GetWindowRect(dlgRect);  
	CRect desktopRect; 
	GetDesktopWindow()->GetWindowRect(desktopRect); 

	MoveWindow( 
		(desktopRect.Width() - dlgRect.Width()) / 2, 
		(desktopRect.Height() - dlgRect.Height()) / 2, 
		0, 
		0 ); 

	nWidth=dlgRect.Width(); 
	nHeight=dlgRect.Height(); 
	dx=8;											 // X方向偏移量 
	dy=8;											 // Y方向偏移量 
	::SetTimer(this->m_hWnd, 1,10 , NULL);			 //启动开始时动态对话框效果

	//"关于"对话框中对话框可收缩效果
	CRect Rect1,Rect2; 							     //对话框收缩时大小		
	
	GetDlgItem(IDC_DYCREDITS)->GetWindowRect(Rect1); 
	GetDlgItem(IDC_COPYRIGHT)->GetWindowRect(Rect2); 
	m_nReducedHeight = Rect1.Height()+(Rect1.top -Rect2.bottom)/2; //收缩后窗体高度
	dlgRect.bottom -= (Rect1.Height()+(Rect1.top -Rect2.bottom)/2); 
//	MoveWindow(&dlgRect);				              //如果要显示对话框起始动态效果的话，不能使用该句

	m_bVertical=false;                                //默认收缩对话框

	//"关于"对话框中荣誉显示设置
	TCHAR *pArrCredit = { 
		_T("精灵助手\t|"
        "1.1版||"
        "组织\r|未来工作室(Future Studio)||"
        "开发者\r"     
		"|徐景周|"
		"|EMail:\r"
		"|xujingzhou2016@gmail.com|"
		"|Github:\r"
		"|https://github.com/xujingzhou|"
		"|||贫交行 -- 杜甫\n"
		"||翻手为云覆手雨，纷纷轻薄何须数。"
		"||君不见管鲍贫时交，此道今人弃如土。"
		"||||")};
	
	m_DyCredits.SubclassDlgItem(IDD_ABOUT,this);
	m_DyCredits.SetCredits(pArrCredit,'|');
	m_DyCredits.SetSpeed(1);							//滚动速度(0-慢速，1-中速，2-快速)
	m_DyCredits.SetColor(BACKGROUND_COLOR, RGB(0,255,200));
	m_DyCredits.SetTransparent();
	m_DyCredits.SetGradient(GRADIENT_RIGHT_DARK);

	//初始化阴影位图按钮
    m_More.SetTextColor(RGB(0,0,0));
	m_More.SetToolTipText("更多信息");  
	m_More.SetShade(SHS_HBUMP,8,20,5,RGB(55,55,255));  //(SHS_HARDBUMP,10,20,5,RGB(55,55,255));

	m_OK.SetTextColor(RGB(0,0,0));
	m_OK.SetToolTipText("关闭");  
	m_OK.SetShade(SHS_HBUMP,8,20,5,RGB(55,55,255));  
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ---------------------------------------------------------
//	名称: OnTimer
//	功能: 显示对话框动态效果
//	变量: nIDEvent -- 计时器ID
//	返回: 无
//	编写: 徐景周(Johnny Xu)，2002.4.8
// ---------------------------------------------------------
void CAbout::OnTimer(UINT nIDEvent) 
{
	CRect dlgRect;
	GetWindowRect(dlgRect);

	CRect desktopRect;
	GetDesktopWindow()->GetWindowRect(desktopRect);

	if(nIDEvent == 1)              //开始动态伸展
	{	
		if(dlgRect.Width() >=nWidth) 
		{
			dx=0; // X偏移量置0
			dlgRect.right = dlgRect.left + nWidth;
		}
//		if(dlgRect.Height() >=nHeight)
//		{
//			dy=0; // Y偏移量置0
//			dlgRect.bottom = dlgRect.top + nHeight;
//      }

		MoveWindow(				   //高度不变,因为要隐藏荣誉显示框
		(-dx+desktopRect.Width() - dlgRect.Width()) / 2,
//		(-dy+desktopRect.Height() - dlgRect.Height()) / 2, 
		 nHeight-m_nReducedHeight,          
		 +dx+dlgRect.Width(),
//		 +dy+dlgRect.Height() );
		 nHeight-m_nReducedHeight); 
		
	}
	

	if((dlgRect.Width() >=nWidth)) //&& (dlgRect.Height() >=nHeight))
		::KillTimer(this->m_hWnd, 1); //关闭定时器

	CDialog::OnTimer(nIDEvent);
}

// ---------------------------------------------------------
//	名称: OnMore
//	功能: 是否荣誉显示
//	变量: 无
//	返回: 无
//	编写: 徐景周(Johnny Xu)，2002.4.8
// ---------------------------------------------------------
void CAbout::OnMore() 
{
	m_bVertical = !m_bVertical; 
	
	if(m_bVertical == FALSE)	//不显示
	{ 
		SetDlgItemText(ID_MORE,_T("更多>>"));

		SizeWindow(m_nReducedHeight,true);

//		m_DyCredits.EndScrolling();              //停止滚动
	} 
	else						//显示
	{ 
		SetDlgItemText(ID_MORE,_T("<<隐藏"));

		SizeWindow(m_nReducedHeight,false);

		m_DyCredits.StartScrolling();			//开始滚动
	} 
	
	UpdateWindow(); 
}

// ---------------------------------------------------------
//	名称: SizeWindow
//	功能: 伸展或收缩对话框    
//	变量: ReduceHeight-收缩高度，bExtend-是否伸展
//	返回: 无
//	编写: 徐景周(Johnny Xu)，2002.4.8
// ---------------------------------------------------------	
void CAbout::SizeWindow(int ReduceHeight, bool bExtend)
{
	CRect rc;
	GetWindowRect(&rc);
	if(bExtend)
	{
		for (int i= 0; i < ReduceHeight; i++)
		{
			rc.bottom--;
			MoveWindow(&rc);
		}
	}
	else
	{
		for (int i= 0; i < ReduceHeight; i++)
		{
			rc.bottom++;
			MoveWindow(&rc);
		}
	}
}

// ---------------------------------------------------------
//	名称: PreTranslateMessage
//	功能: 截获ESC和回车键，避免按下此键时关闭对话框
//	参数: pMsg -- 消息
//	返回: 成功 -- TRUE，失败 -- FALSE
//	编写: 徐景周(Johnny Xu)
//  组织: 未来工作室(Future Studio)
//	日期: 2002.1.8
// ---------------------------------------------------------
BOOL CAbout::PreTranslateMessage(MSG* pMsg) 
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