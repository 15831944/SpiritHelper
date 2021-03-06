/////////////////////////////////////////////////////////////////////////
//类名：CWaterRoutine
//功能：水纹效果
//修改：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2002.1.8
////////////////////////////////////////////////////////////////////////
#include "stdafx.h"				// 加入预编译头文件,jingzhou xu
#include "WaterRoutine.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// 取指定范围内的随机数
#define random( min, max ) (( rand() % (int)((( max ) + 1 ) - ( min ))) + ( min ))

// ---------------------------------------------------------
//	名称: CWaterRoutine
//	功能: 构造涵数，初始化工作
//	参数: 无
//	返回: 无
//	修改: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CWaterRoutine::CWaterRoutine()
{
	m_iHeightField1 = NULL;
	m_iHeightField2 = NULL;

	m_iWidth = 0;
	m_iHeight = 0;

	m_bDrawWithLight = TRUE;
	m_iLightModifier = 1;
	m_iHpage = 0;
	m_density = 5;
}

// ---------------------------------------------------------
//	名称: ~CWaterRoutine
//	功能: 析构涵数，清扫工作
//	参数: 无
//	返回: 无
//	修改: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CWaterRoutine::~CWaterRoutine()
{
	// 清除工作
	if(m_iHeightField1 != NULL)
		delete [] m_iHeightField1;
	if(m_iHeightField2 != NULL)
		delete [] m_iHeightField2;

	m_iHeightField1 = NULL;
	m_iHeightField2 = NULL;
}

// ---------------------------------------------------------
//	名称: Create
//	功能: 初始化水纹效果所需数据，需预先调用
//	参数: iWidth -- 目标位图宽度,iHeight -- 目标位图高度
//	返回: 无
//	修改: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::Create(int iWidth,int iHeight)
{
	if(m_iHeightField1 != NULL)
		delete [] m_iHeightField1;
	if(m_iHeightField2 != NULL)
		delete [] m_iHeightField2;

	// 创建高度字段
	m_iHeightField1 = new int[(iWidth*iHeight)];
	m_iHeightField2 = new int[(iWidth*iHeight)];

	// 清空高度字段
	memset(m_iHeightField1,0,(iWidth*iHeight)*sizeof(int));
	memset(m_iHeightField2,0,(iWidth*iHeight)*sizeof(int));

	m_iWidth = iWidth;
	m_iHeight = iHeight;

	// 设置当前页
	m_iHpage = 0;

}

// ---------------------------------------------------------
//	名称: FlattenWater
//	功能: 清空水纹效果数据
//	参数: 无
//	返回: 无
//	修改: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::FlattenWater()
{
	// 清空高度字段
	memset(m_iHeightField1,0,(m_iWidth*m_iHeight)*sizeof(int));
	memset(m_iHeightField2,0,(m_iWidth*m_iHeight)*sizeof(int));
}

// ---------------------------------------------------------
//	名称: Render
//	功能: 处理水纹效果并将处理好的数据放入目标位图中
//	参数: pSrcImage -- 源位图指针,pTargetImage -- 目标位图指针
//	返回: 无
//	修改: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::Render(DWORD* pSrcImage,DWORD* pTargetImage)
{
	// 创建水纹效果
	if(m_bDrawWithLight == FALSE)
	{
		DrawWaterNoLight(m_iHpage,pSrcImage,pTargetImage);
	}
	else
	{
		DrawWaterWithLight(m_iHpage,m_iLightModifier,pSrcImage,pTargetImage);
	}
	CalcWater(m_iHpage,m_density);

	// 两个高度字段间切换
	m_iHpage ^= 1;
}

// ---------------------------------------------------------
//	名称: CalcWater
//	功能: 两个效果高度字段数据间对照处理水纹(类似内部两个位图)
//	参数: npage -- 所在页面,density -- 密度
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::CalcWater(int npage, int density)
{
  int newh;
  int count = m_iWidth + 1;
  int *newptr;
  int *oldptr;

  // 设置指针
  if(npage == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }

  int x, y;

  for (y = (m_iHeight-1)*m_iWidth; count < y; count += 2)
  {
    for (x = count+m_iWidth-2; count < x; count++)
    {

	  // 一次处理周围8个像素(速度优化)
      newh          = ((oldptr[count + m_iWidth]
                      + oldptr[count - m_iWidth]
                      + oldptr[count + 1]
                      + oldptr[count - 1]
                      + oldptr[count - m_iWidth - 1]
                      + oldptr[count - m_iWidth + 1]
                      + oldptr[count + m_iWidth - 1]
                      + oldptr[count + m_iWidth + 1]
                       ) >> 2 )
                      - newptr[count];


      newptr[count] =  newh - (newh >> density);
/*
	  //  淤泥效果
      newh = (oldptr[count]<<2)
           +  oldptr[count-1-m_iWidth]
           +  oldptr[count+1-m_iWidth]
           +  oldptr[count-1+m_iWidth]
           +  oldptr[count+1+m_iWidth]
           + ((oldptr[count-1]
           +   oldptr[count+1]
           +   oldptr[count-m_iWidth]
           +   oldptr[count+m_iWidth])<<1);

      newptr[count] = (newh-(newh>>6)) >> density;
*/
    }
  }
}

