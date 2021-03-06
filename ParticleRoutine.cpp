//////////////////////////////////////////////////
//类名：CParticleRoutine
//功能：粒子系统可实现仿爆炸、环形波、尾汽等效果
//修改：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2002.1.1
//////////////////////////////////////////////////
#include "stdafx.h"
#include "ParticleRoutine.h"
#include "math.h"							// 加入数学运算头文件

// 环境因素
float particle_wind					= 0;	// 环境风速
float particle_gravity				= 0;	// 环境重力

// 粒子状态
const int PARTICLE_STATE_DEAD		= 0;	// 死亡
const int PARTICLE_STATE_ALIVE		= 1;	// 存活

// 粒子类型
const int PARTICLE_TYPE_FLICKER		= 0;	// 闪烁
const int PARTICLE_TYPE_FADE		= 1;	// 退色

// 粒子基本颜色ID
const int PARTICLE_COLOR_BLUE		= 0;	// 蓝
const int PARTICLE_COLOR_GREEN		= 1;	// 绿
const int PARTICLE_COLOR_CYAN		= 2;    // 青
const int PARTICLE_COLOR_RED		= 3;	// 红
const int PARTICLE_COLOR_MAGENTA	= 4;    // 洋红
const int PARTICLE_COLOR_YELLOW		= 5;	// 黄
const int PARTICLE_COLOR_WHITE		= 6;	// 白

// 粒子基本颜色范围
const COLORREF COLOR_BLUE_START		= RGB(0,0,0);			// 从黑到蓝
const COLORREF COLOR_BLUE_END		= RGB(0,0,255);

const COLORREF COLOR_GREEN_START	= RGB(0,0,255);			// 从蓝到绿
const COLORREF COLOR_GREEN_END		= RGB(0,255,0);

const COLORREF COLOR_CYAN_START		= RGB(0,255,0);			// 从绿到青
const COLORREF COLOR_CYAN_END		= RGB(0,255,255);

const COLORREF COLOR_RED_START		= RGB(0,255,255);		// 从青到红
const COLORREF COLOR_RED_END		= RGB(255,0,0);

const COLORREF COLOR_MAGENTA_START	= RGB(255,0,0);			// 从红到洋红
const COLORREF COLOR_MAGENTA_END	= RGB(255,0,255);

const COLORREF COLOR_YELLOW_START	= RGB(255,0,255);		// 从洋红到黄
const COLORREF COLOR_YELLOW_END		= RGB(255,255,0);

const COLORREF COLOR_WHITE_START	= RGB(255,255,0);		// 从黄到白
const COLORREF COLOR_WHITE_END		= RGB(255,255,255);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------
//	名称: CParticleRoutine
//	功能: 初始化工作
//	参数: 无
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CParticleRoutine::CParticleRoutine()
{
	// 预先计算360度各角度的正弦、余弦值，以避免重复浪费
	for(int ang = 0; ang <360; ++ang)
	{
		float a = static_cast<float>(ang*3.14159/180);

		cos_value[ang] = static_cast<float>(cos(a));
		sin_value[ang] = static_cast<float>(sin(a));
	}
}

// ---------------------------------------------------------
//	名称: ~CParticleRoutine
//	功能: 退出时清除工作
//	参数: 无
//	返回: 无
//	编写: 徐景周(Johnny Xu, xujingzhou2016@gmail.com)，2002.4.8
// ---------------------------------------------------------
CParticleRoutine::~CParticleRoutine()
{

}

// ---------------------------------------------------------
//	名称: Reset_Particles
//	功能: 初始化、复位粒子系统
//	参数: nWidth -- 传入位图宽度, nHeight -- 传入位图高度
//	返回: 无
//	编写: 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Reset_Particles(int nWidth,int nHeight)
{
	for(int index=0; index < MAX_PARTICLES; ++index)
	{
		particles[index].state			= PARTICLE_STATE_DEAD;
		particles[index].type			= PARTICLE_TYPE_FADE;
		particles[index].x				= 0;
		particles[index].y				= 0;
		particles[index].xv				= 0;
		particles[index].yv				= 0;
		particles[index].start_color	= RGB(0,0,0);
		particles[index].end_color		= RGB(0,0,0);
		particles[index].current_color	= RGB(0,0,0);
		particles[index].counter		= 0;
		particles[index].max_count		= 0;
	}

	m_bmpWidth	= nWidth;
	m_bmpHeight = nHeight;
}

