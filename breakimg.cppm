#pragma leco add_shader "breakimg.frag"
#pragma leco add_shader "breakimg.vert"
export module breakimg;
import dotz;
import vee;
import voo;

export class breakimg {
  voo::h2l_image m_img;
  voo::single_cb m_cb;

  voo::single_dset m_dset;
  vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);

  vee::pipeline_layout m_pl;
  voo::one_quad_render m_oqr;

public:
  struct upc {
    dotz::vec2 scale;
  };

  explicit breakimg(const char * file, voo::device_and_queue & dq, vee::render_pass::type rp)
    : m_img { voo::load_image_file(file, dq.physical_device()) }
    , m_cb { dq.queue_family() }
    , m_dset { 
      vee::dsl_fragment_sampler(),
      vee::combined_image_sampler(1),
    }
    , m_pl { vee::create_pipeline_layout(
      { m_dset.descriptor_set_layout() },
      { vee::vertex_push_constant_range<upc>() }
    ) }
    , m_oqr { "breakimg", dq.physical_device(), rp, *m_pl }
  {
    {
      voo::cmd_buf_one_time_submit pcb { m_cb.cb() };
      m_img.setup_copy(m_cb.cb());
    }
    dq.queue()->queue_submit({ .command_buffer = m_cb.cb() });
    vee::update_descriptor_set(m_dset.descriptor_set(), 0, m_img.iv(), *m_smp);
  }

  void run(vee::command_buffer cb, vee::render_pass_begin rpb, upc pc) {
    rpb.command_buffer = cb;
    rpb.clear_colours = { vee::clear_colour({}) };
    voo::cmd_render_pass rp { rpb };

    m_oqr.run(cb, rpb.extent, [&] {
      pc.scale.x *= static_cast<float>(rpb.extent.height) / rpb.extent.width;
      vee::cmd_push_vertex_constants(cb, *m_pl, &pc);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset.descriptor_set());
    });
  }
};
