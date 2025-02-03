#pragma leco app
#pragma leco add_shader "waveform.vert"
#pragma leco add_shader "waveform.frag"

import casein;
import vee;
import voo;
import vapp;

static struct : vapp {
  void run() override {
    main_loop("waveform", [&](auto & dq, auto & sw) {
      auto smp = vee::create_sampler(vee::linear_sampler);
      voo::h2l_image img { dq.physical_device(), 1024, 1, vee::image_format_r8 };
      voo::single_dset dset {
        vee::dsl_fragment_sampler(),
        vee::combined_image_sampler(),
      };
      vee::update_descriptor_set(dset.descriptor_set(), 0, img.iv(), *smp);

      auto pl = vee::create_pipeline_layout({ dset.descriptor_set_layout() });
      voo::one_quad_render oqr { "waveform", &dq, *pl };
      ots_loop(dq, sw, [&](auto cb) {
        oqr.run(cb, sw.extent(), [&] {
          vee::cmd_bind_descriptor_set(cb, *pl, 0, dset.descriptor_set());
        });
      });
    });
  }
} i;
