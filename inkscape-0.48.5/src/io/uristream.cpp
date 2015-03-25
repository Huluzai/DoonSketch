/**
 * Our base String stream classes.  We implement these to
 * be based on Glib::ustring
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "uristream.h"
#include "sys.h"
#include <string>
#include <cstring>

#ifdef WIN32
// For now to get at is_os_wide().
# include "extension/internal/win32.h"
using Inkscape::Extension::Internal::PrintWin32;
#endif


namespace Inkscape
{
namespace IO
{

/*
 * URI scheme types
 */
#define SCHEME_NONE 0
#define SCHEME_FILE 1
#define SCHEME_DATA 2

/*
 * A temporary modification of Jon Cruz's portable fopen().
 * Simplified a bit, since we will always use binary
*/

#define FILE_READ  1
#define FILE_WRITE 2

static FILE *fopen_utf8name( char const *utf8name, int mode )
{
    FILE *fp = NULL;
    if (!utf8name)
        {
        return NULL;
        }
    if (mode!=FILE_READ && mode!=FILE_WRITE)
        {
        return NULL;
        }

#ifndef WIN32
    gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
    if ( filename ) {
        if (mode == FILE_READ)
            fp = std::fopen(filename, "rb");
        else
            fp = std::fopen(filename, "wb");
        g_free(filename);
    }
#else
    if ( PrintWin32::is_os_wide() ) {
        gunichar2 *wideName = g_utf8_to_utf16( utf8name, -1, NULL, NULL, NULL );
        if ( wideName )  {
            if (mode == FILE_READ)
                fp = _wfopen( (wchar_t*)wideName, L"rb" );
            else
                fp = _wfopen( (wchar_t*)wideName, L"wb" );
            g_free( wideName );
        } else {
            gchar *safe = Inkscape::IO::sanitizeString(utf8name);
            g_message("Unable to convert filename from UTF-8 to UTF-16 [%s]", safe);
            g_free(safe);
        }
    } else {
        gchar *filename = g_filename_from_utf8( utf8name, -1, NULL, NULL, NULL );
        if ( filename ) {
            if (mode == FILE_READ)
                fp = std::fopen(filename, "rb");
            else
                fp = std::fopen(filename, "wb");
            g_free(filename);
        }
    }
#endif

    return fp;
}



//#########################################################################
//# U R I    I N P U T    S T R E A M    /     R E A D E R
//#########################################################################


/**
 *
 */
UriInputStream::UriInputStream(Inkscape::URI &source)
   : uri(source)
{
    //get information from uri
    char const *schemestr = uri.getScheme();
    scheme = SCHEME_FILE;
    if (!schemestr || strncmp("file", schemestr, 4)==0)
        scheme = SCHEME_FILE;
    else if (strncmp("data", schemestr, 4)==0)
        scheme = SCHEME_DATA;
    //printf("in schemestr:'%s' scheme:'%d'\n", schemestr, scheme);
    gchar *cpath = NULL;

    switch (scheme) {

        case SCHEME_FILE:
            cpath = uri.toNativeFilename();
            //printf("in cpath:'%s'\n", cpath);
            inf = fopen_utf8name(cpath, FILE_READ);
            //inf = fopen(cpath, "rb");
            g_free(cpath);
            if (!inf) {
                Glib::ustring err = "UriInputStream cannot open file ";
                err += cpath;
                throw StreamException(err);
            }
        break;

        case SCHEME_DATA:
            data        = (unsigned char *) uri.getPath();
            //printf("in data:'%s'\n", data);
            dataPos     = 0;
            dataLen     = strlen((const char *)data);
        break;

    }
    closed = false;
}

/**
 *
 */
UriInputStream::UriInputStream(FILE *source, Inkscape::URI &uri)
    : inf(source),
      uri(uri)
{
    scheme = SCHEME_FILE;
    if (!inf) {
        Glib::ustring err = "UriInputStream passed NULL";
        throw StreamException(err);
    }
    closed = false;
}

/**
 *
 */
