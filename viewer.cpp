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

static auto tokenise(jute::view & rest) {
  int c;
  for (c = 0; c < rest.size(); c++) {
    if (rest[c] == ' ' || rest[c] == '\n') break;
  }

  auto [l, r] = rest.subview(c);
  while (r.size()) {
    if (r[0] != ' ' && r[0] != '\n') break;
    r = r.subview(1).after;
  }

  if (r.size()) silog::log(silog::info, "Taking word: [%s]", l.cstr().begin());
  rest = r;
  return l;
}

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
    siaudio::filler(audio_filler);
  }

  void run() {
    auto txt = jojo::read_cstr("out/script.txt");
    auto chars = count_chars(txt);
    auto tpc = 1000.0 * audio_time / chars;
    silog::log(silog::info, "Number of chars: %d, millis per char: %f", chars, tpc);

    sitime::stopwatch timer {};

    jute::view rest { txt };
    auto word = tokenise(rest);
    bool started {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      ots_loop(dq, sw, [&](auto cb) {
        if (!started) {
          siaudio::rate(audio_rate);
          timer = {};
          started = true;
        }
        auto time = timer.millis() / tpc;
        if (time > word.size()) {
          word = tokenise(rest);
          timer = {};
        }
      });
    });
  }
} i;
