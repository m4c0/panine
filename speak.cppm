#pragma leco add_impl speak_objc
export module speak;
import hai;
import jojo;
import jute;

extern "C" void speak(void * self, const char * txt, unsigned n);

export namespace spk {
  struct word {
    jute::heap text;
    unsigned offset;
  };
  
  hai::varray<float> buffer { 4 * 1024 * 1024 };
  hai::varray<word> words { 16 * 1024 };

  void run() {
    auto script = jojo::read_cstr("out/script.txt");
    words.push_back(word { jute::view { "" }, 0 });
    speak(nullptr, script.begin(), script.size());
    (words.end() - 1)->offset = buffer.size();
  }
}

extern "C" void speak_callback(void * self, float * data, unsigned samples) {
  for (auto i = 0; i < samples; i++) spk::buffer.push_back(data[i]);
}
extern "C" void speak_marker_callback(void * self, const char * str, unsigned offset) {
  if (str == nullptr) str = "";
  offset /= 4; // 4 bytes per sample
  (spk::words.end() - 1)->offset = offset;
  spk::words.push_back(spk::word {
    .text { jute::view::unsafe(str) },
    .offset = offset,
  });
}

