#pragma leco app

import casein;
import jojo;
import jute;
import macspeech;
import pipeline;
import vapp;
import vee;
import voo;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

static auto script = jojo::read_cstr("out/script.txt");

struct init : public vapp {
  init() {
    casein::window_size = { window_width, window_height };
  }

  void run() {
    main_loop("panine", [&](auto & dq, auto & sw) {
      macspeech ms {};

      pipeline ppl { &dq, dq.render_pass() };

      auto movie_run_guard = ppl.play_movie();

      ms.synth(script);

      extent_loop(dq.queue(), sw, [&] {
        if (!ms.playing()) throw 0;

        auto text = jute::view::unsafe(ms.current());
        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          auto rpb = sw.render_pass_begin();
          ppl.run(*pcb, rpb, text);
        });
      });
    });
  }
} i;