// ---------------------------------------------------------
//	名称: Start_Particle
//	功能: 处理各粒子数据
//	参数: type -- 粒子类型, color -- 粒子颜色类型，count -- 粒子生命周期
//        x,y  -- 粒子位置，xv,xy -- 粒子初速度
//	返回: 无
//	编写: 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Start_Particle(int type,int color,int count,int x,int y,float xv,float yv)
{
	int pindex = -1;

	// 寻找一个空闲的粒子
	for(int index=0; index < MAX_PARTICLES; ++index)
	{
		if(PARTICLE_STATE_DEAD == particles[index].state)
		{
			pindex = index;
			break;
		}
	}

	if(-1 == pindex)
		return;
	
	// 找到后给它设置相关值
	particles[pindex].state			= PARTICLE_STATE_ALIVE;
	particles[pindex].type			= type;
	particles[pindex].x				= x;
	particles[pindex].y				= y;
	particles[pindex].xv			= xv;
	particles[pindex].yv			= yv;
	particles[pindex].counter		= 0;
	particles[pindex].max_count		= count;

	switch(color)
	{
	case PARTICLE_COLOR_BLUE:			// 蓝
		{
			particles[pindex].start_color	= COLOR_BLUE_START;
			particles[pindex].end_color		= COLOR_BLUE_END;
			break;
		}
	case PARTICLE_COLOR_GREEN:			// 绿
		{
			particles[pindex].start_color	= COLOR_GREEN_START;
			particles[pindex].end_color		= COLOR_GREEN_END;
			break;
		}   
	case PARTICLE_COLOR_CYAN:			// 青
		{
			particles[pindex].start_color	= COLOR_CYAN_START;
			particles[pindex].end_color		= COLOR_CYAN_END;
			break;
		}
	case PARTICLE_COLOR_RED:			// 红
		{
			particles[pindex].start_color	= COLOR_RED_START;
			particles[pindex].end_color		= COLOR_RED_END;
			break;
		}
	case PARTICLE_COLOR_MAGENTA:		// 洋红
		{
			particles[pindex].start_color	= COLOR_MAGENTA_START;
			particles[pindex].end_color		= COLOR_MAGENTA_END;
			break;
		}
	case PARTICLE_COLOR_YELLOW:			// 黄
		{
			particles[pindex].start_color	= COLOR_YELLOW_START;
			particles[pindex].end_color		= COLOR_YELLOW_END;
			break;
		}
	case PARTICLE_COLOR_WHITE:			// 白
		{
			particles[pindex].start_color	= COLOR_WHITE_START;
			particles[pindex].end_color		= COLOR_WHITE_END;
			break;
		}
	}

	if(PARTICLE_TYPE_FLICKER == type)
	{
		particles[pindex].current_color = GetRandom(particles[pindex].start_color, particles[pindex].end_color);
	}
	else
	{
		particles[pindex].current_color = particles[pindex].start_color;
	}
}

// ---------------------------------------------------------
//	名称: Draw_Particles
//	功能: 将处理后各粒子数据传入位图中
//	参数: pTargetImage -- 传入位图指针
//	返回: 无
//	编写: 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Draw_Particles(DWORD* pTargetImage)
{
	for(int index=0; index < MAX_PARTICLES; ++index)
	{
		int x = particles[index].x;
		int y = particles[index].y;

		if(x >= m_bmpWidth || x < 0 || y >= m_bmpHeight || y < 0)
			continue;

		pTargetImage[y*m_bmpWidth + x] = particles[index].current_color;
	}
}

