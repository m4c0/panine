export module tts;
import carnage;
import embrolho;
import jute;
import silog;

static const char * arpa2sampa(jute::view a) {
  if (a == "AA") return "Q   80 ";
  if (a == "AH") return "V   80 ";
  if (a == "EH") return "E   80 ";
  if (a == "IH") return "I   80 ";
  if (a == "JH") return "dZ  80 ";
  if (a == "N")  return "n   80 ";
  if (a == "NG") return "N   80 ";
  if (a == "R")  return "r   80 ";
  
  silog::log(silog::error, "unknown arpa phoneme: [%.*s]",
             static_cast<int>(a.size()), a.begin());
  throw 0;
}

export class tts {
  carnage::map m_cmdict {};
  embrolho::t m_emb {};

public:
  tts() { m_emb.write_pho("_ 80 "); }

  void word(jute::view w) {
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
  }
};
