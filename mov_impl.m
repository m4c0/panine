@import AVFoundation;

void x(void (* cb)(const void *, int, int)) {
  NSURL * url = [NSURL fileURLWithPath:@"out/IMG_2450.MOV"];
  AVMovie * mov = [AVMovie movieWithURL:url options:nil];
  AVAssetImageGenerator * gen = [AVAssetImageGenerator assetImageGeneratorWithAsset:mov];
  gen.maximumSize = NSMakeSize(1024.0, 1024.0);

  CMTime time = CMTimeMake(1.0, 1);
  CGImageRef img = [gen copyCGImageAtTime:time actualTime:nil error:nil];
  CGDataProviderRef data_prov = CGImageGetDataProvider(img);
  CFDataRef data = CGDataProviderCopyData(data_prov);

  int w = CGImageGetBytesPerRow(img) / 4;
  int h = CGImageGetHeight(img);
  const UInt8 * ptr = CFDataGetBytePtr(data);
  cb(ptr, w, h);

  CFRelease(data);
  CFRelease(data_prov);
}

