export module scriber;
import dotz;
import jute;
import vee;
import voo;
import vtw;
import what_the_font;

struct chr {
  dotz::vec2 pos;
  dotz::vec2 size;
  dotz::vec4 uv;
};

export class scriber {
  static constexpr const auto max_chars = 16;

  voo::host_buffer m_chrs;
  vee::descriptor_set_layout m_dsl;
  vee::descriptor_pool m_dpool;
  vee::descriptor_set m_dset;

  vtw::scriber m_scr;

  void setup_copy(vee::command_buffer cb) { m_scr.setup_copy(cb); }
  void shape(int scr_w, unsigned font_h, jute::view txt);

public:
  scriber(const voo::device_and_queue & dq, dotz::vec2 ext)
    : m_chrs { dq, sizeof(chr) * max_chars }
    , m_dsl { vee::create_descriptor_set_layout({ vee::dsl_fragment_storage() }) }
    , m_dpool { vee::create_descriptor_pool(1, { vee::storage_buffer() }) }
    , m_dset { vee::allocate_descriptor_set(*m_dpool, *m_dsl) }
    , m_scr { dq.physical_device() } {
    m_scr.bounds(ext);
    vee::update_descriptor_set_with_storage(m_dset, 0, m_chrs.buffer());
  }

  [[nodiscard]] auto atlas_dsl() const { return m_scr.descriptor_set_layout(); }
  [[nodiscard]] auto chars_dsl() const { return *m_dsl; }

  [[nodiscard]] auto atlas_dset() const { return m_scr.descriptor_set(); }
  [[nodiscard]] auto chars_dset() const { return m_dset; }

  void shape(vee::command_buffer cb, int scr_w, unsigned font_h, jute::view txt) {
    shape(scr_w, font_h, txt);
    setup_copy(cb);
  }
};

module : private;

void scriber::shape(int scr_w, unsigned font_h, jute::view txt) {
  static wtf::library g_library{};
  static wtf::face g_face = g_library.new_face("out/font.ttf");

  voo::mapmem mem { m_chrs.memory() };
  auto vs = static_cast<chr *>(*mem);

  g_face.set_char_size(font_h);
  auto s = g_face.shape_en(txt);
  m_scr.pen((-s.bounding_box() + scr_w) / 2);
  m_scr.draw(s, [&](auto pen, const auto & glyph) {
    *vs++ = chr {
      .pos  = pen + glyph.d,
      .size = glyph.size,
      .uv   = glyph.uv
    };
  });
}