// ---------------------------------------------------------
//	名称: Process_Particles
//	功能: 处理运动中各粒子后，调用Draw_Particles()
//	参数: pTargetImage -- 传入位图指针
//	返回: 无
//	编写: 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Process_Particles(DWORD* pTargetImage)
{
	for(int index=0; index < MAX_PARTICLES; ++index)
	{
		if(PARTICLE_STATE_ALIVE == particles[index].state)
		{
			particles[index].x	+= static_cast<int>(particles[index].xv);
			particles[index].y	+= static_cast<int>(particles[index].yv);

			particles[index].xv	+= particle_wind;
			particles[index].yv	+= particle_gravity;

			if(PARTICLE_TYPE_FLICKER == particles[index].type)		// 粒子在激活闪烁
			{
				particles[index].current_color = GetRandom(particles[index].start_color, particles[index].end_color);
				// 更新计数器，看是否超过生命周期
				if(++particles[index].counter >= particles[index].max_count)
					particles[index].state = PARTICLE_STATE_DEAD;
			}
			else													// 粒子未激活 
			{
				if(++particles[index].counter >= particles[index].max_count)
				{
					particles[index].counter = 0;
					// 更新颜色值
					if(++particles[index].current_color > particles[index].end_color)
						particles[index].state = PARTICLE_STATE_DEAD;
				}
			}
			
			if(particles[index].x > m_bmpWidth || particles[index].x < 0 || particles[index].y > m_bmpHeight || particles[index].y < 0)
				particles[index].state = PARTICLE_STATE_DEAD;

			// 绘制粒子
			Draw_Particles(pTargetImage);
		}
	}
}

// ---------------------------------------------------------
//	名称: Set_Particle_Explosion
//	功能: 爆炸效果处理
//	参数: type -- 粒子类型, color -- 粒子颜色类型，count -- 粒子生命周期
//        x,y  -- 粒子位置，xv,xy -- 粒子初速度
//	返回: 无
//	编写: 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Set_Particle_Explosion(int type, int color, int count, int x, int y, int xv, int yv)
{
	int nNum = MAX_PARTICLES;
	while(--nNum >= 0)
	{
		int ang = rand()%360;
		float vel = static_cast<float>(2 + rand()%4);

		Start_Particle(type,color,count,x+GetRandom(-4,4),y+GetRandom(-4,4),xv+cos_value[ang]*vel,yv+sin_value[ang]*vel);
	}
}

// ---------------------------------------------------------
//	名称: Set_Particle_Ring
//	功能: 环形波效果处理
//	参数: type -- 粒子类型, color -- 粒子颜色类型，count -- 粒子生命周期
//        x,y  -- 粒子位置，xv,xy -- 粒子初速度
//	返回: 无
//	编写: 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Set_Particle_Ring(int type, int color, int count, int x, int y, int xv, int yv)
{
	float vel = static_cast<float>(2 + rand()%4);
	int nNum = MAX_PARTICLES;
	while(--nNum >= 0)
	{
		int ang = rand()%360;

		Start_Particle(type,color,count,x,y,xv+cos_value[ang]*vel,yv+sin_value[ang]*vel);
	}
}

// ---------------------------------------------------------
//	名称: Set_Particle_Gas
//	功能: 尾汽效果处理
//	参数: type -- 粒子类型, color -- 粒子颜色类型，emit_x -- 发射时X坐标
//        emit_y  -- 发射时Y坐标，emit_xv,emit_xy -- 发射时初速度
//	返回: 无
//	编写 徐景周，2002.4.8
// ---------------------------------------------------------
void CParticleRoutine::Set_Particle_Gas(int type, int color, int emit_x, int emit_y, int emit_xv, int emit_yv)
{
	int nNum = MAX_PARTICLES;
	while(--nNum >= 0)
	{
		if(1 == (rand()%10))
			Start_Particle(type,color,GetRandom(30,60),emit_x+GetRandom(4,-4),emit_y+GetRandom(4,-4),static_cast<float>(emit_xv+GetRandom(2,-2)),static_cast<float>(emit_yv+GetRandom(2,-2)));
	}
}