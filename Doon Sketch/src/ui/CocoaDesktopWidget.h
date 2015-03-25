//
//  CocoaDesktopManager.h
//  Inxcape
//
//  Created by 张 光建 on 14-9-1.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#import <desktop.h>
#import <sigc++/sigc++.h>
#import <ui/view/edit-widget-interface.h>
#import <gtk/gtk.h>
#import <Cocoa/Cocoa.h>
#import <sp-namedview.h>
#import <document.h>

#define WARNING_NOT_IMPLEMENTED     //{g_warning("[ %s ] was  not implemented!", __func__);}


@protocol CocoaEditWidgetDelegate <NSObject>

- (void)updateCoordinateStatus:(CGPoint)point;
- (void)showMessage:(NSString *)message;
- (NSWindow *)window;
- (NSView *)canvasView;
- (void)requestCanvasUpdate;
- (void)requestCanvasUpdateAndWait;
- (void)toggleRulers;
- (void)updateRulers;
- (void)updateScrollbars:(double)scale;

@end


class CocoaEditWidget : public Inkscape::UI::View::EditWidgetInterface {
    
public:
    
    CocoaEditWidget(id<CocoaEditWidgetDelegate> delegate);
    
    CocoaEditWidget();
    
    virtual ~CocoaEditWidget();
    
    void setDelegate(id<CocoaEditWidgetDelegate> delegate) {_delegate = delegate;}
    id getDelegate() {return _delegate;}
    
    virtual void setTitle (gchar const *uri)
    { WARNING_NOT_IMPLEMENTED }
    
    virtual Gtk::Window* getWindow()
    {
        WARNING_NOT_IMPLEMENTED
        return NULL;
    }
    
    virtual void layout() {}
    
    virtual void present()
    { WARNING_NOT_IMPLEMENTED }
    
    virtual void getGeometry (gint &x, gint &y, gint &w, gint &h);
    
    virtual void setSize (gint w, gint h);
    
    virtual void setPosition (Geom::Point p);
    virtual void setTransient (void* p, int transient_policy)
    { WARNING_NOT_IMPLEMENTED }
    virtual Geom::Point getPointer();
    virtual void setIconified()
    { WARNING_NOT_IMPLEMENTED }
    virtual void setMaximized()
    { WARNING_NOT_IMPLEMENTED }
    virtual void setFullscreen()
    { WARNING_NOT_IMPLEMENTED }
    virtual bool shutdown()
    { return FALSE; }
    virtual void destroy()
    { WARNING_NOT_IMPLEMENTED }
    
    virtual void requestCanvasUpdate();
    virtual void requestCanvasUpdateAndWait();
    virtual void enableInteraction()
    { WARNING_NOT_IMPLEMENTED }
    virtual void disableInteraction()
    { WARNING_NOT_IMPLEMENTED }
    virtual void activateDesktop()
    { WARNING_NOT_IMPLEMENTED }
    virtual void deactivateDesktop()
    { WARNING_NOT_IMPLEMENTED }
    virtual void viewSetPosition (Geom::Point p)
    { WARNING_NOT_IMPLEMENTED }
    virtual void updateScrollbars (double scale)
    { [_delegate updateScrollbars:scale]; }
    virtual void toggleScrollbars()
    { WARNING_NOT_IMPLEMENTED }
    virtual void toggleColorProfAdjust()
    { WARNING_NOT_IMPLEMENTED }
    virtual void updateZoom()
    { WARNING_NOT_IMPLEMENTED }
    virtual void letZoomGrabFocus()
    { WARNING_NOT_IMPLEMENTED }
    virtual void setToolboxFocusTo (const gchar * id)
    { WARNING_NOT_IMPLEMENTED }
    virtual void setToolboxAdjustmentValue (const gchar *id, double val)
    { WARNING_NOT_IMPLEMENTED }
    virtual void setToolboxSelectOneValue (gchar const *id, int val)
    { WARNING_NOT_IMPLEMENTED }
    virtual bool isToolboxButtonActive (gchar const* id)
    { WARNING_NOT_IMPLEMENTED return FALSE; }
    
    virtual void setCoordinateStatus (Geom::Point p);
    virtual void setMessage (Inkscape::MessageType type, gchar const* msg);
    virtual bool warnDialog (gchar* text);
    virtual void updateRulers();
    virtual void toggleRulers();
    
    virtual Inkscape::UI::Widget::Dock* getDock ()
    { WARNING_NOT_IMPLEMENTED return NULL; }
    virtual void setCursor(SPCursorType type);
    
private:
    
public:
    __weak id<CocoaEditWidgetDelegate> _delegate;
};

