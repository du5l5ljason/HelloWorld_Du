// DIBDC.h: interface for the CDIBDC class.
//
//*.This class inherit class CDC, user can use all functions of CDC
//*.easy to create a device independent bitmap
//*.user can get(set) data in a bitmap directly,
//*.get(save) data from(in) a bitmap file, free image file and
//  theirs encrypted file.
//*.get(save) data from(in) a map file(in memory) used to translate
//  data between procedures.

//////////////////////////////////////////////////////////////////////

#if !defined THERE_IS_A_DIBDC_H_FILE
#define THERE_IS_A_DIBDC_H_FILE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "Encryption.h"
//�����û�и�ͷ�ļ���˵����û�л����ȫ��ʹ��Ȩ�ޡ�
//�������ʹ������࣬���Լ򵥵�ɾ�����ͷ�ļ��İ��������ߺ�������ϵ��

struct tagRGB
{
	BYTE b;
	BYTE g;
	BYTE r;
};

//#define DISTANCE_RGB(c1, c2) (abs((int)c1.r-c2.r)+abs((int)c1.g-c2.g)+abs((int)c1.b-c2.b))

class CDIBDC : public CDC  
{
public:
	CDIBDC();
	virtual ~CDIBDC();

protected:
//	CRect		m_Rect;			//A rect can be show in DC which be pasted
//	BOOL		m_bShowRect;	//If m_Rect is seen, it is TRUE.

	//DIB attribute
	HBITMAP		m_hDIBitmap;
	BITMAPINFO	m_BitmapInfo;	//Bitmap information(about DIB)
	RGBQUAD		m_pColors[256];	//Store color table
	BYTE**		m_Index;		//Bitmap data index
	BYTE*		m_pData;		//Bitmap data

#ifdef THERE_IS_A_ENCRYPTION_H_FILE
	CEncryption	m_Encryption;
#endif

protected:

	//Reset DIB use parameter have been ready
	void ResetDIB();
	//Set data index
	BYTE** LockIndex();
	//Delete data index
	void UnlockIndex();

public:
	DWORD DIBHeaderSize();
	void SaveToDIBHeader(PVOID pDIB);
	HBITMAP GetBitmapHandle();
	/////////////////////////////////////////////////////
	//Initialize or resize DIB//
	//���ڴ���DIBDC. 

	//Create a DIB with new size, new bits(at the same time, delete old DIB)
	void CreateDIB(LONG width, LONG height, WORD bits);
	//Create a DIB with a new size, colortable is retained, data is deleted
	void ResizeDIB(LONG width,LONG height);
	//Set the DIB with the same color
	void SetFillColor(COLORREF color);
	
	//////////////////////////////////////////////////////
	//CDIBDC <--> array//
	//����ͼ�����ݵ�ֱ�Ӵ�ȡ. 

	//Get data
	BYTE* GetData();
	//Set data
	void SetData(LPVOID pData);
	//Get data index
	BYTE** GetIndex();
	//Get a pixel at (x,y), example: GetAt(x,y,(WORD*)pColor);
	void GetAt(int x, int y, ...);
	//Set a pixel at (x,y), example: SetAt(x,y,(WORD)Color);
	void SetAt(int x, int y, ...);


	///////////////////////////////////////////
	//CDIBDC <--> CDIBDC//
	//����DIBDC��ĸ�ֵ, ����, �еĺ�������һ��, ֻ�������Ĳ�ͬ. 

	//Overload the construct function, Create a copy of DIBDC
	CDIBDC(CDIBDC& DIBDC);
	//Overload the operator '=', left DC is a copy of right DC
	CDIBDC& operator=(CDIBDC& DIBDC);
	//Create a new DIB which is the same as DIB of DIBDC,
	//same data, and same colortable
	void CreateDIB(CDIBDC& DIBDC);
	//Create a new DIB, use the content in rect(x,y,width,height) in DIB in DIBDC
	void CreateDIB(CDIBDC& DIBDC, int x, int y, int width, int height);
	//Add DIB in DIBDC to this CDIBDC, origin points at (x,y), 
	//IsSuitable is a flag to change or nochange the size of this CDIBDC
	BOOL AddDIB(CDIBDC& DIBDC, int x=0, int y=0, BOOL IsSuitable=TRUE);

