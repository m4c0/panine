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

  void run() override {
    main_loop("app", [&](auto & dq, auto & sw) {
      auto pl = vee::create_pipeline_layout();
      voo::one_quad_render oqr { "breakimg", &dq, *pl };
      render_loop(dq, sw, [&](auto cb) {
        oqr.run(cb, sw.extent());
      });
    });
  }
} t;
