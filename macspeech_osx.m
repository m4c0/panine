@import AVFoundation;

@interface PNNSynth : AVSpeechSynthesizer<AVSpeechSynthesizerDelegate>
@end
@implementation PNNSynth
- (void) speechSynthesizer:(AVSpeechSynthesizer *)synth willSpeakRangeOfSpeechString:(NSRange)range utterance:(AVSpeechUtterance *)utt {
  NSString * buf = [utt.speechString substringWithRange:range];
  NSLog(@"Will speak [%@]", buf);
}
@end

void * ms_init() {
  PNNSynth * s = [PNNSynth new];
  s.delegate = s;
  return (__bridge_retained void *)s;
}
void ms_deinit(void * p) {
  id n = (__bridge_transfer PNNSynth *)p;
  NSLog(@"Deallocating %@", n);
}

int ms_playing(void * p) {
  return [(__bridge PNNSynth *)p isSpeaking] ? 1 : 0;
}

void ms_synth(void * p, void * w, int n) {
  float max = AVSpeechUtteranceMaximumSpeechRate;
  float min = AVSpeechUtteranceMinimumSpeechRate;

  NSString * text = [[NSString alloc] initWithBytesNoCopy:w
                                                   length:n
                                                 encoding:NSASCIIStringEncoding
                                             freeWhenDone:false];
  AVSpeechUtterance * utt = [AVSpeechUtterance speechUtteranceWithString:text];
  utt.voice = [AVSpeechSynthesisVoice voiceWithIdentifier:@"com.apple.voice.compact.en-GB.Daniel"];
  utt.rate = (max - min) * 0.7 + min;

  [(__bridge PNNSynth *)p speakUtterance:utt];
}
