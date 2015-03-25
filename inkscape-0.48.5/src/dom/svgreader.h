#ifndef __SVGREADER_H__
#define __SVGREADER_H__

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
 * Copyright (C) 2005-2008 Bob Jamison
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
 *  
 * =======================================================================
 * NOTES
 * 
 * This parser takes an XML document, as a file, string, or DOM Document,
 * and attempts to parse it as an SVG-DOM  SVGDocument.
 * 
 * Look in svg.h and svgtypes.h for the classes which are the target of this
 * parser.
 * 
 * They loosely follow the specification:
 * http://www.w3.org/TR/SVG11/svgdom.html
 *              
 */


#include "svgimpl.h"

namespace org
{
namespace w3c
{
namespace dom
{
namespace svg
{


class SVGReader
{
public:

    /**
     *
     */
    SVGReader()
        {
        }

    /**
     *
     */
    SVGReader(const SVGReader &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~SVGReader()
        {
        }

    /**
     *
     */
    SVGDocumentPtr parse(const DocumentPtr /*sourceDoc*/);

    /**
     *
     */
    SVGDocumentPtr parse(const DOMString &/*buffer*/);

    /**
     *
     */
    SVGDocumentPtr parseFile(const DOMString &/*fileName*/);




private:

    /**
     *  Get the next character in the parse buf,  0 if out
     *  of range
     */
    XMLCh get(int p);

    /**
     *  Test if the given substring exists at the given position
     *  in parsebuf.  Use get() in case of out-of-bounds
     */
    bool match(int pos, char const *str);

    /**
     *
     */
    int skipwhite(int p);

    /**
     * get a word from the buffer
     */
    int getWord(int p, DOMString &result);

    /**
     * get a word from the buffer
     */
    int getNumber(int p0, double &result);


    /**
     *
     */
    bool parseTransform(const DOMString &str);


    /**
     *
     */
    bool parseElement(SVGElementImplPtr destElem,
                      ElementImplPtr sourceElem);


    /**
     *
     */
    void error(char const *format, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    /**
     *
     */
    void trace(char const *format, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;



    DOMString parsebuf;
    int parselen;
    int lastPosition;

    SVGDocumentImplPtr doc;

};





}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif /* __SVGREADER_H__ */
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/



