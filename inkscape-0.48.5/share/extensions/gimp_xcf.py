#!/usr/bin/env python 
'''
Copyright (C) 2006 Aaron Spike, aaron@ekips.org
Copyright (C) 2010-2011 Nicolas Dufour, nicoduf@yahoo.fr (Windows support and various fixes)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''
import inkex
import sys, os, tempfile, shutil
import gettext

try:
    from subprocess import Popen, PIPE
    bsubprocess = True
except:
    bsubprocess = False

class MyEffect(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("-d", "--guides",
                                         action="store", type="inkbool",
                                         dest="saveGuides", default=False,
                                         help="Save the Guides with the .XCF")
        self.OptionParser.add_option("-r", "--grid",
                                         action="store", type="inkbool",
                                         dest="saveGrid", default=False,
                                         help="Save the Grid with the .XCF")

    def output(self):
        pass

    def clear_tmp(self):
        shutil.rmtree(self.tmp_dir)

    def effect(self):
        svg_file = self.args[-1]
        ttmp_orig = self.document.getroot()
        docname = ttmp_orig.get(inkex.addNS('docname',u'sodipodi'))
        if docname is None: docname = self.args[-1]

        pageHeight = int(inkex.unittouu(self.xpathSingle('/svg:svg/@height').split('.')[0]))
        pageWidth = int(inkex.unittouu(self.xpathSingle('/svg:svg/@width').split('.')[0]))

        #create os temp dir
        self.tmp_dir = tempfile.mkdtemp()

        # Guides
        hGuides = []
        vGuides = []
        valid = 0
        if self.options.saveGuides:
            guideXpath = "sodipodi:namedview/sodipodi:guide" #grab all guide tags in the namedview tag
            for guideNode in self.document.xpath(guideXpath, namespaces=inkex.NSS):
                ori = guideNode.get('orientation')
                if  ori == '0,1':
                    #this is a horizontal guide
                    pos = int(guideNode.get('position').split(',')[1].split('.')[0])
                    #GIMP doesn't like guides that are outside of the image
                    if pos > 0 and pos < pageHeight:
                        #the origin is at the top in GIMP land
                        hGuides.append(str(pageHeight - pos))
                elif ori == '1,0':
                    #this is a vertical guide
                    pos = int(guideNode.get('position').split(',')[0].split('.')[0])
                    #GIMP doesn't like guides that are outside of the image
                    if pos > 0 and pos < pageWidth:
                        vGuides.append(str(pos))

        hGList = ' '.join(hGuides)
        vGList = ' '.join(vGuides)

        # Grid
        gridSpacingFunc = ''
        gridOriginFunc = '' 
        #GIMP only allows one rectangular grid
        gridXpath = "sodipodi:namedview/inkscape:grid[@type='xygrid' and (not(@units) or @units='px')]"
        if (self.options.saveGrid and self.document.xpath(gridXpath, namespaces=inkex.NSS)):
            gridNode = self.xpathSingle(gridXpath)
            if gridNode != None:
                #these attributes could be nonexistant
                spacingX = gridNode.get('spacingx')
                if spacingX == None: spacingX = '1  '

                spacingY = gridNode.get('spacingy')
                if spacingY == None: spacingY = '1  '

                originX = gridNode.get('originx')
                if originX == None: originX = '0  '

                originY = gridNode.get('originy')
                if originY == None: originY = '0  '

                gridSpacingFunc = '(gimp-image-grid-set-spacing img %s %s)' % (spacingX[:-2], spacingY[:-2])
                gridOriginFunc = '(gimp-image-grid-set-offset img %s %s)'% (originX[:-2], originY[:-2])

        # Layers
        area = '--export-area-page'
        pngs = []
        names = []
        path = "/svg:svg/*[name()='g' or @style][@id]"
        for node in self.document.xpath(path, namespaces=inkex.NSS):
            if len(node) > 0: # Get rid of empty layers
                valid=1
                id = node.get('id')
                if node.get("{" + inkex.NSS["inkscape"] + "}label"):
                    name = node.get("{" + inkex.NSS["inkscape"] + "}label")
                else:
                    name = id
                filename = os.path.join(self.tmp_dir, "%s.png" % id)
                command = "inkscape -i \"%s\" -j %s -e \"%s\" %s " % (id, area, filename, svg_file)
                if bsubprocess:
                    p = Popen(command, shell=True, stdout=PIPE, stderr=PIPE)
                    return_code = p.wait()
                    f = p.stdout
                    err = p.stderr
                else:
                    _,f,err = os.popen3(command,'r')
                f.read()
                f.close()
                err.close()
                if os.name == 'nt':
                    filename = filename.replace("\\", "/")
                pngs.append(filename)
                names.append(name.encode('utf-8'))

        if (valid==0):
            inkex.errormsg(gettext.gettext('This extension requires at least one non empty layer.'))
            self.clear_tmp()
            sys.exit(0)

        filelist = '"%s"' % '" "'.join(pngs)
        namelist = '"%s"' % '" "'.join(names)
        xcf = os.path.join(self.tmp_dir, "%s.xcf" % docname)
        if os.name == 'nt':
            xcf = xcf.replace("\\", "/")
        script_fu = """
(tracing 1)
(define
  (png-to-layer img png_filename layer_name)
  (let*
    (
      (png (car (file-png-load RUN-NONINTERACTIVE png_filename png_filename)))
      (png_layer (car (gimp-image-get-active-layer png)))
      (xcf_layer (car (gimp-layer-new-from-drawable png_layer img)))
    )
    (gimp-image-add-layer img xcf_layer -1)
    (gimp-drawable-set-name xcf_layer layer_name)
  )
)
(let*
  (
    (img (car (gimp-image-new 200 200 RGB)))
  )
  (gimp-image-undo-disable img)
  (for-each
    (lambda (names)
      (png-to-layer img (car names) (cdr names))
    )
    (map cons '(%s) '(%s))
  )

  (gimp-image-resize-to-layers img)

  (for-each
    (lambda (hGuide)
      (gimp-image-add-hguide img hGuide)
    )
    '(%s)
  )

  (for-each
    (lambda (vGuide)
      (gimp-image-add-vguide img vGuide)
    )
    '(%s)
  )

  %s
  %s

  (gimp-image-undo-enable img)
  (gimp-file-save RUN-NONINTERACTIVE img (car (gimp-image-get-active-layer img)) "%s" "%s"))
(gimp-quit 0)
        """ % (filelist, namelist, hGList, vGList, gridSpacingFunc, gridOriginFunc, xcf, xcf)

        junk = os.path.join(self.tmp_dir, 'junk_from_gimp.txt')
        f = os.popen('gimp -i --batch-interpreter plug-in-script-fu-eval -b - > %s 2>&1' % junk,'w')
        f.write(script_fu)
        f.close()
        # uncomment these lines to see the output from gimp
        #err = open(junk, 'r')
        #inkex.debug(err.read())
        #err.close()

        x = open(xcf, 'rb')
        if os.name == 'nt':
            try:
                import msvcrt
                msvcrt.setmode(1, os.O_BINARY)
            except:
                pass
        sys.stdout.write(x.read())
        x.close()
        self.clear_tmp()


if __name__ == '__main__':
    e = MyEffect()
    e.affect()

