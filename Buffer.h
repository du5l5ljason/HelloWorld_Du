#pragma once
#include "memory.h"
class BaseBuf
{
protected:
	int m_width;
	int m_height;
	int m_widthBytes;
	BYTE* m_data;

public:
	const int width() {return m_width;};
	const int height() {return m_height;};
	const int widthBytes() {return m_widthBytes;};
	BYTE* getData() {return m_data;};

	void zeroBuffer() {
		if(m_data)
			memset(m_data, 0, sizeof(BYTE)*height()*widthBytes());
	}
	void copyBuffer(BYTE* pData) {
		memcpy(m_data, pData, sizeof(BYTE)*height()*widthBytes());
	}
	void copyBuffer(int w, int h, int wb, BYTE* pData) {
		if(w<=0 || h<=0 || wb<=0 || pData == NULL) return;
		if( w != width() || h != height() || wb != widthBytes() ) {
			m_width = w; m_height = h; m_widthBytes = wb;
			if( m_data != NULL ) delete [] m_data;
			m_data = new BYTE[h*wb];
		}
		memcpy(m_data, pData, sizeof(BYTE)*height()*widthBytes());
	}

	void copyBuffer(BaseBuf& buf) {
		copyBuffer(buf.width(), buf.height(), buf.widthBytes(), buf.getData());
	}


	virtual void getPixelAt(int row, int col, BYTE p[])=0;
	virtual unsigned short getPixel16At(int row, int col) { return 0; };
	virtual unsigned short getPixel24At(int row, int col) { return 0; };
	

public:
	BaseBuf(void) : m_width(0), m_height(0), m_widthBytes(0), m_data(NULL) {};
	BaseBuf(int w, int h, int wb): m_width(w), m_height(h), m_widthBytes(wb) {
		m_data = NULL;
		if( w<=0 || h<=0 || wb<=0)
			return;
		m_data = new BYTE[h*wb];
	};

	~BaseBuf(void) {
		if(m_data!=NULL)
			delete [] m_data;
	};
};

class Buffer24: public BaseBuf {
public:
	Buffer24(void) : BaseBuf() {};
	Buffer24(int w, int h, int wb) : BaseBuf(w, h, wb) {};

	void getPixelAt(int row, int col, BYTE p[]) {
		if( row<0 || col < 0 || row>=width() ||col>=height()) return;
		p[0] = m_data[row*widthBytes()+col*3];
		p[1] = m_data[row*widthBytes()+col*3+1];
		p[2] = m_data[row*widthBytes()+col*3+2];
	}

	unsigned short getPixel24At(int row, int col){
		BYTE* p = m_data + row*widthBytes()+col*3;
		return(unsigned short)(*p);
	}
};

class Buffer16: public BaseBuf {
public:
	Buffer16(void) : BaseBuf() {};
	Buffer16(int w, int h, int wb) : BaseBuf(w, h, wb) {};
	void getPixelAt(int row, int col, BYTE p[]) {
		if( row<0 || col < 0 || row>=width() ||col>=height()) return;
		p[0] = m_data[row*widthBytes()+col*2];
		p[1] = m_data[row*widthBytes()+col*2+1];
	}
	unsigned short getPixel16At(int row, int col) {			
		BYTE r = m_data[row*widthBytes()+col*2];
		BYTE g = m_data[row*widthBytes()+col*2+1];
		return (unsigned short)(g)*256+r;
	}
};

class Buffer8: public BaseBuf {
public:
	Buffer8(void) : BaseBuf() {};
	Buffer8(int w, int h, int wb) : BaseBuf(w, h, wb) {};
	void getPixelAt(int row, int col, BYTE p[]) {
		if( row<0 || col < 0 || row>=width() ||col>=height()) return;
		p[0] = m_data[row*widthBytes()+col];
	}
};
