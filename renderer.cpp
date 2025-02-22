#pragma leco app
#pragma leco add_impl video_out

import scriber;
import macspeech;
import pipeline;
import vee;
import voo;

static constexpr const vee::extent extent { 1080, 1920 };
static constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;

extern "C" {
  void * vo_new(int w, int h);
  void vo_delete(void *);
}

int main() {
  auto vo = vo_new(extent.width, extent.height);

  voo::device_and_queue dq { "panine-render" };

  macspeech ms {};

  voo::offscreen::buffers fb { dq.physical_device(), extent, format };
  pipeline ppl { &dq, fb.render_pass() };

  voo::single_cb cb { dq.queue_family() };
  vee::render_pass_begin rpb = fb.render_pass_begin({});

  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
    ppl.run(cb.cb(), rpb, "hello");
    fb.cmd_copy_to_host(cb.cb());
  }
  dq.queue()->queue_submit({
    .command_buffer = cb.cb(),
  });
  
  vee::device_wait_idle();

  vo_delete(vo);
}
