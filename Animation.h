/////////////////////////////////////////////////////////////////////////
//类名：CAnimationEngine
//功能：屏保的各种动画文字及位图显示效果
//修改：徐景周(Johnny Xu, xujingzhou2016@gmail.com)
//组织：未来工作室(Future Studio)
//日期：2002.1.8
////////////////////////////////////////////////////////////////////////
#if !defined(AFX_ANIMATION_H__D3C2A241_5D60_11D6_9F37_00500486CAFC__INCLUDED_)
#define AFX_ANIMATION_H__D3C2A241_5D60_11D6_9F37_00500486CAFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "DIB256.h"

class CAnimationEngine;

class CAnimationItem
{
public:
	CAnimationItem();
	CAnimationItem( CAnimationEngine* pAE );
	virtual ~CAnimationItem();

	CAnimationItem& SetDefaults();
	CAnimationItem& SetFont( CString csFont );
	CAnimationItem& SetBitmap( UINT nBitmapID, int nFirst=-1, int nLast=-1 );
	CAnimationItem& SetTextColor( COLORREF rgb );
	CAnimationItem& SetText( CString csText );
	CAnimationItem& SetStart( int nX, int nY ) { return SetStart(CPoint(nX,nY)); };
	CAnimationItem& SetStart( CPoint pt );
	CAnimationItem& SetFinish( int nX, int nY ) { return SetFinish(CPoint(nX,nY)); };
	CAnimationItem& SetFinishRel( int nX, int nY ) { return SetFinish(CPoint(m_ptStart.x+nX,m_ptStart.y+nY)); };
	CAnimationItem& SetFinish( CPoint pt );
	CAnimationItem& SetPoint( int nX, int nY ) { return SetPoint(CPoint(nX,nY)); };
	CAnimationItem& SetPointRel( int nX, int nY ) { return SetPoint(CPoint(m_ptStart.x+nX,m_ptStart.y+nY)); };
	CAnimationItem& SetPoint( CPoint pt );
	CAnimationItem& SetFrames( int nFrames );
	CAnimationItem& GoAfter( POSITION pos );
	CAnimationItem& GoWith( POSITION pos );
	CAnimationItem& SetEffect( int nEffect ) { m_nEffects|=nEffect; return *this; };
	CAnimationItem& FadeIn() { m_rgbTextColorCurrent=RGB(0,0,0); return SetEffect(AF_FadeIn); };
	CAnimationItem& FadeOut() { return SetEffect(AF_FadeOut); };
	CAnimationItem& AlignRight() { return SetEffect(AF_AlignRight); };
	CAnimationItem& AlignLeft() { return SetEffect(AF_AlignLeft); };
	CAnimationItem& AlignCenter() { return SetEffect(AF_AlignCenter); };

	enum AnimationEffects
	{
		AF_FadeIn = 1,
		AF_FadeOut = 2,
		AF_AlignLeft = 4,
		AF_AlignRight = 8,
		AF_AlignCenter = 16,
	};

	POSITION GetPos() { return m_pos; };

	CString m_csText;
	COLORREF m_rgbTextColor;
	COLORREF m_rgbTextColorCurrent;
	CPoint m_ptStart;
	CPoint m_ptFinish;
	CPoint m_pt;
	double m_dX, m_dY;
	int m_nFrames;
	BOOL m_bDrawnYet;
	int m_nEffects;
	CFont* m_pFont;
	CDIBitmap* m_pBitmap;
	int m_nFirstPic;
	int m_nLastPic;

	BOOL m_bWait;
	POSITION m_pos;
	POSITION m_posNext;
	POSITION m_posKickOff;

	CAnimationEngine* m_pAE;
};

class CAnimationEngine
{
public:
	CAnimationEngine();
	virtual ~CAnimationEngine();

	CAnimationItem& CreateItem( CString csText );
	CAnimationItem& CreateItemAfter( POSITION pos );
	CAnimationItem& GetAt( POSITION pos ) { return m_listItems.GetAt(pos); };

	void SetScreen( CRect rect );
	CPoint RandPt( int nMargin = 0, int nMin = 40 );
	int Rand( int n );
	int RunFrame( CDC& dc );
	CDIBitmap* GetBmp( UINT nBitmapID );
	CFont* GetFont( CString csFont );
	void DestroyGDIs();

	CRect m_rect;
	int HCenter() { return (m_rect.right - m_rect.left)/2 + m_rect.left; };
	int VCenter() { return (m_rect.bottom - m_rect.top)/2 + m_rect.top; };
	int Left() { return m_rect.left; }
	int Top() { return m_rect.top; }
	int Right() { return m_rect.right; }
	int Bottom() { return m_rect.bottom; }
	int Height() { return m_rect.Height(); }
	int Width() { return m_rect.Width(); }

protected:
	void DrawTextItem( CDC& dc, CAnimationItem& item, int nDrawFlags, BOOL bShow );
	void DrawBitmapItem( CDC& dc, CAnimationItem& item, CPoint ptNew, BOOL bShow );

	friend class CAnimationItem;
	COLORREF m_rgbBackground;
	CBrush m_brushBackground;
	CFont* m_pDefaultFont;

	CMap<CString,LPCTSTR,CFont*,CFont*> m_mapFonts;
	CMap<UINT,UINT,CDIBitmap*,CDIBitmap*> m_mapBmps;
	CList<CAnimationItem,CAnimationItem&> m_listItems;
};

#endif // !defined(AFX_ANIMATION_H__D3C2A241_5D60_11D6_9F37_00500486CAFC__INCLUDED_)
