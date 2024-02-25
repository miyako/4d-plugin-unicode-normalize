/* --------------------------------------------------------------------------------
 #
 #  4DPlugin-Unicode-Normalize.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : Unicode Normalize
 #	author : miyako
 #	2024/02/26
 #  
 # --------------------------------------------------------------------------------*/

#include "4DPlugin-Unicode-Normalize.h"

#pragma mark -

void PluginMain(PA_long32 selector, PA_PluginParameters params) {
    
	try
	{
        switch(selector)
        {
			// --- Unicode Normalize
            
			case 1 :
				Unicode_normalize(params);
				break;

        }

	}
	catch(...)
	{

	}
}

#pragma mark -

#if VERSIONWIN
static void _ns(normalization_form_t mode, C_TEXT& src, C_TEXT& dst) {
    
    int len = NormalizeString(
                              (NORM_FORM)mode,
                              (LPCWSTR)src.getUTF16StringPtr(),
        src.getUTF16Length(),
                              NULL,
                              0
                              );
    if (len == 0) {
        dst.setUTF16String(src.getUTF16StringPtr(), src.getUTF16Length());
            return;
    }

    
    std::vector<wchar_t>buf(len);
    
    NormalizeString(
                          (NORM_FORM)mode,
                          (LPCWSTR)src.getUTF16StringPtr(),
        src.getUTF16Length(), 
                          &buf[0],
        len
                          );
    
    dst.setUTF16String((const PA_Unichar *)&buf[0], len);
}
#endif

#if VERSIONMAC
static void _ns(normalization_form_t mode, C_TEXT& src, C_TEXT& dst) {
    
    NSString *_src = src.copyUTF16String();
    NSString *_dsc;
    
    switch (mode) {
        case normalization_form_nfd:
            _dsc= [_src decomposedStringWithCanonicalMapping];
            break;
        case normalization_form_nfc:
            _dsc = [_src precomposedStringWithCompatibilityMapping];
            break;
        case normalization_form_nfkd:
            _dsc = [_src decomposedStringWithCompatibilityMapping];
            break;
        case normalization_form_nfkc:
        default:
            _dsc = [_src precomposedStringWithCanonicalMapping];
            break;
    }
            
    dst.setUTF16String(_dsc);
    
    [_src release];
}
#endif

static void Unicode_normalize(PA_PluginParameters params) {

    sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
    PackagePtr pParams = (PackagePtr)params->fParameters;
   
    C_TEXT src;
    C_TEXT dst;
    
    src.fromParamAtIndex(pParams, 1);
    
    normalization_form_t mode = (normalization_form_t)PA_GetLongParameter(params, 2);
        
    _ns(mode, src, dst);
    
    dst.setReturn(pResult);
}

