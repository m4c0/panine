#pragma leco app
#pragma leco add_shader "waveform.vert"
#pragma leco add_shader "waveform.frag"

import casein;
import vee;
import voo;
import vapp;

static bool copied;
static void load(auto host_mem) {
  voo::mapmem mm { host_mem };
  auto ptr = static_cast<float *>(*mm);
  for (auto i = 0; i < 1024; i++) {
  }
  copied = false;
}
static void copy(auto cb, auto & img) {
  img.setup_copy(cb);
  copied = true;
}

static struct : vapp {
  void run() override {
    main_loop("waveform", [&](auto & dq, auto & sw) {
      auto smp = vee::create_sampler(vee::linear_sampler);
      voo::h2l_image img { dq.physical_device(), 1024, 1, VK_FORMAT_R8_UNORM };
      voo::single_dset dset {
        vee::dsl_fragment_sampler(),
        vee::combined_image_sampler(),
      };
      vee::update_descriptor_set(dset.descriptor_set(), 0, img.iv(), *smp);

      load(img.host_memory());

      auto pl = vee::create_pipeline_layout({ dset.descriptor_set_layout() });
      voo::one_quad_render oqr { "waveform", &dq, *pl };
      ots_loop(dq, sw, [&](auto cb) {
        if (!copied) copy(cb, img);

        auto scb = sw.cmd_render_pass({ cb });
        oqr.run(cb, sw.extent(), [&] {
          vee::cmd_bind_descriptor_set(cb, *pl, 0, dset.descriptor_set());
        });
      });
    });
  }
} i;
