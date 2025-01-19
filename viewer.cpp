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
      .fmap(yoyo::slurp)
      .take([](auto msg) {
          silog::die("failed to open audio: %s", msg.cstr().begin());
      });
static ovo::file audio_file = ovo::open_callbacks(audio);
static int audio_bs {};

static void audio_filler(float * data, unsigned samples) {
  float ** pcm {};
  auto r = ovo::read_float(audio_file, &pcm, samples, &audio_bs);
  if (r <= 0) {
    silog::log(silog::info, "Audio ended with code %ld", r);
    siaudio::rate(0);
    return;
  }
  for (auto i = 0; i < r; i++) {
    data[i] = pcm[0][i];
  }
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
