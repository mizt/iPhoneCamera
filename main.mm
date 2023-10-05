#import <Cocoa/Cocoa.h>
#import "iPhoneCamera.h"
                
@interface AppDelegate:NSObject <NSApplicationDelegate> 
@end

@implementation AppDelegate
-(void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    iPhoneCamera::$()->start();
}
-(void)applicationWillTerminate:(NSNotification *)aNotification {
    iPhoneCamera::$()->stop();
}
@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
        id app = [NSApplication sharedApplication];
        id delegat = [AppDelegate alloc];
        [app setDelegate:delegat];
        [app run];
    }
}
