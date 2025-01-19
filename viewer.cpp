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

static int count_chars(jute::view txt) {
  int count {};
  for (auto c : txt) {
    c |= 0x20;
    if (c >= 'a' && c <= 'z') count++;
  }
  return count;
}

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
    siaudio::filler(audio_filler);
    siaudio::rate(audio_rate);
  }

  void run() {
    auto txt = jojo::read_cstr("out/script.txt");
    auto chars = count_chars(txt);
    auto tpc = 1000.0 * audio_time / chars;
    silog::log(silog::info, "Number of chars: %d, millis per char: %f", chars, tpc);

    sitime::stopwatch timer {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      ots_loop(dq, sw, [&](auto cb) {
      });
    });
  }
} i;
