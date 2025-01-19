#pragma leco app

import casein;
import voo;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
  }

  void run() {
    main_loop("panine", [&](auto & dq, auto & sw) {
      ots_loop(dq, sw, [&](auto cb) {
      });
    });
  }
} i;
