#pragma GCC diagnostic ignored "-Wunguarded-availability-new"
@import AVFoundation;

void speak_callback(void *, float *, unsigned);
void speak_marker_callback(void *, const char * str, unsigned offset);

@interface PNNSpeechDelegate : NSObject<AVSpeechSynthesizerDelegate>
@property (nonatomic) void * user;
@end
@implementation PNNSpeechDelegate
- (void) speechSynthesizer:(AVSpeechSynthesizer *)synth willSpeakMarker:(AVSpeechSynthesisMarker *)marker utterance:(AVSpeechUtterance *)utt {
  NSString * buf = [utt.speechString substringWithRange:marker.textRange];
  speak_marker_callback(self.user, buf.UTF8String, marker.byteSampleOffset);
}
@end

static NSString * voiceId = @"com.apple.voice.compact.en-GB.Daniel";
static float uttRate = 0.6;

static AVSpeechUtterance * create_utt(void * txt, int n) {
  float max = AVSpeechUtteranceMaximumSpeechRate;
  float min = AVSpeechUtteranceMinimumSpeechRate;

  NSString * text = [[NSString alloc] initWithBytesNoCopy:txt
                                                   length:n
                                                 encoding:NSUTF8StringEncoding
                                             freeWhenDone:false];
  AVSpeechUtterance * utt = [AVSpeechUtterance speechUtteranceWithString:text];
  utt.voice = [AVSpeechSynthesisVoice voiceWithIdentifier:voiceId];
  utt.rate = (max - min) * uttRate + min;
  return utt;
}
void speak_rate(float r) { uttRate = r; }
void speak_voice(const char * n) {
  voiceId = [NSString stringWithUTF8String:n];

  NSArray * voices = [AVSpeechSynthesisVoice speechVoices];
  for (AVSpeechSynthesisVoice * v in voices) {
    if ([v.identifier isEqualToString:voiceId]) return;
  }

  for (AVSpeechSynthesisVoice * v in voices) {
    NSLog(@"%@", v.identifier);
  }
}
void speak(void * user, void * txt, unsigned n) {
  PNNSpeechDelegate * del = [PNNSpeechDelegate new];
  del.user = user;
  AVSpeechSynthesizer * sp = [AVSpeechSynthesizer new];
  sp.delegate = del;
  [sp writeUtterance:create_utt(txt, n) toBufferCallback:^(AVAudioBuffer * buf) {
    AVAudioPCMBuffer * pcm = (AVAudioPCMBuffer *)buf;
    speak_callback(user, pcm.floatChannelData[0], pcm.frameLength);
  }];
  while (sp.speaking) {
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
  }
}