UriInputStream::~UriInputStream()
{
    close();
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */
int UriInputStream::available()
{
    return 0;
}


/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */
void UriInputStream::close()
{
    if (closed)
        return;

    switch (scheme) {

        case SCHEME_FILE:
            if (!inf)
                return;
            fflush(inf);
            fclose(inf);
            inf=NULL;
        break;

        case SCHEME_DATA:
            //do nothing
        break;

    }//switch

    closed = true;
}

/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */
int UriInputStream::get()
{
    int retVal = -1;
    if (!closed)
    {
        switch (scheme) {

            case SCHEME_FILE:
                if (!inf || feof(inf))
                {
                    retVal = -1;
                }
                else
                {
                    retVal = fgetc(inf);
                }
                break;

            case SCHEME_DATA:
                if (dataPos >= dataLen)
                {
                    retVal = -1;
                }
                else
                {
                    retVal = data[dataPos++];
                }
                break;
        }//switch
    }
    return retVal;
}






/**
 *
 */
UriReader::UriReader(Inkscape::URI &uri)
{
    inputStream = new UriInputStream(uri);
}

/**
 *
 */
UriReader::~UriReader()
{
    delete inputStream;
}

/**
 *
 */
int UriReader::available()
{
    return inputStream->available();
}

/**
 *
 */
void UriReader::close()
{
    inputStream->close();
}

/**
 *
 */
gunichar UriReader::get()
{
    gunichar ch = (gunichar)inputStream->get();
    return ch;
}


//#########################################################################
//#  U R I    O U T P U T    S T R E A M    /     W R I T E R
//#########################################################################

/**
 * Temporary kludge
 */
UriOutputStream::UriOutputStream(FILE* fp, Inkscape::URI &destination)
    : closed(false),
      ownsFile(false),
      outf(fp),
      uri(destination),
      scheme(SCHEME_FILE)
{
    if (!outf) {
        Glib::ustring err = "UriOutputStream given null file ";
        throw StreamException(err);
    }
}

/**
 *
 */
UriOutputStream::UriOutputStream(Inkscape::URI &destination)
    : closed(false),
      ownsFile(true),
      outf(NULL),
      uri(destination),
      scheme(SCHEME_FILE)
{
    //get information from uri
    char const *schemestr = uri.getScheme();
    if (!schemestr || strncmp("file", schemestr, 4)==0)
        scheme = SCHEME_FILE;
    else if (strncmp("data", schemestr, 4)==0)
        scheme = SCHEME_DATA;
    //printf("out schemestr:'%s' scheme:'%d'\n", schemestr, scheme);
    gchar *cpath = NULL;

    switch (scheme) {

        case SCHEME_FILE:
            cpath = uri.toNativeFilename();
            //printf("out path:'%s'\n", cpath);
            outf = fopen_utf8name(cpath, FILE_WRITE);
            //outf = fopen(cpath, "wb");
            g_free(cpath);
            if (!outf) {
                Glib::ustring err = "UriOutputStream cannot open file ";
                err += cpath;
                throw StreamException(err);
            }
        break;

        case SCHEME_DATA:
            data        = "data:";
        break;

    }//switch
}


/**
 *
 */
UriOutputStream::~UriOutputStream()
{
    close();
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */
void UriOutputStream::close()
{
    if (closed)
        return;

    switch (scheme) {

        case SCHEME_FILE:
            if (!outf)
                return;
            fflush(outf);
            if ( ownsFile )
                fclose(outf);
            outf=NULL;
        break;

        case SCHEME_DATA:
            uri = URI(data.raw().c_str());
        break;

    }//switch

    closed = true;
}

/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */
void UriOutputStream::flush()
{
    if (closed)
        return;

    switch (scheme) {

        case SCHEME_FILE:
            if (!outf)
                return;
            fflush(outf);
        break;

        case SCHEME_DATA:
            //nothing
        break;

    }//switch

}

/**
 * Writes the specified byte to this output stream.
 */
void UriOutputStream::put(int ch)
{
    if (closed)
        return;

    unsigned char uch;
    gunichar gch;

    switch (scheme) {

        case SCHEME_FILE:
            if (!outf)
                return;
            uch = (unsigned char)(ch & 0xff);
            if (fputc(uch, outf) == EOF) {
                Glib::ustring err = "ERROR writing to file ";
                throw StreamException(err);
            }
            //fwrite(uch, 1, 1, outf);
        break;

        case SCHEME_DATA:
            gch = (gunichar) ch;
            data.push_back(gch);
        break;

    }//switch

}





/**
 *
 */
UriWriter::UriWriter(Inkscape::URI &uri)
{
    outputStream = new UriOutputStream(uri);
}

/**
 *
 */
UriWriter::~UriWriter()
{
    delete outputStream;
}

/**
 *
 */
void UriWriter::close()
{
    outputStream->close();
}

/**
 *
 */
void UriWriter::flush()
{
    outputStream->flush();
}

/**
 *
 */
void UriWriter::put(gunichar ch)
{
    int ich = (int)ch;
    outputStream->put(ich);
}





} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
