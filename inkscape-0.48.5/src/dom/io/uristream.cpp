/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2008  Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include "uristream.h"
#include <cstdio>
#include <string.h>


namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{



//#########################################################################
//# U R I    I N P U T    S T R E A M    /     R E A D E R
//#########################################################################


/**
 *
 */
UriInputStream::UriInputStream(const URI &source)
    : uri((URI &)source)
{
    init();
}

/**
 *
 */
void UriInputStream::init()
{
    //get information from uri
    scheme = uri.getScheme();

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            DOMString npath = uri.getNativePath();
            inf = fopen(npath.c_str(), "rb");
            if (!inf)
                {
                DOMString err = "UriInputStream cannot open file ";
                err.append(npath);
                throw StreamException(err);
                }
            break;
            }

        case URI::SCHEME_DATA:
            {
            data        = uri.getPath();
            //printf("in data:'%s'\n", data.c_str());
            dataPos     = 0;
            dataLen     = uri.getPath().size();
            break;
            }

        case URI::SCHEME_HTTP:
        case URI::SCHEME_HTTPS:
            {
            if (!httpClient.openGet(uri))
                {
                DOMString err = "UriInputStream cannot open URL ";
                err.append(uri.toString());
                throw StreamException(err);
                }
            break;
            }

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

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            if (!inf)
                return;
            fflush(inf);
            fclose(inf);
            inf=NULL;
            break;
            }

        case URI::SCHEME_DATA:
            {
            //do nothing
            break;
            }

        case URI::SCHEME_HTTP:
        case URI::SCHEME_HTTPS:
            {
            httpClient.close();
            break;
            }

        }//switch

    closed = true;
}

/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */
int UriInputStream::get()
{
    int retVal = -1;
    if (closed)
        {
        return -1;
        }

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            if (!inf || feof(inf))
                {
                retVal = -1;
                }
            else
                {
                retVal = fgetc(inf);
                }
            break;
            }

        case URI::SCHEME_DATA:
            {
            if (dataPos >= dataLen)
                {
                retVal = -1;
                }
            else
                {
                retVal = data[dataPos++];
                }
            break;
            }

        case URI::SCHEME_HTTP:
        case URI::SCHEME_HTTPS:
            {
            retVal = httpClient.read();
            break;
            }

    }//switch

    return retVal;
}






/**
 *
 */
UriReader::UriReader(const URI &uri)
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
int UriReader::get()
{
    int ch = (int)inputStream->get();
    return ch;
}


//#########################################################################
//#  U R I    O U T P U T    S T R E A M    /     W R I T E R
//#########################################################################

/**
 *
 */
UriOutputStream::UriOutputStream(const URI &destination)
    : closed(false),
      ownsFile(true),
      outf(NULL),
      uri((URI &)destination)
{
    init();
}


/**
 *
 */
void UriOutputStream::init()
{
    //get information from uri
    scheme = uri.getScheme();

    //printf("out schemestr:'%s' scheme:'%d'\n", schemestr, scheme);
    char *cpath = NULL;

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            cpath     = (char *) uri.getNativePath().c_str();
            //printf("out path:'%s'\n", cpath);
            outf = fopen(cpath, "wb");
            if (!outf)
                {
                DOMString err = "UriOutputStream cannot open file ";
                err += cpath;
                throw StreamException(err);
                }
            break;
            }

        case URI::SCHEME_DATA:
            {
            data = "data:";
            break;
            }

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

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            if (!outf)
                return;
            fflush(outf);
            if ( ownsFile )
                fclose(outf);
            outf=NULL;
            break;
            }

        case URI::SCHEME_DATA:
            {
            uri = URI(data);
            break;
            }

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

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            if (!outf)
                return;
            fflush(outf);
            break;
            }

        case URI::SCHEME_DATA:
            {
            //nothing
            break;
            }

        }//switch

}

/**
 * Writes the specified byte to this output stream.
 */
int UriOutputStream::put(XMLCh ch)
{
    if (closed)
        return -1;

    switch (scheme)
        {

        case URI::SCHEME_FILE:
            {
            if (!outf)
                return -1;
            unsigned char uch = (unsigned char)(ch & 0xff);
            fputc(uch, outf);
            //fwrite(uch, 1, 1, outf);
            break;
            }

        case URI::SCHEME_DATA:
            {
            data.push_back(ch);
            break;
            }

        }//switch
    return 1;
}





/**
 *
 */
UriWriter::UriWriter(const URI &uri)
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
int UriWriter::put(XMLCh ch)
{
    int ich = (int)ch;
    if (outputStream->put(ich) < 0)
        return -1;
    return 1;
}





}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
