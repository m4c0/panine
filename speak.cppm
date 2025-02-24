#pragma leco add_impl avspeech
export module speak;
import hai;
import jojo;
import jute;

extern "C" void speak(const char * txt, unsigned n);

export namespace spk {
  struct word {
    jute::heap text;
    unsigned offset;
  };
  
  hai::chain<float> buffer { 4 * 1024 * 1024 };
  hai::chain<word> words { 16 * 1024 };

  void run() {
    auto script = jojo::read_cstr("out/script.txt");
    speak(script.begin(), script.size());
  }
}

extern "C" void speak_callback(float * data, unsigned samples) {
  for (auto i = 0; i < samples; i++) spk::buffer.push_back(data[i]);
}
extern "C" void speak_marker_callback(const char * str, unsigned offset) {
  if (str == nullptr) str = "";
  offset /= 4; // 4 bytes per sample
  offset = offset * 1000 / 44100; // sample to ms
  spk::words.push_back(spk::word {
    .text { jute::view::unsafe(str) },
    .offset = offset,
  });
}

