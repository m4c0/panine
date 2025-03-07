#pragma leco app
#pragma leco add_shader "breakimg.frag"
#pragma leco add_shader "breakimg.vert"
export module breakimg;
import casein;
import dotz;
import vapp;
import vee;
import voo;

struct upc {
  dotz::vec2 scale;
};

export class breakimg {
  voo::h2l_image m_img;
  voo::single_cb m_cb;

  voo::single_dset m_dset;
  vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);

  vee::pipeline_layout m_pl;
  voo::one_quad_render m_oqr;

public:
  breakimg(voo::device_and_queue & dq)
    : m_img { voo::load_image_file("out/assets/smart.jpg", dq.physical_device()) }
    , m_cb { dq.queue_family() }
    , m_dset { 
      vee::dsl_fragment_sampler(),
      vee::combined_image_sampler(1),
    }
    , m_pl { vee::create_pipeline_layout(
      { m_dset.descriptor_set_layout() },
      { vee::vertex_push_constant_range<upc>() }
    ) }
    , m_oqr { "breakimg", &dq, *m_pl }
  {
    {
      voo::cmd_buf_one_time_submit pcb { m_cb.cb() };
      m_img.setup_copy(m_cb.cb());
    }
    dq.queue()->queue_submit({ .command_buffer = m_cb.cb() });
    vee::update_descriptor_set(m_dset.descriptor_set(), 0, m_img.iv(), *m_smp);
  }

  void run(vee::command_buffer cb, const voo::swapchain_and_stuff & sw) {
    auto scb = sw.cmd_render_pass({
      .command_buffer = cb,
      .clear_colours { vee::clear_colour({}) },
    });
    m_oqr.run(cb, sw.extent(), [&] {
      upc pc {};
      pc.scale = { sw.aspect(), 1.0 };
      pc.scale = pc.scale / 0.5;

      vee::cmd_push_vertex_constants(cb, *m_pl, &pc);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset.descriptor_set());
    });
  }
};

struct app : vapp {
  app() {
    casein::window_size = { 600, 900 };
  }

  void loop(voo::device_and_queue & dq, voo::swapchain_and_stuff & sw) {
    breakimg b { dq };
    ots_loop(dq, sw, [&](auto cb) {
      b.run(cb, sw);
    });
  }

  void run() override {
    main_loop("app", [&](auto & dq, auto & sw) { loop(dq, sw); });
  }
} t;
