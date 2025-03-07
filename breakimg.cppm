#pragma leco app
#pragma leco add_shader "breakimg.frag"
#pragma leco add_shader "breakimg.vert"
export module breakimg;
import casein;
import vapp;
import vee;
import voo;

export class breakimg {
public:
};

struct app : vapp {
  app() {
    casein::window_size = { 600, 900 };
  }

  void loop(voo::device_and_queue & dq, voo::swapchain_and_stuff & sw) {
    voo::h2l_image m_img = voo::load_image_file("out/assets/smart.jpg", dq.physical_device());
    voo::single_cb m_cb { dq.queue_family() };
    {
      voo::cmd_buf_one_time_submit pcb { m_cb.cb() };
      m_img.setup_copy(m_cb.cb());
    }
    dq.queue()->queue_submit({ .command_buffer = m_cb.cb() });

    voo::single_dset m_dset { 
      vee::dsl_fragment_sampler(),
      vee::combined_image_sampler(1),
    };
    vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);
    vee::update_descriptor_set(m_dset.descriptor_set(), 0, m_img.iv(), *m_smp);

    vee::pipeline_layout m_pl = vee::create_pipeline_layout({ m_dset.descriptor_set_layout() });
    voo::one_quad_render m_oqr { "breakimg", &dq, *m_pl };

    render_loop(dq, sw, [&](auto cb) {
      m_oqr.run(cb, sw.extent(), [&] {
        vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset.descriptor_set());
      });
    });
  }

  void run() override {
    main_loop("app", [&](auto & dq, auto & sw) { loop(dq, sw); });
  }
} t;
