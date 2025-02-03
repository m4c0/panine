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
      auto pl = vee::create_pipeline_layout();
      voo::one_quad_render oqr { "waveform", &dq, *pl };
      ots_loop(dq, sw, [&](auto cb) {
        oqr.run(cb, sw.extent());
      });
    });
  }
} i;
