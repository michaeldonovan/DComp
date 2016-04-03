//
//  IKnobMultiControlText.h
//
//
//

#ifndef IKnobMultiControlText_h
#define IKnobMultiControlText_h

#include "IControl.h"
#include <string>

class IKnobMultiControlText : public IKnobMultiControl
{
private:
    IRECT mTextRECT, mImgRECT;
    IBitmap mBitmap;
    bool mShowParamLabel;
public:
    IKnobMultiControlText(IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, IText* pText, bool showParamLabel=true)
    : IKnobMultiControl(pPlug, x, y, paramIdx, pBitmap), mBitmap(*pBitmap), mShowParamLabel(showParamLabel)
    {
        mRECT = IRECT(mRECT.L, mRECT.T, mRECT.R, mRECT.B+10);
        mText = *pText;
        mTextRECT = IRECT(mRECT.L, mRECT.B-20, mRECT.R, mRECT.B);
        mImgRECT = IRECT(mRECT.L, mRECT.T, &mBitmap);
        mDisablePrompt = false;
    }
    
    ~IKnobMultiControlText() {}
    
    bool Draw(IGraphics* pGraphics)
    {
        int i = 1 + int(0.5 + mValue * (double) (mBitmap.N - 1));
        i = BOUNDED(i, 1, mBitmap.N);
        pGraphics->DrawBitmap(&mBitmap, &mImgRECT, i, &mBlend);
        //pGraphics->FillIRect(&COLOR_WHITE, &mTextRECT);
        
        char disp[20];
        mPlug->GetParam(mParamIdx)->GetDisplayForHost(disp);
        
        std::string str(disp);
        
        if (CSTR_NOT_EMPTY(disp))
        {
            if (mShowParamLabel)
            {
                str += " ";
                str += mPlug->GetParam(mParamIdx)->GetLabelForHost();
            }
            const char* cstr = str.c_str();
            return pGraphics->DrawIText(&mText, (char*)cstr, &mTextRECT);
        }
        return true;
    }
    
    void OnMouseDown(int x, int y, IMouseMod* pMod)
    {
        if (mTextRECT.Contains(x, y)) PromptUserInput(&mTextRECT);
#ifdef RTAS_API
        else if (pMod->A)
        {
            if (mDefaultValue >= 0.0)
            {
                mValue = mDefaultValue;
                SetDirty();
            }
        }
#endif
        else
        {
            OnMouseDrag(x, y, 0, 0, pMod);
        }
    }
    
    void OnMouseDblClick(int x, int y, IMouseMod* pMod)
    {
#ifdef RTAS_API
        PromptUserInput(&mTextRECT);
#else
        if (mDefaultValue >= 0.0)
        {
            mValue = mDefaultValue;
            SetDirty();
        }
#endif
    }
    
};

class IFaderControlText : public IFaderControl
{
public:
    IFaderControlText(IPlugBase* pPlug, int x, int y, int len, int paramIdx, IBitmap* pBitmap, EDirection direction, bool onlyHandle, IText* pText)
    : IFaderControl(pPlug, x, y, len, paramIdx, pBitmap, direction, onlyHandle)
    {
        mRECT = IRECT(mRECT.L, mRECT.T, mRECT.R, mRECT.B+20);
        mShowParamLabel = true;
        mText = *pText;
        mTextRECT = IRECT(mRECT.L, mRECT.B, mRECT.R, mRECT.B + 20);
        mDisablePrompt = false;

    }
    
    
    bool Draw(IGraphics* pGraphics)
    {
        IFaderControl::Draw(pGraphics);
        
        char disp[20];
        mPlug->GetParam(mParamIdx)->GetDisplayForHost(disp);
        
        std::string str(disp);
        
        if (CSTR_NOT_EMPTY(disp))
        {
            if (mShowParamLabel)
            {
                str += " ";
                str += mPlug->GetParam(mParamIdx)->GetLabelForHost();
            }
            const char* cstr = str.c_str();
            return pGraphics->DrawIText(&mText, (char*)cstr, &mTextRECT);
        }
        return true;
    }
    
    void OnMouseDrag(int x, int y, int dX, int dY, IMouseMod* pMod)
    {
        return SnapToMouse(x, y);
    }
    
    
    void OnMouseDown(int x, int y, IMouseMod* pMod)
    {
        if (mTextRECT.Contains(x, y)) PromptUserInput(&mTextRECT);
#ifdef RTAS_API
        else if (pMod->A)
        {
            if (mDefaultValue >= 0.0)
            {
                mValue = mDefaultValue;
                SetDirty();
            }
        }
#endif
        else
        {
            IFaderControl::OnMouseDown(x, y, pMod);
        }
    }
    
    void showParamLabel(bool show){
        mShowParamLabel = show;
        SetDirty();
    }

private:
    IText mText;
    IRECT mTextRECT;
    bool mShowParamLabel;
};




#endif /* IKnobMultiControlText_h */
