#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include "files/files.h"
#include "tweaks.h"
#include "xutl.h"
#include "xstream.h"
#include "xerrhand.h"

static const char* openMSG	 = "CREATE/OPEN FAILURE";

std::fstream *open_file(const char* name, unsigned f)
{
    std::ios::openmode mode;
    mode = std::ios::binary;
    if (f & XS_IN)
        mode |= std::ios::in;
    if (f & XS_OUT)
        mode |= std::ios::out;
    if (f & XS_APPEND)
        mode |= std::ios::app;

    return new std::fstream(std::filesystem::u8path(name), mode);
}

int XStream::open(const char* name, unsigned f) {
    return XStream::open(std::string(name), f);
}

int XStream::open(const std::string& name, unsigned f)
{
    close();
    /*
	DWORD fa = 0;
	DWORD fs = 0;
	DWORD fc = 0;
	DWORD ff = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;

	if(f & XS_IN) fa |= GENERIC_READ;
	if(f & XS_OUT) fa |= GENERIC_WRITE;

	if(!(f & XS_NOSHARING)) fs |= FILE_SHARE_READ | FILE_SHARE_WRITE;

	if((f & XS_IN) || (f & XS_NOREPLACE)) fc = OPEN_EXISTING;
	else if(f & XS_OUT) fc = CREATE_ALWAYS;

	if(f & XS_NOBUFFERING) ff |= FILE_FLAG_NO_BUFFERING;

	handler = CreateFile(name,fa,fs,0,fc,ff,0);
	if(handler == INVALID_HANDLE_VALUE) {
        if (ErrHUsed) ErrH.Abort(openMSG, XERR_USER, GetLastError(), name);
        else return 0;
    }

	if(f & XS_APPEND)
		if(!SetFilePointer(handler,0,0,FILE_END))
			ErrH.Abort(appendMSG,XERR_USER,GetLastError(),name);

	fname = name;
	pos = SetFilePointer(handler,0L,0,FILE_CURRENT);
	eofFlag = 0;
     */

#ifdef XSTREAM_DEBUG
    if (!name) {
        fprintf(stderr, "ERR: XStream::open name is null\n");
    }
    fprintf(stderr, "DBG: XStream::open(\"%s\", 0x%x)\n", name, f);
#endif


    //Do a conversion to native path format
    std::string path = convert_path_native(name);

    std::fstream *file = path.empty() ? nullptr : open_file(path.c_str(), f);
    handler = file;
    if (file && file->is_open()) {
        fname = path;
        pos = file->tellg();
        eofFlag = 0;
    } else {
#ifdef XSTREAM_DEBUG
        fprintf(stderr, "ERR: XStream::open(\"%s\", 0x%x)\n", path.c_str(), f);
#endif
        if(ErrHUsed) {
            std::string smode = "File name:";
            smode += path;
            ErrH.Abort(openMSG, XERR_USER, 0, smode.c_str());
        } else {
            return 0;
        }
    }

    return 1;
}

int XStream::open(XStream* owner,int64_t s,int64_t ext_sz)
{
    close();
    free_handler = false;
	fname = owner -> fname;
	handler = owner -> handler;
	pos = 0;
	owner -> seek(s,XS_BEG);
	eofFlag = owner -> eof();
	extSize = ext_sz;
	extPos = s;
	return 1;
}

void XStream::close()
{
    /*
    if(handler == INVALID_HANDLE_VALUE) return;

	if(extSize == -1 && !CloseHandle(handler) && ErrHUsed)
		ErrH.Abort(closeMSG,XERR_USER,GetLastError(),fname);

	handler = INVALID_HANDLE_VALUE;
	fname = NULL;
	pos = 0L;
	eofFlag = 1;
	extSize = -1;
	extPos = 0;
    */
#ifdef XSTREAM_DEBUG
    fprintf(stderr, "DBG: XStream::close %s\n", fname.c_str());
#endif
    if (handler && free_handler) {
        if (handler->is_open()) {
            handler->close();
        }
        delete handler;
    }
    handler = nullptr;
    free_handler = true;
    fname.clear();
    pos = 0L;
    eofFlag = 1;
    extSize = -1;
    extPos = 0;
}

