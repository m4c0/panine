export module chars;
import dotz;
import vee;
import voo;

struct chr {
  dotz::vec2 pos;
  dotz::vec2 size;
  dotz::vec4 uv;
};

class chr_pump {
  voo::mapmem m_mm;
  chr * m_vs;

public:
  explicit chr_pump(vee::device_memory::type m)
    : m_mm { m }
    , m_vs { static_cast<chr *>(*m_mm) } {}

  void pump(chr c) { *m_vs++ = c; }
};

export class chars {
  static constexpr const auto max_chars = 16;

  voo::host_buffer m_chrs;
  vee::descriptor_set_layout m_dsl;
  vee::descriptor_pool m_dpool;
  vee::descriptor_set m_dset;

public:
  explicit chars(const voo::device_and_queue & dq)
    : m_chrs { dq, sizeof(chr) * max_chars }
    , m_dsl { vee::create_descriptor_set_layout({ vee::dsl_fragment_storage() }) }
    , m_dpool { vee::create_descriptor_pool(1, { vee::storage_buffer() }) }
    , m_dset { vee::allocate_descriptor_set(*m_dpool, *m_dsl) } {
    vee::update_descriptor_set_with_storage(m_dset, 0, m_chrs.buffer());
  }
    
  [[nodiscard]] auto dsl() const { return *m_dsl; }
  [[nodiscard]] auto dset() const { return m_dset; }

  [[nodiscard]] auto pump() { return chr_pump { m_chrs.memory() }; };
};
