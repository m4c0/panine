#pragma leco app
#pragma leco add_impl "renderer_objc"

import breakimg;
import dotz;
import hai;
import jojo;
import jute;
import pprent;
import rng;
import speak;
import silog;
import tts;
import vee;
import vo;
import voo;

static voo::device_and_queue dq { "panine-render" };
static vo v {};
static int vframes {};

static constexpr const auto audio_rate = 22050; // defined by Apple's TTS
static constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;
static voo::offscreen::buffers fb { dq.physical_device(), vo::extent, format };
static voo::single_cb cb { dq.queue_family() };
static vee::render_pass_begin rpb = fb.render_pass_begin({});

static auto random_movie() {
  hai::chain<jute::heap> files { 1024 };
  for (auto d : pprent::list("out/assets")) {
    auto dv = jute::view::unsafe(d);
    if (!dv.ends_with(".mov")) continue;

    auto dh = jute::heap { "out/assets" } + "/" + dv;
    files.push_back(dh);
  }
  return files.seek(rng::rand(files.size()));
}

static void ots(auto fn) {
  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
    fn();
    fb.cmd_copy_to_host(cb.cb());
  }
  dq.queue()->queue_submit({
    .command_buffer = cb.cb(),
  });
  vee::device_wait_idle();

  auto mm = fb.map_host();
  auto in = static_cast<vo::pix *>(*mm);
  auto out = v.lock();
  for (auto i = 0; i < vo::extent.width * vo::extent.height; i++) {
    *out = {{ in->p[3], in->p[0], in->p[1], in->p[2] }};
    out++;
    in++;
  }
  v.unlock(vframes++);
}

static void run_speech(jute::view bg, jute::view script) {
  tts ppl { bg, &dq, fb.render_pass() };
 
  auto spk = spk::run(script);
  v.write_audio(spk.buffer.begin(), spk.buffer.size());
 
  int frame = 0;
  for (auto & w : spk.words) {
    silog::trace("generate", w.text);
    auto count = w.offset * 30 / audio_rate;
    for (; frame < count; frame++) {
      ots([&] {
        ppl.run(cb.cb(), rpb, *w.text);
      });
      
      ppl.next_frame();
      vee::device_wait_idle();
    }
  }
}
void run_speech(jute::view l) { run_speech(*random_movie(), l); }

extern "C" void read_audio_file(const char * fn, float * out, int count);
static void show_image(jute::view file, float volume, unsigned skip) {
  auto img = ("out/assets/" + file + ".jpg").cstr();
  breakimg b { img.begin(), dq, fb.render_pass() };
  for (int frame = 0; frame < 30; frame++) {
    float t = static_cast<float>(frame) / 30.0f;
    ots([&] {
      breakimg::upc pc {};
      pc.scale = dotz::mix(dotz::vec2 { 1 }, dotz::vec2 { 0 }, t);
      b.run(cb.cb(), rpb, pc);
    });
  }

  auto m4a = ("out/assets/" + file + ".m4a").cstr();
  float audio[audio_rate * 5] {};
  if (volume > 0) {
    read_audio_file(m4a.begin(), audio, audio_rate * 5);
    for (auto & f : audio) f *= volume;
  }
  v.write_audio(audio + skip, audio_rate);
}

static constexpr int atoi(jute::view v) {
  int res = 0;
  for (auto c : v) {
    if (c < '0' || c > '9') throw 0;
    res = res * 10 + (c - '0');
  }
  return res;
}
static constexpr float atof(jute::view v) {
  float res = 0;
  int decimals = -1;
  for (auto c : v) {
    if (c == '.' && decimals == -1) {
      decimals = 0;
      continue;
    } else if (c == '.') throw 0;
    if (c < '0' || c > '9') throw 0;

    res = res * 10 + (c - '0');
    
    if (decimals >= 0) decimals++;
  }
  for (auto i = 0; i < decimals; i++) res /= 10;
  return res;
}

static void run_cmd_rate(jute::view arg) { spk::set_rate(atof(arg)); }
static void run_cmd_voice(jute::view arg) { spk::set_voice(arg.cstr().begin()); }

static void run_command(jute::view v) {
  auto [cmd, arg] = v.split(' ');
       if (cmd == "rate")  run_cmd_rate(arg);
  else if (cmd == "voice") run_cmd_voice(arg); 
}

void run_script() {
  auto script = jojo::read_cstr("out/script.txt");
  jute::view rest { script };
  while (rest.size()) {
    auto [l, r] = rest.split('\n'); 
    if (l.size() == 0) { // Empty line, ignore
    } else if (l[0] == '#') { // Comment, ignore
    } else if (l[0] == ':') {
      run_command(l.subview(1).after);
    } else if (l[0] != '+') {
      run_speech(*random_movie(), l);
    } else {
      auto [img, r0] = l.subview(1).after.split(',');
      auto [vol, skip] = r0.split(',');
      show_image(img, atoi(vol), atoi(skip));
    }
    rest = r;
  }
}

int main() {
  rng::seed();

  run_script();

  float time = vframes / 30.0;
  silog::log(silog::info, "Total frames in output: %d (%3.2fs)", vframes, time);

  v.done();
  while (!v.wait());
}
