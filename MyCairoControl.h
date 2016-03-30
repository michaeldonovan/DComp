#ifndef __MY_CAIRO_CONTROL_H__
#define __MY_CAIRO_CONTROL_H__

// In case you forget it
//#ifndef CAIRO_WIN32_STATIC_BUILD
//#define CAIRO_WIN32_STATIC_BUILD
//#endif

#include <cairo.h>

#include "IControl.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class MyCairoControl : public IControl {
protected:
    cairo_surface_t *surface;
    cairo_t *cr;
    
public:
    MyCairoControl(IPlugBase *pPlug, IRECT pR) : IControl(pPlug, pR) {
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->mRECT.W(), this->mRECT.H());
        cr = cairo_create(surface);
    };
    ~MyCairoControl() {
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    };
    
    bool IsDirty() { return true; };
    
    bool Draw2(IGraphics* pGraphics) {
        // We clear the surface
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        // This parameter (unused for now) will be used soon for the button
        double position = 0.;
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->mRECT.W(), this->mRECT.H());
        cr = cairo_create(surface);
        
        // We define the main color to use, a red color
        cairo_set_source_rgba(cr, 0.8, 0., 0., 0.75);
        
        // We draw two bezier lines with one point at top
        cairo_set_line_width(cr, 4.);
        
        // First bezier, 0 to middle of graphic
        // NOTE that 0:0 coordinate is the top/left corner
        // And cairo is position to 0 not where the IControl is placed
        // So the bottom/left is at 0 - height position
        double width = (double)this->mRECT.W();
        double height = (double)this->mRECT.H();
        
        // This is the 0 - half X path
        cairo_move_to(cr, 0, height);
        cairo_curve_to(cr, width / 4, height, width / 4, position, width / 2, position);
        
        // This is the half - full width X path
        cairo_move_to(cr, width / 2, position);
        cairo_curve_to(cr, 3 * width / 4, position, 3 * width / 4, height, width, height);
        
        // Render
        cairo_stroke(cr);
        
        // Now we draw the center points
        // We make it more red
        cairo_set_source_rgba(cr, 1, 0., 0., 0.75);
        cairo_set_line_width(cr, 1.);
        // Make the circle and fill it
        cairo_arc(cr, width / 2, position, 4., 0, 2 * M_PI);
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
        
        
        // And we render
        cairo_surface_flush(surface);
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        // This is the important part where you bind the cairo data to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        // And we render
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
        return pGraphics->DrawBitmap(&result, &this->mRECT);
    };
    
    bool Draw(IGraphics* pGraphics){
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->mRECT.W(), this->mRECT.H());
        cr = cairo_create(surface);
        
        // We define the main color to use, a red color
        cairo_set_source_rgba(cr, .8, 0., 0., 0.75);
        
        // We draw two bezier lines with one point at top
        cairo_set_line_width(cr, 4.);
        
        cairo_new_path(cr);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 100, 100);
        cairo_line_to(cr, 150, 100);
        cairo_line_to(cr, 100, 200);
        
        cairo_close_path(cr);
        
        cairo_path_t *path = cairo_copy_path(cr);
        
        cairo_stroke(cr);
        
        cairo_append_path(cr, path);

        cairo_fill(cr);
        
        cairo_path_destroy(path);
        
        // And we render
        cairo_surface_flush(surface);
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        // This is the important part where you bind the cairo data to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        // And we render
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
        return pGraphics->DrawBitmap(&result, &this->mRECT);
        
    };
};

#endif
