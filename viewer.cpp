#pragma leco app

import casein;
import ovo;
import siaudio;
import silog;
import voo;
import yoyo;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;
static constexpr const auto audio_rate = 48000; // TODO: read from file

static auto audio = yoyo::file_reader::open("out/audio.ogg")
      .take([](auto msg) {
          silog::die("failed to open audio: %s", msg.cstr().begin());
      });
static ovo::decoder audio_dec { &audio };

static void audio_filler(float * data, unsigned samples) {
  while (audio_dec.preload()) {}
  audio_dec.fill(data, samples);
  for (auto i = 0; i < samples; i++)
  silog::trace("here", data[i]);
}

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
    siaudio::filler(audio_filler);
    siaudio::rate(audio_rate);
  }

  void run() {
    main_loop("panine", [&](auto & dq, auto & sw) {
      ots_loop(dq, sw, [&](auto cb) {
      });
    });
  }
} i;
