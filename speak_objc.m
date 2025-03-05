#pragma GCC diagnostic ignored "-Wunguarded-availability-new"
@import AVFoundation;

void speak_callback(float *, unsigned);
void speak_marker_callback(const char * str, unsigned offset);

@interface PNNSpeechDelegate : NSObject<AVSpeechSynthesizerDelegate>
@end
@implementation PNNSpeechDelegate
- (void) speechSynthesizer:(AVSpeechSynthesizer *)synth willSpeakMarker:(AVSpeechSynthesisMarker *)marker utterance:(AVSpeechUtterance *)utt {
  NSString * buf = [utt.speechString substringWithRange:marker.textRange];
  speak_marker_callback(buf.UTF8String, marker.byteSampleOffset);
}
@end

static AVSpeechUtterance * create_utt(void * txt, int n) {
  float max = AVSpeechUtteranceMaximumSpeechRate;
  float min = AVSpeechUtteranceMinimumSpeechRate;

  NSString * text = [[NSString alloc] initWithBytesNoCopy:txt
                                                   length:n
                                                 encoding:NSASCIIStringEncoding
                                             freeWhenDone:false];
  AVSpeechUtterance * utt = [AVSpeechUtterance speechUtteranceWithString:text];
  utt.voice = [AVSpeechSynthesisVoice voiceWithIdentifier:@"com.apple.voice.compact.en-GB.Daniel"];
  utt.rate = (max - min) * 0.6 + min;
  return utt;
}
void speak(void * txt, unsigned n) {
  PNNSpeechDelegate * del = [PNNSpeechDelegate new];
  AVSpeechSynthesizer * sp = [AVSpeechSynthesizer new];
  sp.delegate = del;
  [sp writeUtterance:create_utt(txt, n) toBufferCallback:^(AVAudioBuffer * buf) {
    AVAudioPCMBuffer * pcm = (AVAudioPCMBuffer *)buf;
    speak_callback(pcm.floatChannelData[0], pcm.frameLength);
  }];
  while (sp.speaking) {
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
  }
}

