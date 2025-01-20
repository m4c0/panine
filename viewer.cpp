#pragma leco app

import casein;
import jojo;
import jute;
import ovo;
import siaudio;
import silog;
import sitime;
import voo;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;
static constexpr const auto audio_rate = 48000; // TODO: read from file

static ovo::file audio = ovo::open_file("out/audio.ogg");
static double audio_time = ovo::time_total(audio, 0);
static int audio_bs {};

static void audio_filler(float * data, unsigned samples) {
  float ** pcm {};
  while (samples > 0) {
    auto r = ovo::read_float(audio, &pcm, samples, &audio_bs);
    if (r <= 0) {
      silog::log(silog::info, "Audio ended with code %ld", r);
      siaudio::rate(0);
      return;
    }
    for (int i = 0; i < r; i++) *data++ = pcm[0][i];
    samples -= r;
  }
}

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
    siaudio::filler(audio_filler);
  }

  void run() {
    sitime::stopwatch timer {};
    bool started {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      ots_loop(dq, sw, [&](auto cb) {
        if (!started) {
          siaudio::rate(audio_rate);
          timer = {};
          started = true;
        }
      });
    });
  }
} i;
