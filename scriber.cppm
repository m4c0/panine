export module scriber;
import chars;
import dotz;
import jute;
import vee;
import voo;
import vtw;
import what_the_font;

export class scriber {
  chars m_chr;
  vtw::scriber m_scr;

  void setup_copy(vee::command_buffer cb) { m_scr.setup_copy(cb); }
  void shape(int scr_w, unsigned font_h, jute::view txt);

public:
  scriber(const voo::device_and_queue & dq, dotz::vec2 ext)
    : m_chr { dq }
    , m_scr { dq.physical_device() } {
    m_scr.bounds(ext);
  }

  [[nodiscard]] auto atlas_dsl() const { return m_scr.descriptor_set_layout(); }
  [[nodiscard]] auto chars_dsl() const { return m_chr.dsl(); }

  [[nodiscard]] auto atlas_dset() const { return m_scr.descriptor_set(); }
  [[nodiscard]] auto chars_dset() const { return m_chr.dset(); }

  void shape(vee::command_buffer cb, int scr_w, unsigned font_h, jute::view txt) {
    shape(scr_w, font_h, txt);
    setup_copy(cb);
  }
};

module : private;

void scriber::shape(int scr_w, unsigned font_h, jute::view txt) {
  static wtf::library g_library{};
  static wtf::face g_face = g_library.new_face("out/font.ttf");

  auto chrs = m_chr.pump();

  g_face.set_char_size(font_h);
  auto s = g_face.shape_en(txt);
  m_scr.pen((-s.bounding_box() + scr_w) / 2);
  m_scr.draw(s, [&](auto pen, const auto & glyph) {
    chrs.pump({
      .pos  = pen + glyph.d,
      .size = glyph.size,
      .uv   = glyph.uv
    });
  });
}
