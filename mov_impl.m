@import AVFoundation;

@interface PNNMovie : NSObject 
@property (nonatomic, strong) AVMovie * mov;
@property (nonatomic, strong) AVAssetImageGenerator * gen;
@property (nonatomic, strong) NSImage * img;
@property (nonatomic, strong) NSData * data;
@end

@implementation PNNMovie
@end

void * mov_alloc() {
  NSURL * url = [NSURL fileURLWithPath:@"out/IMG_2450.MOV"];
  AVMovie * mov = [AVMovie movieWithURL:url options:nil];
  AVAssetImageGenerator * gen = [AVAssetImageGenerator assetImageGeneratorWithAsset:mov];
  gen.maximumSize = NSMakeSize(1024.0, 1024.0);

  PNNMovie * res = [PNNMovie new];
  res.mov = mov;
  res.gen = gen;
  return (__bridge_retained void *)res;
}
void mov_dealloc(void * ptr) {
  id mov = (__bridge_transfer PNNMovie *)ptr;
  NSLog(@"Deallocating %@", mov);
}
void mov_frame(PNNMovie * m, double secs) {
  CMTime time = CMTimeMake(secs, 1);

  CGImageRef img = [m.gen copyCGImageAtTime:time actualTime:nil error:nil];
  m.img = (__bridge_transfer NSImage *)img;

  CGDataProviderRef data_prov = CGImageGetDataProvider(img);
  CFDataRef data = CGDataProviderCopyData(data_prov);
  m.data = (__bridge_transfer NSData *)data;
}

const void * mov_data(PNNMovie * m, int * w, int * h) {
  *w = CGImageGetBytesPerRow((__bridge struct CGImage *)m.img) / 4;
  *h = CGImageGetHeight((__bridge CGImageRef)m.img);
  return CFDataGetBytePtr((__bridge CFDataRef)m.data);
}
