@import AVFoundation;

void * ms_init() {
  return (__bridge_retained void *)[[AVSpeechSynthesizer alloc] init];
}
void ms_deinit(void * p) {
  id n = (__bridge_transfer AVSpeechSynthesizer *)p;
  NSLog(@"Deallocating %@", n);
}

int ms_playing(void * p) {
  return [(__bridge AVSpeechSynthesizer *)p isSpeaking] ? 1 : 0;
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

  [(__bridge AVSpeechSynthesizer *)p speakUtterance:utt];
}
