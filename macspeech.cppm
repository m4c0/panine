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
  void ms_write(void * p, const char * w, int n, void * a, void (*)(float));
}

export class macspeech {
  hai::value<void *, ms_deinit> m_h;
  hai::cstr m_script = jojo::read_cstr("out/script.txt");

public:
  macspeech() : m_h { ms_init() } {}

  auto current() const { return jute::view::unsafe(ms_current(*m_h)); }
  bool playing() const { return ms_playing(*m_h); }
  void synth() const { ms_synth(*m_h, m_script.begin(), m_script.size()); }
  void write(void * ain, void (*cb)(float seconds)) const {
    ms_write(*m_h, m_script.begin(), m_script.size(), ain, cb);
  }
};
