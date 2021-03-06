/////////////////////////////////////////////////////////////////////////
//类名：CPlasmaRoutine
//功能：熔浆效果
//修改：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2002.1.8
////////////////////////////////////////////////////////////////////////
#include "stdafx.h"				// 加入预编译头文件,jingzhou xu
#include "PlasmaRoutine.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------
//	名称: CPlasmaRoutine
//	功能: 构造涵数，初始化工作
//	参数: 无
//	返回: 无
//	修改: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CPlasmaRoutine::CPlasmaRoutine()
{
	m_iHeight = 0;
	m_iWidth  = 0;
	m_pPlasmaBits = NULL;
	m_iAlpha = 255;

	m_a1=0;
	m_a2=0;
	m_a3=0;
	m_a4=0;
	m_b1=0;
	m_b2=0;
	m_b3=0;
	m_b4=0;

	m_iModifier1=1;
	m_iModifier2=2;
	m_iModifier3=1;
	m_iModifier4=2;

	m_iXModifier1 = -1;
	m_iXModifier2 = 3;

	m_iYModifier1 = 1;
	m_iYModifier2 = -2;

	// 初始化熔浆颜色
	m_PlasmaColors[0]=RGB(0,0,0);		// 从黑
	m_PlasmaColors[1]=RGB(0,0,255);		// 到蓝

	m_PlasmaColors[2]=RGB(0,0,255);		// 从蓝
	m_PlasmaColors[3]=RGB(0,255,0);		// 到绿

	m_PlasmaColors[4]=RGB(0,255,0);		// 从绿
	m_PlasmaColors[5]=RGB(0,255,255);	// 到青

	m_PlasmaColors[6]=RGB(0,255,255);	// 青
	m_PlasmaColors[7]=RGB(0,255,255); 

	m_PlasmaColors[8]=RGB(0,255,255);	// 青
	m_PlasmaColors[9]=RGB(0,255,255); 

	m_PlasmaColors[10]=RGB(0,255,255);	// 从青
	m_PlasmaColors[11]=RGB(0,255,0);	// 到绿

	m_PlasmaColors[12]=RGB(0,255,0);	// 到绿
	m_PlasmaColors[13]=RGB(0,0,255);	// 到蓝

	m_PlasmaColors[14]=RGB(0,0,255);	// 到蓝
	m_PlasmaColors[15]=RGB(0,0,0);		// 到黑

}

// ---------------------------------------------------------
//	名称: ~CPlasmaRoutine
//	功能: 析构涵数，清扫工作
//	参数: 无
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
CPlasmaRoutine::~CPlasmaRoutine()
{
	if(m_pPlasmaBits != NULL)
		delete [] m_pPlasmaBits;

	m_pPlasmaBits = NULL;
}

// ---------------------------------------------------------
//	名称: InitCostBLTable
//	功能: 初始化获取余弦各角度值
//	参数: 无
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::InitCostBLTable()
{
  for(int t=0;t<256;t++)
        m_icostbl[t] = (int)( 30 * cos(t * (3.14159/64) ) );
}

// ---------------------------------------------------------
//	名称: SetRGB
//	功能: 设置指定位置颜色值
//	参数: 无
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
inline void CPlasmaRoutine::SetRGB(int iIndex,int R,int G,int B)
{
	COLORREF color = RGB(R+50,G+50,B+50);
	m_pPalletteBuffer[iIndex] = color;	

}

