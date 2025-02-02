export module chars;
import dotz;
import vee;
import voo;

struct chr {
  dotz::vec2 pos;
  dotz::vec2 size;
  dotz::vec4 uv;
};

static constexpr const auto max_chars = 16;

class chr_pump {
  voo::mapmem m_mm;
  chr * m_vs;

public:
  explicit chr_pump(vee::device_memory::type m)
    : m_mm { m }
    , m_vs { static_cast<chr *>(*m_mm) } {
    for (auto i = 0; i < max_chars; i++) m_vs[i] = {};
  }

  void pump(chr c) { *m_vs++ = c; }
};

export class chars {
  voo::single_dset m_dset { vee::dsl_fragment_storage(), vee::storage_buffer() };
  voo::host_buffer m_chrs;

public:
  explicit chars(const voo::device_and_queue & dq)
    : m_chrs { dq.physical_device(), vee::create_buffer(sizeof(chr) * max_chars, vee::buffer_usage::storage_buffer) } {
    vee::update_descriptor_set(m_dset.descriptor_set(), 0, m_chrs.buffer());
  }
    
  [[nodiscard]] auto dsl() const { return m_dset.descriptor_set_layout(); }
  [[nodiscard]] auto dset() const { return m_dset.descriptor_set(); }

  [[nodiscard]] auto pump() { return chr_pump { m_chrs.memory() }; };
};
