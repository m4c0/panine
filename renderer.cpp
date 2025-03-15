#pragma leco app

import cmd;
import dotz;
import hai;
import jojo;
import jute;
import ots;
import pprent;
import rng;
import speak;
import silog;
import tts;
import voo;

static ots ots {};

static constexpr const auto audio_rate = 22050; // defined by Apple's TTS

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

static void run_speech(jute::view bg, jute::view script) {
  tts ppl { bg, ots.dq(), ots.render_pass() };
 
  auto spk = spk::run(script);
  ots.write_audio(spk.buffer.begin(), spk.buffer.size());
 
  int frame = 0;
  for (auto & w : spk.words) {
    silog::trace("generate", w.text);
    auto count = w.offset * 30 / audio_rate;
    for (; frame < count; frame++) {
      ots([&](auto cb) {
        ppl.run(cb, ots.render_pass_begin(), *w.text);
      });
      
      ppl.next_frame();
      vee::device_wait_idle();
    }
  }
}
void run_speech(jute::view l) { run_speech(*random_movie(), l); }

static constexpr int atoi(jute::view v) {
  int res = 0;
  for (auto c : v) {
    if (c < '0' || c > '9') silog::die("invalid integer");
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
    } else if (c == '.') silog::die("invalid float");
    if (c < '0' || c > '9') silog::die("invalid float");

    res = res * 10 + (c - '0');
    
    if (decimals >= 0) decimals++;
  }
  for (auto i = 0; i < decimals; i++) res /= 10;
  return res;
}

static void run_script(jute::view v);

static void run_cmd_load(jute::view arg) { run_script(arg); }
static void run_cmd_rate(jute::view arg) { spk::set_rate(atof(arg)); }
static void run_cmd_voice(jute::view arg) { spk::set_voice(arg.cstr().begin()); }

static void run_command(jute::view v) {
  auto [cmd, arg] = v.split(' ');
       if (cmd == "rate")  run_cmd_rate(arg);
  else if (cmd == "voice") run_cmd_voice(arg); 
  else if (cmd == "load")  run_cmd_load(arg);
  else silog::die("invalid command");
}

static void run_script(jute::view name) {
  static hai::varray<jute::heap> stack { 1024 };
  if (stack.size() > 0) {
    auto n = stack[stack.size() - 1];
    auto [path, fn] = (*n).rsplit('/');
    stack.push_back(jute::heap { path } + "/" + name);
  } else {
    stack.push_back(name);
  }

  auto script = jojo::read_cstr(*stack[stack.size() - 1]);
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
      cmd::image(ots, img, atoi(vol), atoi(skip));
    }
    rest = r;
  }

  stack.pop_back();
}

int main() {
  rng::seed();

  run_script("out/script.txt");
}
