#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
#pragma leco app

import audio;
import casein;
import dotz;
import sdf_texture;
import sitime;
import vapp;
import vee;
import voo;
import vtw;
import what_the_font;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

static constexpr const auto font_h = 128;
static constexpr const auto max_chars = 16;

static wtf::library g_library{};
static wtf::face g_face = g_library.new_face("out/font.ttf");

struct upc {
  float aspect;
};
struct chr {
  dotz::vec2 pos;
  dotz::vec2 size;
  dotz::vec4 uv;
};

struct init : public vapp {
  init() {
    casein::window_size = { window_width, window_height };
    g_face.set_char_size(font_h);
  }

  void run() {
    sitime::stopwatch timer {};
    bool started {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      auto dsl = vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() });
      auto dpool = vee::create_descriptor_pool(1, { vee::combined_image_sampler() });
      vtw::scriber scr { dq.physical_device(), vee::allocate_descriptor_set(*dpool, *dsl) };
      scr.bounds({ 1024, 1024 });

      sdf_texture sdf { dq, { 1024, 1024 } };

      auto chrs = voo::host_buffer { dq, sizeof(chr) * max_chars };
      {
        voo::mapmem mem { chrs.memory() };
        auto vs = static_cast<chr *>(*mem);

        auto s = g_face.shape_en("Engineering");
        scr.pen((-s.bounding_box() + 1024) / 2);
        scr.draw(s, [&](auto pen, const auto & glyph) {
          *vs++ = chr {
            .pos  = pen + glyph.d,
            .size = glyph.size,
            .uv   = glyph.uv
          };
        });
      }

      auto dsl_s = vee::create_descriptor_set_layout({ vee::dsl_fragment_storage() });
      auto dpool_s = vee::create_descriptor_pool(1, { vee::storage_buffer() });
      auto dset = vee::allocate_descriptor_set(*dpool_s, *dsl_s);
      vee::update_descriptor_set_with_storage(dset, 0, chrs.buffer());

      vee::pipeline_layout pl = vee::create_pipeline_layout(
          { *dsl, *dsl_s },
          { vee::vert_frag_push_constant_range<upc>() });

      voo::one_quad_render oqr { "main", &dq, *pl };
      bool copied = false;

      extent_loop(dq.queue(), sw, [&] {
        if (!started) {
          audio::start();
          timer = {};
          started = true;
        }

        upc pc {
          .aspect = sw.aspect(),
        };

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (!copied) {
            scr.setup_copy(*pcb);
            copied = true;
          }

          auto scb = sw.cmd_render_pass({
            .command_buffer = *pcb,
            .clear_colours { vee::clear_colour({}) },
          });
          oqr.run(*pcb, sw.extent(), [&] {
            vee::cmd_push_vert_frag_constants(*pcb, *pl, &pc);
            vee::cmd_bind_descriptor_set(*pcb, *pl, 0, scr.descriptor_set());
            vee::cmd_bind_descriptor_set(*pcb, *pl, 1, dset);
          });
        });
      });
    });
  }
} i;