// ---------------------------------------------------------
//	名称: SmoothWater
//	功能: 平滑处理水纹(两个效果高度字段处理)
//	参数: npage -- 所在页面
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::SmoothWater(int npage)
{
  int newh;
  int count = m_iWidth + 1;

  int *newptr;
  int *oldptr;

  // 设置指针
  if(npage == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }


  int x, y;

  for(y=1; y<m_iHeight-1; y++)
  {
    for(x=1; x<m_iWidth-1; x++)
    {
	  // 一次处理周围8个像素(速度优化)
      newh          = ((oldptr[count + m_iWidth]
                      + oldptr[count - m_iWidth]
                      + oldptr[count + 1]
                      + oldptr[count - 1]
                      + oldptr[count - m_iWidth - 1]
                      + oldptr[count - m_iWidth + 1]
                      + oldptr[count + m_iWidth - 1]
                      + oldptr[count + m_iWidth + 1]
                       ) >> 3 )
                      + newptr[count];


      newptr[count] =  newh>>1;
      count++;
    }
    count += 2;
  }
}

// ---------------------------------------------------------
//	名称: CalcWaterBigFilter
//	功能: 大波纹处理(未用)
//	参数: npage -- 所在页面,density -- 密度
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::CalcWaterBigFilter(int npage, int density)
{
  int newh;
  int count = (2*m_iWidth) + 2;

  int *newptr;
  int *oldptr;

  // 设置指针
  if(npage == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }

  int x, y;

  for(y=2; y<m_iHeight-2; y++)
  {
    for(x=2; x<m_iWidth-2; x++)
    {
	  // 一次处理周围25个像素(速度优化)
      newh        = (
                     (
                      (
                       (oldptr[count + m_iWidth]
                      + oldptr[count - m_iWidth]
                      + oldptr[count + 1]
                      + oldptr[count - 1]
                       )<<1)
                      + ((oldptr[count - m_iWidth - 1]
                      + oldptr[count - m_iWidth + 1]
                      + oldptr[count + m_iWidth - 1]
                      + oldptr[count + m_iWidth + 1]))
                      + ( (
                          oldptr[count - (m_iWidth*2)]
                        + oldptr[count + (m_iWidth*2)]
                        + oldptr[count - 2]
                        + oldptr[count + 2]
                        ) >> 1 )
                      + ( (
                          oldptr[count - (m_iWidth*2) - 1]
                        + oldptr[count - (m_iWidth*2) + 1]
                        + oldptr[count + (m_iWidth*2) - 1]
                        + oldptr[count + (m_iWidth*2) + 1]
                        + oldptr[count - 2 - m_iWidth]
                        + oldptr[count - 2 + m_iWidth]
                        + oldptr[count + 2 - m_iWidth]
                        + oldptr[count + 2 + m_iWidth]
                        ) >> 2 )
                     )
                    >> 3)
                    - (newptr[count]);


      newptr[count] =  newh - (newh >> density);
      count++;
    }
    count += 4;
  }
}

// ---------------------------------------------------------
//	名称: HeightBlob
//	功能: 环形涟漪效果处理
//	参数: x,y -- 坐标,radius -- 半径,height -- 高度,page -- 所在页面
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::HeightBlob(int x, int y, int radius, int height, int page)
{
  int rquad;
  int cx, cy, cyq;
  int left, top, right, bottom;

  int *newptr;
  int *oldptr;

  // 设置指针
  if(page == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }

  rquad = radius * radius;

  // 创建随机水滴效果
  if(x<0) x = 1+radius+ rand()%(m_iWidth-2*radius-1);
  if(y<0) y = 1+radius+ rand()%(m_iHeight-2*radius-1);

  left=-radius; right = radius;
  top=-radius; bottom = radius;

  // 减裁处理
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > m_iWidth-1) right -= (x+radius-m_iWidth+1);
  if(y + radius > m_iHeight-1) bottom-= (y+radius-m_iHeight+1);


  for(cy = top; cy < bottom; cy++)
  {
    cyq = cy*cy;
    for(cx = left; cx < right; cx++)
    {
      if(cx*cx + cyq < rquad)
        newptr[m_iWidth*(cy+y) + (cx+x)] += height;
    }
  }

}

