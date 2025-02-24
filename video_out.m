@import AVFoundation;

@interface PNNVideoOut : NSObject
@property (nonatomic,strong) AVAssetWriter * writer;
@property (nonatomic,strong) AVAssetWriterInput * ain;
@property (nonatomic,strong) AVAssetWriterInput * vin;
@property (nonatomic,strong) AVAssetWriterInputPixelBufferAdaptor * vina;
@property (nonatomic) CVPixelBufferRef buf;
@property (nonatomic) bool actuallyFinished;
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
      AVSampleRateKey: @22050,
      AVNumberOfChannelsKey: @1,
      // AVEncoderBitRateKey: @128000
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
  [self.writer finishWritingWithCompletionHandler:^{
    self.actuallyFinished = YES;
  }];
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

  CMTime time = CMTimeMake(frame, 30);
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

bool vo_wait(void * p) {
  [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
  return [(__bridge PNNVideoOut *)p actuallyFinished];
}

void vo_write_audio(void * p, void * buffer, int count) {
  CMBlockBufferRef blk;
  CMBlockBufferCreateWithMemoryBlock(
    kCFAllocatorDefault,
    buffer,
    count * sizeof(float),
    kCFAllocatorNull,
    NULL,
    0,
    count * sizeof(float),
    kCMBlockBufferAssureMemoryNowFlag,
    &blk
  );

  AudioStreamBasicDescription asbd;
  asbd.mSampleRate = 22050;
  asbd.mFormatID = kAudioFormatLinearPCM;
  asbd.mChannelsPerFrame = 1;
  asbd.mFramesPerPacket = 1;
  asbd.mBitsPerChannel = 32;
  asbd.mBytesPerFrame = 4;
  asbd.mBytesPerPacket = 4;
  asbd.mFormatFlags =
    kAudioFormatFlagIsNonInterleaved |
    kAudioFormatFlagIsPacked |
    kAudioFormatFlagIsFloat;

  CMFormatDescriptionRef fmt;
  CMAudioFormatDescriptionCreate(kCFAllocatorDefault, &asbd, 0, NULL, 0, NULL, NULL, &fmt);

  CMSampleBufferRef smp;
  CMSampleBufferCreate(kCFAllocatorDefault, blk, YES, NULL, NULL, fmt, count, 0, NULL, 0, NULL, &smp);
  if (![[(__bridge PNNVideoOut *)p ain] appendSampleBuffer:smp]) {
    NSLog(@"aw.error");
    return;
  }

  CFRelease(fmt);
  CFRelease(smp);
  CFRelease(blk);
}