// ---------------------------------------------------------
//	名称: InitPallette
//	功能: 初始化调色板颜色值
//	参数: 无
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::InitPallette()
{
	// 创建熔浆渐变色
	CreateGradient(m_PlasmaColors[0],m_PlasmaColors[1],32,&m_pPalletteBuffer[0]);    // 从黑到蓝
	CreateGradient(m_PlasmaColors[2],m_PlasmaColors[3],32,&m_pPalletteBuffer[32]);   // 从绿到蓝
	CreateGradient(m_PlasmaColors[4],m_PlasmaColors[5],32,&m_pPalletteBuffer[64]);   // 从绿到白
	CreateGradient(m_PlasmaColors[6],m_PlasmaColors[6],32,&m_pPalletteBuffer[96]);   // 青
	CreateGradient(m_PlasmaColors[8],m_PlasmaColors[9],32,&m_pPalletteBuffer[128]);  // 青
	CreateGradient(m_PlasmaColors[10],m_PlasmaColors[11],32,&m_pPalletteBuffer[160]);// 从白到绿
	CreateGradient(m_PlasmaColors[12],m_PlasmaColors[13],32,&m_pPalletteBuffer[192]);// 从绿到蓝
	CreateGradient(m_PlasmaColors[14],m_PlasmaColors[15],32,&m_pPalletteBuffer[224]);// 从蓝到黑
			
}

// ---------------------------------------------------------
//	名称: CreateGradient
//	功能: 创建调色板渐变色
//	参数: clrStart、clrEnd -- 起始、终止色，lSteps -- 步长
//		  pBuffer -- 存放处理好颜色值数组
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
inline void CPlasmaRoutine::CreateGradient(COLORREF clrStart,COLORREF clrEnd,long lSteps,COLORREF* pBuffer)
{

	int r, g, b;							
	int rTotal,gTotal,bTotal;
	int roffset,goffset,boffset;
	int scalerR;
	int scalerG;
	int scalerB;


	roffset = goffset = boffset = 0;
	scalerR = scalerG = scalerB = 0;

	// 获取不同颜色比例
	rTotal = GetRValue(clrEnd) - GetRValue(clrStart);
	if(rTotal < 0)
		scalerR = -1;
	else if(rTotal > 0)
		scalerR = 1;	

	gTotal = GetGValue(clrEnd) - GetGValue(clrStart);
	if(gTotal < 0)
		scalerG = -1;
	else if(gTotal > 0)
		scalerG = 1;

	bTotal = GetBValue(clrEnd) - GetBValue(clrStart);

	if(bTotal < 0)
		scalerB = -1;
	else if(bTotal > 0)
		scalerB = 1;

	// 取正
	rTotal=abs(rTotal);
	gTotal=abs(gTotal);
	bTotal=abs(bTotal);

	// 获取初始颜色值
	r = GetRValue(clrStart);
	g = GetGValue(clrStart);
	b = GetBValue(clrStart);

	for(int i=0;i<lSteps;i++)
	{
		roffset = ::MulDiv(i, rTotal, lSteps);
		goffset = ::MulDiv(i, gTotal, lSteps);
		boffset = ::MulDiv(i, bTotal, lSteps);

		roffset*=scalerR;
		goffset*=scalerG;
		boffset*=scalerB;

		COLORREF color = RGB( (b+boffset),(g+goffset),(r+roffset));

		pBuffer[i] = color;

	}

}

// ---------------------------------------------------------
//	名称: CalcPlasma
//	功能: 溶浆数据处理
//	参数: 无
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::CalcPlasma()
{
    m_a1 = m_b1;
    m_a2 = m_b2;
	unsigned char  *tscr = (unsigned char*)&m_pPlasmaBits[0];

    for(long y=0;y<m_iHeight;y++)
    {
	
        m_a3 = m_b3;
        m_a4 = m_b4;

        for(long x=0;x<m_iWidth;x++)
        {
            *tscr++ = m_icostbl[m_a1] +
                      m_icostbl[m_a2] +
                      m_icostbl[m_a3] +
                      m_icostbl[m_a4] ;

            // 取值越高速度会变慢
            m_a3 += m_iModifier1;//4;
            m_a4 += m_iModifier2;//1;
        }

        // 取值越高速度会变慢
        m_a1 += m_iModifier3;//1;
        m_a2 += m_iModifier4;//4;

    }

	m_b1 += m_iYModifier1;//y modifier 1
	m_b2 += m_iYModifier2;//y modifier 2
	m_b3 += m_iXModifier1;//x modifier 1
	m_b4 += m_iXModifier2;//x modifier 2

}

