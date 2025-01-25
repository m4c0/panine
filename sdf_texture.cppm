export module sdf_texture;
#pragma leco add_shader "sdf_texture.comp"

import print;
import stubby;
import vee;
import voo;

class local_buffer {
  vee::buffer m_buf;
  vee::device_memory m_mem;

public:
  local_buffer() = default;
  local_buffer(vee::physical_device pd, unsigned sz) {
    m_buf = vee::create_transfer_buffer(sz);
    m_mem = vee::create_local_buffer_memory(pd, *m_buf);
    vee::bind_buffer_memory(*m_buf, *m_mem);
  }
  local_buffer(const voo::device_and_queue &dq, unsigned sz)
      : local_buffer{dq.physical_device(), sz} {}

  [[nodiscard]] auto buffer() const { return *m_buf; }
  [[nodiscard]] auto memory() const { return *m_mem; }
};

static unsigned buf_sz(vee::extent sz) {
  return sz.width * sz.height * sizeof(float);
}

export class sdf_texture {
  vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
    vee::dsl_compute_storage(),
    vee::dsl_compute_storage(),
  });
  vee::pipeline_layout m_pl = vee::create_pipeline_layout({ *m_dsl });
  
  vee::c_pipeline m_ppl = vee::create_compute_pipeline(
      *m_pl,
      *vee::create_shader_module_from_resource("sdf_texture.comp.spv"),
      "main");

  vee::descriptor_pool m_dpool = vee::create_descriptor_pool(2, { vee::storage_buffer(4) });
  vee::descriptor_set m_ds0 = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
  vee::descriptor_set m_ds1 = vee::allocate_descriptor_set(*m_dpool, *m_dsl);

  vee::fence m_f = vee::create_fence_reset();

  voo::host_buffer m_host;
  local_buffer m_buf0;
  local_buffer m_buf1;

  vee::command_pool m_cpool;
  vee::command_buffer m_cb;

  vee::extent m_ext;

public:
  sdf_texture(const voo::device_and_queue & dq, vee::extent sz)
    : m_host { dq, buf_sz(sz) }
    , m_buf0 { dq, buf_sz(sz) }
    , m_buf1 { dq, buf_sz(sz) }
    , m_cpool { vee::create_command_pool(dq.queue_family()) }
    , m_cb { vee::allocate_primary_command_buffer(*m_cpool) }
    , m_ext { sz } {
    vee::update_descriptor_set_with_storage(m_ds0, 0, m_buf0.buffer());
    vee::update_descriptor_set_with_storage(m_ds0, 1, m_buf1.buffer());

    vee::update_descriptor_set_with_storage(m_ds1, 0, m_buf1.buffer());
    vee::update_descriptor_set_with_storage(m_ds1, 1, m_buf0.buffer());
  }

  auto map_input() { return voo::mapmem { m_host.memory() }; }
};

module :private;

/*
void sdf_texture::xxx() try {
  bool out_to_1 = true;
  {
    voo::cmd_buf_one_time_submit::build(cb, [&](auto & cb) {
      for (auto i = 0; i < 16; i++) {
        out_to_1 = !out_to_1;

        auto ds = out_to_1 ? ds_b : ds_f;
        auto b = out_to_1 ? b1.buffer() : b2.buffer();

        vee::cmd_bind_c_pipeline(*cb, *p);
        vee::cmd_bind_c_descriptor_set(*cb, *pl, 0, ds);
        vee::cmd_dispatch(*cb, img.width, img.height, 1);
        vee::cmd_pipeline_barrier(*cb, b, vee::from_compute_to_compute);
      }
    });
  }
  dq.queue()->queue_submit({
    .fence = *f,
    .command_buffer = cb
  });
  vee::device_wait_idle();

  {
    auto pix = reinterpret_cast<stbi::pixel *>(*img.data);

    voo::mapmem mm { out_to_1 ? b1.memory() : b2.memory() };
    auto p = static_cast<float *>(*mm);
    for (auto i = 0; i < map_sz; i++) {
      unsigned char cc = p[i] >= 16 ? 255 : (16 * p[i]);
      pix[i] = { cc, cc, cc, 255 };
    }

    stbi::write_rgba_unsafe("out/image.png", img.width, img.height, pix);
  }
} catch (...) {
  return 1;
}
*/
