@import AVFoundation;

@interface PNNVideoOut : NSObject
@property (nonatomic,strong) AVAssetWriter * writer;
@property (nonatomic,strong) AVAssetWriterInput * ain;
@property (nonatomic,strong) AVAssetWriterInput * vin;
@property (nonatomic,strong) AVAssetWriterInputPixelBufferAdaptor * vina;
@end

@implementation PNNVideoOut
- (id)initWithWidth:(int)w height:(int)h {
  self = [super init];
  if (self) {
    unlink("out/test.mov");
    NSURL * url = [NSURL fileURLWithPath:@"out/test.mov"];
    self.writer = [AVAssetWriter assetWriterWithURL:url
                                           fileType:AVFileTypeQuickTimeMovie
                                              error:nil];

    NSDictionary * opts = @{
      AVVideoCodecKey: AVVideoCodecTypeH264,
      AVVideoWidthKey: @(w),
      AVVideoHeightKey: @(h),
    };
    self.vin = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo
                                                  outputSettings:opts];
    self.vin.expectsMediaDataInRealTime = YES;

    opts = @{
      AVFormatIDKey: @(kAudioFormatMPEG4AAC),
      AVSampleRateKey: @44100,
      AVNumberOfChannelsKey: @1,
      AVEncoderBitRateKey: @128000
    };
    self.ain = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio
                                                  outputSettings:opts];
    self.ain.expectsMediaDataInRealTime = YES;

    opts = @{
      (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32ARGB),
      (id)kCVPixelBufferWidthKey: @(w),
      (id)kCVPixelBufferHeightKey: @(h),
      (id)kCVPixelBufferBytesPerRowAlignmentKey: @(4 * w)
    };
    self.vina = [[AVAssetWriterInputPixelBufferAdaptor alloc] initWithAssetWriterInput:self.vin
                                                           sourcePixelBufferAttributes:opts];

    [self.writer addInput:self.vin];
    [self.writer addInput:self.ain];
  }
  return self;
}
@end

void * vo_new() { return (__bridge_retained void *)[PNNVideoOut new]; }
void vo_delete(void * p) {
  PNNVideoOut * vo = (__bridge_transfer PNNVideoOut *)p;
  NSLog(@"Deallocating %@", vo);
}
