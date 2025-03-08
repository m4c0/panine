@import AVFoundation;

void read_audio_file(float * out, int count) {
  NSError * err;

  NSURL * url = [NSURL fileURLWithPath:@"out/assets/shrug.m4a"];
  AVAudioFile * file = [[AVAudioFile alloc] initForReading:url error:&err];
  if (err) NSLog(@"%@", err);

  AVAudioPCMBuffer * buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:file.processingFormat frameCapacity:file.length];
  [file readIntoBuffer:buffer error:&err];
  if (err) NSLog(@"%@", err);

  if (count > file.length) count = file.length;
  for (int i = 0; i < count; i++) {
    out[i] = buffer.floatChannelData[0][i];
  }
}

