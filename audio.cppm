export module audio;
import ovo;
import siaudio;
import silog;

namespace audio {
  export void start();
}

module :private;

static constexpr const auto audio_rate = 48000; // TODO: read from file

static ovo::file audio_file = ovo::open_file("out/audio.ogg");
static double audio_time = ovo::time_total(audio_file, 0);
static int audio_bs {};

static void audio_filler(float * data, unsigned samples) {
  float ** pcm {};
  while (samples > 0) {
    auto r = ovo::read_float(audio_file, &pcm, samples, &audio_bs);
    if (r <= 0) {
      silog::log(silog::info, "Audio ended with code %ld", r);
      siaudio::rate(0);
      return;
    }
    for (int i = 0; i < r; i++) *data++ = pcm[0][i];
    samples -= r;
  }
}

void audio::start() {
  siaudio::filler(audio_filler);
  siaudio::rate(audio_rate);
}
