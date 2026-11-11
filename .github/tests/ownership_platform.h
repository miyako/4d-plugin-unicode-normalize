#import <Cocoa/Cocoa.h>

@interface OwnershipURL : NSObject {
    NSString *_text;
}
- (id)initWithString:(NSString *)text;
- (NSString *)absoluteString;
@end
CFURLRef OwnershipCreateURL(CFAllocatorRef, CFStringRef, CFURLPathStyle, Boolean);
CFStringRef OwnershipCopyPath(CFURLRef, CFURLPathStyle);
void OwnershipScale(CGContextRef, CGFloat, CGFloat);
void OwnershipTranslate(CGContextRef, CGFloat, CGFloat);

#define NSURL OwnershipURL
#define CFURLCreateWithFileSystemPath OwnershipCreateURL
#define CFURLCopyFileSystemPath OwnershipCopyPath
#define CGContextScaleCTM OwnershipScale
#define CGContextTranslateCTM OwnershipTranslate
