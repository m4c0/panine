#pragma leco app

import pipeline;
import speak;
import silog;
import vee;
import vo;
import voo;

int main() {
  spk::run();
  silog::log(silog::info, "generated %d words and %d samples", spk::words.size(), spk::buffer.size());

  voo::device_and_queue dq { "panine-render" };
 
  constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;
  voo::offscreen::buffers fb { dq.physical_device(), vo::extent, format };
  pipeline ppl { &dq, fb.render_pass(), false };
 
  voo::single_cb cb { dq.queue_family() };
  vee::render_pass_begin rpb = fb.render_pass_begin({});
 
  vo v {};
  v.write_audio(spk::buffer.begin(), spk::buffer.size());
 
  int frame = 0;
  for (auto & w : spk::words) {
    silog::trace("generate", w.text);
    auto count = w.offset * 30 / 22050;
    for (; frame < count; frame++) {
      {
        voo::cmd_buf_one_time_submit ots { cb.cb() };
        ppl.run(cb.cb(), rpb, *w.text);
        fb.cmd_copy_to_host(cb.cb());
      }
      dq.queue()->queue_submit({
        .command_buffer = cb.cb(),
      });
      
      vee::device_wait_idle();

      {
        auto mm = fb.map_host();
        auto in = static_cast<vo::pix *>(*mm);
        auto out = v.lock();
        for (auto i = 0; i < vo::extent.width * vo::extent.height; i++) {
          *out = {{ in->p[3], in->p[0], in->p[1], in->p[2] }};
          out++;
          in++;
        }
        v.unlock(frame);
      }

      ppl.next_frame();
      vee::device_wait_idle();
    }
  }
  v.done();
  while (!v.wait());
}
