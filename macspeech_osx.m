@import AVFoundation;

@interface PNNSynth : AVSpeechSynthesizer<AVSpeechSynthesizerDelegate>
@property (nonatomic,strong) NSString * current;
@end
@implementation PNNSynth
- (void) speechSynthesizer:(AVSpeechSynthesizer *)synth willSpeakRangeOfSpeechString:(NSRange)range utterance:(AVSpeechUtterance *)utt {
  self.current = [utt.speechString substringWithRange:range];
}
@end

void * ms_init() {
  PNNSynth * s = [PNNSynth new];
  s.current = @"";
  s.delegate = s;
  return (__bridge_retained void *)s;
}
void ms_deinit(void * p) {
  id n = (__bridge_transfer PNNSynth *)p;
  NSLog(@"Deallocating %@", n);
}

const char * ms_current(void * p) {
  return [[(__bridge PNNSynth *)p current] UTF8String];
}

int ms_playing(void * p) {
  return [(__bridge PNNSynth *)p isSpeaking] ? 1 : 0;
}

static AVSpeechUtterance * create_utt(void * w, int n) {
  float max = AVSpeechUtteranceMaximumSpeechRate;
  float min = AVSpeechUtteranceMinimumSpeechRate;

  NSString * text = [[NSString alloc] initWithBytesNoCopy:w
                                                   length:n
                                                 encoding:NSASCIIStringEncoding
                                             freeWhenDone:false];
  AVSpeechUtterance * utt = [AVSpeechUtterance speechUtteranceWithString:text];
  utt.voice = [AVSpeechSynthesisVoice voiceWithIdentifier:@"com.apple.voice.compact.en-GB.Daniel"];
  utt.rate = (max - min) * 0.6 + min;
  return utt;
}

void ms_synth(void * p, void * w, int n) {
  [(__bridge PNNSynth *)p speakUtterance:create_utt(w, n)];
}

void ms_write(void * p, void * w, int n, void * a, void (*cb)(float)) {
  AVAssetWriterInput * ain = (__bridge AVAssetWriterInput *)a;
  AVSpeechUtterance * utt = create_utt(w, n);
  [(__bridge PNNSynth *)p writeUtterance:utt toBufferCallback:^(AVAudioBuffer * _Nonnull buffer) {
    AVAudioPCMBuffer * pcm = (AVAudioPCMBuffer *)buffer;

    CMBlockBufferRef blk;
    CMBlockBufferCreateWithMemoryBlock(
      kCFAllocatorDefault,
      pcm.floatChannelData[0],
      pcm.frameLength * sizeof(float),
      kCFAllocatorNull,
      NULL,
      0,
      pcm.frameLength * sizeof(float),
      kCMBlockBufferAssureMemoryNowFlag,
      &blk
    );

    AudioStreamBasicDescription asbd;
    asbd.mSampleRate = pcm.format.sampleRate;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mChannelsPerFrame = 1;
    asbd.mFramesPerPacket = 1;
    asbd.mBitsPerChannel = 32;
    asbd.mBytesPerFrame = 4;
    asbd.mBytesPerPacket = 4;
    asbd.mFormatFlags = pcm.format.streamDescription->mFormatFlags;

    CMFormatDescriptionRef fmt;
    CMAudioFormatDescriptionCreate(kCFAllocatorDefault, &asbd, 0, NULL, 0, NULL, NULL, &fmt);

    CMSampleBufferRef smp;
    CMSampleBufferCreate(kCFAllocatorDefault, blk, YES, NULL, NULL, fmt, pcm.frameLength, 0, NULL, 0, NULL, &smp);

    while (!ain.readyForMoreMediaData) {
      NSLog(@"audio not ready %@", ain);
      [NSThread sleepForTimeInterval:0.1];
    }
    if (![ain appendSampleBuffer:smp]) {
      NSLog(@"Error in audio"); // TODO: get error from AVAssetWriter
      return;
    }

    CFRelease(fmt);
    CFRelease(smp);
    CFRelease(blk);
    cb(pcm.frameLength / pcm.format.sampleRate);
  }];
}
