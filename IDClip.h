//
//  IDClip.h
//  DClip
//
//  Created by Michael Donovan on 3/23/16.
//  All rights reserved
//

#ifndef IDClip_h
#define IDClip_h

#include "IControl.h"
#include <vector>
#include <cairo.h>

using std::vector;

struct node{
    double val;
    node* next;
};

class IDClip : public IControl
{
public:
    IDClip(IPlugBase* pPlug, IRECT pR, IColor backgroundColor, IColor fillColor, IColor lineColor, int paramIdx, int timeScale, double sampleRate) : IControl(pPlug, pR), mColorBG(backgroundColor), mColorFill(fillColor), mColorLine(lineColor), mParam(paramIdx), sr(sampleRate), mInBufferLength(0) {
        
        width = static_cast<int>(mRECT.W());
        height = static_cast<int>(mRECT.H());
        mDrawBufferLength = width / 5;
        mInBufferMaxLength = (timeScale * sr) / mDrawBufferLength;
        
        mInBuffer = new vector<double>(mInBufferMaxLength, 0);
        mDrawBuffer =  new vector<double>(mDrawBufferLength, percentToCoordinates(0));
        


        
        //        mInBuffFirst = new node;
        //        mInBuffFirst->val = 0.;
        //        mInBuffLast = mInBuffFirst;
        //
        //        mDrawBuffFirst = new node;
        //        mDrawBuffFirst->val = 0.;
        //        mDrawBuffLast = mDrawBuffFirst;
        //
        //        for (int i = 1; i<mDrawBufferLength; i++) {
        //            node* newNode = new node;
        //            newNode->val = 0.;
        //            mDrawBuffLast->next = newNode;
        //            mDrawBuffLast = newNode;
        //        }
        
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->mRECT.W(), this->mRECT.H());
        cr = cairo_create(surface);
    }
    
    ~IDClip(){
        //        for(int i=0; i<mInBufferLength; i++){
        //            if(mInBuffFirst == mInBuffLast){
        //                free(mInBuffFirst);
        //            }
        //            else{
        //                node* temp = mInBuffFirst->next;
        //                free(mInBuffFirst);
        //                mInBuffFirst = temp;
        //            }
        //        }
        //
        //        for(int i=0; i<mDrawBufferLength; i++){
        //            if(mDrawBuffFirst == mDrawBuffLast){
        //                free(mDrawBuffFirst);
        //            }
        //            else{
        //                node* temp = mDrawBuffFirst->next;
        //                free(mDrawBuffFirst);
        //                mDrawBuffFirst = temp;
        //            }
        //}
        
        delete mInBuffer;
        delete mDrawBuffer;
        
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }
    
    bool Draw(IGraphics* pGraphics){
        cairo_save(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);
        
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        cr = cairo_create(surface);
        
        cairo_set_source_rgba(cr, .8, 0., 0., 0.75);
        cairo_set_line_width(cr, 6.);
        cairo_move_to(cr, 0, height);
        
        //    node* current = mDrawBuffFirst;
        //    for (int i=2; i<width; i+=2) {
        //        cairo_line_to(cr, i, percentToCoordinates(current->val));
        //
        //        if (current == mDrawBuffLast) {
        //            break;
        //        }
        //        else{
        //            current = current->next;
        //        }
        //    }
        
        for(int i=0; i<mDrawBufferLength; i++) {
            cairo_line_to(cr, 5 * i, mDrawBuffer->at(i));
        }
        
       
        
        cairo_line_to(cr, width, height);
        
        cairo_close_path(cr);
        
        cairo_path_t *path = cairo_copy_path(cr);
        
        cairo_stroke(cr);
        
        cairo_append_path(cr, path);
        
        cairo_set_source_rgba(cr, 1, 0x97 / 255., 0x97 / 255., 0x97 / 255.);

        cairo_fill(cr);
        
        cairo_path_destroy(path);
        
        
        //cairo_fill(cr);

        cairo_surface_flush(surface);
        
        unsigned int *data = (unsigned int*)cairo_image_surface_get_data(surface);
        // This is the important part where you bind the cairo data to LICE
        LICE_WrapperBitmap WrapperBitmap = LICE_WrapperBitmap(data, this->mRECT.W(), this->mRECT.H(), this->mRECT.W(), false);
        
        // And we render
        IBitmap result(&WrapperBitmap, WrapperBitmap.getWidth(), WrapperBitmap.getHeight());
        return pGraphics->DrawBitmap(&result, &this->mRECT);
        
        
    }
    
    
    bool IsDirty(){
        return true;
    }
    
    void process(double sample){
        //    node* newNode = new node;
        //    newNode->val = sample;
        //
        //    mInBuffLast->next = newNode;
        //    mInBuffLast = newNode;
        
        val = sample;
        
        mInBuffer->at(mInBufferLength) = sample;
        mInBufferLength++;
        
        if (mInBufferLength >= mInBufferMaxLength) {
            double sum = 0;
            for (int i=0; i<mInBuffer->size(); i++) {
                sum += mInBuffer->at(i);
            }
            //sum /= mInBufferMaxLength;
            
            //addToDrawBuffer(percentToCoordinates(sum));
            
            
            //TEST
            addToDrawBuffer(percentToCoordinates(sample));
            //TEST
            
            
            //        node* temp = mDrawBuffFirst->next;
            //        free(mDrawBuffFirst);
            //        mDrawBuffFirst = temp;
            //
            //        node* newNode2 = new node;
            //        newNode2->val = sum(mInBuffFirst) / (double)mInBufferMaxLength;
            //        mDrawBuffLast->next = newNode2;
            //        mDrawBuffLast = newNode2;
            
            mInBufferLength = 0;
            //        resetInBuff();
        }
        
        SetDirty(true);
    }
    
protected:
    cairo_surface_t *surface;
    cairo_t *cr;
    IColor mColorBG, mColorFill, mColorLine;
    int mParam, width, height, mDrawBufferLength, mInBufferLength, mInBufferMaxLength;
    double sr;
    node* mInBuffFirst;
    node* mInBuffLast;
    node* mDrawBuffFirst;
    node* mDrawBuffLast;
    std::vector<double> *mDrawBuffer;
    std::vector<double> *mInBuffer;
    double val = 0;
    
    double percentToCoordinates(double value) {
        return height - value * height;
    }
    
    void addToDrawBuffer(double val){
        int size = mDrawBuffer->size();
        for (int i=0; i< (size-1); i++) {
            mDrawBuffer->at(i) = mDrawBuffer->at(i+1);
        }
        mDrawBuffer->at(mDrawBuffer->size()) = val;
    }
    
    double sumNodes(node* n){
        if (n == mInBuffLast) {
            return mInBuffLast->val;
        }
        else{
            return n->val + sumNodes(n->next);
        }
    }
    
//    void resetInBuff(){
//        while(mInBuffFirst != mInBuffLast){
//            node* temp = mInBuffFirst->next;
//            free(mInBuffFirst);
//            mInBuffFirst = temp;
//        }
//        if(mInBuffFirst == mInBuffLast){
//            mInBuffFirst->val = 0.;
//        }
//    }
};

#endif /* IDClip_h */
