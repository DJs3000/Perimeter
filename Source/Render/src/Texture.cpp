#include "StdAfxRD.h"

cTexture::cTexture(const char *TexName) : cUnknownClass(KIND_TEXTURE)
{ 
	SetName(TexName);
	_x=_y=TimePerFrame=0; 
	number_mipmap=1;
	skin_color.set(255,255,255,0);
}
cTexture::~cTexture()														
{ 
	if(gb_RenderDevice)
		gb_RenderDevice->DeleteTexture(this);
	else
		VISASSERT(0 && "Текстура удалена слишком поздно");
}

int cTexture::GetNumberMipMap()
{
	return number_mipmap;
}
void cTexture::SetNumberMipMap(int number)
{
	number_mipmap=number;
}


uint8_t* cTexture::LockTexture(int& Pitch)
{
	return static_cast<uint8_t*>(gb_VisGeneric->GetRenderDevice()->LockTexture(this, Pitch));
}

void cTexture::UnlockTexture()
{
	gb_VisGeneric->GetRenderDevice()->UnlockTexture(this);
}

void cTexture::SetName(const char *Name)
{
	if(Name)name=Name;
	else name.clear();
}

void cTexture::SetWidth(int xTex)
{
	_x=ReturnBit(xTex);
	if(GetWidth()!=xTex) {
        VisError << "Bad width in texture " << name << VERR_END;
    }
}

void cTexture::SetHeight(int yTex)
{
	_y=ReturnBit(yTex);
	if(GetHeight()!=yTex) {
        VisError << "Bad height in texture " << name << VERR_END;
    }
}

