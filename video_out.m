@import AVFoundation;

@interface PNNVideoOut : NSObject
@property (nonatomic,strong) AVAssetWriter * writer;
@property (nonatomic,strong) AVAssetWriterInput * ain;
@property (nonatomic,strong) AVAssetWriterInput * vin;
@property (nonatomic,strong) AVAssetWriterInputPixelBufferAdaptor * vina;
@property (nonatomic) CVPixelBufferRef buf;
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
      (id)kCVPixelBufferHeightKey: @(h / 2),
      (id)kCVPixelBufferBytesPerRowAlignmentKey: @(4 * w)
    };
    self.vina = [[AVAssetWriterInputPixelBufferAdaptor alloc] initWithAssetWriterInput:self.vin
                                                           sourcePixelBufferAttributes:opts];

    [self.writer addInput:self.vin];
    [self.writer addInput:self.ain];
    [self.writer startWriting];
    [self.writer startSessionAtSourceTime:kCMTimeZero];
  }
  return self;
}

- (void)done {
  [self.vin markAsFinished];
  [self.ain markAsFinished];
  [self.writer finishWritingWithCompletionHandler:^{ NSLog(@"Movie is done"); }];
}

- (unsigned *)lock {
  CVPixelBufferRef buf;
  CVReturn status = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault, self.vina.pixelBufferPool, &buf);
  if (status != kCVReturnSuccess || !buf) {
    NSLog(@"Failed to acquire pixel buffer with error: %d", status);
    return nil;
  }
  self.buf = buf;

  CVPixelBufferLockBaseAddress(buf, 0);
  return CVPixelBufferGetBaseAddress(buf);
}
- (void)unlock:(unsigned)frame {
  CVPixelBufferUnlockBaseAddress(self.buf, 0);

  CMTime time = CMTimeMake(frame, 24);
  for (int i = 0; i < 30; i++) {
    if (!self.vin.readyForMoreMediaData) {
      NSLog(@"Buffer wasnt ready");
      [NSThread sleepForTimeInterval:0.05];
      continue;
    }
  }
  if (![self.vina appendPixelBuffer:self.buf withPresentationTime:time]) {
    NSLog(@"Failed to write video frame: %@", self.writer.error);
    return;
  }
  CVBufferRelease(self.buf);
  self.buf = nil;
}
@end

void * vo_new(int w, int h) {
  return (__bridge_retained void *)[[PNNVideoOut alloc] initWithWidth:w height:h];
}
void vo_delete(void * p) {
  PNNVideoOut * vo = (__bridge_transfer PNNVideoOut *)p;
  NSLog(@"Deallocating %@", vo);
}

unsigned * vo_lock(void * p) {
  return [(__bridge PNNVideoOut *)p lock];
}
void vo_unlock(void * p, unsigned frame) {
  [(__bridge PNNVideoOut *)p unlock:frame];
}

void vo_done(void * p) {
  [(__bridge PNNVideoOut *)p done];
}

void vo_wait() {
  [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:3]];
}
