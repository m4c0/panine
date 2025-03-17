#pragma leco add_impl "renderer_objc"

module;
extern "C" void read_audio_file(const char * fn, float * out, int count);

module cmd;
import breakimg;
import dotz;

void cmd::zoom_out(ots & ots, jute::view line) {
  auto [file, r0] = line.subview(1).after.split(',');
  auto [vol, sk] = r0.split(',');
  auto volume = atof(vol);
  auto skip = atoi(sk);

  auto img = ("out/assets/" + file + ".jpg").cstr();
  breakimg b { img.begin(), *ots.dq(), ots.render_pass() };
  for (int frame = 0; frame < 30; frame++) {
    float t = static_cast<float>(frame) / 30.0f;
    ots([&](auto cb) {
      breakimg::upc pc {};
      pc.scale = dotz::mix(dotz::vec2 { 1 }, dotz::vec2 { 0 }, t);
      b.run(cb, ots.render_pass_begin(), pc);
    });
  }

  auto m4a = ("out/assets/" + file + ".m4a").cstr();
  float audio[audio_rate * 5] {};
  if (volume > 0) {
    read_audio_file(m4a.begin(), audio, audio_rate * 5);
    for (auto & f : audio) f *= volume;
  }
  ots.write_audio(audio + skip, audio_rate);
}
