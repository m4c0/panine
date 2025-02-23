#pragma leco add_impl "macspeech_osx"
export module macspeech;
import hai;
import jojo;
import jute;

extern "C" {
  void * ms_init();
  void ms_deinit(void *);

  const char * ms_current(void *);
  bool ms_playing(void *);
  void ms_synth(void *, const char * c, int n);
}

struct deleter {
  void operator()(void * p) { ms_deinit(p); }
};
export class macspeech {
  hai::value_holder<void *, deleter> m_h;
  hai::cstr m_script = jojo::read_cstr("out/script.txt");

public:
  macspeech() : m_h { ms_init() } {}

  auto current() const { return jute::view::unsafe(ms_current(*m_h)); }
  bool playing() const { return ms_playing(*m_h); }
  void synth(jute::view word) const { ms_synth(*m_h, word.begin(), word.size()); }
};
