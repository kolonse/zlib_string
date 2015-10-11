#include "zlib_string.h"
#include "../../../ref/zlib/include/zlib.h"

#ifdef CHUNK
#undef CHUNK
#endif

#define CHUNK 16384

bool CZlibString::StringCompress( const string& src, string & dst ) 
{
	CZlibString compressString;
	if( Z_OK != compressString.def( src, dst, Z_DEFAULT_COMPRESSION ) )
	{
		return false ;
	}
	return true ;
}

bool CZlibString::StringUnCompress( const string& src, string & dst ) 
{
	CZlibString uncompressString;
	if( Z_OK != uncompressString.inf( src, dst ) )
	{
		return false ;
	}
	return true ;
}

int CZlibString::def( const string& src, string & dst, int level )
{
	int ret, flush;
	unsigned have;
	z_stream strm;
	//unsigned char in[CHUNK];
	unsigned char out[CHUNK];
	//int _offset = 0 ;
	const char* src_buff = src.c_str();
	int once_deal_size = 0 ;
	int src_buff_size = src.size();
	int deal_size = 0 ;
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;
    /* compress until end of file */
    do {
		flush = Z_NO_FLUSH ;
		once_deal_size = CHUNK ;
		if( src_buff_size - deal_size < CHUNK )
		{
			once_deal_size = src_buff_size - deal_size ;
			flush = Z_FINISH ;
		}
        strm.avail_in =  once_deal_size ;//fread(in, 1, CHUNK, source);
        //flush = ( once_deal_size <= 0 )? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = (Bytef*)(src_buff + deal_size) ;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			if( ret == Z_STREAM_ERROR )
			{
				(void)deflateEnd(&strm);
				return Z_STREAM_ERROR ;
			}

            have = CHUNK - strm.avail_out;
            //if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
            //    (void)deflateEnd(&strm);
            //    return Z_ERRNO;
            //}
			dst.append( (char*)out, have );
        } while (strm.avail_out == 0);
        //assert(strm.avail_in == 0);     /* all input will be used */
		if( strm.avail_in != 0 )
		{
			(void)deflateEnd(&strm);
			return Z_STREAM_ERROR ;
		}

		deal_size += once_deal_size ;
		/* done when last data in file processed */
    } while (flush != Z_FINISH);
	
    if(ret == Z_STREAM_END)       /* stream will be complete */
	{
		ret = Z_OK ;
	}
    /* clean up and return */
    (void)deflateEnd(&strm);
    return ret;
}

int CZlibString::inf( const string& src, string & dst )
{
    int ret;
    unsigned have;
    z_stream strm;
    //unsigned char in[CHUNK];
    unsigned char out[CHUNK];
	const char* src_buff = src.c_str();
	int once_deal_size = 0 ;
	int src_buff_size = src.size();
	int deal_size = 0 ;
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
		//flush = Z_NO_FLUSH ;
		once_deal_size = CHUNK ;
		if( src_buff_size - deal_size < CHUNK )
		{
			once_deal_size = src_buff_size - deal_size ;
			//flush = Z_FINISH ;
		}
        strm.avail_in =  once_deal_size ;//fread(in, 1, CHUNK, source);
        //flush = ( once_deal_size <= 0 )? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = (Bytef*)(src_buff + deal_size) ;

        //strm.avail_in = fread(in, 1, CHUNK, source);
        //if (ferror(source)) {
        //    (void)inflateEnd(&strm);
        //    return Z_ERRNO;
        //}
        if (strm.avail_in <= 0)
            break;
        //strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            //assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			if( ret == Z_STREAM_ERROR )
			{
				(void)inflateEnd(&strm);
				return Z_STREAM_ERROR ;
			}

			switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
			dst.append( (char*)out, have );
        } while (strm.avail_out == 0);
		deal_size += once_deal_size ;
        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