// ---------------------------------------------------------
//	名称: HeightBox
//	功能: 圆角方形涟漪效果处理
//	参数: x,y -- 坐标,radius -- 半径,height -- 高度,page -- 所在页面
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::HeightBox (int x, int y, int radius, int height, int page)
{
  int cx, cy;
  int left, top, right, bottom;
  int *newptr;
  int *oldptr;

  // 设置指针
  if(page == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }

  if(x<0) x = 1+radius+ rand()%(m_iWidth-2*radius-1);
  if(y<0) y = 1+radius+ rand()%(m_iHeight-2*radius-1);

  left=-radius; right = radius;
  top=-radius; bottom = radius;

  // 减裁处理
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > m_iWidth-1) right -= (x+radius-m_iWidth+1);
  if(y + radius > m_iHeight-1) bottom-= (y+radius-m_iHeight+1);

  for(cy = top; cy < bottom; cy++)
  {
    for(cx = left; cx < right; cx++)
    {
        newptr[m_iWidth*(cy+y) + (cx+x)] = height;
    }
  }

}

// ---------------------------------------------------------
//	名称: WarpBlob
//	功能: 波浪形水纹处理
//	参数: x,y -- 坐标,radius -- 半径,height -- 高度,page -- 所在页面
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::WarpBlob(int x, int y, int radius, int height, int page)
{
  int cx, cy;
  int left,top,right,bottom;
  int square;
  int radsquare = radius * radius;
  int *newptr;
  int *oldptr;

  // 设置指针
  if(page == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }
//  radsquare = (radius*radius) << 8;
  radsquare = (radius*radius);

  height /= 64;

  left=-radius; right = radius;
  top=-radius; bottom = radius;

  // 减裁处理
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > m_iWidth-1) right -= (x+radius-m_iWidth+1);
  if(y + radius > m_iHeight-1) bottom-= (y+radius-m_iHeight+1);

  for(cy = top; cy < bottom; cy++)
  {
    for(cx = left; cx < right; cx++)
    {
      square = cy*cy + cx*cx;
//      square <<= 8;
      if(square < radsquare)
      {
//        Height[page][WATERWID*(cy+y) + cx+x]
//          += (sqrt(radsquare)-sqrt(square))*height;
        newptr[m_iWidth*(cy+y) + cx+x]
          += int((radius - sqrt((float)square))*(float)(height));
      }
    }
  }
}

// ---------------------------------------------------------
//	名称: SineBlob
//	功能: 具有正弦角度水痕效果处理
//	参数: x,y -- 坐标,radius -- 半径,height -- 高度,page -- 所在页面
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::SineBlob(int x, int y, int radius, int height, int page)
{
  int cx, cy;
  int left,top,right,bottom;
  int square, dist;
  int radsquare = radius * radius;
  float length = float((1024.0/(float)radius)*(1024.0/(float)radius));
  int *newptr;
  int *oldptr;

  // 设置指针
  if(page == 0)
  {
	newptr = &m_iHeightField1[0];
	oldptr = &m_iHeightField2[0];
  }
  else
  {
	newptr = &m_iHeightField2[0];
	oldptr = &m_iHeightField1[0];
  }

  if(x<0) x = 1+radius+ rand()%(m_iWidth-2*radius-1);
  if(y<0) y = 1+radius+ rand()%(m_iHeight-2*radius-1);


//  radsquare = (radius*radius) << 8;
  radsquare = (radius*radius);

//  height /= 8;

  left=-radius; right = radius;
  top=-radius; bottom = radius;


  // 减裁处理
  if(x - radius < 1) left -= (x-radius-1);
  if(y - radius < 1) top  -= (y-radius-1);
  if(x + radius > m_iWidth-1) right -= (x+radius-m_iWidth+1);
  if(y + radius > m_iHeight-1) bottom-= (y+radius-m_iHeight+1);

  for(cy = top; cy < bottom; cy++)
  {
    for(cx = left; cx < right; cx++)
    {
      square = cy*cy + cx*cx;
      if(square < radsquare)
      {
        dist = int(sqrt(square*length));
        newptr[m_iWidth*(cy+y) + cx+x]
          += (int)((cos((float)dist)+0xffff)*(height)) >> 19;
      }
    }
  }
}