// ---------------------------------------------------------
//	名称: Create
//	功能: 初始化溶浆效果所需数据，需预先调用
//	参数: iWidth -- 目标位图宽度,iHeight -- 目标位图高度
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::Create(int iWidth,int iHeight)
{
	if(m_pPlasmaBits != NULL)
		delete [] m_pPlasmaBits;
	
	m_pPlasmaBits = new BYTE[(iWidth*iHeight)];
	m_iHeight = iHeight;
	m_iWidth  = iWidth;

	// 清空溶浆数组
	memset(m_pPlasmaBits,0,(iWidth*iHeight));
	// 初始化余弦角度值表
	InitCostBLTable();
	// 初始化调色板
	InitPallette();
	// 计算熔浆
	CalcPlasma();
}

// ---------------------------------------------------------
//	名称: Render
//	功能: 溶浆数据处理，并将处理后数据放入目标位图中
//	参数: pBits -- 目标位图指针，iwidth,iheight -- 位图宽度、高度
//		  iLineLength -- 位图每行长度
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::Render(DWORD* pBits,int iwidth,int iheight,int iLineLength)
{
	// 将计算好的值放入参数pBits中
	unsigned char* pSrcBitlin;// = m_pFireBits+(m_iWidth*3);
	BYTE *dst;//=(BYTE*)Dib->pVideoMemory;
	COLORREF Color;
	BYTE r;
	BYTE g;
	BYTE b;

	for(int i=0;i<m_iHeight;i++)
	{
		if(i <= iheight)
		{
			dst = (BYTE*)&pBits[(iLineLength*i)];
			pSrcBitlin =&m_pPlasmaBits[m_iWidth*i];

			for(int x=0;x<m_iWidth;x++)
			{
				if(x <= iLineLength)
				{
					Color =m_pPalletteBuffer[pSrcBitlin[x]];

					r = GetRValue(Color);
					g = GetGValue(Color);
					b = GetBValue(Color);

					dst[0]=(BYTE)(((r-dst[0])*m_iAlpha+(dst[0]<<8))>>8);
					dst[1]=(BYTE)(((g-dst[1])*m_iAlpha+(dst[1]<<8))>>8);
					dst[2]=(BYTE)(((b-dst[2])*m_iAlpha+(dst[2]<<8))>>8);	
					dst+=4;
				}
			}
		}
	}
	CalcPlasma();

}