	//////////////////////////////////////////
	//CDIBDC <--> Clipboard//
	//���ڼ��������
	
	//Test the data in clipboard if it is CF_DIB
	BOOL ClipboardAvailable();
	//Paste DIB from clipboard
	BOOL ClipboardPaste();
	//Copy DIB to clipboard
	BOOL ClipboardCopy();

	////////////////////////////////////////////
	//CDIBDC <--> Memory //
	//���ڼ���������е��ڴ�ת�ƻ��Զ���Ľ��̼乲������. 

	//pDIBָ����ڴ����ݸ�ʽ����CF_DIB(�������к��豸�޹ص�λͼ)��ʽ.
	void GetFromMem(PVOID pDIB);
	void SaveToMem(PVOID pDIB);

	//�õ�����CF_DIB��ʽ���ڴ�����С. 
	DWORD DIBSize();

	////////////////////////////////////////////
	//CDIBDC <--> file//
	//����DIBDC���ļ���������. 
	//Open image from indicated file.
	BOOL OpenImage(LPCTSTR lpszPathName);
	//Save image from indicated file.
	BOOL SaveImage(LPCTSTR lpszPathName);

	//Set cryptic version of the image file.
	//Save Bitmap to a file
	void SaveBitmap(CFile* pFile);		//*.bmp
	void SaveCrypticBmp(CFile* pFile);	//*.bmc
	//Get Bitmap from a file
	bool GetBitmap(CFile* pFile);
	bool GetCrypticBmp(CFile* pFile);
	//Get free image from a file. 
	//NOTE: format of free image
	//		word width
	//		word height
	//		byte ppData[height][width]
	bool GetFreeImage(CFile* pFile);	//*.fri
	bool GetCrypticFri(CFile* pFile);	//*.fic
	//Save free image to a file
	void SaveFreeImage(CFile* pFile, BOOL IsGray=TRUE);
	void SaveCrypticFri(CFile* pFile, BOOL IsGray=TRUE);
	//Save to file, support all type in list.
	void SaveToFile(CFile *pFile);
	//Open from file
	bool GetFromFile(CFile* pFile);
	
	//////////////////////////////////////////////
	//CDIBDC <--> client area//

	//Paste to ClientDC,origin point at (xDest,yDest), 
	//dwRop is paste mode
	void Paste(int XDest, int YDest, DWORD dwRop=SRCCOPY);
	void Paste(int XDest, int YDest, int nWidth, int nHeight, int XSrc, int YSrc, DWORD dwRop=SRCCOPY);
	void Paste(int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth, int nSrcHeight, DWORD dwRop=SRCCOPY);
	void Paste(int XDest, int YDest, int nWidth, int nHeight, DWORD dwRop=SRCCOPY);
	
	////////////////////////////////////////////////
	//CDIBDC <--> other CDC//

	//Create compatible device context with pDC
	virtual void CreateCompatibleDC(CDC* pDC);
	
	//Paste to pDC, like StrechDIBits(...)
	void Paste(CDC* pDC,
					 int XDest, int YDest, 
					 int nDestWidth, int nDestHeight, 
					 int XSrc, int YSrc, 
					 int nSrcWidth, int nSrcHeight, 
					 DWORD dwRop=SRCCOPY);
	void Paste(CDC* pDC,
					 int XDest, int YDest, 
					 int nWidth, int nHeight, 
					 int XSrc, int YSrc, 
					 DWORD dwRop=SRCCOPY);
	void Paste(CDC* pDC,
					 int XDest, int YDest, 
					 int nDestWidth, int nDestHeight, 
					 DWORD dwRop=SRCCOPY);
	void Paste(CDC* pDC,
					 int XDest, int YDest, 
					 DWORD dwRop=SRCCOPY);

	////////////////////////////////////////////////
	//SetColorTable//
	void SetDefaultColors(BOOL IsGray=TRUE);
	void SetColors(int index, BYTE r, BYTE g, BYTE b);
	void SetColorTable(RGBQUAD* pColors);

	//////////////////////////////////////////////////////
	//Attributes//

	LONG RowBytes();
	DWORD ColorTableSize();
	DWORD ImageSize();
	RGBQUAD* ColorTable();
	WORD& Bits();
	LONG& Height();
	LONG& Width();
};

#endif // !defined(THERE_IS_A_DIBDC_H_FILE)