// ---------------------------------------------------------
//	名称: DrawWaterNoLight
//	功能: 暗色处理水纹，并真正交它放入目标位图中(未用)
//	参数: page -- 所在页,pSrcImage -- 源位图,pTargetImage -- 目标位图
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::DrawWaterNoLight(int page,DWORD* pSrcImage,DWORD* pTargetImage)
{

//  int ox, oy;
  int dx, dy;
  int x, y;
  DWORD c;

  int offset=m_iWidth + 1;

  int *ptr = &m_iHeightField1[0];

  for (y = (m_iHeight-1)*m_iWidth; offset < y; offset += 2)
  {
    for (x = offset+m_iWidth-2; offset < x; offset++)
    {
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+m_iWidth];

	  // 绘制有阴影水纹(通过dx值)
//      c = BkGdImage[offset + WATERWID*(dy>>3) + (dx>>3)];
	  c = pSrcImage[offset + m_iWidth*(dy>>3) + (dx>>3)];

     // If anyone knows a better/faster way to do this, please tell me...
//      temp[offset] = (c < 0) ? 0 : (c > 255) ? 255 : c;
	  pTargetImage[offset] = c;

      offset++;
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+m_iWidth];
//    c = BkGdImage[offset + m_iWidth*(dy>>3) + (dx>>3)];
	  c = pSrcImage[offset + m_iWidth*(dy>>3) + (dx>>3)];
	  pTargetImage[offset] = c;
//      temp[offset] = (c < 0) ? 0 : (c > 255) ? 255 : c;
 
    }
  }
}

// ---------------------------------------------------------
//	名称: DrawWaterWithLight
//	功能: 亮色处理水纹，并真正交它放入目标位图中
//	参数: page -- 所在页,LightModifier -- 亮度系数,
//		  pSrcImage -- 源位图,pTargetImage -- 目标位图
//	返回: 无
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
void CWaterRoutine::DrawWaterWithLight(int page, int LightModifier,DWORD* pSrcImage,DWORD* pTargetImage)
{

//  int ox, oy;
  int dx, dy;
  int x, y;
  DWORD c;

//  int offset=m_iWidth + 1;
  int offset = 0;
  long lIndex;
  long lBreak = m_iWidth*m_iHeight;

  int *ptr = &m_iHeightField1[0];

  // 修正原循环中遗漏掉部分处理，原部分已注释掉，jingzhou xu.
  // 利用原不变pSrcImage背景图中像素，经处理后放入pTargetImage效果图中
  for (y = m_iHeight*m_iWidth; offset < y; )		//y = (m_iHeight-1)*m_iWidth; offset < y; offset += 2)
  {
    for (x = offset+m_iWidth; offset < x; offset++)	//x = offset+m_iWidth-2; offset < x; offset++)
    {
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+m_iWidth];

	  lIndex = offset + m_iWidth*(dy>>3) + (dx>>3);
	  if(lIndex < lBreak && lIndex >= 0)
	  {
		  c = pSrcImage[lIndex];// - (dx>>LightModifier);
		 
		  c = GetShiftedColor(c,dx);

 		  pTargetImage[offset] = c;
 	  }

      offset++;
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+m_iWidth];

	  lIndex = offset + m_iWidth*(dy>>3) + (dx>>3);
	  if(lIndex < lBreak && lIndex >= 0)
	  {
		  c = pSrcImage[lIndex];// - (dx>>LightModifier);
		  c = GetShiftedColor(c,dx);
	//      temp[offset] = (c < 0) ? 0 : (c > 255) ? 255 : c;
		  pTargetImage[offset] = c;
	  }
 
    }
  }
 
}

// ---------------------------------------------------------
//	名称: GetShiftedColor
//	功能: 获取差值颜色
//	参数: COLORREF -- 源颜色,shift -- 差值
//	返回: 目标颜色
//	修改: 徐景周，2002.4.8
// ---------------------------------------------------------
inline COLORREF CWaterRoutine::GetShiftedColor(COLORREF color,int shift)
{
	long R;
	long G;
	long B;
	int ir;
	int ig;
	int ib;

	R = GetRValue(color)-shift;
	G = GetGValue(color)-shift;
	B = GetBValue(color)-shift;

	ir = (R < 0) ? 0 : (R > 255) ? 255 : R;
	ig = (G < 0) ? 0 : (G > 255) ? 255 : G;
	ib = (B < 0) ? 0 : (B > 255) ? 255 : B;

	return RGB(ir,ig,ib);
}

// ---------------------------------------------------------
//	名称: DrawDiffuse
//	功能: 类似油画扩散效果(以像素周围4x4方阵随机像素添充)
//	参数: pSrcImage -- 源不变图数据地址，pTargetImage -- 目标效果图数据地址
//	返回: 无
//	编写: 徐景周，2002.1.8
// ---------------------------------------------------------
void CWaterRoutine::DrawDiffuse(DWORD* pSrcImage,DWORD* pTargetImage)
{
	int i,j,k;
	DWORD c;

	// 以时间为随机数种子
//	srand(static_cast<unsigned int>(time(NULL)));
	double f = RAND_MAX;
	for(i = 2;i < m_iWidth; ++i)
		for(j = 2;j < m_iHeight; ++j)
		{
			k = static_cast<int>(15*(rand()/f));
			c = pSrcImage[(i+k%4-2) + (j+k/4-2)*m_iWidth];
			pTargetImage[i+j*m_iWidth] = c;
		}
}