#pragma leco app
#pragma leco add_shader "waveform.vert"
#pragma leco add_shader "waveform.frag"

import casein;
import hai;
import ovo;
import siaudio;
import silog;
import vee;
import voo;
import vapp;

// TODO: draw "pointer" when playing

static constexpr const auto image_w = 2048;
static constexpr const auto sample_rate = 48000;

static constexpr const auto seconds = 1;
static constexpr const auto skip_len = 0.1f;
static constexpr const auto micro_skip_len = 0.0125f;

static float g_timestamp = 0.0f;

static const auto samples = [] {
  auto f = ovo::open_file("out/audio.ogg");

  hai::chain<float> res { sample_rate * 60 };
  while (true) {
    float ** pcm {};
    int i {};
    auto sz = ovo::read_float(f, &pcm, 1024, &i);
    if (sz <= 0) break;

    for (auto i = 0; i < sz; i++) res.push_back(pcm[0][i]);
  }
  silog::trace("size", res.size());
  return res;
}();
static float sample(unsigned i) {
  if (i > seconds * sample_rate) return 0;

  i += g_timestamp * sample_rate;
  if (i > samples.size()) return 0;

  return samples.seek(i);
}

static bool copied;
static void load(auto host_mem) {
  voo::mapmem mm { host_mem };
  auto ptr = static_cast<char *>(*mm);
  for (auto i = 0; i < image_w; i++) {
    auto s = seconds * i * sample_rate / image_w;
    *ptr++ = sample(s) * 128;
  }
  copied = false;
}
static void copy(auto cb, auto & img) {
  img.setup_copy(cb);
  copied = true;
}

static float g_skip = skip_len;
static unsigned g_audio_i {};
static void audio_filler(float * data, unsigned count) {
  for (auto i = 0; i < count; i++) {
    data[i] = sample(g_audio_i++);
  }
}
static void play() {
  g_audio_i = 0;
  siaudio::filler(audio_filler);
  siaudio::rate(sample_rate);
}
static void skip(int x, auto host_mem) {
  g_timestamp += x * g_skip;
  if (g_timestamp < 0) g_timestamp = 0;
  load(host_mem);
}
static void setup_keys(auto host_mem) {
  using namespace casein;
  handle(KEY_DOWN, K_LEFT,  [=] { skip(-1, host_mem); });
  handle(KEY_DOWN, K_RIGHT, [=] { skip(+1, host_mem); });
  handle(KEY_DOWN, K_SPACE, play);
  handle(KEY_DOWN, K_Q, [] { interrupt(IRQ_QUIT); });

  handle(KEY_DOWN, K_M, [] { g_skip = micro_skip_len; });
  handle(KEY_UP,   K_M, [] { g_skip = skip_len; });
}

static struct : vapp {
  struct upc {
    float probe;
  };

  void run() override {
    main_loop("waveform", [&](auto & dq, auto & sw) {
      auto smp = vee::create_sampler(vee::linear_sampler);
      voo::h2l_image img { dq.physical_device(), image_w, 1, VK_FORMAT_R8_UNORM };
      voo::single_dset dset {
        vee::dsl_fragment_sampler(),
        vee::combined_image_sampler(),
      };
      vee::update_descriptor_set(dset.descriptor_set(), 0, img.iv(), *smp);

      load(img.host_memory());
      setup_keys(img.host_memory());

      auto pl = vee::create_pipeline_layout(
          { dset.descriptor_set_layout() },
          { vee::fragment_push_constant_range<upc>() });
      voo::one_quad_render oqr { "waveform", &dq, *pl };
      ots_loop(dq, sw, [&](auto cb) {
        if (!copied) copy(cb, img);

        auto scb = sw.cmd_render_pass({ cb });
        oqr.run(cb, sw.extent(), [&] {
          float f = static_cast<float>(g_audio_i) / (seconds * sample_rate); 
          upc pc { f };
          vee::cmd_push_fragment_constants(cb, *pl, &pc);
          vee::cmd_bind_descriptor_set(cb, *pl, 0, dset.descriptor_set());
        });
      });
    });
  }
} i;
