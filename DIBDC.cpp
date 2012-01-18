/**************************************************************************/
/*      版权所有：倪征                                                    */
/*      联系地址：清华大学26号楼213室                                     */
/*      联系电话：62774450                                                */
/*      版权声明：本文件版权归倪征所有，任何未经许可的修改、复制和引用均  */
/*                属于侵权行为。                                          */
/**************************************************************************/
// DIBDC.cpp: implementation of the CDIBDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DIBDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIBDC::CDIBDC()
{
	m_hDIBitmap = NULL;
	m_Index=NULL;
	m_pData=NULL;
	CClientDC ClientDC(NULL);
	CDC::CreateCompatibleDC(&ClientDC);

	m_BitmapInfo.bmiHeader.biSize=40;
	m_BitmapInfo.bmiHeader.biPlanes=1;
	m_BitmapInfo.bmiHeader.biCompression=BI_RGB;
	m_BitmapInfo.bmiHeader.biSizeImage=0;
	m_BitmapInfo.bmiHeader.biXPelsPerMeter=3780;
	m_BitmapInfo.bmiHeader.biYPelsPerMeter=3780;
	m_BitmapInfo.bmiHeader.biClrUsed=0;
	m_BitmapInfo.bmiHeader.biClrImportant=0;

#ifdef THERE_IS_A_SHARE_DATA_H_FILE
	m_pShareData=NULL;
#endif
}

CDIBDC::~CDIBDC()
{
	UnlockIndex();
	HBITMAP hDIB=HBITMAP(GetCurrentObject(m_hDC,OBJ_BITMAP));
	DeleteDC();
	::DeleteObject(hDIB);
}

void CDIBDC::CreateDIB(LONG width, LONG height, WORD bits)
{ 
	m_BitmapInfo.bmiHeader.biHeight=height;
	m_BitmapInfo.bmiHeader.biWidth=width;
	m_BitmapInfo.bmiHeader.biBitCount=bits;
	ResetDIB();
	if(bits<=8)
	{
		SetDefaultColors();
	}
}

void CDIBDC::ResetDIB()
{
	m_hDIBitmap=::CreateDIBSection(m_hDC,&m_BitmapInfo,DIB_RGB_COLORS,(void**)&m_pData,NULL,0);
	::DeleteObject(::SelectObject(m_hDC,m_hDIBitmap));

	LockIndex();
}


