export module tts;
import carnage;
import embrolho;
import jute;
import mtx;
import siaudio;
import silog;

export namespace tts {
  void word(jute::view w);
  bool playing();
}

module :private;

static const char * arpa2sampa(jute::view a) {
  if (a == "AA") return "Q   80 ";
  if (a == "AH") return "V   80 ";
  if (a == "DH") return "D   80 ";
  if (a == "EH") return "E   80 ";
  if (a == "IH") return "I   80 ";
  if (a == "IY") return "i:  80 ";
  if (a == "JH") return "dZ  80 ";
  if (a == "N")  return "n   80 ";
  if (a == "NG") return "N   80 ";
  if (a == "R")  return "r   80 ";
  if (a == "Z")  return "z   80 ";
  
  silog::log(silog::error, "unknown arpa phoneme: [%.*s]",
             static_cast<int>(a.size()), a.begin());
  throw 0;
}

// Magic rate from MBROLA
static constexpr const auto audio_rate = 16000;
static void audio_filler(float * data, unsigned samples);

class globals {
  carnage::map m_cmdict {};
  embrolho::t m_emb {};

  mtx::mutex m_mutex {};
  jute::view m_next {};
  bool m_playing {};

public:
  globals() {
    m_emb.write_pho("_ 80 ");
    siaudio::filler(audio_filler);
    siaudio::rate(audio_rate);
  }
  ~globals() {
    siaudio::rate(0);
  }

  void word(jute::view w) {
    mtx::lock m { &m_mutex };

    auto arpa = m_cmdict[w];
    while (arpa.size()) {
      auto [l, rest] = arpa.split(' ');
      arpa = rest;

      auto e = l.data()[l.size() - 1];
      if (e >= '0' && e <= '9') l = l.subview(l.size() - 1).before;

      m_emb.write_pho(arpa2sampa(l));
    }

    m_emb.write_pho("_ 80 ");
    m_emb.write_pho("#");
    m_playing = true;
  }

  unsigned read(short * data, unsigned n) {
    mtx::lock m { &m_mutex };

    if (!m_playing) return 0;
    auto res = m_emb.read(data, n);
    if (res == 0) m_playing = false;
    return res;
  }

  bool playing() { return m_playing; }
};
static auto & t() {
  static globals g {};
  return g;
}

static void audio_filler(float * data, unsigned samples) {
  for (auto i = 0; i < samples; i++) data[i] = 0;

  short buf[16000];
  auto i = t().read(buf, samples);
  for (auto x = 0; x < i; x++) {
    float s = buf[x];
    data[x] = s / ((2 << 16) - 1);
  }
}

bool tts::playing() { return t().playing(); }
void tts::word(jute::view w) { t().word(w); }

