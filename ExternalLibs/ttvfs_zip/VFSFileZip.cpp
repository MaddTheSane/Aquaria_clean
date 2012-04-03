#include "VFSFileZip.h"
#include "VFSInternal.h"
#include "VFSTools.h"

VFS_NAMESPACE_START


VFSFileZip::VFSFileZip(mz_zip_archive *zip)
: VFSFile(NULL), _fixedStr(NULL), _zip(zip)
{
    _mode = "b"; // binary mode by default
    _pos = 0;
}

void VFSFileZip::_init()
{
    _setName(_zipstat.m_filename);
}

VFSFileZip::~VFSFileZip()
{
    dropBuf(true);
}

bool VFSFileZip::open(const char *mode /* = NULL */)
{
    VFS_GUARD_OPT(this);

    _pos = 0;
    if(mode)
    {
        if(_fixedStr && _mode != mode)
        {
            delete [] _fixedStr;
            _fixedStr = NULL;
        }

        _mode = mode;
    }
    return true; // does not have to be opened
}

bool VFSFileZip::isopen(void) const
{
    return true; // is always open
}

bool VFSFileZip::iseof(void) const
{
    VFS_GUARD_OPT(this);
    return _pos >= _zipstat.m_uncomp_size;
}

bool VFSFileZip::close(void)
{
    return flush(); // TODO: write to zip file on close
}

bool VFSFileZip::seek(vfspos pos)
{
    if(pos >= 0xFFFFFFFF) // zip files have uint32 range only
        return false;

    VFS_GUARD_OPT(this);
    _pos = (unsigned int)pos;
    return true;
}

bool VFSFileZip::flush(void)
{
    // FIXME: use this to actually write to zip file?
    return false;
}

vfspos VFSFileZip::getpos(void) const
{
    VFS_GUARD_OPT(this);
    return _pos;
}

unsigned int VFSFileZip::read(void *dst, unsigned int bytes)
{
    VFS_GUARD_OPT(this);
    char *mem = (char*)getBuf();
    char *startptr = mem + _pos;
    char *endptr = mem + size();
    bytes = std::min((unsigned int)(endptr - startptr), bytes); // limit in case reading over buffer size
    if(_mode.find('b') == std::string::npos)
        strnNLcpy((char*)dst, (const char*)startptr, bytes); // non-binary == text mode
    else
        memcpy(dst, startptr, bytes); //  binary copy
    _pos += bytes;
    return bytes;
}

unsigned int VFSFileZip::write(const void *src, unsigned int bytes)
{
    /*VFS_GUARD_OPT(this);
    if(getpos() + bytes >= size())
        size(getpos() + bytes); // enlarge if necessary

    memcpy(_buf + getpos(), src, bytes);

    // TODO: implement actually writing to the Zip file.

    return bytes;*/

    return VFSFile::write(src, bytes);
}

vfspos VFSFileZip::size(void)
{
    VFS_GUARD_OPT(this);
    return (vfspos)_zipstat.m_uncomp_size;
}

vfspos VFSFileZip::size(vfspos newsize)
{
    // TODO: write me
    /*
    VFS_GUARD_OPT(this);
    if(newsize == size())
        return newsize;

    _zipstat.m_uncomp_size = newsize;
    void *newbuf = allocHelper(...); // TODO: need to know last used allocator too?
    return n;
    */

    return VFSFile::size(newsize);
}

const void *VFSFileZip::getBuf(allocator_func alloc /* = NULL */, delete_func del /* = NULL */)
{
    assert(!alloc == !del); // either both or none may be defined. Checked extra early to prevent possible errors later.

    VFS_GUARD_OPT(this);
    // _fixedStr gets deleted on mode change, so doing this check here is fine
    if(_fixedStr)
        return _fixedStr;

    if(!_buf)
    {
        size_t sz = (size_t)size();
        _buf = allocHelperExtra(alloc, sz, 4);
        if(!_buf)
            return NULL;
        _delfunc = del;
        mz_zip_reader_extract_to_mem(_zip, _zipstat.m_file_index, _buf, sz, 0);

        if(_mode.find("b") == std::string::npos) // text mode?
        {
            _fixedStr = allocHelperExtra(alloc, sz, 4);
            strnNLcpy(_fixedStr, (const char*)_buf);

            // FIXME: is this really correct?
            VFSFile::dropBuf(true);

            return _fixedStr;
        }

    }

    return _buf;
}

void VFSFileZip::dropBuf(bool del)
{
    VFSFile::dropBuf(del);

    VFS_GUARD_OPT(this);
    if(del)
        delBuf(_fixedStr);
    _fixedStr = NULL;

}

VFS_NAMESPACE_END
