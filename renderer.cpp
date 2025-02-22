#pragma leco app
#pragma leco add_impl video_out

import scriber;
import macspeech;
import pipeline;
import vee;
import voo;

static constexpr const vee::extent extent { 1080, 1920 };
static constexpr const auto format = VK_FORMAT_A8B8G8R8_SRGB_PACK32;

extern "C" {
  void * vo_new(int w, int h);
  void vo_delete(void *);
  void vo_done(void *);
  unsigned * vo_lock(void * p);
  void vo_unlock(void * p, unsigned frame);
  void vo_wait();
}

int main() {
  auto vo = vo_new(extent.width, extent.height);

  voo::device_and_queue dq { "panine-render" };

  macspeech ms {};

  voo::offscreen::buffers fb { dq.physical_device(), extent, format };
  pipeline ppl { &dq, fb.render_pass(), false };

  voo::single_cb cb { dq.queue_family() };
  vee::render_pass_begin rpb = fb.render_pass_begin({});

  for (auto i = 0; i < 1; i++) {
    {
      voo::cmd_buf_one_time_submit ots { cb.cb() };
      ppl.run(cb.cb(), rpb, "hello");
      fb.cmd_copy_to_host(cb.cb());
    }
    dq.queue()->queue_submit({
      .command_buffer = cb.cb(),
    });
    
    vee::device_wait_idle();

    {
      auto mm = fb.map_host();
      auto * in = static_cast<unsigned *>(*mm);
      auto * out = vo_lock(vo);
      for (auto i = 0; i < extent.width * extent.height; i++) {
        *out++ = *in++;
      }
      vo_unlock(vo, 0);
    }

    ppl.next_frame();
    vee::device_wait_idle();
  }

  vo_done(vo);
  vo_wait();

  vo_delete(vo);
}
