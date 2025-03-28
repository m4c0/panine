#pragma leco tool

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

static ots ots { false };

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

static void run_command(jute::view v) {
  auto [cmd, arg] = v.split(' ');
       if (cmd == "rate")    spk::set_rate(atof(arg));
  else if (cmd == "voice")   spk::set_voice(arg.cstr().begin()); 
  else if (cmd == "load")    run_script(arg);
  else if (cmd == "image")   cmd::image(arg);
  else if (cmd == "zoomout") cmd::zoom_out(ots, arg);
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

  jojo::on_error([name](auto, auto err) {
    silog::die("Error reading [%.*s]: %.*s",
        static_cast<unsigned>(name.size()), name.begin(),
        static_cast<unsigned>(err.size()), err.begin());
  });

  auto script = jojo::read_cstr(*stack[stack.size() - 1]);
  silog::log(silog::info, "parsing [%.*s] with %d bytes",
        static_cast<unsigned>(name.size()), name.begin(),
        script.size());

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
      cmd::zoom_out(ots, l);
    }
    rest = r;
  }

  stack.pop_back();
}

constexpr const char * shift(int & c, char ** & v) { return (c <= 0) ? nullptr : (--c, *v++); }

int main(int argc, char ** argv) {
  rng::seed();

  if (argc == 1) {
    run_script("out/script.txt");
    return 0;
  }

  shift(argc, argv);
  while (auto c = shift(argc, argv)) {
    run_script(jute::view::unsafe(c));
  }
  return 0;
}
