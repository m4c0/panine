@import AVFoundation;

@interface PNNMovie : NSObject 
@property (nonatomic, strong) AVMovie * mov;
@property (nonatomic, strong) AVAssetReader * rdr;
@property (nonatomic, strong) AVAssetReaderTrackOutput * out;
@property (nonatomic) CMSampleBufferRef smp;
@property (nonatomic) CVPixelBufferRef img;
@end

@implementation PNNMovie
@end

void * mov_alloc(void * pstr, unsigned plen) {
  PNNMovie * res = [PNNMovie new];

  NSString * path = [[NSString alloc] initWithBytesNoCopy:pstr
                                                   length:plen
                                                 encoding:NSASCIIStringEncoding
                                             freeWhenDone:false];
  NSURL * url = [NSURL fileURLWithPath:path];
  res.mov = [AVMovie movieWithURL:url options:nil];

  NSDictionary * opts = @{
    (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
  };
  AVAssetTrack * trk = [[res.mov tracksWithMediaType:AVMediaTypeVideo] firstObject];
  res.out = [[AVAssetReaderTrackOutput alloc] initWithTrack:trk outputSettings:opts];

  res.rdr = [[AVAssetReader alloc] initWithAsset:res.mov error:nil];
  [res.rdr addOutput:res.out];
  [res.rdr startReading];

  return (__bridge_retained void *)res;
}

void mov_dealloc(void * ptr) {
  id mov = (__bridge_transfer PNNMovie *)ptr;
  NSLog(@"Deallocating %@", mov);
}
void mov_skip(PNNMovie * m, int frames) {
  for (int i = 0; i < frames; i++) {
    [m.out copyNextSampleBuffer];
  }
}
int mov_begin_frame(PNNMovie * m) {
  m.smp = [m.out copyNextSampleBuffer];
  m.img = (CVPixelBufferRef)CMSampleBufferGetImageBuffer(m.smp);

  CVPixelBufferLockBaseAddress(m.img, kCVPixelBufferLock_ReadOnly);
  return CMTimeGetSeconds(CMSampleBufferGetPresentationTimeStamp(m.smp)) * 1000.0;
}
void * mov_frame(PNNMovie * m, int * w, int * h) {
  *w = CVPixelBufferGetBytesPerRow(m.img) / 4;
  *h = CVPixelBufferGetHeight(m.img);
  return CVPixelBufferGetBaseAddress(m.img);
}
void mov_end_frame(PNNMovie * m) {
  CVPixelBufferUnlockBaseAddress(m.img, kCVPixelBufferLock_ReadOnly);
  CFRelease(m.smp);
}