// ---------------------------------------------------------
//	名称: SetDefaultValues
//	功能: 设置默认值(未用)
//	参数: pExtParms -- VARIANT参数溶浆结构
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::SetDefaultValues(VARIANT* pExtParms)
{
	m_iAlpha = 255;
	pExtParms[2].intVal =m_iAlpha;

	m_iModifier1=1;
	m_iModifier2=2;
	m_iModifier3=1;
	m_iModifier4=2;

	pExtParms[3].intVal = m_iModifier1;
	pExtParms[4].intVal = m_iModifier2;
	pExtParms[5].intVal = m_iModifier3;
	pExtParms[6].intVal = m_iModifier4;

	m_iXModifier1 = -1;
	m_iXModifier2 = 3;

	m_iYModifier1 = 1;
	m_iYModifier2 = -2;

	pExtParms[7].intVal = m_iXModifier1;
	pExtParms[8].intVal = m_iXModifier2;
	pExtParms[9].intVal = m_iYModifier1;
	pExtParms[10].intVal = m_iYModifier2;

	// 设置默认熔浆颜色
	m_PlasmaColors[0]=RGB(0,0,0);		// 从黑
	m_PlasmaColors[1]=RGB(0,0,255);		// 到蓝
	m_PlasmaColors[2]=RGB(0,0,255);		// 从蓝           
	m_PlasmaColors[3]=RGB(0,255,0);		// 到绿
	m_PlasmaColors[4]=RGB(0,255,0);		// 从绿
	m_PlasmaColors[5]=RGB(0,255,255);	// 到青
	m_PlasmaColors[6]=RGB(0,255,255);	// 青
	m_PlasmaColors[7]=RGB(0,255,255); 
	m_PlasmaColors[8]=RGB(0,255,255);	// 青
	m_PlasmaColors[9]=RGB(0,255,255); 
	m_PlasmaColors[10]=RGB(0,255,255);	// 从青
	m_PlasmaColors[11]=RGB(0,255,0);	// 到绿
	m_PlasmaColors[12]=RGB(0,255,0);	// 到绿
	m_PlasmaColors[13]=RGB(0,0,255);	// 到蓝
	m_PlasmaColors[14]=RGB(0,0,255);	// 到蓝
	m_PlasmaColors[15]=RGB(0,0,0);		// 到黑	


	pExtParms[11].ulVal = m_PlasmaColors[0];
	pExtParms[12].ulVal = m_PlasmaColors[1];
	pExtParms[13].ulVal = m_PlasmaColors[2];
	pExtParms[14].ulVal = m_PlasmaColors[3];
	pExtParms[15].ulVal = m_PlasmaColors[4];
	pExtParms[16].ulVal = m_PlasmaColors[5];
	pExtParms[17].ulVal = m_PlasmaColors[6];
	pExtParms[18].ulVal = m_PlasmaColors[7];
	pExtParms[19].ulVal = m_PlasmaColors[8];
	pExtParms[20].ulVal = m_PlasmaColors[9];
	pExtParms[21].ulVal = m_PlasmaColors[10];
	pExtParms[22].ulVal = m_PlasmaColors[11];
	pExtParms[23].ulVal = m_PlasmaColors[12];
	pExtParms[24].ulVal = m_PlasmaColors[13];
	pExtParms[25].ulVal = m_PlasmaColors[14];
	pExtParms[26].ulVal = m_PlasmaColors[15];

	InitPallette();
}

// ---------------------------------------------------------
//	名称: InitializePlasma
//	功能: 初始化溶浆数据(未用)
//	参数: pExtParms -- VARIANT溶浆结构
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CPlasmaRoutine::InitializePlasma(VARIANT* pExtParms)
{
	m_iAlpha = pExtParms[2].intVal;

	m_iModifier1 = pExtParms[3].intVal;
	m_iModifier2 = pExtParms[4].intVal;
	m_iModifier3 = pExtParms[5].intVal;
	m_iModifier4 = pExtParms[6].intVal;

	m_iXModifier1 = pExtParms[7].intVal;
	m_iXModifier2 = pExtParms[8].intVal;
	m_iYModifier1 = pExtParms[9].intVal;
	m_iYModifier2 = pExtParms[10].intVal;

	m_PlasmaColors[0]=pExtParms[11].ulVal;
	m_PlasmaColors[1]=pExtParms[12].ulVal;
	m_PlasmaColors[2]=pExtParms[13].ulVal;
	m_PlasmaColors[3]=pExtParms[14].ulVal;
	m_PlasmaColors[4]=pExtParms[15].ulVal;
	m_PlasmaColors[5]=pExtParms[16].ulVal;
	m_PlasmaColors[6]=pExtParms[17].ulVal;
	m_PlasmaColors[7]=pExtParms[18].ulVal;
	m_PlasmaColors[8]=pExtParms[19].ulVal;
	m_PlasmaColors[9]=pExtParms[20].ulVal;
	m_PlasmaColors[10]=pExtParms[21].ulVal;
	m_PlasmaColors[11]=pExtParms[22].ulVal;
	m_PlasmaColors[12]=pExtParms[23].ulVal;
	m_PlasmaColors[13]=pExtParms[24].ulVal;
	m_PlasmaColors[14]=pExtParms[25].ulVal;
	m_PlasmaColors[15]=pExtParms[26].ulVal;

	if(pExtParms[0].intVal != m_iWidth || pExtParms[1].intVal != m_iHeight)
		Create(pExtParms[0].intVal,pExtParms[1].intVal);
	else
		InitPallette();
}

