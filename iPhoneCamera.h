#import <AVFoundation/AVFoundation.h>
#import <objc/runtime.h>

class iPhoneCamera {
    
    private:
        
        id<AVCaptureVideoDataOutputSampleBufferDelegate> _observer;
    
        AVCaptureSession *_session;
        AVCaptureDeviceInput *_videoDeviceInput;
        AVCaptureVideoDataOutput *_dataOutput;
    
        NSLock *_lock = [[NSLock alloc] init];
    
        bool _isRunning = false;
        
        void addMethod(Class cls,NSString *method,id block,const char *type,bool isClassMethod=false) {
            SEL sel = NSSelectorFromString(method);
            int ret = ([cls respondsToSelector:sel])?1:(([[cls new] respondsToSelector:sel])?2:0);
            if(ret) {
                class_addMethod(cls,(NSSelectorFromString([NSString stringWithFormat:@"_%@",(method)])),method_getImplementation(class_getInstanceMethod(cls,sel)),type);
                class_replaceMethod((ret==1)?object_getClass((id)cls):cls,sel,imp_implementationWithBlock(block),type);
            }
            else {
                class_addMethod((isClassMethod)?object_getClass((id)cls):cls,sel,imp_implementationWithBlock(block),type);
            }
        }
        
        void lock() { [this->_lock lock]; }
        void unlock() { [this->_lock unlock]; }
    
        iPhoneCamera() {
            
            objc_registerClassPair(objc_allocateClassPair(objc_getClass("NSObject"),"AVCaptureVideoDataOutputSampleBuffer",0));
            Class AVCaptureVideoDataOutputSampleBuffer = objc_getClass("AVCaptureVideoDataOutputSampleBuffer");

            this->addMethod(AVCaptureVideoDataOutputSampleBuffer,
                @"captureOutput:didOutputSampleBuffer:fromConnection:",
                ^(id me,AVCaptureOutput *captureOutput, CMSampleBufferRef sampleBuffer,AVCaptureConnection *connection) {
                
                [this->_lock lock];
                
                NSLog(@"!!!");
                
                [this->_lock unlock];
            
            },"v@:@@@");
            
            this->_observer = [[AVCaptureVideoDataOutputSampleBuffer alloc] init];
        }
    
        iPhoneCamera(const iPhoneCamera &cam) {}
        virtual ~iPhoneCamera() {}
    
    public:
        
        static iPhoneCamera *$() {
            static iPhoneCamera instance;
            return &instance;
        }
    
        void start() {
            
            if(this->_isRunning==true) this->stop();
            
            AVCaptureDevice *device = nil;
            AVCaptureDeviceDiscoverySession *session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[
                AVCaptureDeviceTypeExternal
            ] mediaType:AVMediaTypeVideo position: AVCaptureDevicePositionUnspecified];
            NSArray<AVCaptureDevice *> *devices = [session devices];
            for(int n=0; n<devices.count; n++) {
                if([devices[n].localizedName rangeOfString:@"iPhone"].location!=NSNotFound) {
                    device = devices[n];
                    break;
                }
            }
            
            if(device) {
                
                NSLog(@"%@",device.localizedName);

                AVCaptureDeviceFormat *format = nil;
                NSArray<AVCaptureDeviceFormat *> *formats = device.formats;
                for(int k=0; k<[formats count]; k++) {
                    
                    CMFormatDescriptionRef desc = formats[k].formatDescription;
                    CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(desc);
                    if(dimensions.height==1080) {
                        if(formats[k].videoSupportedFrameRateRanges[0].maxFrameRate==30.0) {
                            format = formats[k];
                            break;
                        }
                    }
                }
                
                if(format) {
                    
                    [device lockForConfiguration:nil];
                    [device setActiveFormat:format];
                    [device unlockForConfiguration];
                    
                    // NSLog(@"%@",device.activeFormat);
                    
                    this->_videoDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:device error:nullptr];
                    
                    NSDictionary *settings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA],(id)kCVPixelBufferPixelFormatTypeKey,nil];
                    
                    this->_dataOutput = [[AVCaptureVideoDataOutput alloc] init];
                    this->_dataOutput.videoSettings = settings;
                    
                    // NSLog(@"%@",settings);
                    
                    [this->_dataOutput setSampleBufferDelegate:this->_observer queue:dispatch_queue_create("iPhoneCamera",nullptr)];
                    this->_session = [[AVCaptureSession alloc] init];
                    [this->_session addInput:this->_videoDeviceInput];
                    [this->_session addOutput:this->_dataOutput];
                    this->_session.sessionPreset = AVCaptureSessionPresetHigh;
                    [this->_session startRunning];
                    this->_isRunning=true;
                }
            }
        }
    
        void stop() {
            if(this->_isRunning==true) {
                if(this->_session!=nil) [this->_session stopRunning];
                this->_session = nil;
                this->_isRunning = false;
            }
        }
};