void CDIBDC::Paste(int XDest, int YDest, DWORD dwRop)
{
	CView* pView=((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
	CDC* pClientDC=pView->GetDC();
	pClientDC->StretchBlt(XDest,YDest,Width(),abs(Height()),this,0,0,Width(),abs(Height()),dwRop);
	pView->ReleaseDC(pClientDC);
}

void CDIBDC::Paste(int XDest, int YDest, int nWidth, int nHeight, int XSrc, int YSrc, DWORD dwRop)
{
	CView* pView=((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
	CDC* pClientDC=pView->GetDC();
	pClientDC->StretchBlt(XDest,YDest,nWidth,nHeight,this,XSrc,YSrc,nWidth,nHeight,dwRop);
	pView->ReleaseDC(pClientDC);
}

void CDIBDC::Paste(int XDest, int YDest, int nWidth, int nHeight, DWORD dwRop)
{
	CView* pView=((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
	CDC* pClientDC=pView->GetDC();
	pClientDC->StretchBlt(XDest,YDest,nWidth,nHeight,this,0,0,Width(),abs(Height()),dwRop);
	pView->ReleaseDC(pClientDC);
}

void CDIBDC::Paste(int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, DWORD dwRop)
{
	CView* pView=((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
	CDC* pClientDC=pView->GetDC();
	pClientDC->StretchBlt(XDest,YDest,nDestWidth,nDestHeight,this,XSrc,YSrc,nSrcWidth,nSrcHeight,dwRop);
	pView->ReleaseDC(pClientDC);
}

void CDIBDC::SetColors(int index, BYTE r, BYTE g, BYTE b)
{
	if(index>255)
		return;
	m_pColors[index].rgbRed=r;
	m_pColors[index].rgbGreen=g;
	m_pColors[index].rgbBlue=b;
	::SetDIBColorTable(m_hDC,index,1,&m_pColors[index]);
}

bool CDIBDC::GetFreeImage(CFile *pFile)
{
	WORD width,height;
	pFile->SeekToBegin();
	pFile->Read(&width,2);
	pFile->Read(&height,2);
	if(DWORD(width)*height+4!=pFile->GetLength())
		return false;
	CreateDIB(width,height,8);
	SetDefaultColors(true);
	for(WORD i=0;i<height;i++)
	{
		pFile->Read(&(m_pData[(height-i-1)*RowBytes()]),width);
	}
	return true;
}

void CDIBDC::SaveFreeImage(CFile *pFile, BOOL IsGray)
{
	if(Bits()!=8 || IsGray==false)
	{
		CDIBDC tempDC;
		tempDC.CreateDIB(Width(),abs(Height()),8);
		tempDC.SetDefaultColors(true);
		this->Paste(&tempDC,0,0);
		*this=tempDC;
	}

	pFile->SeekToBegin();
	WORD width=WORD(Width());
	WORD height=WORD(Height());
	pFile->Write(&width,2);
	pFile->Write(&height,2);
	for(WORD i=0;i<height;i++)
	{
		pFile->Write(&(m_pData[(height-i-1)*RowBytes()]),width);
	}
}

bool CDIBDC::GetBitmap(CFile * pFile)
{
	char bm[3];
	DWORD value;
	DWORD lHeadLength;
	DWORD TotalSize;
	pFile->SeekToBegin();
	pFile->Read(bm,2);
	if(bm[0]!='B' || bm[1]!='M')
		return false;
	pFile->Read(&TotalSize,4);
	if(pFile->GetLength()<TotalSize)
		return false;
	pFile->Read(&value,4);
	pFile->Read(&lHeadLength,4);
	pFile->Read(&m_BitmapInfo.bmiHeader.biSize,4);
	DWORD current=pFile->Seek(-4l,CFile::current);
	if(m_BitmapInfo.bmiHeader.biSize+current>pFile->GetLength())
		return false;
	pFile->Read(&m_BitmapInfo.bmiHeader,m_BitmapInfo.bmiHeader.biSize);

	ResetDIB();

	LONG ColorsSize=lHeadLength-m_BitmapInfo.bmiHeader.biSize-0xE;
	if(ColorsSize>0)
	{
		pFile->Read(m_pColors,ColorsSize);
		SetColorTable(m_pColors);
	}
	pFile->Read(m_pData,ImageSize());
	return true;
}

void CDIBDC::SaveBitmap(CFile * pFile)
{
	char bm[3]="BM";
	LONG value=0;
	LONG lHeadLength=0xE+m_BitmapInfo.bmiHeader.biSize;
	if(Bits()<=8)
		lHeadLength+=4*(1<<Bits());
	LONG TotalSize=ImageSize()+lHeadLength;
	BITMAPINFOHEADER bmiHeader=m_BitmapInfo.bmiHeader;
	if(Height()<0)
		bmiHeader.biHeight=-Height();
	pFile->SeekToBegin();
	pFile->Write(bm,2);
	pFile->Write(&TotalSize,4);
	pFile->Write(&value,4);
	pFile->Write(&lHeadLength,4);
	pFile->Write(&bmiHeader,bmiHeader.biSize);
	if(Bits()<=8)
	{
		pFile->Write(ColorTable(),4*(1<<Bits()));
	}
	if(Height()>0)
	{
		pFile->Write(m_pData,ImageSize());
	}
	else
	{
		for(int i=-Height()-1;i>=0;i--)
		{
			pFile->Write(m_Index[i], RowBytes());
		}
	}
}

void CDIBDC::SetDefaultColors(BOOL IsGray)
{
	switch(Bits())
	{
	case 1:
		m_pColors[0].rgbRed=0;
		m_pColors[0].rgbGreen=0;
		m_pColors[0].rgbBlue=0;
		m_pColors[0].rgbReserved=0;
		m_pColors[1].rgbRed=0xff;
		m_pColors[1].rgbGreen=0xff;
		m_pColors[1].rgbBlue=0xff;
		m_pColors[1].rgbReserved=0;
		SetColorTable(m_pColors);
		break;
	case 4:
		if(IsGray)
		{
			for(int i=0;i<16;i++)
			{
				m_pColors[i].rgbRed=i<<4;
				m_pColors[i].rgbGreen=i<<4;
				m_pColors[i].rgbBlue=i<<4;
				m_pColors[i].rgbReserved=0;
			}
		}
		else
		{
			for(int i=0;i<16;i++)
			{
				int r,g,b,l;
				if(i&8)l=0x7f;
				else l=0;
				if(i&1)r=0x80;
				else r=0;
				if(i&2)g=0x80;
				else g=0;
				if(i&4)b=0x80;
				else b=0;

				m_pColors[i].rgbRed=r+l;
				m_pColors[i].rgbGreen=g+l;
				m_pColors[i].rgbBlue=b+l;
				m_pColors[i].rgbReserved=0;
			}
		}
		SetColorTable(m_pColors);
		break;
	case 8:
		if(IsGray)
		{
			for(int i=0;i<256;i++)
			{
				m_pColors[i].rgbRed=i;
				m_pColors[i].rgbGreen=i;
				m_pColors[i].rgbBlue=i;
				m_pColors[i].rgbReserved=0;
			}
		}
		else
		{
			BYTE r=0,g=0,b=0xc0;
			for(int i=0;i<256;i++)
			{
				b+=0x40;
				if(b>0x80)b=0xff;
				if(b==0x3f)
				{	
					b=0;
					g+=0x20;
					if(g>0xc0)g=0xff;
					if(g==0x1f)
					{
						g=0;
						r+=0x20;
						if(r>0xc0)r=0xff;
					}
				}
				m_pColors[i].rgbRed=r;
				m_pColors[i].rgbGreen=g;
				m_pColors[i].rgbBlue=b;
				m_pColors[i].rgbReserved=0;
			}
		}
		SetColorTable(m_pColors);
		break;
	default:break;
	}
}

void CDIBDC::Paste(CDC* pDC,
					 int XDest, int YDest, 
					 int nDestWidth, int nDestHeight, 
					 int XSrc, int YSrc, 
					 int nSrcWidth, int nSrcHeight, 
					 DWORD dwRop)
{
	pDC->StretchBlt(
		XDest,YDest,nDestWidth,nDestHeight,
		this,
		XSrc,YSrc,nSrcWidth,nSrcHeight,
		dwRop);
}

void CDIBDC::Paste(CDC* pDC,
					 int XDest, int YDest, 
					 int nWidth, int nHeight, 
					 int XSrc, int YSrc, 
					 DWORD dwRop)
{
	pDC->StretchBlt(
		XDest,YDest,nWidth,nHeight,
		this,
		XSrc,YSrc,nWidth,nHeight,
		dwRop);
}

void CDIBDC::Paste(CDC* pDC,
					 int XDest, int YDest, 
					 int nDestWidth, int nDestHeight, 
					 DWORD dwRop)
{
	pDC->StretchBlt(
		XDest,YDest,nDestWidth,nDestHeight,
		this,
		0,0,Width(),abs(Height()),
		dwRop);
}

void CDIBDC::Paste(CDC* pDC,
					 int XDest, int YDest, 
					 DWORD dwRop)
{
	pDC->StretchBlt(
		XDest,YDest,Width(),abs(Height()),
		this,
		0,0,Width(),abs(Height()),
		dwRop);
}

void CDIBDC::CreateCompatibleDC(CDC * pDC)
{
	HBITMAP hDIB=HBITMAP(GetCurrentObject(m_hDC,OBJ_BITMAP));
	DeleteDC();
	CDC::CreateCompatibleDC(pDC);
	::SelectObject(m_hDC,hDIB);
}

BOOL CDIBDC::ClipboardCopy()
{
	BOOL bHaveCopied=false;
	if(OpenClipboard(AfxGetMainWnd()->m_hWnd))
	{
		if(EmptyClipboard())
		{
			HGLOBAL hDIB=GlobalAlloc(GMEM_MOVEABLE,DIBSize());
			PVOID pDIB=GlobalLock(hDIB);
			SaveToMem(pDIB);
			SetClipboardData(CF_DIB,hDIB);
			GlobalUnlock(hDIB);
			bHaveCopied=true;
		}
		CloseClipboard();
	}
	return bHaveCopied;
}

BOOL CDIBDC::ClipboardPaste()
{
	BOOL bHavePasted=false;
	if(OpenClipboard(AfxGetMainWnd()->m_hWnd))
	{
		if(IsClipboardFormatAvailable(CF_DIB))
		{
			HGLOBAL hDIB=GetClipboardData(CF_DIB);
			PVOID pDIB=GlobalLock(hDIB);
			GetFromMem(pDIB);
			GlobalUnlock(hDIB);
			bHavePasted=true;
		}
		CloseClipboard();
	}
	return bHavePasted;
}

BOOL CDIBDC::ClipboardAvailable()
{
	BOOL bIsAvailable=false;
	if(OpenClipboard(AfxGetMainWnd()->m_hWnd))
	{
		if(IsClipboardFormatAvailable(CF_DIB))
		{
			bIsAvailable=true;
		}
		CloseClipboard();
	}
	return bIsAvailable;

}

LONG& CDIBDC::Width()
{
	return m_BitmapInfo.bmiHeader.biWidth;
}

LONG& CDIBDC::Height()
{
	return m_BitmapInfo.bmiHeader.biHeight;
}

DWORD CDIBDC::ImageSize()
{
	return abs(Height())*RowBytes();
}

WORD& CDIBDC::Bits()
{
	return m_BitmapInfo.bmiHeader.biBitCount;
}

RGBQUAD* CDIBDC::ColorTable()
{
	if(Bits()<=8)
	{
		GetDIBColorTable(m_hDC,0,1<<Bits(),m_pColors);
		return m_pColors;
	}
	else
		return NULL;
}

LONG CDIBDC::RowBytes()
{
	return (Width()*Bits()/8+3)/4*4;
}

void CDIBDC::SetColorTable(RGBQUAD * pColors)
{
	if(Bits()<=8)
	::SetDIBColorTable(m_hDC,0,1<<Bits(),pColors);
}

void CDIBDC::ResizeDIB(LONG width, LONG height)
{
	if(Width()==width && Height()==height)
		return;

	LONG tempWidth=Width();
	LONG tempHeight=Height();

	Height()=height;
	Width()=width;
	m_BitmapInfo.bmiHeader.biSizeImage=0;

	ColorTable();
	HBITMAP hDIBitmap=::CreateDIBSection(m_hDC,&m_BitmapInfo,DIB_RGB_COLORS,(PVOID*)&m_pData,NULL,0);
	HBITMAP hTempDIB=HBITMAP(::SelectObject(m_hDC,hDIBitmap));
	SetColorTable(m_pColors);

	LONG minWidth=Width();
	if(minWidth>tempWidth)
		minWidth=tempWidth;
	LONG minHeight=abs(Height());
	if(minHeight>abs(tempHeight))
		minHeight=abs(tempHeight);

	CDC tempDC;
	tempDC.CreateCompatibleDC(this);
	tempDC.SelectObject(hTempDIB);
	this->BitBlt(0,0,minWidth,minHeight,&tempDC,0,0,SRCCOPY);
	tempDC.DeleteDC();
	::DeleteObject(hTempDIB);

	LockIndex();
}

BOOL CDIBDC::AddDIB(CDIBDC & DIBDC, int x, int y, BOOL IsSuitable)
{
	if(x<0 || y<0)
		return false;
	if(IsSuitable)
	{
		LONG width=x+DIBDC.Width();
		if(Width()>width)
			width=Width();
		LONG height=y+abs(DIBDC.Height());
		if(abs(Height())>height)
			height=abs(Height());
		ResizeDIB(width,height);
	}
	BitBlt(x,y,DIBDC.Width(),DIBDC.Height(),&DIBDC,0,0,SRCCOPY);
	return true;
}

void CDIBDC::CreateDIB(CDIBDC & DIBDC, int x, int y, int width, int height)
{
	memcpy(&m_BitmapInfo,&DIBDC.m_BitmapInfo,40);
	Height()=height;
	Width()=width;
	ResetDIB();
	SetColorTable(DIBDC.ColorTable());
	ColorTable();
	BitBlt(0,0,width,height,&DIBDC,x,y,SRCCOPY);
}

void CDIBDC::CreateDIB(CDIBDC & DIBDC)
{
	memcpy(&m_BitmapInfo,&DIBDC.m_BitmapInfo,40);
	ResetDIB();
	memcpy(m_pData,DIBDC.m_pData,ImageSize());
	if(Bits()<=8)
	{
		SetColorTable(DIBDC.ColorTable());
	}
}

CDIBDC::CDIBDC(CDIBDC & DIBDC)
{
	m_Index=NULL;
	m_pData=NULL;
	CDC::CreateCompatibleDC(&DIBDC);
	CreateDIB(DIBDC);
}

CDIBDC& CDIBDC::operator=(CDIBDC& DIBDC)
{
	CreateCompatibleDC(&DIBDC);
	CreateDIB(DIBDC);
	return DIBDC;
}

bool CDIBDC::GetFromFile(CFile *pFile)
{
	CString suffix=pFile->GetFileName();
	int PointAt=suffix.Find('.');
	do
	{
		suffix=suffix.Mid(PointAt+1);
	}
	while((PointAt=suffix.Find('.'))!=-1);
	suffix.MakeLower();
	if(suffix=="bmp")
	{
		return GetBitmap(pFile);
	}
	if(suffix=="fri")
	{
		return GetFreeImage(pFile);
	}
	if(suffix=="bmc")
	{
		return GetCrypticBmp(pFile);
	}
	if(suffix=="fic")
	{
		return GetCrypticFri(pFile);
	}
	return false;
}

void CDIBDC::SaveToFile(CFile *pFile)
{
	CString suffix=pFile->GetFileName();
	int PointAt=suffix.Find('.');
	do
	{
		suffix=suffix.Mid(PointAt+1);
	}
	while((PointAt=suffix.Find('.'))!=-1);
	suffix.MakeLower();
	if(suffix=="fri")
	{
		SaveFreeImage(pFile);
		return;
	}
	if(suffix=="bmp")
	{
		SaveBitmap(pFile);
		return;
	}
	if(suffix=="bmc")
	{
		SaveCrypticBmp(pFile);
		return;
	}
	if(suffix=="fic")
	{
		SaveCrypticFri(pFile);
		return;
	}
	SaveBitmap(pFile);
}

BYTE** CDIBDC::LockIndex()
{
	if(m_Index!=NULL)
	{
		delete[] m_Index;
	}
	LONG height=Height();
	LONG rowBytes=RowBytes();
	if(height>0)
	{
		m_Index=new PBYTE[height];
		for(int i=0;i<height;i++)
			m_Index[height-i-1]=&m_pData[i*rowBytes];
	}
	else
	{
		m_Index=new PBYTE[-height];
		for(int i=0;i<-height;i++)
			m_Index[i]=&m_pData[i*rowBytes];
	}
	return m_Index;
}

void CDIBDC::UnlockIndex()
{
	if(m_Index!=NULL)
	{
		delete[] m_Index;
		m_Index=NULL;
	}
}

BYTE* CDIBDC::GetData()
{
	return m_pData;
}

void CDIBDC::SetData(LPVOID pData)
{
	memcpy(m_pData,pData,ImageSize());
}

BYTE** CDIBDC::GetIndex()
{
	return m_Index;
}

void CDIBDC::SetAt(int x, int y, ...)
{
	va_list marker;
	va_start(marker,y);
	switch(Bits())
	{
	case 1:
		m_Index[y][x>>3]&=~(0x1<<(7-x%8));
		m_Index[y][x>>3]|=(va_arg(marker,BYTE)&1)<<(7-x%8);
		break;
	case 4:
		m_Index[y][x>>1]&=x&1?0xf0:0xf;
		m_Index[y][x>>1]|=(va_arg(marker,BYTE)&0xf)<<(x&1?0:4);
		break;
	case 8:
		m_Index[y][x]=va_arg(marker,BYTE);
		break;
	case 16:
		((WORD*)(m_Index[y]))[x]=va_arg(marker,WORD);
		break;
	case 24:
		((tagRGB*)(m_Index[y]))[x]=va_arg(marker,tagRGB);
		break;
	case 32:
		((DWORD*)(m_Index[y]))[x]=va_arg(marker,DWORD);
		break;
	default:;
	}
	va_end(marker);
}

void CDIBDC::GetAt(int x, int y, ...)
{
	va_list marker;
	va_start(marker,y);
	switch(Bits())
	{
	case 1:
		*va_arg(marker,BYTE*)=(m_Index[y][x>>3]&(0x1<<(7-x%8)))>>(7-x%8);
		break;
	case 4:
		*va_arg(marker,BYTE*)=(m_Index[y][x>>1]&(x&1?0xf:0xf0))>>(x&1?0:4);
		break;
	case 8:
		*va_arg(marker,BYTE*)=m_Index[y][x];
		break;
	case 16:
		*va_arg(marker,WORD*)=((WORD*)(m_Index[y]))[x];
		break;
	case 24:
		*va_arg(marker,tagRGB*)=((tagRGB*)(m_Index[y]))[x];
		break;
	case 32:
		*va_arg(marker,DWORD*)=((DWORD*)(m_Index[y]))[x];
		break;
	default:;
	}
	va_end(marker);
}

void CDIBDC::SaveCrypticBmp(CFile* pFile)
{
#ifdef THERE_IS_A_ENCRYPTION_H_FILE

	char bm[3]="BM";
	LONG value=0;
	LONG lHeadLength=0xE+m_BitmapInfo.bmiHeader.biSize;
	if(Bits()<=8)
		lHeadLength+=1<<(Bits()+2);
	LONG TotalSize=ImageSize()+lHeadLength;
	pFile->SeekToBegin();
	DWORD seed=time(NULL);
	pFile->Write(&seed, sizeof(seed));
	m_Encryption.SetSeed(seed);
	m_Encryption.Encrypt(bm, 2);
	pFile->Write(bm,2);
	m_Encryption.Encrypt(&TotalSize, 4);
	pFile->Write(&TotalSize,4);
	m_Encryption.Encrypt(&value, 4);
	pFile->Write(&value,4);
	m_Encryption.Encrypt(&lHeadLength, 4);
	pFile->Write(&lHeadLength,4);
	BITMAPINFOHEADER temp_bmiHeader=m_BitmapInfo.bmiHeader;
	m_Encryption.Encrypt(&temp_bmiHeader,m_BitmapInfo.bmiHeader.biSize);
	pFile->Write(&temp_bmiHeader,m_BitmapInfo.bmiHeader.biSize);

	PBYTE pData;
	if(Bits()<=8)
	{
		pData=new BYTE[1<<(Bits()+2)];
		memcpy(pData, ColorTable(), 1<<(Bits()+2));
		m_Encryption.Encrypt(pData,1<<(Bits()+2));
		pFile->Write(pData,1<<(Bits()+2));
		delete[] pData;
	}

	pData=new BYTE[ImageSize()];
	memcpy(pData, m_pData, ImageSize());
	m_Encryption.Encrypt(pData, ImageSize());
	pFile->Write(pData,ImageSize());
	delete[] pData;

#endif
}

bool CDIBDC::GetCrypticBmp(CFile* pFile)
{
#ifdef THERE_IS_A_ENCRYPTION_H_FILE

	char bm[3];
	DWORD value;
	DWORD lHeadLength;
	DWORD TotalSize;
	pFile->SeekToBegin();

	DWORD seed;
	pFile->Read(&seed, sizeof(seed));
	m_Encryption.SetSeed(seed);
	pFile->Read(bm, 2);
	m_Encryption.Decrypt(bm, 2);
	if(bm[0]!='B' || bm[1]!='M')
		return false;
	pFile->Read(&TotalSize,4);
	m_Encryption.Decrypt(&TotalSize, 4);
	if(pFile->GetLength()<TotalSize)
		return false;
	pFile->Read(&value,4);
	m_Encryption.Decrypt(&value,4);
	pFile->Read(&lHeadLength,4);
	m_Encryption.Decrypt(&lHeadLength,4);
	pFile->Read(&m_BitmapInfo.bmiHeader.biSize,4);
	m_Encryption.Decrypt(&m_BitmapInfo.bmiHeader.biSize,4);
	pFile->Read(PBYTE(&m_BitmapInfo.bmiHeader)+4,m_BitmapInfo.bmiHeader.biSize-4l);
	m_Encryption.Decrypt(PBYTE(&m_BitmapInfo.bmiHeader)+4,m_BitmapInfo.bmiHeader.biSize-4l);

	ResetDIB();

	LONG ColorsSize=lHeadLength-m_BitmapInfo.bmiHeader.biSize-0xE;
	if(ColorsSize>0)
	{
		pFile->Read(m_pColors,ColorsSize);
		m_Encryption.Decrypt(m_pColors, ColorsSize);
		SetColorTable(m_pColors);
	}

	pFile->Read(m_pData,ImageSize());
	m_Encryption.Decrypt(m_pData, ImageSize());

	return true;

#endif

	return false;
}

void CDIBDC::SaveCrypticFri(CFile *pFile, BOOL IsGray)
{
#ifdef THERE_IS_A_ENCRYPTION_H_FILE

	if(Bits()!=8 || IsGray==false)
	{
		CDIBDC tempDC;
		tempDC.CreateDIB(Width(),abs(Height()),8);
		tempDC.SetDefaultColors(true);
		this->Paste(&tempDC,0,0);
		*this=tempDC;
	}

	pFile->SeekToBegin();
	WORD width=WORD(Width());
	WORD height=WORD(Height());
	DWORD seed=time(NULL);
	pFile->Write(&seed, sizeof(seed));
	m_Encryption.SetSeed(seed);
	m_Encryption.Encrypt(&width, 2);
	pFile->Write(&width,2);
	m_Encryption.Encrypt(&height, 2);
	pFile->Write(&height,2);

	width=WORD(Width());
	height=WORD(Height());
	PBYTE pData=new BYTE[width];
	for(WORD i=0;i<height;i++)
	{
		memcpy(pData, &(m_pData[(height-i-1)*RowBytes()]), width);
		m_Encryption.Encrypt(pData, width);
		pFile->Write(pData, width);
	}
	delete[] pData;

#endif

}

bool CDIBDC::GetCrypticFri(CFile *pFile)
{
#ifdef THERE_IS_A_ENCRYPTION_H_FILE

	WORD width,height;
	DWORD seed;
	pFile->SeekToBegin();
	pFile->Read(&seed, sizeof(seed));
	m_Encryption.SetSeed(seed);
	pFile->Read(&width,2);
	m_Encryption.Decrypt(&width, 2);
	pFile->Read(&height,2);
	m_Encryption.Decrypt(&height, 2);
	LONG currentPos=pFile->Seek(0l, CFile::current);
	if(DWORD(width)*height+currentPos<pFile->GetLength())
		return false;
	CreateDIB(width,height,8);
	SetDefaultColors(true);
	for(WORD i=0;i<height;i++)
	{
		pFile->Read(&(m_pData[(height-i-1)*RowBytes()]),width);
		m_Encryption.Decrypt(&(m_pData[(height-i-1)*RowBytes()]),width);
	}
	return true;

#endif

	return false;
}

void CDIBDC::SetFillColor(COLORREF color)
{
	CDIBDC tempDC;
	tempDC.CreateDIB(Width(), abs(Height()), 8);
	tempDC.SetColors(255, BYTE(color&0xff), BYTE((color>>8)&0xff), BYTE((color>>16)&0xff) );
	memset(tempDC.GetData(), 255, tempDC.ImageSize());
	tempDC.Paste(this, 0, 0, Width(), abs(Height()));
}

DWORD CDIBDC::DIBSize()
{
	DWORD dwTotalSize=ImageSize()+DIBHeaderSize();

	return dwTotalSize;
}

void CDIBDC::SaveToMem(PVOID pDIB)
{
	SaveToDIBHeader(pDIB);

	memcpy((BYTE*)pDIB + DIBHeaderSize(), m_pData,ImageSize());
}

void CDIBDC::GetFromMem(PVOID pDIB)
{
	memcpy(&m_BitmapInfo,pDIB,((PBITMAPINFO)pDIB)->bmiHeader.biSize);

	ResetDIB();
	LPSTR pData=(LPSTR)pDIB+(WORD)( ((PBITMAPINFO)pDIB)->bmiHeader.biSize);
	WORD bits=Bits();
	if(bits<=8)
	{
		LPSTR pColors=pData;
		SetColorTable((RGBQUAD*)pColors);
		pData+=ColorTableSize();
	}
	memcpy(m_pData,pData,ImageSize());
}

DWORD CDIBDC::ColorTableSize()
{
	DWORD dwColorSize=0;
	if(Bits()<=8)
	{
		dwColorSize=m_BitmapInfo.bmiHeader.biClrUsed<<2;
		if(dwColorSize==0)
			dwColorSize=1<<(Bits()+2);
	}
	return dwColorSize;
}

HBITMAP CDIBDC::GetBitmapHandle()
{
	return m_hDIBitmap;
}

void CDIBDC::SaveToDIBHeader(PVOID pDIB)
{
	memcpy(pDIB,&m_BitmapInfo,m_BitmapInfo.bmiHeader.biSize);
	LPSTR pData=(LPSTR)pDIB+(WORD)(m_BitmapInfo.bmiHeader.biSize);
	if(Bits()<=8)
	{
		LPSTR pColors=pData;
		memcpy(pColors,ColorTable(),ColorTableSize());
		pData=pColors+ColorTableSize();
	}
}

DWORD CDIBDC::DIBHeaderSize()
{
	DWORD dwHeaderSize = m_BitmapInfo.bmiHeader.biSize + ColorTableSize();
	return dwHeaderSize;
}

BOOL CDIBDC::SaveImage(LPCTSTR lpszPathName)
{
	CFile file;
	if(!file.Open(lpszPathName, CFile::modeWrite | CFile::modeCreate))
		return FALSE;

	SaveBitmap(&file);

	file.Close();

	return TRUE;
}

BOOL CDIBDC::OpenImage(LPCTSTR lpszPathName)
{
	CFile file;
	if(!file.Open(lpszPathName, CFile::modeRead))
		return FALSE;

	BOOL bRet = GetBitmap(&file);

	file.Close();
	return bRet;
}

