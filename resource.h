#define PLUG_MFR "Michael Donovan"
#define PLUG_NAME "DComp"

#define PLUG_CLASS_NAME DComp

#define BUNDLE_MFR "michaeldonovan"
#define BUNDLE_NAME "DComp"

#define PLUG_ENTRY DComp_Entry
#define PLUG_VIEW_ENTRY DComp_ViewEntry

#define PLUG_ENTRY_STR "DComp_Entry"
#define PLUG_VIEW_ENTRY_STR "DComp_ViewEntry"

#define VIEW_CLASS DComp_View
#define VIEW_CLASS_STR "DComp_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00000100
#define VST3_VER_STR "0.1.0"

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes. At least one capital letter
#define PLUG_UNIQUE_ID 'lim7'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'Dnvn'

// ProTools stuff

#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[1] = {'SCN1'};
#endif

#define PLUG_MFR_PT "michaeldonovan\nmichaeldonovan\nDnvn"
#define PLUG_NAME_PT "DComp\nlim7"
#define PLUG_TYPE_PT "Dynamics"
#define PLUG_DOES_AUDIOSUITE 1

/* PLUG_TYPE_PT can be "None", "EQ", "Dynamics", "PitchShift", "Reverb", "Delay", "Modulation", 
"Harmonic" "NoiseReduction" "Dither" "SoundField" "Effect" 
instrument determined by PLUG _IS _INST
*/

#ifdef RTAS_API
// RTAS can only have a mono sc input
// at the moment this is required instead of "2-2 3-2"
#define PLUG_CHANNEL_IO "3-2"
#define PLUG_SC_CHANS 1

#else // AU & VST2
#define PLUG_CHANNEL_IO "2-2 4-2"
#define PLUG_SC_CHANS 2
#endif

#define PLUG_LATENCY 0
#define PLUG_IS_INST 0

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 0

#define PLUG_DOES_STATE_CHUNKS 0

// Unique IDs for each image resource.
#define BACKGROUND_ID 101
#define SLIDER_ID 102
#define SLIDERHANDLES_ID 103
#define SLIDERREV_ID 104
#define SHADOW_ID 105
#define KNOB_ID 106
#define SMALLKNOB_ID 107
#define HPBUTTON_ID 108
#define LPBUTTON_ID 109
#define AUDITION_ID 110
#define BYPASS_ID 111

// Image resource locations for this plug.
#define BACKGROUND_FN "resources/img/Background.png"
#define SLIDER_FN "resources/img/Slider.png"
#define SLIDERHANDLES_FN "resources/img/SliderHandles.png"
#define SLIDERREV_FN "resources/img/SliderReverse.png"
#define SHADOW_FN "resources/img/PlotShadow.png"
#define KNOB_FN "resources/img/Knob.png"
#define SMALLKNOB_FN "resources/img/SmallKnob.png"
#define HPBUTTON_FN "resources/img/HPButton.png"
#define LPBUTTON_FN "resources/img/LPButton.png"
#define AUDITION_FN "resources/img/Audition.png"
#define BYPASS_FN "resources/img/Bypass.png"

// GUI default dimensions
#define GUI_WIDTH 629
#define GUI_HEIGHT 444

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API) && !defined(OS_IOS)
#include "app_wrapper/app_resource.h"
#endif

// vst3 stuff
#define MFR_URL "www.github.com/michaeldonovan"
#define MFR_EMAIL "info@michaeldonovan.audio"
#define EFFECT_TYPE_VST3 "Fx|Dynamics"

/* "Fx|Analyzer"", "Fx|Delay", "Fx|Distortion", "Fx|Dynamics", "Fx|EQ", "Fx|Filter",
"Fx", "Fx|Instrument", "Fx|InstrumentExternal", "Fx|Spatial", "Fx|Generator",
"Fx|Mastering", "Fx|Modulation", "Fx|PitchShift", "Fx|Restoration", "Fx|Reverb",
"Fx|Surround", "Fx|Tools", "Instrument", "Instrument|Drum", "Instrument|Sampler",
"Instrument|Synth", "Instrument|Synth|Sampler", "Instrument|External", "Spatial",
"Spatial|Fx", "OnlyRT", "OnlyOfflineProcess", "Mono", "Stereo",
"Surround"
*/
